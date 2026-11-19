"""Record raw external SPI bytes without rejecting radar headers or lengths."""
from pathlib import Path
import sys,time,json,re,struct,zlib,datetime
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R.parent/'radar_spi_debug_20261112/.tool_deps'))
import serial
out=R/'evidence'/datetime.datetime.now().strftime('raw_board_%Y%m%d_%H%M%S');out.mkdir()
result={'flash_written':False,'frames':[]}
p=serial.Serial(port=None,baudrate=115200,timeout=.1);p.port='COM7';p.dtr=False;p.rts=False
with (out/'uart_rx.txt').open('wb') as log:
 def run(c,end,seconds=4):
  with (out/'commands.txt').open('a') as tx:tx.write(c+' '+(c+'\r').encode().hex()+'\n')
  p.write((c+'\r').encode());p.flush();data=b'';deadline=time.monotonic()+seconds
  while time.monotonic()<deadline:
   b=p.read(max(1,p.in_waiting));data+=b;log.write(b);log.flush()
   if end in data and data.endswith(b'\n'):break
   if (b'FAILED' in data or b'ERROR ' in data) and data.endswith(b'\n'):break
  return data.decode('ascii','replace')
 try:
  p.open();result['reset']=run('r',b'MCU separately before p')
  result['status']=run('s',b'STATUS raw=');print(result['status'],flush=True)
  assert 'ready=1' in result['status'] and 'mcu_ready=1' in result['status']
  result['prime']=run('p',b'PRIME ACK:');print(result['prime'],flush=True)
  assert 'PRIME ACK:' in result['prime']
  for n in range(3):
   text=run('c',b'END RADAR_RAW',40);(out/f'capture_{n}.txt').write_text(text)
   print('\n'.join(t for t in text.splitlines() if not re.fullmatch(r'[0-9a-fA-F ]+',t))[:1800],flush=True)
   m=re.search(r'BEGIN RADAR_RAW bytes=(\d+)\s+(.*?)END RADAR_RAW',text,re.S)
   assert m,'No complete UART raw dump'
   raw=bytes.fromhex(m[2]);assert len(raw)==int(m[1])
   (out/f'raw_{n}.bin').write_bytes(raw)
   pages={int(k):int(v,16) for k,v in re.findall(r'RAW_DIAG page=(\d+) value=([0-9a-fA-F]+)',text)}
   report={'stored_bytes':len(raw),'first_64_bytes':raw[:64].hex(' '),'pages':pages,'local_crc32':f'{zlib.crc32(raw)&0xffffffff:08X}',
    'A610_wire_msb_offsets':[m.start() for m in re.finditer(b'\xa6\x10',raw)],
    'A610_little_endian_offsets':[m.start() for m in re.finditer(b'\x10\xa6',raw)],
    'cs_count':pages.get(1),'partial_cs_count':pages.get(2),'timeout':bool(pages.get(3,0)&(1<<17)),
    'truncated':bool(pages.get(3,0)&(1<<16))}
   if len(raw)>=32:
    report['first_16_words_little_endian']=list(struct.unpack_from('<16H',raw))
    report['first_16_words_wire_msb']=list(struct.unpack_from('>16H',raw))
   result['frames'].append(report);print(json.dumps(report),flush=True)
 except Exception as e:result['error']=str(e);print('STOP',e,flush=True)
 finally:
  if p.is_open:p.close()
  (out/'result.json').write_text(json.dumps(result,indent=2));print('EVIDENCE',out,flush=True)
