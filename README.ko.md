# Z100 FPGA 데이터 수집·DDR 저장·FFT 테스트 환경

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

SWU 24학번 학행반 신호처리 3조의 세 번째 파트를 위한 학습 프로젝트입니다. 실제 레이더 데이터가 준비되기 전에 알려진 DA→AD 루프백 신호로 수집·저장·분석 경로를 검증합니다. 완성된 위상배열 레이더 시스템은 아닙니다.

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## 구현 범위

PL은 CORDIC 사인파 생성, 8비트 AD 수집, 4샘플의 32비트 패킹을 수행합니다. 데이터는 비동기 FWFT FIFO→AXI-Stream→DMA S2MM→PS HP0→DDR로 이동합니다. PS는 제어·캐시 관리·UART0 출력을, PC는 FFT를 담당합니다. 주황색 +90° 곡선은 피팅으로 계산한 기준이며 두 번째 AD 실측 채널이 아닙니다.

## 의존성과 버전

- FPGA/PS 빌드 및 디버깅: Vivado **2018.3**, Xilinx SDK **2018.3**, Zynq-7000/IP 지원 및 동봉 ARM 도구 체인. 준비된 절차는 Windows 기준입니다.
- 로컬 웹 서버: Python 표준 라이브러리, 확인한 런타임 **3.12.14**. Web Serial을 지원하는 데스크톱 Chrome/Edge와 localhost HTTP, JTAG 및 PS USB-UART 드라이버가 필요합니다. 특정 브라우저 버전은 고정하지 않았습니다.
- 선택적 Python 분석: `numpy>=1.26`, `matplotlib>=3.8`, `pyserial>=3.5`; 로컬 설치 버전은 2.5.4/3.11.2/3.5입니다. 개발자 테스트에는 Node.js **22.19.0**을 사용했습니다. 웹 실행만 할 때는 NumPy/Node/npm이 필요 없습니다.

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## 보드 없이 시작

Release에서 전체 프로젝트 또는 웹 ZIP을 받아 start_dashboard.cmd를 실행합니다. 저장소 루트에서는 아래 명령도 가능합니다. “载入已验证的仿真数据”는 보드에 접근하지 않는 디지털 시뮬레이션 데모입니다. 현재 UI 라벨은 중국어입니다.

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## 보드 Debug 절차

1. J37 방향, IO 전압 및 AD 입력 범위를 확인하고 DA 아날로그 출력을 AD 입력에 연결합니다. JTAG와 **PS USB 시리얼**을 연결합니다.
2. SDK 2018.3을 독립 실행하고 압축을 푼 sdk_workspace를 선택합니다. 필요하면 z100_hw, adc_bsp, adc_uart 프로젝트를 Import합니다.
3. Program FPGA에서 hardware/z100_cordic.bit를 선택하고 adc_uart ELF를 System Debugger로 실행합니다. main에서 멈추면 Resume을 누릅니다.
4. 다른 시리얼 도구를 닫고 웹에서115200/8N1로 연결합니다. 먼저 RAMP 자체 테스트, 다음으로 실제 AD 수집을 실행합니다. 연속 모드는 프레임 검증 후 다음 프레임을 요청합니다. 중지 후 연결 해제·새로고침합니다.

## 파라미터와 결과

샘플링 10 MSPS, AXI 100 MHz, FIFO 1024×32비트입니다. N=256/512/1024/2048/4096, DA 주파수1 kHz~1 MHz, 웹 기본값100 kHz·4096점·delay=3·amp=8입니다. 관측 구간409.6 μs, FFT 간격2441.40625 Hz입니다. 2026-10-29 실측 캡처에는10.019/99.991/999.970 kHz와 CRC 통과가 나타납니다. 절대 정확도나 장시간 안정성을 입증한 것은 아닙니다. UART 전송 때문에 프레임 간 공백이 있으며 연속10 MSPS 스트림이 아닙니다.

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## 소스 및 상세 문서

완전한 SDK/IP 생성물과 bit/ELF는 Release에서 받으십시오. GitHub 자동 소스 ZIP과 다릅니다. Git은 UTF-8, 중국어 Windows용 전체 패키지의 수동 IDE 소스는 GBK입니다. 필요하면 python scripts/source_encoding.py --target gbk를 실행하되 Web/Python/Markdown은 UTF-8로 유지합니다. 상세 문서와 코드 주석은 주로 중국어입니다. Flash 기록은 필요 없습니다.

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
