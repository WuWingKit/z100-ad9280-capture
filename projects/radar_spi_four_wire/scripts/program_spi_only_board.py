"""Download current external-SPI bit/ELF; only sends read-only UART s command."""
from pathlib import Path
import sys,json,time,subprocess,os,datetime,hashlib
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R.parent/'radar_spi_debug_20261112/.tool_deps'))
import serial
out=R/'evidence'/datetime.datetime.now().strftime('jtag_spi_only_%Y%m%d_%H%M%S');out.mkdir()
j={'port':'COM7','flash_written':False,'acquisition_requested':False,'programmed':False}
j['sha256']={n:hashlib.sha256((R/'hardware'/n).read_bytes()).hexdigest() for n in ['spi_only_master.bit','spi_only_ps.elf','radar_sync.hdf','ps7_init.tcl']}
p=serial.Serial();p.port='COM7';p.baudrate=115200;p.timeout=.1;p.dtr=False;p.rts=False
try:
 try:p.open()
 except Exception as ex:j['uart_open_error']=str(ex)
 env=os.environ.copy();env['PROCESSOR_ARCHITECTURE']='AMD64'
 with (out/'xsct_download.txt').open('w',encoding='utf-8') as log:
  proc=subprocess.run(['E:/Xilinx/SDK/2018.3/bin/xsct.bat','scripts/debug_spi_only_jtag.tcl'],cwd=R,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
 logtext=(out/'xsct_download.txt').read_text(encoding='utf-8',errors='replace')
 j['programmed']=proc.returncode==0 and 'SPI_ONLY_MASTER_RUNNING_NO_AUTO_TRANSFER' in logtext
 assert j['programmed'],'JTAG download failed; inspect XSCT log'
 if p.is_open:
  data=b'';deadline=time.monotonic()+5
  while time.monotonic()<deadline:
   data+=p.read(max(1,p.in_waiting))
   if b'NO AUTO TRANSFER' in data and data.endswith(b'\n'):break
  j['banner_verified']=b'Z100 SPI-ONLY MASTER' in data
  if j['banner_verified']:
   p.write(b's\r');p.flush();deadline=time.monotonic()+3
   while time.monotonic()<deadline:
    data+=p.read(max(1,p.in_waiting))
    at=data.rfind(b'SPI STATUS')
    if at>=0 and b'\n' in data[at:]:break
   j['status']=data[data.rfind(b'SPI STATUS'):].decode('ascii','replace').strip() if b'SPI STATUS' in data else None
  (out/'uart_startup.txt').write_bytes(data)
 print(json.dumps(j,ensure_ascii=True));print(str(out))
finally:
 if p.is_open:p.close()
 (out/'download_result.json').write_text(json.dumps(j,indent=2),encoding='utf-8')
