# Z100 SPI副本：PS从机模拟数据 → PL主机 → DDR

工程入口：`E:/FPGAWork/z100_spi_sim_20261104/spi_capture.xpr`。

本副本来自`z100_cordic_loop_20261028`，原工程保留。当前需求为：**每个CS周期收8bit（1字节），每次启动只收8字节，多出不采。**

## 实际结构

PS ARM准备8字节递增码或查表正弦 → PS硬SPI0配置从机 → 内部EMIO → PL SPI主机 → 下降沿移位寄存器 → CS上升时锁存8位寄存器 → 四字节打包32位 → FIFO → AXIS → DMA S2MM → HP0 → DDR → PS核对源数据、CRC及UART输出。

- SPI Mode 2：CPOL=1、CPHA=0，MSB先发；PS在上升沿更新MISO，PL在下降沿采样。
- 板载100MHz差分输入→MMCM输出16MHz SCLK；同步÷16生成交易内1MHz CS节拍。
- CS低500ns，包含8个SCLK下降沿；CS高500ns。每次8个CS低脉冲，完成后保持高，不再采集。
- 每个CS上升时，将8位移位寄存器锁存到`rx_data`；第8字节结束时另锁存64位`frame_data`，首字节在低8位。
- DDR只有8字节有效数据，即2个32位字；TLAST在第2个32位字。
- PS SPI0实际连接BD中的SPI0_SCLK_I、SS_I、MOSI_I与MISO_O，无需外接SPI设备。
- 当前BD已移除CORDIC和并行AD/DA端口，顶层不再声明任何外部AD/DA数据或时钟引脚。

## 文件

- `rtl/spi_master_burst8.v`：最新16MHz/1MHz、下降沿、8字节及并行锁存逻辑。
- `rtl/spi_capture_core.v`：保留原控制/DMA状态协议，样本有效脉冲来自SPI，帧长严格为8。
- `software/main.c`：PS SPI0从机、源数据准备、FIFO服务、DMA及源/DDR逐字节对比。
- `sim/spi_slave_model.v`：仅供仿真，模拟PS SPI移位行为；不会执行ARM C或仿真真实PS FIFO。
- `sim/tb_spi_burst8.v`：检查256种字节、CS宽度与周期、每次恰好8字节、64位锁存和多余不采。
- `sim/tb_spi_capture.v`：SPI→32位打包→真实FIFO IP→AXIS，含背压、重复启动、错误恢复。
- `sim/tb_spi_ddr.v`：真实BD/DMA/DDR仿真；只在testbench内用模型代替PS MISO，明确不证明PS C已上板运行。
- `archive/legacy_files/`：原始ADC文件与历史产物，仅用于追溯，不参与当前编译，不包含在新ZIP中。

## 如何验证

Vivado 2018.3，SDK2018.3。各命令从副本根目录执行，按本机安装路径调整：

```powershell
$env:PROCESSOR_ARCHITECTURE='AMD64'
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/test_spi_burst8.tcl
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/run_sim.tcl
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/run_ddr_sim.tcl
```

本次交付的PS程序与SPI版BD/HDF已经重新编译，FPGA比特流已从当前SPI源码重新综合/实现。仅源/模型仿真通过，不代表PS硬SPI从机上板验证完成。此任务不操作JTAG、Flash或串口。

串口命令：`t 100000 8 3 8`测试PS递增码，`m 100000 8 3 8`测试PS正弦；`q`停止/复位。其他N会被拒绝。PS预先填满8字节TX，不依赖CPU在每个62.5ns时钟沿做软件翻转。

本副本网页在独立端口运行：`python web/serve.py --port 8766 --open`，不要与原ADC版8765混用。模式标记`SPI_SIM`，每帧8字节；8点只用于串行/存储调试，不报告可靠FFT主频。

旧README各语言、历史周报、历史仿真记录描述的是原ADC工程；以本README及`docs/SPI_CHANGELOG.md`为本副本准则。IDE源码GBK，网页/Python/Markdown为UTF-8。

## 当前交付结果（2026-11-04）

- 新硬件文件：`hardware/z100_spi_sim.bit`、`hardware/z100_spi_sim.hdf`。
- 新PS程序：`hardware/spi_uart.elf`（SDK应用名spi_uart，BSP名spi_bsp）。
- 模型验证：32次8字节交易覆盖256个字节值；CS低500ns，重复周期1us，64位顺序与多余不采均通过。
- 核心验证：5帧、145项检查，覆盖FIFO背压、非法帧长与满异常恢复。
- BD/DDR验证：2帧共16字节，966项检查，752个保护字节未改写；PS从机由testbench模型代替，不执行ARM软件。
- bit已重新生成，全设计WNS=+5.783ns、WHS=+0.055ns；DRC仅1条Advisory。复位/FIFO仍有3条CDC Critical诊断，详见cdc_routed.rpt，不能视为全项CDC签核。
- PS SPI外设的真实MISO延迟、FIFO状态与模式兼容性仍待JTAG Debug上板；本任务没有下载硬件、写Flash或占用串口。
- 实际仿真波形：`evidence/spi_burst8_timing.png`，原始波形为同目录VCD。

后续在SDK独立打开本副本sdk_spi_workspace，手动Program FPGA选择**SPI版bit**，Debug运行本副本spi_uart；网页使用端口8766。不要混用原ADC版bit/ELF。

## SPI命名清理

当前顶层为`z100_spi_top`，采集模块为`spi_capture_core`，MMCM为`spi_clk_wiz`。活动设计不含并行AD/DA端口与CORDIC实例；软件应用为`spi_uart`、BSP为`spi_bsp`。页面与CSV采用SPI byte / sample_code。旧文件集中于archive/legacy_files，仅用于追溯，不参与编译，也不进入新工程ZIP。
