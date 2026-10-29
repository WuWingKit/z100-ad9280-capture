# Z100 FPGA：データ収集・DDR保存・FFTテスト環境

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

SWU 2024年度学行班・信号処理第3グループ、第3パートの学習用プロジェクトです。実際のレーダーデータが届く前に、既知のDA→ADループバック信号で収集・保存・解析の経路を確認します。完全なフェーズドアレイレーダーではありません。

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## 実装範囲

PLでCORDIC正弦波生成、8ビットAD収集、4サンプルを32ビットにパックし、非同期FWFT FIFO→AXI-Stream→DMA S2MM→PS HP0→DDRへ保存します。PSは制御・キャッシュ処理・UART0送信を担当し、FFTはPCで実行します。橙色の+90°曲線はフィットから作る参照で、2本目の実測AD信号ではありません。

## 依存関係とバージョン

- FPGA/PSのビルドとデバッグ：Vivado **2018.3**、Xilinx SDK **2018.3**、Zynq-7000/IPサポートと同梱ARMツールチェーン。準備済みの手順はWindows向けです。
- ローカルWebサーバー：Python標準ライブラリ。確認済みランタイムは **3.12.14**。デスクトップChrome/EdgeのWeb Serialとlocalhost HTTPを使用し、JTAG/PS USB-UARTドライバーを用意します。ブラウザーの特定バージョンは固定していません。
- 任意のPython解析：`numpy>=1.26`、`matplotlib>=3.8`、`pyserial>=3.5`。本環境の導入版は2.5.4、3.11.2、3.5。開発テストのみNode.js **22.19.0**を使用。Web画面だけならNumPy/Node/npmは不要です。

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## ハードウェアなしで開始

Releaseから完全プロジェクトまたはWeb ZIPを取得し、`start_dashboard.cmd`を実行します。リポジトリのルートなら次のコマンドを使用します。画面の「载入已验证的仿真数据」はデジタルシミュレーションで、基板にはアクセスしません。画面の操作ラベルは現在中国語です。

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## 実機でのDebug手順

1. J37の向き、IO電圧、アナログ入力範囲を確認し、DA出力をAD入力へ接続します。JTAGと**PS側USBシリアル**を接続します。
2. SDK 2018.3を単独起動し、展開した`sdk_workspace`を選びます。必要ならz100_hw、adc_bsp、adc_uartをImportします。
3. Program FPGAで`hardware/z100_cordic.bit`を選択し、adc_uartのELFをSystem Debuggerで実行します。mainで停止したらResumeします。
4. 他のシリアルソフトを閉じ、画面から115200/8N1で接続します。まずRAMP自己テスト、次に実AD収集を実行します。連続収集は1フレームの検証後に次を要求します。停止してから切断・画面更新を行います。

## パラメータと結果

サンプリングは10 MSPS、AXIは100 MHz、FIFOは1024×32ビット。Nは256/512/1024/2048/4096、DA周波数は1 kHz～1 MHz、Web既定値は100 kHz・4096点・delay=3・amp=8です。4096点の観測時間は409.6 μs、FFT間隔は2441.40625 Hz。2026-10-29の実測スクリーンショットは10.019/99.991/999.970 kHz、CRC合格を示しますが、絶対精度や長時間安定性の保証ではありません。UART送信中にフレーム間隔があり、連続10 MSPSストリームではありません。

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## ソースと詳細資料

完全なSDK/IP生成物とbit/ELFはReleaseで配布します。GitHubの自動ソースZIPとは異なります。GitはUTF-8、中文Windows向け完全パッケージの手書きIDEソースはGBKです。必要に応じて`python scripts/source_encoding.py --target gbk`を実行し、Web/Python/MarkdownはUTF-8のままにします。詳細資料・コード注釈は主に中国語です。Flashへの書き込みは不要です。

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
