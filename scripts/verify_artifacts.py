# 本文件的执行方式、输入输出及与其它模块的关系见 docs/CODE_GUIDE.md。
"""Offline artifact inventory and source-preservation audit. No board access."""

from pathlib import Path
import hashlib, json, re, datetime

root = Path(__file__).resolve().parent.parent


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def read(p):
    return (root / p).read_text(encoding="utf-8-sig", errors="replace")


# ===== 1. 核对旧基线未改动、本次仿真/编译日志通过 =====
baseline = json.loads(read("evidence/source_baseline_sha256.json"))
preserved = {p: sha(Path(p)) == h for p, h in baseline.items()}
assert all(preserved.values()), "Original project changed"
assert "Z100_BITSTREAM_PASS" in read("evidence/build.log")
assert "Z100_SOFTWARE_REBUILD_PASS" in read("evidence/software_rebuild.log")
assert "PASS frames=5" in read("evidence/measurement_result.txt")
assert "PASS frames=2" in read("evidence/ddr_result.txt")
assert "MEASUREMENT_SIM_PASS checks=" in read("evidence/simulation.log")
assert "DDR_INTEGRATION_PASS checks=" in read("evidence/ddr_simulation.log")
assert "PC_ANALYSIS_PASS" in read("evidence/pc_analysis_test.txt")
assert "WEB_DSP_PASS" in read("evidence/web_test_result.txt")
# ===== 2. 提取全设计时序汇总，确认bit/ELF比对应源码更新 =====
timing = read("hardware/timing_routed.rpt")
assert "All user specified timing constraints are met." in timing
timing_lines = timing.splitlines()
summary_index = next(
    i for i, line in enumerate(timing_lines) if line.strip().startswith("WNS(ns)")
)
summary_values = timing_lines[summary_index + 2].split()
timing_global = {
    k: float(summary_values[i])
    for k, i in [
        ("WNS_ns", 0),
        ("TNS_ns", 1),
        ("WHS_ns", 4),
        ("THS_ns", 5),
        ("WPWS_ns", 8),
        ("TPWS_ns", 9),
    ]
}
bit = root / "hardware/z100_cordic.bit"
assert (
    bit.stat().st_mtime > (root / "rtl/measurement_core.v").stat().st_mtime
), "Stale bitstream"
elf = root / "hardware/adc_uart.elf"
assert elf.stat().st_mtime > (root / "software/main.c").stat().st_mtime, "Stale ELF"
# ===== 3. 记录代码与二进制的SHA256、验证范围和待实测项目 =====
files = [
    "bd_start.xpr",
    "rtl/measurement_core.v",
    "rtl/fifo_to_axis.v",
    "rtl/z100_adc_top.v",
    "constraints/z100_adc.xdc",
    "software/main.c",
    "hardware/z100_cordic.bit",
    "hardware/z100_cordic.hdf",
    "hardware/adc_uart.elf",
    "pc/capture_analyze.py",
    "evidence/measurement_result.txt",
    "evidence/ddr_result.txt",
]
result = {
    "created_local": datetime.datetime.now().astimezone().isoformat(),
    "project": str(root / "bd_start.xpr"),
    "part": "xc7z035ffg900-2",
    "tools": "Vivado / SDK 2018.3",
    "original_key_files_unchanged": preserved,
    "core_simulation": read("evidence/measurement_result.txt").strip(),
    "bd_dma_ddr_simulation": read("evidence/ddr_result.txt").strip(),
    "pc_analysis": read("evidence/pc_analysis_test.txt").strip(),
    "web_dashboard": read("evidence/web_test_result.txt").strip(),
    "web_browser_check": "IAB demo, 4096 samples, 99.992 kHz, time-range switching verified; physical Web Serial untested",
    "firmware": "SDK compilation PASS; not executed on board in this task",
    "bitstream": "generated after latest measurement_core edit",
    "timing_met": True,
    "timing_global": timing_global,
    "timing_checks": [
        f"Global WNS={summary_values[0]} ns, TNS={summary_values[1]} ns",
        f"Global WHS={summary_values[4]} ns, THS={summary_values[5]} ns",
        f"Global WPWS={summary_values[8]} ns, TPWS={summary_values[9]} ns",
    ],
    "analog_model_result": json.loads(read("evidence/plot_simulated/result.json")),
    "hardware_operations_performed": False,
    "flash_programmed": False,
    "pending": [
        "Physical DAC/ADC analog voltage/gain/polarity/delay calibration",
        "New bit+ELF board Debug, UART commands and CRC validation",
        "Remaining reset/FIFO CDC diagnostics reviewed in docs/CDC_REVIEW.md; physical stress tests pending",
        "PS C software execution and q abort behavior have not been board-tested",
    ],
    "sha256": {f: sha(root / f) for f in files},
}
# ===== 4. 同时生成机器可读JSON和供成员阅读的TEST_REPORT.md =====
(root / "evidence/verification.json").write_text(
    json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8"
)
plot = result["analog_model_result"]
report = f"""# 离线验证记录

项目：`{root.name}`；器件 XC7Z035FFG900-2；Vivado/SDK 2018.3。

| 项目 | 结果 |
|---|---|
| 原工程关键文件 | 8/8 SHA256 未变化 |
| 实际 CORDIC/FIFO IP 数字核心测试 | {result['core_simulation']} |
| 全 BD / PS VIP / AXI DMA / DDR 测试 | {result['bd_dma_ddr_simulation']} |
| PC FFT、CRC 和错误拒绝测试 | {result['pc_analysis']} |
| ARM C 程序 | SDK 编译通过，尚未执行本版实物测试 |
| FPGA | 已生成 bit，全部用户时序约束满足 |

数字核心测试含 4096 点 100 kHz 连续两帧、1024 点 250 kHz、RAMP、自定义背压、非法点数、满 FIFO 及复位恢复。两次同参数数据逐点完全一致。CORE 测试的模拟通路是数字延迟模型，不能证明真实模拟相位。

完整 BD 测试由 PS VIP 写配置和 DMA 寄存器，分别采集 REAL、RAMP 各 256 点，通过 DMA S2MM/HP0 写入 DDR，读回逐字节与采样点比较，并检查 256 字节总保护区。它没有运行 ARM C 指令，也没有仿真电路板上的模拟放大器。

时序结果：

```text
{chr(10).join(result['timing_checks'])}
```

4096 点数字模型：参考 100000 Hz，FFT 插值 {plot['fft_interpolated_hz']:.6f} Hz，绝对误差 {plot['absolute_error_hz']:.6f} Hz。FFT 格点间距 {plot['fft_bin_hz']} Hz，插值误差不是可分辨带宽。首点 128，参考频率下拟合相位 {plot['phase_at_first_sample_deg']:.6f}°。

图像、原始 CSV 和 JSON 在 `evidence/plot_simulated/`；完整时序、DRC、CDC 报告在 `hardware/`。代码/输出哈希及未完成实测项目见 `evidence/verification.json`。

## 尚需实测

没有进行本版 JTAG 下载、串口访问、板上程序运行或 Flash 写入。真实 DA 电压、模拟增益、极性、整数/小数延迟补偿、实际频率误差和重复采集稳定性均需实测。默认 delay=3 只在数字模型中对齐。先验证接线/供电/输出电压，再进行易失 JTAG Debug。
"""
(root / "docs/TEST_REPORT.md").write_text(report, encoding="utf-8")
print(
    "OFFLINE_VERIFICATION_PASS original_files=8 preserved, artifacts current, simulations PASS"
)
