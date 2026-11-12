# 无雷达前端的 Z100 Debug 操作

## 1. 使用本工程而不是外部前端工程

工程绝对目录：`E:/WorkSpace/Phased-array radar/fpga-learning/radar_spi_debug_20261112`。

连接Z100电源、JTAG下载器和**PS USB串口**。SPI/SYNC/READY均在FPGA内部相连；扩展口不需要接任何设备。上板前确认板型为Z100、XC7Z035FFG900-2。此工程只做易失JTAG配置，不烧写Flash。

## 2. SDK 中启动 Debug

1. 打开 Xilinx SDK 2018.3，选择工作区 `E:/FPGAWork/radar_spi_debug_sdk_20261112`。应看到 `debug_hw`、`debug_bsp`、`radar_debug_uart`；不要使用旧雷达工程的ELF或硬件平台。
2. 如需看源代码，打开应用下 `src/main.c`。本目录 `software/main.c` 为交付源文件；修改后需同步到SDK应用并Build。
3. `Xilinx → Program FPGA`：Hardware Platform选 `debug_hw`，Bitstream浏览到本工程 `hardware/radar_debug.bit`，点击Program。不要选择旧的 `radar_sync.bit`。
4. 右键 `radar_debug_uart → Debug As → Launch on Hardware (System Debugger)`。如需配置，选择处理器 `ps7_cortexa9_0`，ELF为 `hardware/radar_debug_uart.elf`；开启系统复位和PS初始化，使用 `debug_hw` 对应的 `ps7_init.tcl`，不要沿用其他硬件平台的初始化文件。
5. Debug启动器应完成PS初始化、FPGA配置及ELF下载。若已在第3步配置FPGA，可在配置中避免重复下载；首次使用默认完整启动即可。程序通常停在 `main()`，点击绿色 **Resume/F8** 继续。仅下载bit不能启动PS的C程序。
6. 用串口助手打开板子实际枚举的PS串口，115200、8位、无校验、1停止位、无流控；勿同时在SDK Terminal和其他程序中占用同一个串口。先连接串口再Resume，可看到启动提示；若错过提示，发送`s`和真实回车。

## 3. 最简测试

ASCII发送 `t` 并回车；HEX发送 `74 0D`。程序自行：

- 同时重置DMA、接收器、内部模拟源；等待READY。
- 发第一次SYNC，仅建立A缓存。
- 连续三次请求，把A/B/A的上一帧存入DDR。
- 核对每帧头部、边界保护区、全部11776个IQ样点及序号0/1/2、波束9/10/11。

最后应出现 `FPGA LOOP SELFTEST PASS`。保留整个串口日志，不只截最后一行。若失败，先保存 `s` 输出，再发 `r`；不要把未收到结果当作通过。

也可分步操作：`r → p → c → v → c → v`。`p`之后没有雷达帧返回是正确行为；第一次`c`应得到序号0和波束9，而不是序号1和波束10。

## 4. SDK 单步看哪些变量

在 `run_debug_selftest()`、`capture()`、`verify_pattern()` 设置断点。`capture()` DMA完成并执行DCacheInvalidate之后再看 `buffer`，长度47136字节：

- 头部前4字节：`10 A6 03 00`。
- 偏移28的u16为序号，偏移30的u16为XOR。
- 偏移32起：第一帧 `00 02 00 80 01 02 00 80 ...`，即I=512、513…，Q=8000。
- 第二帧Q应变为8001；头部序号和波束也相应更新。

不要停在DMA等待循环里很久后用超时值评价正常速度；PS断点不暂停PL时钟。完成抓取后再断点查看更清楚。

## 5. ILA 看时序

本工程已加入 `debug_ila_0`。先通过SDK初始化PS，使FCLK0有100MHz时钟；再在Vivado `Open Hardware Manager → Open Target → Auto Connect`，选择FPGA并关联**同一次构建**的 `hardware/radar_debug.ltx`。不要在SDK运行过程中再次Program FPGA，否则需要重新启动软件。

|Probe|内容|宽度|
|---|---|---|
|0|SYNC|1|
|1|模拟READY|1|
|2/3/4|SPI SCLK / CS_n / MOSI|各1|
|5|接收器status|32|
|6/7|FIFO写 / 读使能|各1|
|8/9/10|TVALID / TREADY / TLAST|各1|
|11|TDATA|32|
|12/13|下一采集序号 / 最近发送序号|各16|

先执行 `r,p`，在ILA设置CS_n下降沿触发，点击Run，再串口发 `c`。深度4096点、采样周期10ns，共40.96µs，可看头部开始的一段，不能一次装下一整帧约30.5ms。看DMA输出时改用FIFO写使能或TVALID上升触发；看帧尾时用TLAST=1触发。ILA时钟是100MHz，采样观察SCLK不是把SCLK作为ILA时钟。

## 6. 异常路径

各项独立执行：`r,p,b`（头部错），`r,p,l`（短帧），`r,p,n`（无响应）。预期状态进入FAULT、没有有效帧释放到DMA；保存报错和`s`状态。之后 `r,p,c,v` 应恢复通过。

## 7. 本次实测状态

2026-11-12已在Z100上通过JTAG实际下载并运行，串口COM11：连续22帧、三类异常拒绝和复位恢复均通过。还通过ILA抓取并独立解码了板内SPI头部。完整记录见 `BOARD_TEST_REPORT.md`；原始数据见 `evidence/board_20261112_170218/`。以后重新连接时COM号可能变化，应以实际枚举结果为准。

ILA显示的是连接线名称，部分沿用原BD的历史命名：`adc_board_control_0_status`实际为雷达接收状态，`adc_capture_pack_0_fifo_wr_en`实际为雷达FIFO写使能，`Net`是TREADY。它们不是外部ADC的数据源，源码中没有依赖ADC输入。
