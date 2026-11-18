# 相控阵前端：MCU SYNC乒乓采集与Z100 SPI-DDR接收

更新：2026-11-12。目录名保留最初创建日期20261111，当前为已接入MCU的开发版本。

本工程面向第三组的数据采集和存储部分：Z100输出SYNC，雷达MCU响应上升沿，采集本轮并发送上一轮的帧头和原始I/Q；FPGA验证完整SPI帧，经FIFO、AXI DMA和HP0写入DDR，PS检查后通过串口报告和导出。没有把这部分宣称为完整雷达定位/目标分离系统。

原始老师工程与旧8字节模拟工程均保留。新增MCU副本在mcu_frontend；通用逻辑mcu_sync直接加入Keil构建，已完成真实ADC/RF/波束/SPI函数和GPIO中断绑定。

## 从哪里打开

| 内容 | 入口 |
|---|---|
| Vivado | radar_sync.xpr |
| Block Design | radar_sync.srcs/sources_1/bd/capture_bd/capture_bd.bd |
| Keil：两种MCU目标 | mcu_frontend/05_application/firmware_app/keil_project/firmware_app.uvprojx |
| SDK工作区 | E:/FPGAWork/radar_sync_sdk_20261111 |
| bit及配套HDF | hardware/radar_sync.bit、radar_sync.hdf |
| Z100的PS应用 | hardware/radar_uart.elf |
| 雷达真实采集版 | hardware/radar_sync_mcu.axf / .hex / .bin |
| 雷达链路自检版 | hardware/radar_sync_selftest.axf / .hex / .bin |
| 中文接线和Debug流程 | docs/上板接线与调试.md |
| 中文代码分段解释 | docs/代码逐段讲解.md |
| 协议和寄存器 | docs/PROTOCOL.md |
| 验证范围及结果 | docs/VALIDATION.md、evidence/ |

bin只是应用代码，不是带Boot Area的整片Flash镜像。当前使用Debug/RAM下载，不执行外部Flash烧写。

## 工具和依赖

- XC7Z035FFG900-2，Vivado 2018.3 / Xilinx SDK 2018.3。
- Keil MDK 5，ARM Compiler 5.06 update7 build960；工程已按本机E:/Keil_v5配置。原参考工程指定的5.06u6未安装，因此改用本机u7。
- MCU的CMSIS、HAL和所需库包含在mcu_frontend，保留原版权；Keil应用代码区0x8000起32KB，普通数据区0x20000000起32KB；ADC A/B缓冲在独立后续RAM，构建检查其不重叠。
- Python3用于数据转换和本地审计；UART导出只使用标准库。主机C模块测试用本机VS2022，SPI/DDR仿真用Vivado自带XSim。
- Z100与雷达板分别供电；JTAG、J-Link/SWD和PS USB串口各有不同作用。

## 硬件协议

```text
PS命令 -> AXI GPIO -> FPGA SYNC -> MCU GPIO上升沿ISR
                                 | READY先低后高确认处理
                                 v
MCU ADC DMA -> A/B本轮缓冲 -> 快照帧头
上一轮缓冲 -> MCU SPI主机 -> FPGA SPI从机
 -> 整帧BRAM校验 -> 32bit FIFO -> AXIS -> AXI DMA S2MM -> HP0 -> DDR
 -> PS确认DDR完成 -> ACK -> 允许下一次SYNC
```

首次SYNC只采A，无SPI发送；之后采B发A、采A发B。头和数据同属一个缓冲区，波束/序号是采集时快照。按原RF时序，先完成ADC并关闭RF再SPI发送，不声称ADC和SPI物理并发。

每帧47136B=32B头+23×512×4B原始I/Q；实际采24 chirp，丢弃第一个chirp的采样。SPI0 Mode0、16bit、MSB先发，MCU APB50MHz /4，目标12.5MHz。CS跨整帧保持低，帧内允许停钟。Version3的Word14是采集序号，Word15是头XOR；没有线上载荷CRC。

SYNC高10us；PS限制相邻请求至少50ms。PRIME要求MCU READY下降后上升，默认50ms为ACK超时，不再仅按定时当作采集成功。普通接收/流转超时200ms。MCU故障READY低，FPGA故障锁定，需重置两端会话后重新prime。

## 接线概要

用户已确认Z100没有其他外接模块，采用官方J37接口；全部五根信号为预期3.3V，接线前仍需核对实际IO电压。

