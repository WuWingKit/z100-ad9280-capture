# Z100真实ADC采集测试工程

本目录是单独制作的上板工作副本，使用Vivado/SDK 2018.3，目标器件XC7Z035FFG900-2。原来的仿真工程没有修改。

## 文件入口

- Vivado：`E:/FPGAWork/z100_adc_hw_20261022/bd_start.xpr`
- Block Design：`bd_start.srcs/sources_1/bd/capture_bd/capture_bd.bd`
- 硬件顶层：`rtl/z100_adc_top.v`，在Design Sources中选择它作为Top。
- 实物引脚及时序：`constraints/z100_adc.xdc`
- 新增采集控制：`rtl/adc_board_control.v`
- ARM应用源代码：`software/main.c`
- SDK工作区：`sdk_workspace`；应用工程`adc_uart`，BSP工程`adc_bsp`。
- ARM调试程序：`sdk_workspace/adc_uart/Debug/adc_uart.elf`
- FPGA临时配置：`hardware/z100_adc.bit`
- 硬件描述：`hardware/z100_adc.hdf`（包含比特流）；`hardware/system.hdf`（用于已生成的SDK硬件平台）。
- 首先阅读：`POWER_CHECK.md`。

原工程的完整归档位于：
`E:/WorkSpace/Phased-array radar/fpga-learning/backups/bd_start_before_z100_20261022.zip`

## 本版做什么

AD9280的8位数据先四点打包为32位，写入32位、1024深度的异步FIFO；FIFO读口经AXI-Stream送AXI DMA S2MM，通过PS的HP0接口写入DDR。ARM从DDR取出采样值，通过PS UART0输出给电脑。

每次命令采集100点，即25个32位字、100字节DDR数据。不是连续高速串口上传；串口发送的是带标题的十六进制文本。115200波特率不足以连续传输10MHz采样流。

ADC驱动时钟10MHz，采样时钟相位180度。四点打包时先采到的点在32位字低字节，ARM小端读取后仍按采样顺序输出。

PS GP0用于配置DMA和AXI GPIO：DMA地址`0x40400000`，GPIO地址`0x41200000`。GPIO第1通道输出采集请求翻转位/测试模式，第2通道读取完成、溢出、采样数量、ADC时钟就绪等状态。跨时钟请求使用同步后的翻转位；软件先设置模式，再发请求。

## 验证文件及边界

- `hardware/z100_sim_result.txt`：完整PL集成仿真，真实输入数字模型和内部计数自检两帧，检查打包、握手反压、TLAST、DMA及DDR模型回读。
- `hardware/timing_routed.rpt`：布局布线后的建立/保持时序与未约束路径。
- `hardware/drc_routed.rpt`、`hardware/cdc_routed.rpt`、`hardware/io_routed.rpt`：设计规则、跨时钟及最终引脚报告。
- `hardware/offline_preflight.json`：静态检查、厂家引脚对照、原工程哈希不变检查。
- `hardware/verification_summary.json`：最终产物哈希及验证状态。
- `hardware/REVIEW_NOTES.md`：CDC保留提示及复位/计数使用限制；必须阅读，不能将当前结果理解为所有CDC检查均无告警。

仿真中的ADC是数字时序模型，DDR是PS VIP模型；这些结果不能替代实物ADC、DDR及UART测试。采样计数跨时钟仅作诊断，软件在完成后再等待10us才检查其稳定值。

当前不执行任何下载。供电极性、J37模块方向、V_B10/V_B11的3V3跳线、核心板型号仍需要按`POWER_CHECK.md`在断电状态下核实。

## 完成实物核对之后：Debug流程

1. 正确设置板上JTAG启动模式，连接JTAG和PS USB串口，受控上电；有异常立即断电。
2. 打开SDK 2018.3，选择本目录下`sdk_workspace`。使用`adc_uart`的Debug配置，目标Cortex-A9 #0，配置PS初始化并下载`hardware/z100_adc.bit`，启用在`main`暂停。不要使用Flash编程功能。
3. 也可以在XSCT中使用`scripts/debug_jtag.tcl`；它检查器件型号，初始化PS、临时配置PL、下载ELF，并停在main附近。该脚本要求事先完成断电核对后再设置环境变量`Z100_PHYSICAL_CHECKED=YES`。此变量只记录人工确认，不是自动测量。
4. 确认暂停位置后，先打开串口终端，115200、8数据位、无校验、1停止位、无流控，再继续运行ARM程序。
5. 先发送`t`：经过同一个FIFO/DMA/DDR链路的0~99计数自检，预期`SELFTEST PASS`。
6. 再发送`c`：真实ADC采集，预期100个十六进制数及`CAPTURE OK`。首次模拟输入接模块信号地；数据不会因此保证全零，尚未做电压标定。出现OTR提示先检查输入是否超量程。
7. `s`读取状态。任何FIFO溢出、DMA错误、超时后程序停止再次采集，需要重新Debug初始化；不要仅按PL按键后继续发送命令。

若使用电脑自动保存脚本，先关闭占用该串口的终端，再执行：

```powershell
# COMx改成开发板PS USB串口的实际端口，不能使用蓝牙COM口。
powershell -ExecutionPolicy Bypass -File E:\FPGAWork\z100_adc_hw_20261022\scripts\capture_uart.ps1 -Port COMx -Mode t
powershell -ExecutionPolicy Bypass -File E:\FPGAWork\z100_adc_hw_20261022\scripts\capture_uart.ps1 -Port COMx -Mode c
```

脚本会检查100点及自检序列，保存`capture_t.csv`或`capture_c.csv`和串口日志。CSV包含序号、相对时间us、原始8位码值。

## 本地资料依据

开发板资料根目录：`E:/BaiduNetdiskDownload/Z100 ZYNQ开发板资料盘（A盘）`。

- `4_Source_Code/1_FPGA_Design/XC7Z035/XC7Z035/27_hs_ad_da`：AD信号引脚；注意D1为AC16、D2为AD14。
- FPGA开发指南高速AD/DA章节：J37连接及V_B10/V_B11跳线要求。
- 嵌入式Vitis开发指南及厂家Hello World硬件配置：PS时钟、DDR、PS UART0 MIO14/15。
- 提取到`board_reference`的文件只作参考，当前工程不依赖大型资料压缩包。
