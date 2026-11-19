"""Exercise the physical MCU slave and verify its exact selftest IQ pattern."""
from pathlib import Path
import sys,time,json,re,struct,hashlib,datetime
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R.parent/'radar_spi_debug_20261112/.tool_deps'))
import serial
out=R/'evidence'/datetime.datetime.now().strftime('spi_only_selftest_%Y%m%d_%H%M%S');out.mkdir()
j={'flash_written':False,'status_queries':[],'frames':[],'passed':False,'source':'MCU selftest; not radar measurement'}
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
  for i in range(5):
   s=command('s',b'SPI STATUS',3);j['status_queries'].append(s)
   assert 'flags=0000' in s and 'fault=0' in s,s
  print('STATUS_QUERIES_PASS count=5',flush=True)
  j['prime']=command('p',b'SPI PRIME COMPLETE',8);print(j['prime'].strip(),flush=True)
  assert 'SPI PRIME COMPLETE' in j['prime'] and 'FAILED' not in j['prime']
  for n in range(3):
   text=command('c',b'END RADAR_RAW',90);(out/f'capture_{n}.txt').write_text(text)
   print(text[:200],flush=True)
   m=re.search(r'BEGIN RADAR_RAW bytes=(\d+)\s+(.*?)END RADAR_RAW',text,re.S)
   assert m and 'CRC16_PER_CHUNK=PASS' in text,'SPI read failed or incomplete dump'
   raw=bytes.fromhex(m[2]);assert len(raw)==47136 and int(m[1])==len(raw)
   (out/f'frame_{n}.bin').write_bytes(raw)
   h=struct.unpack_from('<16H',raw);xor=0
   for v in h:xor^=v
   assert h[0]==0xa610 and h[1]==3 and h[2]==16 and h[3]==9 and h[4]==0 and h[12]==47136 and h[13]==32 and h[14]==n and xor==0,'Header mismatch'
   errors=[]
   for k,(iv,qv) in enumerate(struct.iter_unpack('<HH',raw[32:])):
    if iv!=k+512 or qv!=(0x8000|n):
     if len(errors)<8:errors.append({'sample':k,'i':iv,'q':qv})
   f={'sequence':h[14],'bytes':len(raw),'complex_samples':11776,'header_xor':xor,'pattern_errors_first8':errors,'sha256':hashlib.sha256(raw).hexdigest(),'chunk_crc':'PASS'}
   j['frames'].append(f);print(json.dumps(f),flush=True);assert not errors,'Selftest data differs from MCU source pattern'
   s=command('s',b'SPI STATUS',3);assert 'flags=0000' in s,'ACK did not release frame'
  j['passed']=True
 except Exception as e:
  j['error']=str(e);print('STOP:',e,flush=True)
 finally:
  if p.is_open:p.close()
  (out/'result.json').write_text(json.dumps(j,indent=2));print('EVIDENCE:',out,flush=True);print('PASS:',j['passed'],flush=True)
