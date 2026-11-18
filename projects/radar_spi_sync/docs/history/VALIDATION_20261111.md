# 开发验证记录（2026-11-11）

本次验证针对新建的 `radar_spi_sync_20261111`，不是旧8字节实验的结果，也不是实际雷达上板结果。

| 层次 | 结果 | 证据 |
|---|---|---|
| SPI真实信号/协议/AXIS单元仿真 | PASS，4个有效帧、633项检查、337次反压；覆盖首次只采不发、上一帧波束/数据关联、忙时重复请求、头XOR错误、短帧、超长、非完整16bit、超时、序号不连续 | `evidence/radar_sync_unit_result.txt`、`unit_console.txt` |
| 全长BD→DDR集成仿真 | PASS，2×47136B=94272B，逐字节检查头/载荷和前后保护区 | `evidence/radar_ddr_result.txt`、`ddr_console.txt` |
| 实际MCU可移植C模块主机测试 | PASS，188504项检查，覆盖A/B所有权、旧帧头/载荷、RF关闭后才发送、ISR忙时拒绝、发送失败锁定 | `evidence/mcu_host_result.txt` |
| Cortex-M3编译器语法检查 | PASS，C99，`-Wall -Wextra -Werror` | `evidence/mcu_c_syntax.txt` |
| PS UART/DMA控制应用 | 编译及链接成功，生成`hardware/radar_uart.elf`；未执行上板调试 | `evidence/software_build.txt`、`software_rebuild.txt` |
| 核心BRAM推断 | PASS，16个RAMB36E1，423 LUT、514寄存器（独立核心，不是整板资源） | `evidence/core_synthesis.txt`、`core_utilization.rpt` |
| 完整Vivado工程综合 | PASS，3167 LUT、4477寄存器、18个BRAM Tile；无综合Error/Critical Warning | `evidence/synthesis_console.txt`、`hardware/synthesis_utilization.rpt` |
| 原SPI工程保护 | 6个关键文件SHA256均与开发前一致 | `evidence/baseline_verification.json` |

完整工程综合结果另见 `evidence/synthesis_console.txt` 的最终 `RADAR_SYNC_SYNTHESIS_PASS_NO_BITSTREAM` 标志和 `hardware/synthesis_utilization.rpt`。没有此标志时不要把“独立核心综合成功”当成“完整工程综合成功”。

综合后估计时序：100MHz，Setup WNS=+3.531ns，Hold WHS=-0.091ns，报告仍写明“Timing constraints are not met”。这是尚未布局布线的估计结果，**没有完成时序收敛**；外部SPI输入及SYNC输出时序约束也仍待接线信息补全。不能据此批准上板，后续须完成实现、保持时间修复和完整约束检查。

## 各测试的实际边界

单元仿真通过异步12.4MHz左右的SPI波形驱动真实接收器；使用4点、2个有效chirp的小帧以加快异常测试。FIFO在该层为小型行为模型，强制制造满及下游反压。忙时重复请求专门安排在SPI字到达附近，确认拒绝请求不会使原事务漏字。

DDR集成仿真使用真实BD中的FIFO Generator、AXI DMA、SmartConnect及PS7 DDR模型。默认 `WIRE_MODE=0` 在SPI解码输出边界注入23568个16bit字，因此全长DDR结果**不覆盖**真实MCU、完整SPI串行链路或PS应用ELF执行。该层验证全长解析、BRAM缓存、字节序、AXIS TLAST、DMA配置与边界。`WIRE_MODE=1`可以运行完整SPI模型，但试运行因PS7模型耗时很长而停止，`ddr_wire_trial_incomplete.txt`不包含完成证据。

MCU主机测试执行的是`mcu_sync/radar_sync_pipeline.c`本身，但ADC、波束写入、SPI发送、临界区均由回调替身实现；不能替代真实MCU固件编译、GPIO复用和中断验证。Vivado中的MCU模型使用合成的波束和载荷标记，目的是识别一帧延迟后的数据归属，不是雷达回波物理模型。

## 研究中发现并修正的问题

1. **旧DMA长度不足。** 14位长度仅支持16383B，本次完整帧47136B，因此改为16位；AXIS字计数器也加宽。
2. **波束错配风险。** 发上一帧时，当前波束可能已切换。MCU把采集序号、波束、角度、配置快照与数据放入同一缓冲区，发送时不改头。
3. **非法帧污染DMA。** 直接边接收边写DDR会让短帧或坏头进入DMA。本次先收整帧，CS上升后验证通过再发送到FIFO/DMA。
4. **BRAM推断失败。** 初版把存储器写入放在异步复位状态机中，Vivado不能推断BRAM。修正为无复位的独立同步RAM读写，整帧覆写后才允许读取；没有提高拆散寄存器数量限制来绕过。
5. **忙时请求与数据同时到达。** 控制请求不能抢占当前SPI处理。本版对忙时请求只更新拒绝标志，仍处理正在接收的字和流转状态。
6. **“prime完成”不等于硬件确认。** 目前仅等待可配置时间；真实硬件仍须确认首次采集耗时，必要时新增MCU-ready。
7. **老师参考P0复用冲突。** P0将用于SPI CS，但旧采集完成函数还会拉低P0。MCU接入前必须移除该旧写入，否则会产生假SPI帧边界。

## 尚未完成的硬件验证

- 四个外部信号的引脚、IO Bank电压、连线及外部时序约束。
- MCU GPIO上升沿ISR与实际ADC/RF/SPI回调绑定、Keil完整固件构建和下载。
- FPGA布局布线、实现后时序、引脚DRC、bit生成以及Z100真实联调。
- 载荷独立校验。现协议只有头XOR；本地CRC留存不能证明源端载荷正确。
- MCU与FPGA复位后的重新同步；当前需要重置两端再prime，不能只复位一端继续。

因此本版可交付为“逻辑、仿真及综合开发副本”，不能称为“已经通过雷达实测的可直接烧写固件”。
