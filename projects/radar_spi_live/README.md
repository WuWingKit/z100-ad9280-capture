# Z100 外部雷达SPI接收与串口输出版

日期：2026-11-18。此版本不包含板内雷达数据发生器，必须接外部MCU。

路径：`E:/WorkSpace/Phased-array radar/fpga-learning/radar_spi_live_20261118`。

## 当前链路与处理内容

MCU作为SPI主机发送帧 → FPGA SPI从机恢复16位字 → 校验完整帧头/长度/序号并缓存 → FIFO → AXI-Stream → DMA S2MM → HP0 → DDR → PS解析并通过UART0输出电脑。

处理指帧校验、按帧头保留上一轮波束对应、解析16位I/Q、计算I/Q最小值/最大值/总和、记录载荷CRC32，并自动导出完整帧。没有加入FFT、滤波或目标检测；I/Q有符号值只是二补码解释，没有标定为伏特。

PL硬件直接沿用已实现的外部SPI接收版，未采用11月12日的板内模拟工程。接收RTL和bit未变化；新编译的PS程序将原来的“c接收、d导出”改为“c接收成功后自动输出统计和整帧”。原外部工程及内部模拟工程均保留。

## 下载文件

- Vivado工程：`radar_sync.xpr`。
- PL：`hardware/radar_sync.bit`，仅用本目录外部接收版。
- PS：**`hardware/radar_live_uart.elf`**，不能误用旧的radar_uart.elf或内部模拟版ELF。
- SDK硬件平台：`hardware/radar_sync.hdf`。
- PS源文件：`software/main.c`；原SDK2018.3 BSP副本：`bsp/`。
- 直接编译：`python scripts/build_live.py`，默认使用本机SDK2018.3 ARM工具链。

在SDK创建/更新匹配的HDF硬件平台，选择ps7_cortexa9_0，执行对应ps7_init后下载ELF并Resume。若想查看源代码断点，可将software/main.c导入SDK应用重新编译；交付ELF已含Debug信息。此次未下载板卡，未写Flash。

## 外部接线

| MCU | Z100 J37 | 用途 |
|---|---|---|
| P3 | 31 / AC24 | SCLK，MCU输出 |
| P0 | 29 / AD24 | CS_n，MCU输出 |
| P6 | 32 / AG24 | MOSI，MCU输出 |
| P1 | 30 / AG25 | SYNC，FPGA输出 |
| P12 | 27 / AA24 | READY，MCU输出 |
| GND | 2或4 | 共地 |

按已核对的接口与3.3V电平连接，移除与P0/P1冲突的雷达板UART0跳线。两板分别供电，勿接J37的5V。详细说明见docs/上板接线与调试.md；其中旧的v自检命令不适用于本次PS程序。

## 串口使用

电脑连接Z100的 **PS USB串口**，115200、8N1、无流控。启动提示为`Z100 RADAR SPI LIVE`。COM号以本次枚举为准。

1. 雷达MCU从Flash运行新SPI固件，Z100配置bit、初始化PS并运行新ELF。上电后不自动发SYNC。
2. 发`s`加回车，确认mcu_ready=1、ready=1。
3. 发`p`加回车：第一次SYNC只建立MCU的A缓存，不返回SPI帧。
4. 发`c`加回车：先武装DMA再发SYNC，接收上一轮帧，成功后自动打印FRAME、IQ_STATS和BEGIN RADAR_RAW…END RADAR_RAW完整数据。
5. 等END RADAR_RAW之后再发下一次c，获得下一帧。SPI来源仅外部引脚，MCU等待SYNC，不是任意时刻主动发帧。

|命令|HEX（含回车）|含义|
|---|---|---|
|s|73 0D|状态|
|p|70 0D|首帧预热|
|c|63 0D|接收、统计并自动输出整帧|
|d|64 0D|重发最近有效帧，不产生新SYNC|
|r / q|72 0D / 71 0D|复位FPGA接收/DMA；不能复位MCU，重新预热前要重置两端会话|

没有t、v、b、l、n这些板内自检/故障命令。失败帧不输出为有效数据。当前没有连续自动循环，避免UART发送期间再次触发前端。

每帧47136字节（32字节头+11776个IQ样点）。UART采用可读HEX文本，115200下整帧输出约13秒，不是连续高速雷达流。输出期间PS阻塞发送，q/r等命令也需等当前输出结束后处理；不要打开串口助手的循环发送。

## 电脑导出

保存纯接收文本为capture.txt，在本目录执行：

```powershell
python scripts/export_uart_frame.py capture.txt --out captures/run1
```

输出完整帧BIN、帧头元信息JSON及11776行IQ CSV。CSV包含chirp、样点索引、原始u16与有符号s16解释；可供后续绘图和FFT使用。解析器当前面向本次MCU版本3头部。

## 验证边界

新PS ELF已经编译；沿用外部版已实现且时序通过的PL。PC导出已用保留的完整串口数据验证。未将FPGA内部模拟板测结果冒充本版本真实雷达联调结果；此版本还需在MCU实际烧录后验证READY、SYNC、外部SPI及真实数据。


## 发布文件

完整工程、MCU烧录镜像及Z100编程文件请从 [v0.6.0-external-radar-20261118](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.6.0-external-radar-20261118) 下载。Git源码目录不存放大型生成文件。
