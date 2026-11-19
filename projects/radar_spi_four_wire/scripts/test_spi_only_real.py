"""Capture ten physical real-acquisition frames; inspect transport and raw IQ statistics."""
from pathlib import Path
import sys,time,json,re,struct,hashlib,datetime,statistics,csv
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R.parent/'radar_spi_debug_20261112/.tool_deps'))
import serial
out=R/'evidence'/datetime.datetime.now().strftime('spi_only_real_%Y%m%d_%H%M%S');out.mkdir()
j={'flash_written':False,'status_queries':[],'frames':[],'passed':False,'source':'User reports MCU real-acquisition firmware; no firmware readback','real_signal_quality_assessed':False}
p=serial.Serial(port=None,baudrate=115200,timeout=.1);p.port='COM7';p.dtr=False;p.rts=False
with (out/'uart_rx.txt').open('wb') as rx,(out/'commands.jsonl').open('w') as tx:
 def command(c,end,seconds):
  tx.write(json.dumps({'command':c,'hex':(c+'\r').encode().hex(),'time':time.time()})+'\n');tx.flush()
  p.write((c+'\r').encode());p.flush();data=b'';deadline=time.monotonic()+seconds
  while time.monotonic()<deadline:
   b=p.read(max(1,p.in_waiting));data+=b;rx.write(b);rx.flush()
   if end in data and data.endswith(b'\n'):break
   if (b'FAILED' in data or b'ERROR ' in data) and data.endswith(b'\n'):break
  return data.decode('ascii','replace')
 try:
  p.open()
  for i in range(10):
   s=command('s',b'SPI STATUS',3);j['status_queries'].append(s)
   assert 'flags=0000' in s and 'fault=0' in s,s
  print('STATUS_QUERIES_PASS count=10',flush=True)
  j['prime']=command('p',b'SPI PRIME COMPLETE',8);print(j['prime'].strip(),flush=True)
  assert 'SPI PRIME COMPLETE' in j['prime'] and 'FAILED' not in j['prime']
  for n in range(10):
   text=command('c',b'END RADAR_RAW',90);(out/f'capture_{n}.txt').write_text(text)
   print(text[:200],flush=True)
   m=re.search(r'BEGIN RADAR_RAW bytes=(\d+)\s+(.*?)END RADAR_RAW',text,re.S)
   assert m and 'CRC16_PER_CHUNK=PASS' in text,'SPI read failed or incomplete dump'
   raw=bytes.fromhex(m[2]);assert len(raw)==47136 and int(m[1])==len(raw)
   (out/f'frame_{n}.bin').write_bytes(raw)
   h=struct.unpack_from('<16H',raw);xor=0
   for v in h:xor^=v
   assert h[0]==0xa610 and h[1]==3 and h[2]==16 and h[3]==9 and h[4]==0 and h[12]==47136 and h[13]==32 and h[14]==n and xor==0,'Header mismatch'
   iq=list(struct.iter_unpack('<HH',raw[32:]));si=[v if v<32768 else v-65536 for v,q in iq];sq=[q if q<32768 else q-65536 for v,q in iq]
   pattern_match=all(iv==k+512 and qv==(0x8000|n) for k,(iv,qv) in enumerate(iq))
   def stats(v):return {'min':min(v),'max':max(v),'mean':statistics.mean(v),'stddev':statistics.pstdev(v),'unique_codes':len(set(v))}
   f={'sequence':h[14],'beam_index':h[3],'angle_cdeg':struct.unpack_from('<h',raw,8)[0],'bytes':len(raw),'complex_samples':len(iq),'header_xor':xor,'sha256':hashlib.sha256(raw).hexdigest(),'payload_sha256':hashlib.sha256(raw[32:]).hexdigest(),'chunk_crc':'PASS','exact_selftest_pattern':pattern_match,'all_payload_zero':not any(raw[32:]),'i_signed16_interpretation':stats(si),'q_signed16_interpretation':stats(sq),'i_unsigned16':stats([i for i,q in iq]),'q_unsigned16':stats([q for i,q in iq])}
   with (out/f'frame_{n}_iq.csv').open('w',newline='') as st:
    writer=csv.writer(st);writer.writerow(['payload_chirp','capture_chirp','sample','i_u16','q_u16','i_s16_interpretation','q_s16_interpretation'])
    for k,((iv,qv),iv_s,qv_s) in enumerate(zip(iq,si,sq)):writer.writerow([k//512,k//512+1,k%512,iv,qv,iv_s,qv_s])
   j['frames'].append(f);print(json.dumps(f),flush=True)
   s=command('s',b'SPI STATUS',3);assert 'flags=0000' in s,'ACK did not release frame'
  j['passed']=len(j['frames'])==10
  j['distinct_payloads']=len(set(f['payload_sha256'] for f in j['frames']))
  j['selftest_pattern_detected']=any(f['exact_selftest_pattern'] for f in j['frames'])
 except Exception as e:
  j['error']=str(e);print('STOP:',e,flush=True)
 finally:
  if p.is_open:p.close()
  (out/'result.json').write_text(json.dumps(j,indent=2));print('EVIDENCE:',out,flush=True);print('PASS:',j['passed'],flush=True)
