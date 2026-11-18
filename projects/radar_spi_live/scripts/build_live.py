"""Build PS ELF using the copied SDK 2018.3 BSP; no board is accessed."""
from pathlib import Path
import subprocess,os
ROOT=Path(__file__).resolve().parents[1]
GCC=Path('E:/Xilinx/SDK/2018.3/gnu/aarch32/nt/gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe')
env=os.environ.copy();env['PROCESSOR_ARCHITECTURE']='AMD64'
common=['-mcpu=cortex-a9','-mfpu=vfpv3','-mfloat-abi=hard']
commands=[
 [str(GCC),*common,'-Wall','-Wextra','-O0','-g3','-c','-I'+str(ROOT/'bsp/include'),str(ROOT/'software/main.c'),'-o',str(ROOT/'hardware/main.o')],
 [str(GCC),*common,'-Wl,-build-id=none','-specs=Xilinx.spec','-Wl,-T','-Wl,'+str(ROOT/'software/lscript.ld'),'-L'+str(ROOT/'bsp/lib'),'-o',str(ROOT/'hardware/radar_live_uart.elf'),str(ROOT/'hardware/main.o'),'-Wl,--start-group','-lxil','-lgcc','-lc','-Wl,--end-group']]
with (ROOT/'evidence/software_build.txt').open('w',encoding='utf-8') as log:
 for command in commands:
  proc=subprocess.run(command,cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
  log.write(proc.stdout.decode('utf-8',errors='replace'));log.flush()
  if proc.returncode:raise RuntimeError('Build failed; see evidence/software_build.txt')
 log.write('RADAR_SPI_LIVE_SOFTWARE_BUILT\n')
print('RADAR_SPI_LIVE_SOFTWARE_BUILT')
