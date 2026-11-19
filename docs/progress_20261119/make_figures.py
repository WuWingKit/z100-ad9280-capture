from pathlib import Path
import sys,json,struct,hashlib,shutil
R=Path(__file__).resolve().parents[2]/"projects/radar_spi_four_wire"
REPORT=Path(__file__).resolve().parent
sys.path.insert(0,str(REPORT/'_deps'))
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import font_manager
from matplotlib.patches import FancyBboxPatch
O=REPORT/'figures';O.mkdir(exist_ok=True)
E=R/'evidence/spi_only_real_20261119_154059'
J=json.loads((E/'result.json').read_text());F=J['frames'];assert J['passed'] and len(F)==10
font=font_manager.FontProperties(fname='C:/Windows/Fonts/msyh.ttc')
plt.rcParams.update({'font.family':font.get_name(),'axes.unicode_minus':False,'font.size':12,'axes.spines.top':False,'axes.spines.right':False,'axes.edgecolor':'#98a5b6','grid.color':'#dfe6ee','figure.facecolor':'#ffffff','axes.facecolor':'#ffffff'})
blue='#2563eb';orange='#ea7a23';navy='#172b4d';muted='#62748c';green='#09856f'
def footer(fig,text):fig.text(.065,.025,text,fontsize=10,color=muted)
def save(fig,name):fig.savefig(O/name,dpi=180,facecolor=fig.get_facecolor(),bbox_inches='tight');plt.close(fig)
raw=(E/'frame_0.bin').read_bytes();assert len(raw)==47136
v=np.frombuffer(raw[32:],dtype='<i2').reshape(-1,2)
fig,ax=plt.subplots(figsize=(11,5.9));fig.subplots_adjust(left=.09,right=.96,top=.79,bottom=.18)
ax.plot(np.arange(512),v[:512,0],label='I：低16位，signed16解释',color=blue,linewidth=1.4)
ax.plot(np.arange(512),v[:512,1],label='Q：高16位，signed16解释',color=orange,linewidth=1.3,alpha=.92)
ax.set(xlabel='第一条保留 chirp 的样点索引（0–511）',ylabel='原始码值（signed16解释）',xlim=(0,511));ax.grid(alpha=.75);ax.legend(fontsize=10,loc='upper right')
fig.text(.065,.92,'真实采集数据｜首帧首条保留 chirp 的 I/Q',fontsize=20,fontweight='bold',color=navy)
fig.text(.065,.855,'帧序号0 · 波束元数据9 / 0° · 每条 chirp 512点',fontsize=12,color=muted)
footer(fig,'来源：frame_0.bin 的32字节帧头之后前512个复数点；不是伏特、距离或目标检测结果。')
save(fig,'01_real_iq_first_chirp.png')
fig,axes=plt.subplots(2,1,figsize=(11,7),sharex=True);fig.subplots_adjust(left=.09,right=.96,top=.84,bottom=.14,hspace=.35)
x=np.arange(10)
for key,color,label in [('i_signed16_interpretation',blue,'I'),('q_signed16_interpretation',orange,'Q')]:
 axes[0].plot(x,[f[key]['mean'] for f in F],marker='o',lw=1.8,color=color,label=label)
 axes[1].plot(x,[f[key]['stddev'] for f in F],marker='o',lw=1.8,color=color,label=label)
