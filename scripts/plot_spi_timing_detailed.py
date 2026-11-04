from pathlib import Path
import bisect, hashlib, json, re
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'evidence/report_20261105'
VCD=ROOT/'evidence/spi_burst8_timing.vcd'
OUT.mkdir(exist_ok=True)
FONT=Path('C:/Windows/Fonts/msyh.ttc')
MONO=Path('C:/Windows/Fonts/consola.ttf')
def font(n,mono=False):return ImageFont.truetype(str(MONO if mono else FONT),n)
F=font(22); SMALL=font(18); TITLE=font(33); MONOF=font(21,True)
COLORS={'ink':'#16314c','muted':'#5b7089','grid':'#dbe4ed','blue':'#236ad2','teal':'#087f8c','orange':'#dc6c20','green':'#dff3eb'}

names={};traces={};now=0;scale=None
for line in VCD.read_text(encoding='ascii').splitlines():
 line=line.strip()
 if line=='1ps':scale='1ps'
 if line.startswith('$var'):
  p=line.split(); names[p[3]]=p[4];traces[p[4]]=[]
 elif line.startswith('#'):now=int(line[1:])
 elif line.startswith(('0','1')):
  key=line[1:]
  if key in names:traces[names[key]].append((now,int(line[0])))
 elif line.startswith('b'):
  val,key=line[1:].split()
  if key in names and not re.search('[xz]',val):traces[names[key]].append((now,int(val,2)))
assert scale=='1ps','Expected XSim timescale 1ps'
TIMES={n:[t for t,v in a] for n,a in traces.items()}
def value(name,t):
 i=bisect.bisect_right(TIMES[name],t)-1
 assert i>=0,(name,t)
 return traces[name][i][1]
def changes(name,a,b):return [(t,v) for t,v in traces[name] if a<t<b]
def pulses(name):
 out=[];low=None;previous=None
 for t,v in traces[name]:
  if previous==1 and v==0:low=t
  if previous==0 and v==1 and low is not None:out.append((low,t));low=None
  previous=v
 return out
cs_all=pulses('cs_n');first_start=cs_all[0][0]
busy_end=next(t for t,v in traces['busy'] if t>first_start and v==0)
windows=[(a,b) for a,b in cs_all if first_start<=a<busy_end]
assert len(windows)==8
sample_edges=[];payload=[]
for i,(a,b) in enumerate(windows):
 assert b-a==500000,(i,a,b)
 if i:assert a-windows[i-1][0]==1000000
 edges=[t for t,v in traces['sclk'] if v==0 and a<t<b]
 assert len(edges)==8,(i,edges)
 assert all(y-x==62500 for x,y in zip(edges,edges[1:]))
 bits=[value('miso',t) for t in edges];word=0
 for bit in bits:word=(word<<1)|bit
 assert value('rx_data',b)==word
 assert value('rx_valid',b)==1
 sample_edges.append(edges);payload.append(word)
assert payload==list(range(8))
assert value('frame_done',busy_end)==1
assert value('frame_data',busy_end)==0x0706050403020100
stop_end=first_start+9500000
assert not any(a>busy_end and a<stop_end for a,b in cs_all)
assert all(value('busy',t)==0 and value('cs_n',t)==1 for t in [busy_end+100000,busy_end+1000000])

