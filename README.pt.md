# Z100 FPGA: aquisição, armazenamento DDR e análise FFT

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

Projeto didático da parte 3 do Grupo 3 de Processamento de Sinais, turma Xuexing 2024 da SWU. Antes de receber dados reais de radar, usamos um sinal conhecido em loopback DA→AD para validar aquisição, armazenamento e análise. Não é um radar de arranjo faseado completo.

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## Funcionalidades

O PL gera seno com CORDIC, captura amostras AD de 8 bits e agrupa quatro amostras em 32 bits. Caminho: FIFO assíncrona FWFT→AXI-Stream→DMA S2MM→PS HP0→DDR. O PS controla a aquisição, o cache e a UART0; a FFT é executada no PC. A curva laranja +90° é uma referência calculada por ajuste, não um segundo canal AD medido.

## Dependências e versões

- Compilação e depuração FPGA/PS: Vivado **2018.3**, Xilinx SDK **2018.3**, suporte Zynq-7000/IP e ferramentas ARM incluídas. O procedimento preparado é para Windows.
- Servidor local: biblioteca padrão do Python; runtime verificado **3.12.14**. Use Chrome/Edge desktop com Web Serial, HTTP localhost e drivers JTAG/PS USB-UART. Não foi fixada uma versão específica do navegador.
- Análise Python opcional: numpy>=1.26, matplotlib>=3.8, pyserial>=3.5; versões instaladas 2.5.4/3.11.2/3.5. Testes JavaScript usam Node.js **22.19.0**. A página por si só não exige NumPy, Node ou npm.

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## Iniciar sem a placa

Baixe o projeto completo ou o ZIP da interface no Release e execute start_dashboard.cmd. Na raiz do repositório, também pode usar o comando abaixo. “载入已验证的仿真数据” carrega uma simulação digital sem acessar a placa. Os controles da interface continuam em chinês.

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## Teste com Debug

1. Confira orientação J 37, tensão dos bancos IO e faixa analógica permitida. Conecte a saída DA à entrada AD. Conecte JTAG e a **USB-UART do PS**.
2. Abra o SDK 2018.3 separadamente e selecione sdk_workspace extraído. Importe z100_hw, adc_bsp e adc_uart se necessário.
3. Em Program FPGA, escolha hardware/z100_cordic.bit. Execute o ELF adc_uart no System Debugger e pressione Resume quando parar em main.
4. Feche outros programas seriais e conecte a página em 115200/8 N 1. Execute primeiro RAMP e depois aquisição real. O modo repetido pede um novo quadro somente após validar o anterior. Pare e desconecte antes de atualizar a página.

## Parâmetros e resultados

Amostragem 10 MSPS, AXI 100 MHz e FIFO 1024 × 32 bits. N=256/512/1024/2048/4096; DA 1 kHz–1 MHz. Padrões da página:100 kHz,4096 pontos,delay=3,amp=8. Janela 409.6 μs e espaçamento FFT 2441.40625 Hz. Capturas de tela de 2026-10-29 mostram 10.019/99.991/999.970 kHz com CRC válido. São quadros representativos, não uma certificação de exatidão absoluta ou estabilidade prolongada. Há intervalos de transmissão UART entre quadros; não é um fluxo contínuo de 10 MSPS.

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## Código e documentação

Os produtos completos de IP/SDK e bit/ELF estão no Release; o ZIP automático do GitHub contém apenas fontes. Git usa UTF-8; fontes IDE preparadas para Windows chinês usam GBK. Se necessário, execute python scripts/source_encoding.py --target gbk; mantenha Web/Python/Markdown em UTF-8. Documentos técnicos e comentários permanecem principalmente em chinês. Não é necessário gravar Flash.

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
