# Z100 SPI 版本：SDK 2018.3 上板 Debug

## 本次使用的文件

- Vivado 工程：`E:/FPGAWork/z100_spi_sim_20261104/spi_capture.xpr`
- SDK Workspace：`E:/FPGAWork/z100_spi_sim_20261104/sdk_spi_workspace`
- PL Bitstream：`E:/FPGAWork/z100_spi_sim_20261104/hardware/z100_spi_sim.bit`
- 硬件平台 HDF：`E:/FPGAWork/z100_spi_sim_20261104/hardware/system.hdf`
- PS ELF：`E:/FPGAWork/z100_spi_sim_20261104/sdk_spi_workspace/spi_uart/Debug/spi_uart.elf`
- 相同 ELF 的便捷副本：`E:/FPGAWork/z100_spi_sim_20261104/hardware/spi_uart.elf`

当前活动设计没有外部 AD/DA 信号引脚。PL 主机通过内部 EMIO 连接 PS SPI0 从机，不需要外接 SPI 数据源或跳线。首次调试可在断电状态移除不需要的 AD/DA 模块，避免与历史 ADC 回环实验混淆；不要带电插拔。使用开发板配套电源、JTAG 下载器与 PS USB 串口。

## 操作步骤

1. 按 Z100 板卡说明设置 JTAG 启动方式（若需调整启动拨码，先断电）。连接 JTAG 和 **PS 串口**，上电。不要选择 PL 串口。
2. 启动 Xilinx SDK 2018.3，在 File → Switch Workspace → Other 选择上述 `sdk_spi_workspace`。Project Explorer 应有 `z100_hw`、`spi_bsp`、`spi_uart`，不要使用历史 `adc_uart` 应用。
3. Project → Build All，确认 `spi_uart/Debug/spi_uart.elf` 已生成且无编译错误。
4. Xilinx → Program FPGA，Hardware Platform 选 `z100_hw`，Connection 选 Local，Device 识别为 XC7Z035；Bitstream 通过 Browse 选择上述 `z100_spi_sim.bit`。不勾选 Partial Bitstream；本工程没有 MicroBlaze，无需填写 BMM/MMI。点击 Program。
5. 右键 `spi_uart` → Debug As → Launch on Hardware (System Debugger)。如进入配置窗口，确认处理器为 `ps7_cortexa9_0`、应用为 SPI 版 ELF，并启用 PS 初始化（使用 `z100_hw/ps7_init.tcl`）与 Stop at main。若配置自动 Program FPGA，也必须使用本 SPI 版 bit。
6. 进入 Debug 视图后，应停在 `main()`，不是停在 `.bit` 文件中。先连接串口，再点击 Resume（F8）运行。
7. 串口配置：115200、8 data bits、no parity、1 stop bit、no flow control；发送 ASCII 文本并以回车结束。串口助手、SDK Terminal 和网页三者同一时间只能有一个占用该 COM。
8. 正常启动应打印 `Z100 PS-SPI-SLAVE LOOP v1 / UART0 115200 8N1 / NO AUTO START`。程序不会自动启动 SPI 采集。

## 首次验收：先递增码，再正弦

逐条发送，每条末尾加回车：

```text
s
t 100000 8 3 8
```

递增码成功时，8 字节必须为：

```text
00 01 02 03 04 05 06 07
END MEAS sum=28 min=0 max=7 errors=0 crc=88aa689f
SELFTEST PASS
```

CRC 大小写不影响数值。每次命令只采 8 字节，即 DMA 写入 2 个 32 位字；再次发送测试命令应得到相同结果。递增码未通过时不要以波形是否好看作为验收依据。

递增码通过后再发送：

```text
m 100000 8 3 8
```

这表示 PS 生成正弦测试字节，经 **PS SPI0 → PL SPI → FIFO → DMA → DDR** 回传。应出现 `mode=SPI_SIM`、`samples=8`、`errors=0` 与 `MEASUREMENT OK (PS SPI simulated source)`。实际字节由 PS 查表和定点参数决定。8 点用于接口和存储链路验证，不用于可靠 FFT 频率测量。参数中的 delay 字段为兼容原命令而保留，不代表外部 ADC 延迟。

结束或失败后发送 `q` 回车停止/复位；失败时先保存完整日志，再发送 `r` 回车复位。重新测试前确认 `RESET OK`。

## 网页测试与留存

SPI 网页地址：`http://127.0.0.1:8766/`，不要使用原 ADC 版 8765 页面。先断开串口助手/SDK Terminal，再在网页连接 PS 串口。先执行递增码自检，再采 SPI 模拟正弦；导出串口原文、CSV/分析 JSON 并截图。每次一帧 8 字节，记录重复测试是否稳定。

## 不通过时如何定位

- 没有 COM：检查 PS USB 串口连接、数据线和驱动；COM3/COM4 蓝牙端口不代表开发板。
- 没有 JTAG 目标：检查上电、下载器、USB/JTAG 连接和启动模式；未识别到 XC7Z035 时不要下载到其他目标。
- 没有启动横幅：确认停在 main 后已 Resume、选中正确 ELF、UART 为 PS UART0/MIO14–15，并且串口已连接。
- SPI 或 DMA 报错：保存全部状态与串口日志，核对 bit、HDF、ELF 来自同一 SPI 副本。仿真模型未执行真实 PS ARM 软件；真实 PS SPI FIFO、首位输出时序仍需本次硬件验证。

本流程采用易失性 JTAG Debug 下载，不写 QSPI Flash。当前 bit 已通过实现时序检查，仍有复位/FIFO 相关 CDC Critical 诊断需要结合硬件结果继续审查；不将仿真通过等同于已经完成上板验收。
