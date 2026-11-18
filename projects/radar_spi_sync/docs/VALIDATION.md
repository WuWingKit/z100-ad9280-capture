# 构建与验证记录

日期：2026-11-12。只描述本地代码/工具验证，不包含开发板真实下载、雷达回波实测或外部Flash烧写。

## 结果

| 层次 | 结果与证据 |
|---|---|
| MCU真实采集版完整Keil构建 | 0 Error、0 Warning；Code=23984、RO=1372、RW=144、ZI=16712B；见evidence/build_radar_sync_mcu.txt及hardware/radar_sync_mcu.map |
| MCU链路自检版完整Keil构建 | 0 Error、0 Warning；Code=22668、RO=252、RW=144、ZI=16696B；见evidence/build_radar_sync_selftest.txt |
| 通用MCU C模块实际执行 | PASS，188508项检查；A/B所有权、上一帧头/载荷、忙时ISR拒绝、RF关闭后发、发送失败锁定；见evidence/mcu_host_result.txt |
| FPGA小帧SPI单元仿真 | PASS，660项检查、337次反压；见evidence/radar_sync_unit_result.txt |
| 全长SPI线级仿真 | PASS，两帧94272B，无解码器强制注入；使用实际C模块序列化的参考数据；见evidence/radar_full_wire_result.txt |
| 完整BD→DDR集成仿真 | PASS，两帧94272B，95023项检查，逐字节头/载荷及前后保护区；见evidence/radar_ddr_result.txt |
| 串口离线解析器 | 4项测试通过：C帧、截断、错误XOR、有效XOR但角度错误；见evidence/uart_export_test.txt |
| Vivado完整综合与实现 | PASS，bit已生成；实现后Setup WNS=+5.088ns、Hold WHS=+0.026ns，全部已指定时序约束满足；见hardware/implemented_timing.rpt |
| 实现后DRC | 无Error、Critical Warning或Warning；仅AXI DMA内部FIFO的WRITE_FIRST模式REQP-181 Advisory，未降低检查级别；见hardware/implemented_drc.rpt |
| CDC | 5个输入端点均Safe，Unsafe=0、Unknown=0；见hardware/implemented_cdc.rpt |
| 实现后资源 | 2665 LUT、4037寄存器、18 BRAM Tile、0 DSP；见hardware/implemented_utilization.rpt |
| PS程序 | 最新HDF下重新生成BSP并编译链接ELF；见evidence/sdk_latest_20261112.txt |

上述MCU尺寸按构建日志记录；最终地址范围以.map和交付审计为准。Keil使用ARM Compiler 5.06u7 build960，Vivado/SDK为2018.3。

## SPI与DDR测试边界

单元仿真由MCU行为模型给出真实异步SPI波形，驱动真实接收器、解析/缓存核心和AXIS。使用缩小帧加速故障覆盖，包括：MCU未就绪、READY不往返、首次无发送、忙时重复请求、19个角度表、头XOR、短/长帧、非完整16bit、无响应超时、非法波束和角度、序号不连续。FIFO在这一层为小模型，用于强制满和反压。

全长线级测试使用两帧47136B，SCLK半周期40.25ns，与100MHz系统时钟不整齐对齐。未强制接收器内部信号，完整走SPI引脚→16bit接收→帧解析→整帧BRAM→FIFO模型→AXIS。参考帧来自实际radar_sync_pipeline.c在主机运行时输出的mcu_frame_0.bin和mcu_frame_1.bin，再转换为mcu_reference_words.hex，因此可交叉检查C与Verilog的帧头、字节序和一轮延迟关联。

DDR集成测试使用真实BD里的FIFO Generator、AXI DMA、SmartConnect与PS7 DDR模型。为避免PS7模型处理30ms串行时间导致极长运行，默认在SPI解码输出边界注入同一批C参考字，逐字节检查DDR与保护区。这一层不验证线级SPI，线级由上一个测试补足；它也不执行PS ELF。将TB的WIRE_MODE=1可把全部IP与线级串行放入同一次仿真，但该昂贵模式没有作为本次完成证据。

实际MCU C主机测试通过回调替身实现ADC/RF/SPI/临界区，不验证MCU真实寄存器、中断时延或模拟回波。完整Keil构建证明硬件绑定可编译链接，不等于已在真实MCU执行。

## 本次解决的问题

1. 原SPI/UART0关闭且无发送调用：在独立MCU副本接入SPI配置和主循环事件处理。
2. P0/P1旧输出冲突：P0由片选独占，P1输入SYNC；旧采集完成函数不再写这两个脚；P12为可见排针上的READY。
3. 首次prime仅定时：改为READY高→低→高往返，超时失败；不会因为时间到了就报告成功。
4. 乒乓波束错位：采集前快照元信息，帧头和载荷同缓冲；发上一帧时不重建为当前波束。
5. 旧DMA长度14位不足47136B：改16位；AXIS字计数扩展，11784次握手后TLAST。
6. 无效帧污染DMA：先整帧缓存，验头、长度、完整字、序号再放行。
7. Vivado无法推断BRAM：独立同步RAM模板；没有扩大寄存器拆散限制绕过。
8. REQP-1839：控制/地址改同步复位，RAM写使能、地址和数据统一打一拍，彻底切断异步控制直达BRAM端口；最终DRC无此警告。
9. 组合角度乘法占DSP并引入流水线建议：改19项角度常量表，全表验证，最终DSP=0。
10. 忙时请求跳过当前SPI字：拒绝新请求但继续原事务，专门在接收字到达附近测试。
11. 生成的BD仿真镜像缓存旧端口：同步export_ip_user_files，确保READY端口编译到仿真。
12. SDK含空格路径启动截断：只把生成工作区放到E:/FPGAWork/radar_sync_sdk_20261111，主工程仍在当前工作目录。

## 尚须硬件实测

- 核对实际IO电压、跳线帽和J37针号；软件原理图校对不能替代现场接线检查。
- MCU自检版运行后，用s/p/c/v验证真实GPIO→SPI→DDR；再切换真实采集版。
- 实测SCLK、CS包络、SYNC宽度和READY往返，核对外部连线的时序裕量。
- 真实ADC/RF/波束预写与LOAD时序、回波内容及采样参数标定。
- 源端载荷没有CRC；本地CRC只能留存，不能独立证明真实雷达数据正确。
- 两端复位需协调；当前无跨板RESET或自动会话重同步协议。

旧失败日志或docs/history内的20261111记录属于研发历史，不能替代本页所列最终结果。
