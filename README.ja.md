# Z100 SPI→DDR — デバッグ版

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

PLマスターとPS SPI0ハードウェアスレーブを内部EMIOで接続します。1回の開始で8バイトのみ受信し、FIFO/DMAを経由してDDRへ保存します。

Vivado / SDK **2018.3** · XC7Z035FFG900-2 · Mode 2 · SCLK 16 MHz · CS 1 MHz · UART 115200 8N1.

[Release / ZIP](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.4.0-spi-20261105) · [English documentation](README.md) · [中文详细说明](README.zh-CN.md) · [Report](https://scnk2ajxowqf.feishu.cn/docx/XQePdnk7Hon2r9xViTucwbkNnDb)

```text
t 100000 8 3 8
m 100000 8 3 8
s
q
```

CR = `0D`. XSim VCD ≠ hardware ILA. 8-byte debug payload; FFT disabled.
