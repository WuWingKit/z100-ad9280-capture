# 2026-11-19 progress report and evidence

[飞书用户身份文档](https://scnk2ajxowqf.feishu.cn/docx/KmGMdcwLgoUDa8x2wbrckl13nxb) · [Release](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.7.0-spi-four-wire-20261119)

本组第三部分：相控阵雷达数据接收与存储。本期四线SPI主从、自检3帧、真实采集10帧、最后帧DDR/UART独立核对通过。前两部分在飞书文档留空供对应成员填写。

- 当前源码：`projects/radar_spi_four_wire`。
- 旧CS异常原始透传留样：`projects/radar_spi_raw`。
- 真数据：`projects/radar_spi_four_wire/evidence/spi_only_real_20261119_154059`。
- 自检：`projects/radar_spi_four_wire/evidence/spi_only_selftest_20261119_153253`。
- SVG架构/时序图可编辑，PNG来自实际BIN/源码/日志。
- 历史实物照片仅作接口参考，不能证明本次接线。
- 图像依赖精确版本在requirements-figures.txt；make_figures.py从仓库相对路径读取当前证据，Windows字体要求见脚本。

当前路径不经过DMA，不做FPGA FFT，尚未标定目标距离、角度或SNR。旧版片选异常根因没有由新版通过而获得单独证明。
