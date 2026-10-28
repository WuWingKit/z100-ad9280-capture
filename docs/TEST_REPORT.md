# 离线验证记录

项目：`z100_cordic_loop_20261028`；器件 XC7Z035FFG900-2；Vivado/SDK 2018.3。

| 项目 | 结果 |
|---|---|
| 原工程关键文件 | 8/8 SHA256 未变化 |
| 实际 CORDIC/FIFO IP 数字核心测试 | PASS frames=5 checks=39707 cordic_checks=9306 stalls=1876 invalid_config=PASS overflow_recovery=PASS |
| 全 BD / PS VIP / AXI DMA / DDR 测试 | PASS frames=2 samples=512 DDR_bytes=512 guards=256 checks=1572 |
| PC FFT、CRC 和错误拒绝测试 | PC_ANALYSIS_PASS frequencies=4 CRC/truncation/error_rejection=PASS flat_input_rejection=PASS |
| ARM C 程序 | SDK 编译通过，尚未执行本版实物测试 |
| FPGA | 已生成 bit，全部用户时序约束满足 |

数字核心测试含 4096 点 100 kHz 连续两帧、1024 点 250 kHz、RAMP、自定义背压、非法点数、满 FIFO 及复位恢复。两次同参数数据逐点完全一致。CORE 测试的模拟通路是数字延迟模型，不能证明真实模拟相位。

完整 BD 测试由 PS VIP 写配置和 DMA 寄存器，分别采集 REAL、RAMP 各 256 点，通过 DMA S2MM/HP0 写入 DDR，读回逐字节与采样点比较，并检查 256 字节总保护区。它没有运行 ARM C 指令，也没有仿真电路板上的模拟放大器。

时序结果：

```text
Global WNS=4.630 ns, TNS=0.000 ns
Global WHS=0.051 ns, THS=0.000 ns
Global WPWS=3.000 ns, TPWS=0.000 ns
```

4096 点数字模型：参考 100000 Hz，FFT 插值 99992.044307 Hz，绝对误差 7.955693 Hz。FFT 格点间距 2441.40625 Hz，插值误差不是可分辨带宽。首点 128，参考频率下拟合相位 0.000132°。

图像、原始 CSV 和 JSON 在 `evidence/plot_simulated/`；完整时序、DRC、CDC 报告在 `hardware/`。代码/输出哈希及未完成实测项目见 `evidence/verification.json`。

## 尚需实测

没有进行本版 JTAG 下载、串口访问、板上程序运行或 Flash 写入。真实 DA 电压、模拟增益、极性、整数/小数延迟补偿、实际频率误差和重复采集稳定性均需实测。默认 delay=3 只在数字模型中对齐。先验证接线/供电/输出电压，再进行易失 JTAG Debug。