axes[0].set_ylabel('每帧均值（码值）');axes[1].set_ylabel('每帧标准差（码值）');axes[1].set_xlabel('帧序号');axes[1].set_xticks(x)
for ax in axes:ax.grid(alpha=.75);ax.legend(loc='upper right')
fig.text(.065,.94,'10帧统计｜I/Q均值与波动',fontsize=20,fontweight='bold',color=navy)
fig.text(.065,.885,'每帧11,776个复数点；10份载荷哈希互不相同，无全零帧、无固定自检模式',fontsize=11,color=muted)
footer(fig,'来源：result.json；均值与标准差按16位补码解释，不用于判断标定、信噪比或目标运动。')
save(fig,'02_ten_frame_statistics.png')
fig=plt.figure(figsize=(12,6.5));fig.text(.06,.92,'四线SPI真实采集｜板测结果总览',fontsize=22,fontweight='bold',color=navy)
fig.text(.06,.855,'2026-11-19 · FPGA主机 / MCU从机 · SPI → AXI GPIO → PS DDR → UART',fontsize=11,color=muted)
for x0,big,small in [(.06,'10','完整帧 / 序号0–9'),(.29,'471,360','累计字节'),(.52,'117,760','I/Q复数样本'),(.75,'7,370','READ块CRC16通过')]:
 fig.patches.append(FancyBboxPatch((x0,.55),.205,.22,boxstyle='round,pad=0.012,rounding_size=0.018',transform=fig.transFigure,facecolor='#edf6f4',edgecolor='#cde5dd'))
 fig.text(x0+.015,.665,big,fontsize=25,fontweight='bold',color=green);fig.text(x0+.015,.592,small,fontsize=11,color=navy)
lines=[('帧长 / 帧头异或 / 帧序号 / ACK释放缓存','全部通过'),('第9帧：JTAG独立读DDR vs UART BIN','47,136字节逐字节一致'),('最终状态','busy=0  frame=0  fault=0')]
for k,(a,b) in enumerate(lines):
 y=.445-k*.09;fig.text(.075,y,a,fontsize=12,color=navy);fig.text(.63,y,b,fontsize=12,color=green,fontweight='bold')
footer(fig,'范围：有限轮数通信与存储验证；不代表目标距离、角度、射频质量或长期稳定性已经标定。')
save(fig,'03_board_test_summary.png')
# Source excerpts, line numbers are taken from unchanged source files.
def code_card(name,title,subtitle,path,ranges):
 lines=path.read_text(encoding='utf-8').splitlines();text=[]
 for a,b in ranges:
  if text:text.append('      ...')
  for n in range(a,b+1):
   source=lines[n-1]
   for part in range(0,max(1,len(source)),118):
    prefix=f'{n:3d} | ' if part==0 else '    | '
    text.append(prefix+source[part:part+118])
 fig=plt.figure(figsize=(13,max(5,2+len(text)*.22)),facecolor='#102238')
 fig.text(.035,.94,title,fontsize=19,fontweight='bold',color='#e7effb')
 fig.text(.035,.895,subtitle,fontsize=10,color='#a9bdd5')
 mono=font_manager.FontProperties(fname='C:/Windows/Fonts/consola.ttf',size=10.5)
 fig.text(.035,.85,'\n'.join(text),fontproperties=mono,color='#d5e6f8',va='top',linespacing=1.5)
 fig.text(.035,.025,'真实源码节录排版｜不是IDE屏幕截图｜长行自动折行，省略号表示跳过源代码行',fontsize=10,color='#a9bdd5')
 save(fig,name)