class Plot:
 def __init__(self,width,height,a,b,title,subtitle):
  self.im=Image.new('RGB',(width,height),'#f8fbff');self.d=ImageDraw.Draw(self.im)
  self.a=a;self.b=b;self.x0=230;self.x1=width-65
  self.d.text((40,22),title,font=TITLE,fill=COLORS['ink'])
  self.d.text((40,76),subtitle,font=SMALL,fill=COLORS['muted'])
 def x(self,t):return self.x0+(t-self.a)/(self.b-self.a)*(self.x1-self.x0)
 def row(self,name,y,h=48,color=None):
  d=self.d;color=color or COLORS['blue'];zero=y+h;one=y
  d.text((35,y+10),name,font=MONOF,fill=COLORS['ink'])
  if name=='rx_data':
   # Render bus transitions and labels directly from VCD.
   pts=[self.a]+[t for t,v in changes(name,self.a,self.b)]+[self.b]
   for ta,tb in zip(pts,pts[1:]):
    xa,xb=self.x(ta),self.x(tb);v=value(name,ta)
    d.line([(xa,y+4),(xb,y+4)],fill=color,width=2);d.line([(xa,y+h-4),(xb,y+h-4)],fill=color,width=2)
    if tb!=self.b:d.line([(xb-4,y+4),(xb+4,y+h-4)],fill=color,width=2);d.line([(xb-4,y+h-4),(xb+4,y+4)],fill=color,width=2)
    if xb-xa>45:
     txt=f'{v:02X}';w=d.textlength(txt,font=MONOF);d.text(((xa+xb-w)/2,y+9),txt,font=MONOF,fill=color)
   return
  pts=[(self.a,value(name,self.a))]+changes(name,self.a,self.b)+[(self.b,value(name,self.b))]
  coords=[];prev=pts[0][1]
  for t,v in pts:
   x=self.x(t)
   if coords:coords.append((x,one if prev else zero))
   coords.append((x,one if v else zero));prev=v
  d.line(coords,fill=color,width=3)
  d.text((self.x0-29,y-4),'1',font=SMALL,fill=COLORS['muted']);d.text((self.x0-29,y+h-17),'0',font=SMALL,fill=COLORS['muted'])
 def vline(self,t,y0,y1,color=COLORS['grid'],width=1):self.d.line((self.x(t),y0,self.x(t),y1),fill=color,width=width)
 def label(self,t,y,text,font_=SMALL,color=None):
  w=self.d.textlength(text,font=font_);self.d.text((self.x(t)-w/2,y),text,font=font_,fill=color or COLORS['ink'])
 def dimension(self,a,b,y,text,color=None):
  d=self.d;color=color or COLORS['orange'];x1,x2=self.x(a),self.x(b)
  d.line((x1,y,x2,y),fill=color,width=2)
  for x in [x1,x2]:d.line((x,y-6,x,y+6),fill=color,width=2)
  self.label((a+b)/2,y-30,text,color=color)

# Full burst: first hardware-model capture, not hardware ILA.
a=first_start-180000;b=stop_end
p=Plot(2000,1130,a,b,'SPI完整8字节突发时序（真实XSim VCD）','仿真波形，不是硬件ILA实测。时间零点为首次CS下降沿；数据逐事件读取自 spi_burst8_timing.vcd（1ps）。')
rows=['start','busy','cs_n','sclk','miso','rx_valid','rx_data','frame_done'];ys={n:225+i*88 for i,n in enumerate(rows)}
for i,(ca,cb) in enumerate(windows):
 p.d.rectangle((p.x(ca),170,p.x(cb),960),fill='#e1f3eb')
 p.label((ca+cb)/2,130,f'B{i}: {payload[i]:02X}',font_=F)
 p.dimension(ca,cb,190,'500ns')
for t in range(0,10):
 tt=first_start+t*1000000
 if tt<=b:p.vline(tt,210,960);p.label(tt,980,f'{t}.0')
for name in rows:p.row(name,ys[name],color=COLORS['teal'] if name in ['miso','rx_data'] else COLORS['blue'])
p.vline(busy_end,210,960,COLORS['orange'],3)
p.label(busy_end+900000,140,'8字节结束：CS保持高',font_=F,color=COLORS['orange'])
p.label(busy_end+900000,170,'SCLK仍运行，不再采样',color=COLORS['orange'])
p.d.text((p.x0,1020),'相对时间 / μs',font=F,fill=COLORS['ink'])
p.d.text((40,1060),'断言通过：8个CS低窗口 × 每窗8个下降沿；payload=00 01 02 03 04 05 06 07；parallel64=0706050403020100。',font=SMALL,fill=COLORS['muted'])
p.im.save(OUT/'spi_timing_burst_detail.png')

