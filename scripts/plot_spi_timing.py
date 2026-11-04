"""Plot actual XSim VCD events; no synthetic timing trace or hardware access."""
from pathlib import Path
import sys,re
root=Path(__file__).resolve().parents[1]
# Optional dependency: pip install -r requirements-tools.txt
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
names={}; traces={}; now=0; definitions=True
for line in (root/'evidence/spi_burst8_timing.vcd').read_text().splitlines():
 if line.startswith('$var'):
  parts=line.split();names[parts[3]]=parts[4];traces[parts[4]]=[]
 elif line.startswith('#'):now=int(line[1:])/1e6 # ps -> us
 elif line.startswith(('0','1')):
  key=line[1:]
  if key in names:traces[names[key]].append((now,int(line[0])))
 elif line.startswith('b'):
  value,key=line[1:].split()
  if key in names and not re.search('[xz]',value):traces[names[key]].append((now,int(value,2)))
start=next(t for t,v in traces['cs_n'] if v==0)
fig,axes=plt.subplots(6,1,figsize=(14,8),sharex=True)
for ax,name in zip(axes,['cs_n','sclk','mosi','miso','rx_valid','frame_done']):
 points=traces[name];xs=[t-start for t,v in points];ys=[v for t,v in points]
 ax.step(xs,ys,where='post',color='#2258ce',linewidth=1)
 ax.set_ylim(-.25,1.35);ax.set_yticks([0,1]);ax.set_ylabel(name,rotation=0,labelpad=38)
 ax.grid(alpha=.18)
 for j in range(8):ax.axvspan(j,j+.5,color='#46ad99',alpha=.1)
axes[0].set_title('Actual XSim trace: 16 MHz SCLK / 1 MHz CS / 8-byte capture (Mode 2)',loc='left')
for j in range(8):axes[0].text(j+.25,1.15,f'byte {j}',ha='center',fontsize=9)
axes[-1].set_xlim(-.1,9);axes[-1].set_xlabel('Time relative to first CS falling edge (us)')
fig.text(.13,.01,'Green: CS active (500 ns), 8 falling SCLK edges per byte. CS stays high after byte 7; parallel64 = 0x0706050403020100.',fontsize=10)
fig.tight_layout(rect=(0,.035,1,1));fig.savefig(root/'evidence/spi_burst8_timing.png',dpi=160)
print('SPI_TIMING_PLOT_SAVED')