p=R/'software/main.c';lines=p.read_text().splitlines()
start=next(i+1 for i,l in enumerate(lines) if 'static int exchange' in l)
end=next(i+1 for i,l in enumerate(lines) if 'static int wait_ready' in l)-1
code_card('04_ps_crc_read_source.png','关键代码｜PS读取SPI响应并验证CRC','software/main.c · 实际发布源码的带行号摘录',p,[(start,end)])
p=R/'mcu_frontend/05_application/firmware_app/source/src/app_sync_frontend.c';lines=p.read_text().splitlines()
a=next(i+1 for i,l in enumerate(lines) if l.startswith('static int port_send('));b=a+7
c=next(i+1 for i,l in enumerate(lines) if 'if(op==2u)' in l);d=next(i+1 for i,l in enumerate(lines) if '} else if(op!=1u)' in l)
code_card('05_mcu_lease_source.png','关键代码｜MCU保留上一帧，ACK后释放','app_sync_frontend.c · 实际发布源码的带行号摘录',p,[(a,b),(c,d)])
# Literal log snippets + independent comparison, not an app screenshot.
cap=(E/'capture_9.txt').read_text();summary=next(l for l in cap.splitlines() if l.startswith('SPI FRAME DDR STORED'))
ddr=json.loads((E/'ddr_uart_comparison.json').read_text());guard=json.loads((E/'final_state_and_guard.json').read_text())
body=['[capture_9.txt]',summary,'BEGIN RADAR_RAW bytes=47136','... 完整原文及载荷保留在证据目录 ...','END RADAR_RAW','','[ddr_uart_comparison.json]',f'bytes_uart={ddr["bytes_uart"]}  bytes_jtag={ddr["bytes_jtag"]}',f'identical={str(ddr["identical"]).lower()}  cpu_resumed={str(ddr["cpu_resumed"]).lower()}','sha256_uart / sha256_jtag:',ddr['sha256_uart'],'','[final_state_and_guard.json]',guard[0]['response'].strip(),guard[1]['response'].strip()]
fig=plt.figure(figsize=(13,7),facecolor='#102238');fig.text(.035,.94,'证据节录｜第9帧、DDR交叉核对与最终状态',fontsize=19,color='#e7effb',fontweight='bold')
fig.text(.035,.885,'由保存日志与JSON排版，不是串口软件屏幕截图；未把截断节录作为完整数据',fontsize=10,color='#a9bdd5')
mono=font_manager.FontProperties(fname='C:/Windows/Fonts/consola.ttf',size=11)
# Use Chinese-capable font for the one explanatory line separately.
body[3]='... [full UART text and payload retained in evidence] ...'
fig.text(.035,.825,'\n'.join(body),fontproperties=mono,color='#d5e6f8',va='top',linespacing=1.5)
fig.text(.035,.025,'来源：evidence/spi_only_real_20261119_154059；校验结果只扩大到实际执行的测试范围。',fontsize=10,color='#a9bdd5')
save(fig,'06_log_evidence_excerpt.png')
photo=O/'07_historical_radar_board_photo.jpg'
# Historical photo is retained as an existing asset, never fabricated.
captions={
'01_real_iq_first_chirp.png':'真实采集第0帧的第一条保留chirp。按16位补码解释I/Q；横轴为样点，纵轴为码值，未经电压或距离标定。',
'02_ten_frame_statistics.png':'10帧I/Q均值与标准差趋势，来自每帧11,776点统计；不据此推断目标运动、信噪比或射频质量。',
'03_board_test_summary.png':'本轮四线SPI真实采集、DDR保存及UART导出的验证指标；最后一帧另经JTAG逐字节核对。',
'04_ps_crc_read_source.png':'PS真实源码的带行号节录，展示请求、响应与CRC16校验；不是IDE屏幕截图。',
'05_mcu_lease_source.png':'MCU真实源码的带行号节录，展示缓存租约、触发拒绝、块读取与ACK释放；不是IDE屏幕截图。',
'06_log_evidence_excerpt.png':'从保存日志及JSON生成的证据节录图；完整原文和载荷保存在证据包，不是串口软件屏幕截图。',
'07_historical_radar_board_photo.jpg':'用户此前提供的区控板接口照片，原样复制；仅说明实物接口，不能证明本次接线或测试场景。'}
manifest={'source_data':str(E),'figure_generator':'make_figures.py; matplotlib standard plotting; no fabricated measurements','images':[]}
for p in sorted(O.iterdir()):
 if p.suffix.lower() in ('.png','.jpg'):
  manifest['images'].append({'file':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'caption':captions[p.name]})
(O/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
(REPORT/'findings.md').write_text('# 报告图像素材\n\n'+'\n\n'.join('## '+n+'\n\n'+c for n,c in captions.items())+'\n\n图像按真实源码、日志和BIN生成，数据来源可追溯。未生成FFT、距离、角度或实验场景的虚假图像。\n',encoding='utf-8')
print(json.dumps(manifest,ensure_ascii=False,indent=2))
