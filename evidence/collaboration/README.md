# 多人协作基线检查

2026-10-22新增协作配置，不修改ADC、FIFO或AXI转换RTL，不重新生成bit/ELF，也不覆盖历史Release。

本地已执行：

- `python scripts/check_repository.py`：PASS。
- Vivado 2018.3 xvlog/xelab/XSim轻量测试：`RTL_SMOKE_PASS frames=2 samples=200 words=50 stalls=44 full_abort=PASS`。

测试使用理想单时钟FWFT队列，验证有效点间隙、两帧字节顺序、TKEEP/TLAST、反压保持和FIFO满时中止。它不覆盖实际异步FIFO、CDC、PS、DMA、DDR或ADC模拟精度。之前完整PS VIP集成仿真复跑内存失败仍然保留，不能被此轻量PASS替代。

GitHub Actions使用Icarus Verilog运行同一轻量测试，并检查可移植的工程配置基线。远程结果以Actions为准。
