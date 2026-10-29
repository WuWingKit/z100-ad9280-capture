# Z100 FPGA : acquisition, stockage DDR et analyse FFT

[![Release](https://img.shields.io/github/v/release/WuWingKit/z100-ad9280-capture)](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029)
[![Source checks](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml/badge.svg?branch=main)](https://github.com/WuWingKit/z100-ad9280-capture/actions/workflows/source-checks.yml)
![Vivado](https://img.shields.io/badge/Vivado-2018.3-904cab)
![SDK](https://img.shields.io/badge/Xilinx_SDK-2018.3-904cab)
![Device](https://img.shields.io/badge/Zynq-XC7Z035FFG900--2-2258ce)
![Python](https://img.shields.io/badge/Python-tested_3.12.14-3776ab)
![Node](https://img.shields.io/badge/Node.js-tested_22.19.0-43853d)

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)

Projet pédagogique de la troisième partie du groupe 3 de traitement du signal, classe Xuexing 2024 de SWU. Avant de recevoir des données radar réelles, une boucle analogique DA→AD avec un signal connu permet de valider acquisition, stockage et analyse. Il ne s’agit pas d’un radar à réseau phasé complet.

[Release v0.3.0-20261029](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.3.0-20261029) · [English reference](README.md)

## Fonctions réalisées

Le PL génère une sinusoïde par CORDIC, acquiert les échantillons AD 8 bits et regroupe quatre échantillons en 32 bits. Chaîne : FIFO asynchrone FWFT→AXI-Stream→DMA S2MM→PS HP0→DDR. Le PS assure commande, gestion du cache et UART0 ; la FFT est exécutée sur le PC. La courbe orange +90° est une référence calculée par ajustement, pas une seconde voie AD mesurée.

## Dépendances et versions

- Compilation et débogage FPGA/PS : Vivado **2018.3**, Xilinx SDK **2018.3**, prise en charge Zynq-7000/IP et chaîne ARM fournie. Le parcours préparé cible Windows.
- Serveur local : bibliothèque standard Python ; environnement vérifié **3.12.14**. Chrome/Edge de bureau avec Web Serial, HTTP localhost et pilotes JTAG/PS USB-UART. Aucune version précise du navigateur n’est figée.
- Analyse Python facultative : numpy>=1.26, matplotlib>=3.8, pyserial>=3.5 ; versions installées 2.5.4/3.11.2/3.5. Node.js **22.19.0** a été utilisé pour les tests JavaScript. La page seule ne demande ni NumPy, ni Node, ni npm.

**Z100 / XC7Z035FFG900-2 · J37 / AD9280 · 8-bit DAC**

## Démarrage sans carte

Téléchargez le projet complet ou le ZIP du tableau de bord dans Release, puis lancez start_dashboard.cmd. Depuis la racine du dépôt, la commande ci-dessous convient aussi. «载入已验证的仿真数据» charge une simulation numérique sans accéder à la carte. Les commandes de l’interface restent en chinois.

```powershell
python web/serve.py --open
```

http://127.0.0.1:8765/

## Essai avec Debug

1. Vérifiez orientation J 37, tensions IO et plage analogique autorisée. Reliez la sortie DA à l’entrée AD. Branchez JTAG et **USB-UART côté PS**.
2. Lancez SDK 2018.3 séparément et choisissez le dossier sdk_workspace extrait. Importez z100_hw, adc_bsp et adc_uart au besoin.
3. Dans Program FPGA, sélectionnez hardware/z100_cordic.bit. Lancez ELF adc_uart avec System Debugger ; à l’arrêt dans main, cliquez sur Resume.
4. Fermez les autres applications série et connectez la page en 115200/8 N 1. Testez RAMP avant l’acquisition réelle. Le mode répété attend la validation complète d’une trame avant la suivante. Arrêtez et déconnectez avant de recharger la page.

## Paramètres et résultats

Échantillonnage 10 MSPS, AXI 100 MHz, FIFO 1024 × 32 bits. N=256/512/1024/2048/4096 ; DA 1 kHz–1 MHz. Réglages web :100 kHz,4096 points,delay=3,amp=8. Fenêtre 409.6 μs, pas FFT 2441.40625 Hz. Les captures du 2026-10-29 montrent 10.019/99.991/999.970 kHz avec CRC valide. Ce sont des trames représentatives, pas une qualification de précision absolue ou de stabilité prolongée. La transmission UART crée des intervalles entre trames ; ce n’est pas un flux continu 10 MSPS.

```text
t 100000 4096 3 8
m 100000 4096 3 8
s
q
r
h
```

## Sources et documentation

Les produits IP/SDK complets et les fichiers bit/ELF sont dans Release ; le ZIP automatique GitHub n’est pas le projet précompilé complet. Git utilise UTF-8 ; les sources IDE du paquet Windows chinois utilisent GBK. Si nécessaire, lancez python scripts/source_encoding.py --target gbk ; conservez Web/Python/Markdown en UTF-8. Documentation détaillée et commentaires restent majoritairement chinois. Aucun flashage n’est nécessaire.

[BOARD_TEST](docs/BOARD_TEST.md) · [CODE_GUIDE](docs/CODE_GUIDE.md) · [IMPLEMENTATION](docs/IMPLEMENTATION.md) · [2026-10-29](docs/weekly_20261029/README.md)

```powershell
python -m pip install -r pc/requirements.txt
python pc/test_analysis.py
node web/test_signal.cjs
node web/test_live.cjs
```

---

[English](README.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md) | [Русский](README.ru.md) | [Português](README.pt.md) | [Español](README.es.md) | [Français](README.fr.md)
