# Z100 / AD9280 采集测试工程

Vivado / Xilinx SDK **2018.3**，目标 **XC7Z035FFG900-2**。

本项目验证8位ADC数据四点打包为32位，经异步FIFO、AXI-Stream、AXI DMA S2MM、PS HP0写入DDR，再由ARM通过PS UART0发送到电脑。

- [详细飞书进度文档](https://scnk2ajxowqf.feishu.cn/docx/H72Wd9MzooIdQ8xeYZtcTJGTnPg)
- [完整测试工程包及bit/HDF/ELF下载](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.1.0-20261022)
- [上电前实物核对](POWER_CHECK.md)
- [CDC和时序审查说明](hardware/REVIEW_NOTES.md)

## 截止2026-10-22的状态

| 验证 | 结果 | 来源/限制 |
|---|---|---|
| 板上`t`计数自检 | SELFTEST PASS；100点0～99，总和4950 | 用户串口截图；代理未重新操作板卡 |
| 板上`c`真实采集 | 100点统计：sum12503、min125、max126 | 用户提供；模拟输入未接信号源，不证明模拟精度 |
| 历史PL集成仿真 | 两帧200点、2676项检查PASS | 数字ADC模型、PS VIP DDR模型 |
| 本次仿真复跑 | 受阻，没有新的PASS | XSim申请536870924字节失败；无波形重试同样失败 |
| 数据复核 | 序列/统计一致；4项校验器负向测试通过 | 截图手工转录，非新硬件采集 |
| 产物一致性 | 7个文件SHA-256与历史验证记录一致 | bit、HDF、ELF及关键源文件 |
| 源码重建 | SOURCE_PROJECT_RECREATED | Vivado2018.3中创建工程、验证BD、生成IP输出；未重新综合 |
| 布局布线 | WNS3.562ns、WHS0.060ns；DRC0错误 | 历史构建；保留CDC Critical/Warning与DRC Advisory |
| Flash | 尚未固化 | 当前用于JTAG Debug；没有本项目BOOT.BIN |

原始工程快照中两个RTL与BD仍一致，`.xpr`已有变化；不声称原始工程所有文件均未改变。历史`verification_summary.json`和`offline_preflight.json`是当时结果，当前状态以`evidence/20261022_progress/progress_status.json`为准。

## 获取与打开

**推荐新成员下载Release中的完整ZIP**，校验`SHA256SUMS.txt`后解压到无中文、无空格目录，例如`E:/FPGAWork/z100_capture`。打开根目录`bd_start.xpr`。顶层应为`z100_adc_top`，BD为`capture_bd`。完整包包含生成的IP输出、SDK项目、bit/HDF/ELF、报告与证据；不包含运行缓存或巨大波形。

Git仓库存放源码、BD/XCI配置和证据，生成的IP及构建二进制位于Release。直接克隆后可在Vivado 2018.3 Tcl Console执行：

```tcl
source E:/your_checkout/z100-ad9280-capture/scripts/recreate_project.tcl
```

重建脚本创建`_rebuild/bd_start.xpr`，不会覆盖已有重建工程，也不会连接或下载板卡。IP输出需由本机Vivado重新生成。没有实机重新验证这些步骤前，不要将“源工程可重建”等同于“模拟输入已标定”。

## SDK与串口

完整包的SDK工作区为`sdk_workspace`。如项目未显示，使用SDK的Import → General → Existing Projects into Workspace，导入`z100_hw`、`adc_bsp`和`adc_uart`，不要导入旧机器的`.metadata`。

- FPGA配置：`hardware/z100_adc.bit`
- ARM应用：`sdk_workspace/adc_uart/Debug/adc_uart.elf`
- 硬件描述：`hardware/system.hdf`，以及包含bit的`hardware/z100_adc.hdf`
- 裸机源码：`software/main.c`
- UART：板载PS USB串口，UART0 RX=MIO14/TX=MIO15，115200、8N1、无流控
- `t`：内部0～99自检；`c`：真实ADC100点；`s`：状态

先完成断电实物核对，再使用JTAG Debug。按键没有绑定采样启动。ADC时钟持续运行，`c`只启动一帧记录。串口发送十六进制文本，不是100个原始二进制串口字节。

若需重新生成SDK应用，先在本目录运行SDK 2018.3 XSCT的`scripts/build_software.tcl`创建全新工作区；已有SDK工作区用`scripts/rebuild_software.tcl`。原有`scripts/verify_preflight.py`含本机原始快照路径，属于原开发机检查，不适合作为其他电脑的通用入口。

## 关键配置

ADC10MHz；采样180°；AXI100MHz；FIFO32×1024、FWFT；每帧100点/25字/100字节；末字TLAST。GPIO控制0x41200000，DMA0x40400000。数据位D0～D7：AD13、AC16、AD14、AC17、AB16、AB14、AB17、AB15；OTR=AE18；ADC时钟=AA17。必须使用配套J37连接并核实V_B10/V_B11为3V3。

## 证据来源与许可边界

截图原图、手工转录CSV、统计JSON、测试脚本及日志位于`evidence/20261022_progress`。`c`仅保留统计摘要，不补造缺失原始日志。厂家资料盘PDF及Vivado/SDK安装文件不随仓库上传；生成的AMD/Xilinx IP和SDK组件遵循其原有许可，不能据此认为整仓库都属于某个新增开源许可证。
