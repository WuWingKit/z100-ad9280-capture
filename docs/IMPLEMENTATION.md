# 实现与后续测试说明

## 1. 任务边界

沿用已经验证过的 Z100 AD→FIFO→DMA→DDR→PS UART 架构，增加实际 CORDIC IP 发波、PS 参数配置、相位重置、采样窗口和 PC 频率分析。

本版是单帧实验；采完一帧、确认 DMA 写完 DDR、回传串口后才能发下一帧。ARM 裸机 C 程序未在仿真器中执行，PS VIP 测试通过 AXI 事务检查 BD 数据路径。真实模拟硬件需要另行验证。

## 2. 时钟、发波和采样

原时钟向导由板载 100 MHz 差分时钟生成 10 MHz ADC 驱动时钟和相位相差 180° 的采样时钟。FPGA 在 ADC 输出稳定后采样。DAC 数据由采样时钟更新，ODDR 使 DAC 时钟上升沿在半个周期后到达，为数据提供建立时间。AXI/PS PL 接口由 FCLK_CLK0 100 MHz 驱动。

相位累加器每个有效时钟执行 `phase += K`：

`f_out = K × 10,000,000 / 2^32`

PS 用 64 位整数运算四舍五入计算 K。32 位相位转换成 16 位 Scaled Radians（π 单位、13 位小数）送给 CORDIC。CORDIC 实际配置为 Sin_and_Cos、Parallel、Maximum Pipeline、Nearest_Even、Coarse Rotation、ARESETN、NonBlocking；输出高 16 位是有符号正弦，低 16 位是余弦，正弦 1.0 对应 16384。

每次 START 都将累加器清零，CORDIC 先复位、解除复位后等待 3 个启动周期，再送第一笔相位 0。这个等待解决了仿真发现的第一笔零相位数据丢失问题。发波一旦开始，不能因 DMA 的 TREADY 变化而停顿。

`dac_code = clip(128 - ((sine × amplitude) >>> 14), 0, 255)`

减号用于补偿配套 DA 模块模拟输出的反相特性。8 位 DAC 128 对应名义零电压，具体零偏、增益和输出极性都要实测确认。AMP 是码值幅度，不是物理电压。

状态机：

```text
IDLE → PREP → WAIT_ZERO → DELAY → CAPTURE → WAIT_DMA → IDLE
                        实际CORDIC首个有效输出   软件确认DMA完成
非法参数/满FIFO → ERROR → 软件 r 复位恢复
```

DELAY 可编程。当前实现的第一笔采样发生在首个零相位 DAC 码寄存器更新后的 `delay + 2` 个采样周期。数字回环模型包含 DAC 极性、2 ns 模拟稳定时间、ADC 3 周期流水线和 25 ns 数字输出延迟，默认 delay=3 时首点为 128，随后向正方向增加。它没有模拟板上滤波器、线缆、ADC 转换孔径、温漂、噪声及器件差异。

**数据每次从同一个数字相位开始，不等于真实 ADC 第一笔必然落在模拟零点。** 真实 delay 应通过波形和相位拟合标定。必要时增加更细的时钟相位或可编程发波相位补偿，整数采样延迟只能以 100 ns 调整。

## 3. 打包、FIFO 和 AXI

每 4 点打包一个 32 位字，第一点位于 [7:0]，依次向高位；FIFO 输入和输出都是 32 位。FIFO 使用独立读写时钟、深度 1024、FWFT。N 是 4 的倍数，最大 4096 点恰好 1024 个字。

FWFT 表示 FIFO 非空时 dout 已经是队首数据。`TVALID = reset_released && !empty`，仅 `TVALID && TREADY` 时置 rd_en 并移动读指针；没有握手则数据和 TLAST 保持。TKEEP 恒为 F。TLAST 在第 N/4 个字产生，并持续到该字握手完成。Standard FIFO 模式不能直接套用此读法。

`stream_done` 仅表示最后一个 AXI-Stream 字被 DMA 接收，不能当作 DDR 已经写完。PS 必须等待 DMA S2MM IOC 且不忙、无错误，再读取 DDR，最后用命令 ACK 释放测量状态机。

DMA 为 Simple 模式、仅 S2MM。AXI-Lite 配置经 GP0 / AXI Interconnect，数据写入经 SmartConnect / HP0 到 DDR。PS 先提交接收地址和 N 字节长度，再允许 START，因此不会依赖 CPU 精确控制采样边沿。

PS 程序对 DMA 缓冲区做 Cache Flush/Invalidate，保留 64 字节尾部保护区；完成后检查 count、stream_done、DMA 错误、保护字节，再计算 CRC32。RAMP 模式还逐点核对递增码。软件超时 2 秒，失败后禁止再次 START，必须用 r 同时复位 DMA/FIFO/控制逻辑。采集轮询期间可发送字符 q 中止，它会复位 DMA/FIFO 并将 DAC 停回中点；串口打印阶段的 q 在打印结束后处理。没有额外的 PL 硬件看门狗。

跨域：START/ACK 使用 toggle 加两级同步；配置总线由 PS 保持稳定，写完后等待 1 ms 再触发，直到完成前不修改，属于稳定参数总线协议。状态和计数仅用于诊断，软件等待完成后额外 10 μs 才检查，不能用异步计数的中间值作实时控制。静态 CDC 工具的报告需结合该协议审阅，不能仅凭时序通过声称不存在 CDC 风险。

## 4. 地址与字段

