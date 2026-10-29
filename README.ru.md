# Z100 FPGA: сбор данных, хранение в DDR и анализ FFT

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

Учебный стенд третьей части работы группы 3 по обработке сигналов, SWU, класс Xuexing набора 2024 года. Пока реальные радиолокационные данные недоступны, тракт проверяется известным сигналом в аналоговой петле ЦАП→АЦП. Это не законченная система радара с фазированной решёткой.

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## Что реализовано

PL формирует синус с помощью CORDIC, принимает 8-битные отсчёты и упаковывает четыре отсчёта в 32-битное слово. Тракт: асинхронный FWFT FIFO→AXI-Stream→DMA S2MM→PS HP0→DDR. PS управляет захватом, кэшем и UART0; FFT выполняется на ПК. Оранжевая кривая +90° — расчётная опорная кривая по аппроксимации, а не второй измеренный канал АЦП.

## Зависимости и версии

- Сборка и отладка FPGA/PS: Vivado **2018.3**, Xilinx SDK **2018.3**, поддержка Zynq-7000/IP и комплектный ARM toolchain. Подготовленные инструкции рассчитаны на Windows.
- Локальный веб-сервер: стандартная библиотека Python; проверенная среда **3.12.14**. Нужны настольный Chrome/Edge с Web Serial, localhost HTTP и драйверы JTAG/PS USB-UART. Конкретная версия браузера не закреплена.
- Необязательный анализ Python: numpy>=1.26, matplotlib>=3.8, pyserial>=3.5; установленные версии 2.5.4/3.11.2/3.5. Для тестов JavaScript проверен Node.js **22.19.0**. Самой странице не нужны NumPy, Node или npm.

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## Запуск без платы

Скачайте полный проект или архив веб-интерфейса из Release и запустите start_dashboard.cmd. Из корня репозитория можно выполнить команду ниже. Кнопка «载入已验证的仿真数据» загружает цифровую симуляцию без обращения к плате. Подписи интерфейса пока на китайском.

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## Отладка на плате

1. Проверьте ориентацию J 37, напряжение IO и допустимый аналоговый вход. Соедините выход ЦАП со входом АЦП. Подключите JTAG и **USB-UART стороны PS**.
2. Отдельно запустите SDK 2018.3 и выберите распакованную папку sdk_workspace. При необходимости импортируйте z100_hw, adc_bsp и adc_uart.
3. В Program FPGA выберите hardware/z100_cordic.bit. Запустите ELF adc_uart через System Debugger; после остановки в main нажмите Resume.
4. Закройте другие программы последовательного порта. Подключитесь со страницы на 115200/8 N 1. Сначала RAMP, затем реальный захват AD. Повторный захват начинается только после проверки предыдущего кадра. Перед обновлением страницы остановите захват и отключите порт.

## Параметры и результаты

Частота дискретизации 10 MSPS, AXI 100 MHz, FIFO 1024 × 32 бита. N=256/512/1024/2048/4096; выход 1 kHz–1 MHz. Значения страницы по умолчанию:100 kHz,4096 отсчётов,delay=3,amp=8. Окно 409.6 μs, шаг FFT 2441.40625 Hz. Скриншоты от 2026-10-29 показывают 10.019/99.991/999.970 kHz и успешный CRC. Это отдельные кадры, а не аттестация абсолютной точности или длительной стабильности. Между кадрами есть паузы передачи UART; непрерывного потока 10 MSPS нет.

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## Исходники и документация

Полные продукты IP/SDK и bit/ELF находятся в Release, а не в автоматически созданном архиве исходников GitHub. Git хранит UTF-8; подготовленные IDE-исходники для китайской Windows используют GBK. При необходимости выполните python scripts/source_encoding.py --target gbk; Web/Python/Markdown оставьте UTF-8. Подробная документация и комментарии преимущественно китайские. Запись Flash не требуется.

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
