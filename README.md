# Z100：CORDIC 发波、同步 AD 采集、DDR 存储与 PC 频谱分析

本分支：`feature/cordic-loop-20261028`。下载完整工程、bit 和 ELF：
[v0.2.0-20261028 离线验证预发布版](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.2.0-20261028)。

Git 中保存源码、BD/XCI、文档和验证记录；生成的 IP、SDK 工作区及二进制位于 Release 完整工程包。源码检出后可用 Vivado 2018.3 执行 `scripts/recreate_project.tcl`，在 `_rebuild_cordic/bd_start.xpr` 重建工程。若希望直接查看本机已验证的完整工程，请解压 Release ZIP 后打开其中的 `bd_start.xpr`。

本版 [测试报告](docs/TEST_REPORT.md) / [CDC 审阅](docs/CDC_REVIEW.md) / [仿真图](evidence/plot_simulated/waveform_spectrum.png)。历史 2026-10-22 的证据及模块继续保留供追溯；本版使用 `rtl/measurement_core.v`、`rtl/fifo_to_axis.v` 和新的两套 testbench。

## HTML 波形/频谱页面与上板入口

双击 `web/start_dashboard.cmd`，用桌面 Chrome/Edge 打开 `http://127.0.0.1:8765`。通过 Web Serial 接收本版 PS 数据，显示时域波形、FFT 和频率，检查 CRC 并导出 CSV/JSON/PNG/串口日志。先离线演示，再递增码自检，最后真实回环。网页默认数字幅度 8，命令行无参数默认仍为 24；两者都不是电压值。具体 SDK Debug 和测试步骤见 [BOARD_TEST.md](docs/BOARD_TEST.md)。

这是从 `E:/FPGAWork/z100_adc_hw_20261022` 创建的独立开发副本。工具为 Vivado/SDK 2018.3，器件 XC7Z035FFG900-2。当前版本用于单帧、可重复触发的 AD/DA 回环实验。

**当前输出属于离线构建和仿真结果。没有下载到开发板，没有连接串口执行指令，没有写 Flash；真实模拟回环的延迟、增益、极性与频率误差尚待实测。**

## 从哪里打开

| 内容 | 本目录内位置 |
|---|---|
| Vivado 工程 | `bd_start.xpr` |
| BD | `bd_start.srcs/sources_1/bd/capture_bd/capture_bd.bd` |
| FPGA 顶层 | `rtl/z100_adc_top.v` |
| 发波、采集和控制核心 | `rtl/measurement_core.v` |
| FIFO 到 AXI-Stream | `rtl/fifo_to_axis.v` |
| 引脚和时序约束 | `constraints/z100_adc.xdc` |
| PS C 程序 | `software/main.c` |
| SDK 工作区 | `sdk_workspace`，Application 为 `adc_uart` |
| bit 流 | `hardware/z100_cordic.bit` |
| 含 bit 的硬件描述 | `hardware/z100_cordic.hdf` |
| ARM ELF | `sdk_workspace/adc_uart/Debug/adc_uart.elf`，同时复制到 `hardware/adc_uart.elf` |
| 数字核心仿真工程 | `sim_project/simcheck.xpr`，顶层 `tb_measurement` |
| 全 BD 仿真 | 主工程 Simulation Sources 中的 `tb_loop_ddr` |
| PC 采集和绘图 | `pc/capture_analyze.py` |
| 仿真数据及示例图 | `evidence/plot_simulated/` |
| 实现说明 | `docs/IMPLEMENTATION.md` |

Vivado 使用 **Open Project** 打开本目录的 xpr。打开 BD 后可看到实际 CORDIC IP、AXI DMA、PS、GPIO 配置及 FIFO。这里保留 `z100_adc_top` / `bd_start` 等旧名称，是为了沿用已有工程结构。

本版生效的自定义 Verilog 在 `rtl/`。`bd_start.srcs/sources_1/new/` 中沿用的旧采集模块仅作为历史参考，未加入本版编译；修改本版逻辑时请编辑上表中的文件。

## 数据通路

