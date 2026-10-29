# Z100 FPGA: adquisición, almacenamiento DDR y análisis FFT

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

Proyecto educativo de la parte 3 del Grupo 3 de Procesamiento de Señales, clase Xuexing 2024 de SWU. Antes de disponer de datos reales de radar, se utiliza una señal conocida en bucle DA→AD para validar adquisición, almacenamiento y análisis. No es un radar de matriz en fase completo.

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## Funciones implementadas

El PL genera una senoide con CORDIC, captura muestras AD de 8 bits y empaqueta cuatro muestras en 32 bits. Ruta: FIFO asíncrona FWFT→AXI-Stream→DMA S 2 MM→PS HP 0→DDR. El PS gestiona control, caché y UART 0; la FFT se calcula en el PC. La curva naranja +90° es una referencia obtenida mediante ajuste, no otro canal AD medido.

## Dependencias y versiones

- Compilación y depuración FPGA/PS: Vivado **2018.3**, Xilinx SDK **2018.3**, soporte Zynq-7000/IP y herramientas ARM incluidas. El procedimiento preparado está orientado a Windows.
- Servidor local: biblioteca estándar de Python; entorno verificado **3.12.14**. Chrome/Edge de escritorio con Web Serial, HTTP localhost y controladores JTAG/PS USB-UART. No se fija una versión concreta del navegador.
- Análisis Python opcional: numpy>=1.26, matplotlib>=3.8, pyserial>=3.5; versiones instaladas 2.5.4/3.11.2/3.5. Para pruebas JavaScript se utilizó Node.js **22.19.0**. La página no necesita NumPy, Node ni npm.

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## Inicio sin placa

Descargue el proyecto completo o el ZIP de la interfaz desde Release y ejecute start_dashboard.cmd. Desde la raíz del repositorio también puede usar el comando siguiente. “载入已验证的仿真数据” carga una simulación digital sin acceder a la placa. Los controles de la interfaz siguen en chino.

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## Prueba con Debug

1. Compruebe orientación J 37, tensión IO y rango analógico permitido. Conecte la salida DA a la entrada AD. Conecte JTAG y la **USB-UART del PS**.
2. Abra SDK 2018.3 por separado y seleccione sdk_workspace extraído. Importe z 100_hw, adc_bsp y adc_uart si hace falta.
3. En Program FPGA seleccione hardware/z 100_cordic.bit. Ejecute el ELF adc_uart con System Debugger; si se detiene en main, pulse Resume.
4. Cierre otros programas serie y conecte la página a 115200/8 N 1. Primero ejecute RAMP, después adquisición real. El modo repetido solicita el siguiente cuadro solo tras validar el anterior. Detenga y desconecte antes de actualizar.

## Parámetros y resultados

Muestreo 10 MSPS, AXI 100 MHz, FIFO 1024 × 32 bits. N=256/512/1024/2048/4096; DA 1 kHz–1 MHz. Valores web:100 kHz,4096 puntos,delay=3,amp=8. Ventana 409.6 μs y separación FFT 2441.40625 Hz. Las capturas de 2026-10-29 muestran 10.019/99.991/999.970 kHz y CRC correcto. Son cuadros representativos, no una certificación de exactitud absoluta ni estabilidad prolongada. Hay pausas UART entre cuadros; no existe transmisión continua de 10 MSPS.

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## Código y documentación

Los productos IP/SDK completos y bit/ELF están en Release; el ZIP automático de GitHub no es el paquete completo. Git usa UTF-8; los fuentes IDE preparados para Windows chino usan GBK. Si hace falta ejecute python scripts/source_encoding.py --target gbk, manteniendo Web/Python/Markdown en UTF-8. La documentación técnica y los comentarios están principalmente en chino. No hace falta programar Flash.

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