| MCU | Z100 | J37 | 用途 |
|---|---|---|---|
| P3 SPI0 CLK | AC24 | 31 | SCLK，MCU输出 |
| P0 原TX0 | AD24 | 29 | CS低有效，MCU输出 |
| P6 SPI0 MOSI | AG24 | 32 | 数据，MCU输出 |
| P1 原RX0 | AG25 | 30 | SYNC，FPGA输出 |
| P12 | AA24 | 27 | READY，MCU输出 |
| GND | GND | 2或4 | 共地 |

必须拆除雷达底板UART0的RX0/TX0 USB桥跳线。P4启动Flash片选保持高，P19及SWD用途不变。不要接J37-1的5V；不跨板连接电源；J37-28不是GND。完整步骤见中文接线文档。

## 两个MCU版本与串口命令

Keil目标radar_sync_selftest在RAM填充带序号的数据，不调用采集/RF开启回调；优先用它验证电气链路。radar_sync_mcu使用真实ADC、RF与ADT2011，默认固定0°。需要扫描时在project_config.h把PROJECT_SYNC_SCAN_ENABLE改为1并重新编译。

Z100 PS UART0为115200、8N1，每条命令需要回车：

| 命令 | 作用 |
|---|---|
| s | 状态，含真实mcu_ready |
| p | 首次预填充SYNC，等待READY往返确认 |
| c | 武装DMA、发SYNC、接上一轮完整帧到DDR |
| v | 仅自检版：逐点检查确定性RAM图案 |
| d | 导出上一有效帧的全部HEX数据 |
| r/q | 复位FPGA/DMA；MCU需要单独复位后才能重新prime |

首次调试顺序：MCU自检版运行 -> Z100 bit+PS ELF运行 -> s -> p -> c -> v -> c -> v。随后重置双方，再换真实采集版并用p/c/d。旧AD/DA网页的m/t协议不适用于本版。

## 构建和验证

先读docs/VALIDATION.md中各测试的实际边界。PowerShell在本目录运行：

```powershell
$env:PROCESSOR_ARCHITECTURE='AMD64'
./scripts/build_mcu.ps1
./scripts/test_mcu_host.cmd
python scripts/make_c_vectors.py
python sim/test_uart_export.py
& 'E:/Xilinx/Vivado/2018.3/bin/vivado.bat' -mode batch -source scripts/run_unit.tcl
& 'E:/Xilinx/Vivado/2018.3/bin/vivado.bat' -mode batch -source scripts/run_full_wire.tcl
& 'E:/Xilinx/Vivado/2018.3/bin/vivado.bat' -mode batch -source scripts/run_ddr.tcl
& 'E:/Xilinx/Vivado/2018.3/bin/vivado.bat' -mode batch -source scripts/synthesis.tcl
& 'E:/Xilinx/Vivado/2018.3/bin/vivado.bat' -mode batch -source scripts/implementation.tcl
& 'E:/Xilinx/SDK/2018.3/bin/xsct.bat' scripts/update_software_hardware.tcl
python scripts/verify_delivery.py
```

不要同时让两个Vivado进程修改主工程；独立单元/线级测试工程可单独运行。改变BD端口后先运行configure_hardware.tcl；普通重新打开xpr无需运行create_project.tcl。SDK首次新建工作区用build_software.tcl，已有工作区用update_software_hardware.tcl，单改PS源码可用rebuild_software.tcl。

SDK生成工作区使用无空格路径，是因为2018.3启动Eclipse时会错误截断带空格路径；硬件主工程仍在当前工作目录。

## 当前交付边界

交付包括完整MCU工程、Vivado工程、PS程序、配套bit/HDF/ELF和测试脚本。测试覆盖C逻辑、C生成的协议参考帧、真实SPI线级接收、全长DDR、综合与实现后时序；软件模型不等于实际雷达回波。没有连接开发板下载，没有烧写Flash，也没有声称真实采集、接线和RF时序已经硬件通过。

接线后先完成自检版c/v，再切换真实雷达。当前没有在新雷达数据链上加入FFT、目标检测或方向估计结果；输出是原始I/Q与采集元信息。


## 发布文件

完整工程、MCU烧录镜像及Z100编程文件请从 [v0.6.0-external-radar-20261118](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.6.0-external-radar-20261118) 下载。Git源码目录不存放大型生成文件。
