# f1_0 静态审计记录

## 已核对

- Keil 工程 XML 可正常解析，工程中引用的全部文件路径存在。
- `05_application` 的 9 个 C 文件已使用 Clang C99 做语法/声明静态检查，通过；宿主机检查对 CMSIS/ARM 专用 intrinsic 关闭了“隐式函数声明”诊断，因此它不能替代 ARMCC/Keil 目标编译。
- 当前工程命令处理存在 `APP_PROTOCOL_CMD_FMCW_BANDWIDTH = 0x29` 和 `APP_PROTOCOL_CMD_RUN_INTERVAL_MS = 0x2A`。
- 应用源码中无 slot/query/FIFO/Flash persistence/FFT/phase/absolute-distance 业务残留。
- Keil `06_third_party` 编译组为空；原相位与绝对距离算法库已从源码树和工程引用中删除。
- 19 组 TX/RX PSR 已与原 V2.7 73 波束表逐项比对：新表第 `i` 项等于原表第 `4*i` 项，对应 -45° 到 +45°、5° 步进。
- ADC0 仍使用 DMA1；SPI0 TX 当前由 CPU 顺序写 TX FIFO 输出。
- ADC DMA1/2 的 DONE 中断在采集配置中显式关闭；DMA 完成只由 `adcSampleWait()` 在主循环轮询 transfer count 判定。
- ADT2011 RF 关闭不再出现在 `dmaAHandler` 或任何 ADC DMA ISR 路径；稳态顺序为 `RF on + ADC DMA -> 100 us -> TX PSR -> RX PSR -> poll ADC DMA -> RF off -> TX LOAD -> RX LOAD`。
- A/B 缓冲区分别为 `0x20008000` 与 `0x20014000`，各 `0xC000` B；结束地址 `0x20020000`，位于工程配置的 `0x20000000..0x2001FFFF` RAM 范围内。
- SPI 输出长度为 23568 个 16-bit word，即 47136 B/帧。
- Keil target、AXF/HEX/BIN 及打包脚本产物名已统一为 `firmware_f1_0*`。

## 尚需上板/Keil确认

当前执行环境没有 ARMCC/Keil，因此没有生成新的目标机 AXF/HEX/BIN，也没有做硬件波形验证。交付后建议至少确认：

1. Keil `Rebuild all` 为 0 errors。
2. 示波器确认 P3≈12.5 MHz、P0 在每个 47136 B 帧期间保持低电平。
3. 接收端确认每帧为 23568 个 16-bit word，前 16 word 为元信息，随后为 I/Q 交替。
4. 同时观察 ADC DMA 与 SPI 输出并发期间无 starvation/overrun。
5. 连续 19 帧确认角度顺序 -45° → +45° 后回卷。
6. UART0 P1 下发 `0x29` 带宽帧后带宽即时生效，且复位后恢复 1400 MHz。
7. UART0 P1 下发 `0x2A` 周期帧后调度周期即时生效，且复位后恢复 50 ms。
