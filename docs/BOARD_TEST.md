# Z100 Debug 与 HTML 串口测量步骤

适用：本工程 Z100 XC7Z035FFG900-2、J37 配套 AD/DA 模块，Vivado/SDK 2018.3。本版尚未执行真实 DA→AD 模拟回环测试。沿用配套示例引脚，不需要更改 J37 定义。

## 1. 用哪几个文件

- FPGA：`hardware/z100_cordic.bit`
- ARM：`hardware/adc_uart.elf`（与 SDK 工作区内 Debug/adc_uart.elf 相同）
- 硬件描述：`hardware/z100_cordic.hdf`
- SDK 工作区：`sdk_workspace`
- HTML：`web/index.html`；入口双击 `web/start_dashboard.cmd`

如果从 GitHub 下载，请从 Release 取完整工程包和附件，普通源码 ZIP 不含生成的 IP/SDK 工作区。上板时不能混用旧 `z100_adc.bit` 或旧 100 点程序。

## 2. 连接与运行

1. 按配套示例检查 J37 方向、3.3V 数字 IO 银行供电。模拟 DA OUT 接模拟 AD IN，共地；模拟接口不得接 FPGA GPIO。DA 模块有模拟增益调节，先测电压确认 AD 输入范围。
2. 上电后连接 JTAG 与 PS USB 串口。此次只用 JTAG Debug 易失下载，不生成或烧写 Flash 启动镜像。
3. SDK 打开上述工作区；在 Program FPGA 中选择本版 bit。确认硬件平台包含新的配置 GPIO `axi_gpio_config`。
4. 对 `adc_uart` 执行 Debug As → Launch on Hardware (System Debugger)，使用本版 ELF。若有旧调试会话先结束，避免重复下载/多个处理器实例混淆。
5. 停在 main 时点击 Resume（绿色继续按钮），让 ARM 进入串口命令循环。仅配置 FPGA 不会执行 ARM C 程序。
6. 应看到 `Z100 CORDIC LOOP v1 / UART0 115200 8N1 / NO AUTO START`。若用 SDK Terminal 查看该提示，确认后关闭 Terminal 释放串口。

命令行调试替代入口是 `scripts/debug_jtag.tcl`，它有物理检查环境变量门槛，只做 bit+DDR ELF 下载、不写 Flash；没有在本次准备工作中执行。

## 3. 打开 HTML

电脑安装 Python 3 后，双击 `web/start_dashboard.cmd`。它只启动本地静态网页服务，不访问串口。浏览器打开：

`http://127.0.0.1:8765`

必须用桌面 Chrome 或 Edge。浏览器不提供 Web Serial 时可使用离线演示。页面、FFT 和 CRC 代码均在本地，无 CDN，无云端上传。关闭命令窗口或 Ctrl+C 可停止服务。

手动启动也可：

```powershell
python web/serve.py --open
```

## 4. 测试顺序

1. 先点“载入已验证的仿真数据”，确认网页能显示 4096 点和约 99.992 kHz。顶部会明确标为“数字仿真演示”。此动作不打开串口、不发指令。
2. 点“连接串口”，在浏览器弹窗选择 Z100 的 PS USB 串口，避免选择 COM3/COM4 等蓝牙端口。连接采用 115200/8N1，不自动发波。
3. 先点“递增码自检”。期望返回完整 RAMP、CRC32 通过且逐点为 0..255 循环。RAMP 不作正弦频率分析。
4. 确认模拟回环和电压，勾选确认框，再点“采集真实 AD 数据”。网页默认 100 kHz、4096 点、delay=3、数字幅度=8，比工程命令默认幅度 24 更小。它不构成物理电压保证。
5. 检查顶部显示“串口实测”、完整性校验通过、码值未饱和、OTR=0；观察时域及频谱。delay=3 只在数字模型中验证，实际波形不一定首点为 128 或零相位。
6. 保存 CSV、分析 JSON、图像及串口原文。串口原文包含命令和错误，出错时也可以保存。
7. “停止/复位”发送 `q`；采集轮询期间立即中止，串口打印阶段则打印结束后处理。失败后复位，再自检、再真实测量。页面没有自动循环采集。

## 5. 显示值怎样解释

- 横轴时间按 10 MHz 换算，4096 点为 409.6 μs。
- 纵轴为 0..255 的 ADC code，128 为名义中点；没有电压标定时不能当成伏特。
- FFT 去直流、加 Hann 窗；峰值用对数抛物线插值。4096 点的格点间隔是 2441.40625 Hz，插值得到的小数频率不意味着能分辨同样间隔的双音。
- 相位是已知发波参考频率下拟合的结果，不等同于真实模拟延迟已校准。
- 采集窗口不足 3 个参考周期时页面会警告；增加点数或提高频率。输入变化小于 2 code 时不报告频率。
- DA 的电压、模拟增益、极性、AD 流水线及滤波器相移需要真实测量。不要仅凭 CRC 通过判断模拟信号质量。
- 导入 CSV 没有板端 CRC 来源证明；导入串口 TXT 会检查完整帧、CRC、统计与成功标志。

浏览器测试仅验证页面、数据解析及数字算法，不等同于串口物理链路和开发板硬件验收。
