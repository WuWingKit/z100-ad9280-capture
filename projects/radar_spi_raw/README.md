# 外部SPI原始接收诊断版（2026-11-19）

从radar_spi_live_20261118独立复制，MCU固件及接线不变。FPGA保持SPI从机、Mode0、MSB先发；改为逐字节接收，不检查雷达帧头、波束、序号或固定帧长，也不计算FFT、不解释I/Q。

p：首次SYNC预热。c：配置DMA、发SYNC，跨CS收集原始字节直到READY先低后高且CS空闲；200ms超时也输出已接收部分并标记timeout。d：重复导出上一批保存数据。r：只复位FPGA，重新预热前MCU也需独立复位。

上限47136字节；total_bytes记录容量外观察到的完整字节，overflow/truncated标记超限。串口导出有效长度；DMA固定长度中未使用区域补零，不计入串口有效载荷。字节按线上MSB顺序组装，完整字节跨CS连续保存；不足8位的片段不伪装成字节，记录片段次数、最后片段bit数及低位对齐的bit值。并非无限缓存或持续高速串口转发。

RAW_DIAG页：0=完整字节总数；1=CS下降沿次数；2=不足8位的CS段数；3 bit17=timeout、bit16=overflow、bit15:13=最后片段bit数、bit7:0=最后片段原始值。

build_diagnostic.tcl生成evidence/diagnostic_20261119/radar_diagnostic.bit；build_live.py生成hardware/radar_live_uart.elf。program_diagnostic_board.py临时JTAG下载，不写Flash。旧hardware/radar_sync.bit仅为复制的原live版，不能与raw ELF搭配。以诊断目录中新生成bit为准。

scripts/test_raw.tcl：Vivado行为仿真，验证无效帧头仍可接收、跨CS累计、非4字节对齐、半字节事件和DMA补零。
scripts/capture_raw.py：保存原始UART、二进制及统计，雷达帧头只在PC上观察，不作为接收条件。

PS串口115200/8N1，COM7。接线：雷达SCK→J37-31/AC24；MOSI→32/AG24；TX0(片选)→29/AD24；RX0(SYNC)←30/AG25；TX1(READY)→27/AA24；GND→2或4。两板分别供电，IO电压匹配，TX0/RX0不与USB-UART争用。
