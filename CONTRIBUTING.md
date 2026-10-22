# 共同开发约定

## 成员与权限

仓库所有者为`@WuWingKit`。已向`@SunXingzhi`、`@Qing8889494`发出Write权限邀请，接受邀请后可以推送功能分支和参与PR审阅。第三位成员un.暂缺GitHub用户名，待确认后邀请。不要把个人邮箱、密码、访问令牌或下载器凭据提交到公开仓库。

个人仓库的合作者不是管理员；main的合并由保护规则控制。公开仓库允许外部人员Fork和提交PR，但这不等于获得仓库写入权限。

## 每次改动的流程

1. 从最新main新建分支：`feat/简短主题`、`fix/简短主题`、`docs/简短主题`或`test/简短主题`。
2. 在Issue中说明负责的模块和验证方法，再开始修改。BD、XDC、PS配置同一时段只安排一人编辑。
3. 一次PR聚焦一个目的；先提交源文件，再附测试日志或截图，并说明哪些测试没有做。
4. 推送分支后创建PR，按模板填写，至少由另一人审阅。
5. `Source and RTL checks`通过、讨论解决且审阅通过后，使用Squash merge合并。
6. 合并后删除已合并分支，下个任务从最新main重新建分支。main禁止强推和删除；所有者保留紧急管理能力。

## FPGA工程特别要求

- 工具基线：Vivado/SDK 2018.3；器件XC7Z035FFG900-2。不要在同一PR混入未经说明的IP升级。
- 修改采集RTL需要验证点数、打包字节顺序、反压保持、TLAST位置和复位后恢复。
- 修改BD或XDC需要本地Validate Design，必要时重新生成输出、综合、实现并记录WNS/WHS/DRC/CDC。
- 输入时序预算、引脚电平、ADC驱动频率变化必须单独说明；CI不能证明接线、供电或模拟精度安全正确。
- 修改PS配置后，必须同步HDF、BSP、应用构建与版本说明；不能混用旧bit和新ELF。
- 不自动处理BD冲突。发生冲突时由该次BD负责人在Vivado中重做改动并验证。

## 什么进入Git

提交RTL、BD/XCI配置、XDC、C源码、Tcl、测试、说明及精简证据。不要提交`.Xil`、SDK`.metadata`、`.runs`、`.cache`、巨大WDB波形或安装资料盘。

bit/HDF/ELF和完整可打开工程包使用Release保存，并提供SHA-256、对应源码提交和验证范围。原有`v0.1.0-20261022`是历史测试基线，不因协作说明改动而重打包。

本地运行：

```text
python scripts/check_repository.py
iverilog -g2012 -s tb_pack_axis_smoke -o rtl_smoke bd_start.srcs/sources_1/new/adc_capture_pack.v bd_start.srcs/sources_1/new/fifo_to_axis.v sim/tb_pack_axis_smoke.v
vvp rtl_smoke
```

轻量测试使用理想FWFT队列连接两个RTL模块，不覆盖真实异步FIFO、PS、DMA、DDR或ADC模拟电路；完整集成测试仍需要Vivado与板卡环境。

## 提交测试结果时的措辞

标明来源：本次本地仿真、历史报告、用户板上截图、原始日志或人工转录。未做或受阻要明确记录，不能把历史PASS写成本次PASS。已知CDC提示不能直接删除或降级来让报告变绿。