```mermaid
flowchart LR
 PS[PS 串口命令/参数] -->|GP0 / AXI-Lite| CTRL[配置GPIO和测量状态机]
 CTRL --> P[32位相位累加器]
 P --> C[CORDIC Sin/Cos IP]
 C --> SCALE[幅度缩放/极性补偿/8位码]
 SCALE --> DA[AD9708 DA]
 DA -->|实际回环接线，待实测| AD[AD9280 AD]
 AD --> PACK[每4个8位点打包32位]
 PACK --> FIFO[异步FIFO 32×1024 FWFT]
 FIFO --> AXIS[AXI-Stream / TLAST]
 AXIS --> DMA[AXI DMA S2MM]
 DMA -->|SmartConnect / HP0| DDR[PS DDR]
 DDR --> UART[PS检查/CRC/串口]
 UART --> PC[PC CSV/FFT/频谱和波形]
```

CORDIC 用于**产生正弦波**，FFT 用于**估计采集信号的频率**。本版 FFT 放在 PC，PS 负责配置、DMA 和数据回传。尚未加入 PL FFT、连续流、乒乓缓存或雷达距离/角度处理。

## 本版参数和使用

- ADC/DAC 更新频率 10 MHz；AXI 时钟 100 MHz。
- N：256、512、1024、2048、4096；默认 4096 个 8 位样本。
- 发波频率 1 kHz～1 MHz；默认 100 kHz。
- 幅度参数 0～96；默认 24。单位为理想数字幅度码，不是伏特。
- 延迟 0～255 个采样时钟；默认 3 **仅对应本工程数字模型**，需重新标定真实模拟链路。
- 不发波时 DAC 保持 128；不自动开始测量。

将来进行 JTAG Debug 测试时，串口采用 **PS UART0、115200、8N1**，使用实际枚举的 USB 串口。每条命令需带回车：

```text
h
t 100000 4096 3 24
m 100000 4096 3 24
s
r
```

`t` 是 PL 内部递增码完整性自检，DAC 保持中点；`m`（或 `c`）是真实 AD 采样，同时开启 CORDIC DA 发波；`s` 读状态；`r` 同时复位 DMA 和采集链路。`q` 停止并复位，在采集轮询期间单独发送字符 q 也可中止；已经进入串口打印阶段则需等打印结束。无参数的 `m`、`c`、`t` 使用默认值。测量失败后必须 `r` 才允许下一次。

上板前阅读 `docs/IMPLEMENTATION.md` 的接线与标定说明。尤其不要将 FPGA 数字排针当成模拟接口。

## 先离线看效果

安装 Python 依赖：

```powershell
python -m pip install -r pc/requirements.txt
python pc/capture_analyze.py --input evidence/loopback_simulated.csv --frequency 100000 --source "SIMULATED digital loopback; not board measurement" --output evidence/plot_simulated
python pc/test_analysis.py
```

打开 `evidence/plot_simulated/waveform_spectrum.png`。这是实际 Xilinx CORDIC / FIFO IP 数字仿真导出的 4096 点，不是开发板测量：参考 100 kHz，FFT 插值结果约 99.992 kHz；完整数据和数值在同目录 CSV / JSON。

也可在 Vivado 打开主工程，选择 Run Simulation → Run Behavioral Simulation，再在仿真 Tcl Console 输入 `run all`，观察 `tb_loop_ddr` 的完整 BD 测试；打开 `sim_project/simcheck.xpr` 同样操作可运行 `tb_measurement` 数字核心与异常恢复测试。

后续实测使用（COM 号按电脑实际情况修改，串口助手须先关闭）：

```powershell
python pc/capture_analyze.py --port COM7 --frequency 100000 --samples 4096 --delay 3 --amplitude 24 --output measurements/run001
```

程序只在明确提供 `--port` 时访问串口；会发送测量命令并保存原始返回、CSV、CRC 校验结果、图像及频率分析。串口是单帧低速回传，不能承担 10 MSPS 连续实时输出。

## 重新验证和构建

在本目录运行，确保 Xilinx 2018.3 可执行文件路径正确：

```powershell
$env:PROCESSOR_ARCHITECTURE='AMD64'
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/run_sim.tcl
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/refresh.tcl
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/run_ddr_sim.tcl
& E:/Xilinx/Vivado/2018.3/bin/vivado.bat -mode batch -source scripts/build_bitstream.tcl
```

已有 SDK 工作区中修改 C 后可直接在 SDK 执行 Build Project。`scripts/build_software.tcl` 用于**空 SDK 工作区首次创建**，不要在已有相同名称工程的工作区重复创建。

验证范围和最终状态见 `evidence/verification.json`。启动真实开发板前，需确认模块、J37 方向、3.3V 数字 IO 和模拟输出幅度。默认延迟不构成实物零相位保证。
