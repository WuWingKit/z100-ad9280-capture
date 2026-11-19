# 四线SPI主从改版（2026-11-19）

FPGA主机，区控板MCU从机；仅CS、SCK、MOSI、MISO与GND作为板间接口。TX0/RX0/TX1不再作片选、SYNC或READY。独立副本，不覆盖此前live/raw工程。

## 当前验证结果（2026-11-19）

MCU两目标编译0错误0警告；PS编译无编译器警告；FPGA主机Mode0仿真、实现、setup/hold及DRC检查通过。

- 自检版真实四线链路：3帧、141408字节、35328个复数点逐点匹配，零错误。
- 真实采集版：10帧、471360字节、117760个复数点，序号0～9连续；7370个READ块CRC16通过，载荷互不相同且非全零/非自检模式。
- 最后一帧由JTAG独立读取DDR，与UART BIN逐字节一致。
- 这验证数据链路，尚未标定目标距离、角度、SNR；本版不经DMA、不做FPGA FFT。

用户确认数据SPI与启动Flash独立；本次四线通信已通过。源码及启动横幅中的Flash隔离提示是先前核对阶段保留的说明，不是运行时检测错误。旧主机固件不得与新FPGA主机混接，必须升级两端并断电改线。
新版SP328整片镜像由Release下载，包含BootV3头与FF填充；从源码构建产生的hardware根目录裸BIN不能直接整片烧录。

## 已采用的接口方案
| 区控板丝印/逻辑脚 | Z100 J37 | 方向 |
|---|---|---|
| SCK/P3 | 31 / AC24 | FPGA→MCU |
| CS/P4 | 29 / AD24 | FPGA→MCU |
| MOSI/P6 | 32 / AG24 | FPGA→MCU |
| MISO/P5 | 27 / AA24 | MCU→FPGA |
| GND | 2或4 | 共地 |

不连接TX0、RX0、TX1、3V3。原SYNC的J37-30/AG25不用。两板独立供电、IO电压必须匹配3.3V。HDL内部暂保留旧BD名称mcu_ready，实际承载MISO，不是就绪信号。

## 协议与数据流

SCLK初始100kHz，Mode0，16位MSB先发；每16位字单独CS事务，字间100us空隙。整条命令/应答由魔数、长度和CRC定界，可跨多个CS。**此协议不再采用旧版整帧CS保持低的规则。**

请求8个16位字：D5A0、opcode、request_id、offset低16、offset高16、length_bytes、reserved=0、CRC16。
响应：5AD0、opcode、request_id、flags、length_bytes、offset低16、offset高16、payload、CRC16。
payload按内存字节顺序打包，先高字节再低字节上线路；奇数字节尾部填0但不计入length。CRC16/CCITT-FALSE初值FFFF、多项式1021，覆盖CRC前每字高字节再低字节。

MCU空闲输出FFFF，主机发完请求后继续用FFFF时钟寻找5AD0，应答允许FIFO延迟。仅一条请求未完成，必须收到完整应答及CRC后再发下一条。

| opcode | 用途 |
|---|---|
| 1 STATUS | flags bit0忙、bit1上一帧可读、bit2故障、bit15请求错误 |
| 2 TRIGGER | 替代SYNC。重复最后同一request_id不会重复触发 |
| 3 READ | offset+length读取上一帧，单块最多64字节，可重读，不释放缓存 |
| 4 ACK | offset填写已读帧序号，释放上一帧缓存 |

首个TRIGGER采A，不发布上一帧；之后采B，发布A。上一帧未ACK时拒绝新TRIGGER，防止覆盖。帧头与其数据始终属于同一缓存。MCU中断只服务SPI命令和小块应答，射频/ADC采集仍在主循环。

PS先STATUS确认，再TRIGGER、STATUS轮询、READ分块读取47136字节到DDR、校验每块CRC、ACK、UART输出。UART命令s查询、p首次采集、c再采并读上一帧。没有开机自动通信。链路异常停止并保留缓存，需双方复位重新开始；不自动跳过CRC错误或重复触发采集。

当前调试路径为PL SPI主机→AXI GPIO寄存器→PS DDR数组→UART，不经过AXI DMA。旧BD中DMA/FIFO保持空闲；确认协议与电气后，可再做PL批量接收及DMA优化。它不是已验证的高速雷达吞吐版本。

## 工程与构建

- Keil：mcu_frontend/05_application/firmware_app/keil_project/firmware_app.uvprojx，radar_sync_mcu真实采集、radar_sync_selftest固定数据。
- MCU适配：app_sync_frontend.c，保留原RF/ADC回调，增加SPI从机邮箱。
- FPGA：rtl/radar_frame_capture.v、radar_sync.xpr；旧模块名为BD兼容保留，内部已是主机。
- PS：software/main.c。
- build_mcu.ps1生成裸应用；build_live.py生成PS ELF；test_spi_master.tcl行为仿真；build_diagnostic.tcl综合、实现及检查。
- 匹配PL输出为evidence/diagnostic_20261119/radar_diagnostic.bit；不能混用其他工程bit或ELF。

本次未更改已连接设备的固件，也未发布GitHub。

配套产物：hardware/spi_only_master.bit、hardware/spi_only_ps.elf。verification.json记录哈希与验证边界。

## 2026-11-19新增实测结果
四线SPI主从自检已完成三帧上板验证，141408字节、35328个复数样本逐点一致，2211个READ块CRC16通过。报告位于evidence/spi_only_selftest_20261119_153253/上板自检报告.md。真实射频采集未验证，数据路径不经过DMA。

## 真实采集版最新实测
2026-11-19完成真实采集10帧（序号0～9），471360字节、117760个复数样本、7370个READ块CRC通过；最后一帧JTAG DDR与UART逐字节一致。详情见evidence/spi_only_real_20261119_154059/真实采集上板测试报告.md。未验证目标距离/角度或信号质量标定。

历史构建时点的evidence/verification.json保留原状态；最新硬件结果以自检及真实采集目录内result.json和上板报告为准。
