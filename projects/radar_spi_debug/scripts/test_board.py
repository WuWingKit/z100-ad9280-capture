"""Board-local positive/fault/recovery tests, retaining raw UART and binary data.

Requires pyserial==3.5. Run only after hardware timing/DRC and build checks pass.
The --program option loads the matching bit/ELF through JTAG, never Flash.
"""
import argparse,datetime,hashlib,json,os,re,struct,subprocess,sys,time,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'.tool_deps'))
import serial

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--port',required=True)
    ap.add_argument('--program',action='store_true');args=ap.parse_args()
    stamp=datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    out=ROOT/'evidence'/('board_'+stamp);out.mkdir()
    result={'started':datetime.datetime.now().isoformat(),'port':args.port,
            'baud':115200,'synthetic_data_only':True,'flash_written':False,'tests':[]}
    for name in ('radar_debug.bit','radar_debug_uart.elf','radar_debug.ltx'):
        result[name+'_sha256']=hashlib.sha256((ROOT/'hardware'/name).read_bytes()).hexdigest()
    port=serial.Serial();port.port=args.port;port.baudrate=115200
    port.timeout=.1;port.write_timeout=2;port.dtr=False;port.rts=False
    port.open()
    raw=open(out/'uart_rx.bin','wb');log=open(out/'uart_log.txt','w',encoding='utf-8')
    def note(s):
        log.write('\n['+datetime.datetime.now().isoformat()+'] '+s+'\n');log.flush()
    def receive(marker,timeout=5):
        data=b'';deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            chunk=port.read(max(1,port.in_waiting))
            if chunk:
                data+=chunk;raw.write(chunk);raw.flush();log.write(chunk.decode('ascii','replace'));log.flush()
                at=data.find(marker.encode())
                if at>=0 and b'\n' in data[at+len(marker):]:return data.decode('ascii','replace')
        raise AssertionError('Timeout waiting for '+marker+'; last bytes: '+repr(data[-400:]))
    def send(command,marker,timeout=5):
        note('TX ASCII '+command+' / HEX '+(command+'\r').encode().hex(' '))
        port.write((command+'\r').encode());port.flush();return receive(marker,timeout)
    def parse_frame(text,seq):
        m=re.search(r'FRAME version=(\d+) seq=(\d+) beam_index=(\d+) angle_cdeg=(-?\d+) bytes=(\d+) header_xor=([0-9a-f]+) check_errors=(\d+) local_payload_crc=([0-9a-f]+)',text,re.I)
        assert m,'Missing complete FRAME metadata'
        v,s,b,a,n,x,e,crc=m.groups();beam=(9+seq)%19
        assert (int(v),int(s),int(b),int(a),int(n),int(x,16),int(e))==(3,seq,beam,(beam-9)*500,47136,0,0),m.group(0)
        return {'seq':seq,'beam':beam,'bytes':int(n),'payload_crc':crc}
    try:
        if args.program:
            env=os.environ.copy();env['PROCESSOR_ARCHITECTURE']='AMD64'
            with open(out/'jtag_program.txt','w',encoding='utf-8') as fp:
                # SDK 2018.3's batch launcher mishandles a quoted absolute
                # script path containing spaces; use a relative script + cwd.
                proc=subprocess.run(['E:/Xilinx/SDK/2018.3/bin/xsct.bat','scripts/debug_jtag.tcl'],
                    cwd=ROOT,env=env,stdout=fp,stderr=subprocess.STDOUT,timeout=90,
                    creationflags=subprocess.CREATE_NO_WINDOW)
            assert proc.returncode==0,'JTAG process failed'
            assert 'DEBUG_PROGRAM_RUNNING_NO_FLASH' in (out/'jtag_program.txt').read_text(encoding='utf-8'),'JTAG did not finish'
            receive('Enter required.',10)
        send('s','STATUS')
        text=send('t','FPGA LOOP SELFTEST PASS',15)
        for seq in range(3):
            row=next(m for m in text.splitlines() if m.startswith('FRAME ') and 'seq='+str(seq)+' ' in m)
            result['tests'].append({'name':'positive',**parse_frame(row,seq),'pass':True})
            assert re.search(r'FPGA PATTERN seq='+str(seq)+r' complex_samples=11776 errors=0 PASS',text)
        # Continue across a complete beam cycle, without resetting or re-priming.
        for seq in range(3,22):
            text=send('c','SIMULATED FRAME STORED')
            row=parse_frame(text,seq)
            verify=send('v',' PASS')
            assert 'errors=0 PASS' in verify and 'seq='+str(seq)+' ' in verify
            result['tests'].append({'name':'positive',**row,'pass':True})
        text=send('d','END RADAR_RAW',40)
        payload=text.split('BEGIN RADAR_RAW bytes=47136',1)[1].split('END RADAR_RAW',1)[0]
        values=bytes(int(s,16) for s in re.findall(r'\b[0-9a-fA-F]{2}\b',payload))
        assert len(values)==47136,len(values)
        header=struct.unpack('<16H',values[:32]);x=0
        for w in header:x^=w
        assert x==0 and header[14]==21 and header[3]==11
        errors=0
        for i in range(11776):
            actual=struct.unpack_from('<I',values,32+i*4)[0]
            errors+=actual!=(((0x8000|21)<<16)|(512+i))
        assert errors==0,errors
        crc=f'{zlib.crc32(values[32:]):08x}'
        assert crc==row['payload_crc'].lower(),(crc,row)
        (out/'frame_seq0021.bin').write_bytes(values)
        result['tests'].append({'name':'independent_PC_full_frame','pass':True,'bytes':len(values),'errors':errors,'payload_crc':crc})
        for command,code in [('b',2),('l',16),('n',64)]:
            send('r','INTERNAL SOURCE RESET');send('p','PRIME ACK')
            send(command,'CAPTURE FAILED',5);time.sleep(.08)
            state=send('s','STATUS')
            # capture() also prints a status line. Its UART tail can precede
            # the explicit `s` reply; evaluate the LAST complete status record.
            statuses=re.findall(r'^STATUS raw=[^\r\n]+',state,re.M)
            assert statuses,state
            latest=statuses[-1];m=re.search(r'fault_code=([0-9a-f]+)',latest,re.I)
            assert m and int(m.group(1),16)==code and 'state=7 ' in latest,state
            result['tests'].append({'name':'fault_'+command,'pass':True,'fault_code':code})
            send('r','INTERNAL SOURCE RESET');send('p','PRIME ACK')
            text=send('c','SIMULATED FRAME STORED');parse_frame(text,0)
            assert 'errors=0 PASS' in send('v',' PASS')
            result['tests'].append({'name':'recovery_'+command,'pass':True})
        send('s','STATUS');result['pass']=True
        print('BOARD_TEST_PASS '+str(out),flush=True)
    except Exception as ex:
        result['pass']=False;result['failure']=repr(ex)
        print('BOARD_TEST_FAIL '+repr(ex)+' '+str(out),flush=True)
        raise
    finally:
        result['finished']=datetime.datetime.now().isoformat()
        (out/'results.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
        port.close();raw.close();log.close()

if __name__=='__main__':main()