| 地址 | 方向 | 内容 |
|---|---|---|
| 0x40400000 | R/W | AXI DMA 寄存器区域（S2MM 偏移 0x30 起） |
| 0x41200000 | W | 命令 GPIO channel1 |
| 0x41200008 | R | 状态 GPIO channel2 |
| 0x41210000 | W | 配置 GPIO channel1：K32 |
| 0x41210008 | W | 配置 GPIO channel2：N/delay/amp |

命令：bit0 START toggle；bit1 RAMP；bit2 软件复位电平；bit3 DMA 已提交接收；bit4 DMA 完成 ACK toggle。其它位写 0。不能把 bit0 当作单拍脉冲使用。

参数：bits[12:0] N；[20:13] delay；[27:21] amplitude；[31:28] 0。

状态：bit0 busy；1 overflow；2 frame_done；3 START ack；4 rejected；5 heartbeat；6 ADC ready；7 OTR；[20:8] count；[24:21] state；25 streamed；26 wave_started；27 waiting_dma。count 只在完成稳定后读取。

## 5. 串口和 PC

UART0 115200、8 数据位、无校验、1 停止位。PS 每帧先输出 BEGIN 元数据，再输出 N 个十六进制字节，最后输出 END 统计、CRC 和成功标志。4096 点以 ASCII 表示超过 12 KB，传输约 1 秒以上，与采集 409.6 μs 是两个不同阶段；不能按此串口方案连续传输 10 MSPS。

PC 检查 N、板端错误数、sum、CRC32、成功标志后保存数据。FFT 去直流、加 Hann 窗、计算单边幅度谱，对谱峰作对数抛物线插值；没有足够交流幅度则拒绝报告频率。幅度谱单位是 ADC code，相对 1 code 的 dB，不是 dBm。

4096 点/10 MHz 的 FFT 格点间距是 2441.40625 Hz。插值能改善单音峰值估计，但不改变两个相邻频率可分辨能力，也不能消除参考时钟误差。首点相位用已知发波参考频率做正弦/余弦最小二乘拟合，属于参考相位测量，与独立 FFT 频率估计分开输出。

## 6. 配套引脚及首次实测

采用配套示例 J37 对应引脚，全部 AD/DA 数字信号为 LVCMOS33：

| 信号 | FPGA 引脚，按 bit0→bit7 |
|---|---|
| AD D[7:0] | AD13 AC16 AD14 AC17 AB16 AB14 AB17 AB15 |
| AD CLK / OTR | AA17 / AE18 |
| DA D[7:0] | AK25 AC22 AJ25 AC23 AF17 W23 AF18 AC21 |
| DA CLK | AA13 |
| SYS_CLK P/N | D9 / D8，DIFF_SSTL135 |
| SYS_RST_N | AA25 |

AD 口均为输入，DA 数据/AD及DA时钟为输出。对应银行跳帽使用 3.3V；模块电源的 5V 与数字 IO 的 3.3V 是不同概念。DAC 不使用时保持中点 128，避免以 0 码作为名义零电压。

实际测试顺序：

1. 断电确认 J37 模块方向、板卡型号、引脚及银行供电。用配套模块模拟 DA 输出连接模拟 AD 输入，共地；禁止将模拟 ±5V 误接 FPGA GPIO。
2. 初次不做 Flash 固化，只做 JTAG 易失下载，SDK Debug 停 main，确认对应 bit/ELF 都来自本副本。
3. 先执行 t 验证数字链路，再以低数字幅度进行真实 m 测量。模块有模拟增益调节，需先测 DA 电压确认不超 AD 允许输入；幅度参数 24 本身不能保证实际电压。
4. 起始可用 100 kHz、4096 点、amp=24；比较 delay=0..若干值的前三周期，选择上升过零附近。保存原始串口、CSV 和相位拟合结果，而不是只截屏。
5. 重复同一参数多次，验证首点/相位一致性；再测试 50 kHz、250 kHz 等频率，检查增益、相位及频率误差。若每个频率延迟不同，应测量滤波器群延迟，不能沿用一个整数 delay 作全频段保证。
6. OTR、饱和、CRC、DMA 错误、超时都需保留日志。r 后先 t 再恢复真实回环。

工程中的时序约束包含 AD 数据输入延迟和 DA 数据相对转发时钟的建立/保持要求。布局布线的正裕量只验证约束下的数字实现，不替代模块接线、电压和模拟相位验证。

## 7. 后续 ILA 观察清单

当前交付 bit 未插入 ILA；先用 UART 与原始数据验证。需要硬件波形时，在本副本中另行加入 ILA 并重新生成 bit：

| ILA 时钟 | 探针 | 建议触发及用途 |
|---|---|---|
| ADC 采样时钟 10 MHz | measurement_core 的 state、phase、phase_valid、cordic_valid、cordic_data、dac_data、adc_raw、count、fifo_full、fifo_wr_en | state 进入 PREP 或 CAPTURE；观察相位清零、首个有效正弦和第一笔 ADC。8192 深度可覆盖一帧 4096 点及启动延迟。 |
| AXI 时钟 100 MHz | fifo_empty、TVALID、TREADY、TDATA、TKEEP、TLAST、stream_done、status | TVALID 且非 TREADY，或 TLAST；观察背压下保持和包尾。4096/8192 深度仅覆盖部分帧，若需全帧需更大深度或限定采样条件。 |

两个时钟域分别使用 ILA，不应把另一时钟域的未同步信号当成本域精确时序。跨域审阅及剩余诊断说明见 `docs/CDC_REVIEW.md`。