# Byte[5] = 0x05, sixth byte: 0000 0101, sample MISO at falling edges.
idx=5;ca,cb=windows[idx];edges=sample_edges[idx];bits=[value('miso',t) for t in edges]
a=ca-110000;b=cb+200000
p=Plot(2000,1110,a,b,'单字节放大：Byte[5] = 0x05（第6字节）','真实XSim VCD事件；Mode 2，MSB先发。绿色窗口为CS有效，橙色虚线表示8次SCLK下降沿采样。')
rows=['cs_n','sclk','miso','rx_valid','rx_data','frame_done'];ys={n:260+i*100 for i,n in enumerate(rows)}
p.d.rectangle((p.x(ca),230,p.x(cb),870),fill='#e1f3eb')
p.dimension(ca,cb,155,'CS低电平 = 500 ns')
p.dimension(edges[0],edges[1],205,'62.5 ns',COLORS['teal'])
for t in range(-100,701,100):
 tt=ca+t*1000
 if a<=tt<=b:p.vline(tt,230,870);p.label(tt,900,str(t))
for name in rows:p.row(name,ys[name],color=COLORS['teal'] if name in ['miso','rx_data'] else COLORS['blue'])
for k,t in enumerate(edges):
 x=p.x(t)
 for yy in range(240,870,12):p.d.line((x,yy,x,yy+6),fill='#d58a43',width=1)
 p.label(t,230,f'b{7-k}',color=COLORS['orange'])
 p.label(t,ys['miso']+62,f'{bits[k]}',font_=F,color=COLORS['orange'])
 p.d.ellipse((x-5,ys['miso']+(0 if bits[k] else 48)-5,x+5,ys['miso']+(0 if bits[k] else 48)+5),fill=COLORS['orange'])
p.vline(cb,240,870,COLORS['orange'],3)
p.label(cb+90000,190,'CS↑锁存0x05',font_=F,color=COLORS['orange'])
p.label(cb+90000,220,'rx_valid置1',color=COLORS['orange'])
p.d.text((p.x0,945),'相对本字节CS下降沿的时间 / ns',font=F,fill=COLORS['ink'])
p.d.text((40,995),'下降沿采样位序列：b7..b0 = 0 0 0 0 0 1 0 1 → 0x05；第8次采样后31.25ns，CS上升并锁存字节。',font=SMALL,fill=COLORS['muted'])
p.d.text((40,1035),'该字节不是帧尾，frame_done仍为0；只有第8字节结束时才产生帧完成脉冲。未绘制或推测FIFO内部信号。',font=SMALL,fill=COLORS['muted'])
p.im.save(OUT/'spi_timing_byte_detail.png')

result={'source':str(VCD.relative_to(ROOT)),'source_sha256':hashlib.sha256(VCD.read_bytes()).hexdigest(),'timescale':scale,'scope':'First simulated burst from real XSim VCD; not hardware ILA','first_cs_fall_ps':first_start,'busy_fall_ps':busy_end,'cs_windows_ps':windows,'falling_edges_ps':sample_edges,'payload_hex':[f'{x:02X}' for x in payload],'frame_parallel64':f'{value("frame_data",busy_end):016X}','assertions':{'eight_cs_low_windows':True,'CS_low_500ns':True,'CS_period_1us_during_burst':True,'eight_falling_edges_per_byte':True,'SCLK_period_62_5ns':True,'rx_data_matches_sampled_MISO':True,'frame_done_after_eighth_byte':True,'no_extra_CS_or_sampling_after_capture':True},'detail_byte':{'zero_based_index':idx,'ordinal':idx+1,'value_hex':f'{payload[idx]:02X}','sampled_bits_MSB_first':bits},'figures':['spi_timing_burst_detail.png','spi_timing_byte_detail.png']}
(OUT/'spi_timing_detailed_checks.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print('DETAILED_SPI_VCD_TIMING_PASS 8 CS windows / 64 falling edges / 0..7 payload / 64-bit latch / no extra sample')
