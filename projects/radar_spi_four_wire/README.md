# Four-wire SPI radar front-end integration

[English](README.md) | [简体中文](README.zh-CN.md) · [Release / downloads](https://github.com/WuWingKit/z100-ad9280-capture/releases/tag/v0.7.0-spi-four-wire-20261119) · [Progress report](https://scnk2ajxowqf.feishu.cn/docx/KmGMdcwLgoUDa8x2wbrckl13nxb)

The Z100 PL is the SPI **master** and the external radar MCU is the SPI **slave**. Only CS, SCK, MOSI, MISO and GND connect the boards. SPI STATUS/TRIGGER/READ/ACK commands replace the old TX/RX-based SYNC and READY wiring.

## Verified on 2026-11-19

| Test | Observed result |
|---|---|
| MCU synthetic source over physical SPI | 3 frames / 141,408 bytes; 35,328 complex samples exactly match the reference pattern |
| Real-acquisition firmware | 10 frames / 471,360 bytes / 117,760 complex samples; sequences 0–9 |
| Chunk integrity | 7,370 READ chunks passed CRC16 |
| Buffer and frame checks | Header XOR, length, layout, sequence and ACK release passed |
| Independent cross-check | Last frame: JTAG DDR readback equals the UART BIN byte for byte |

Ten real-acquisition payloads differ and are neither all-zero nor the synthetic reference pattern. This validates the data path, **not calibrated distance, angle, SNR or target detection**. Actual MCU Flash contents were not read back; the operator selected the real-acquisition build.

## Current data route

MCU ADC/FIR → ping-pong RAM → SPI slave → PL SPI word master → AXI GPIO → PS DDR array → UART → PC.

**This diagnostic release does not use AXI DMA and does not run FFT in PL.** Historical DMA/FFT projects remain elsewhere in the repository. The first trigger captures A; the next trigger captures B, then publishes A. Capture and publication are sequential in the current firmware. A published frame must be read and ACKed before another trigger can reuse storage.

## Wiring

Disconnect old TX0/RX0/TX1/SYNC wiring. Independently power both boards and connect GND. Do not cross-connect 3V3 or 5V.

| Radar header | Z100 J37 | Direction |
|---|---|---|
| SCK / P3 | 31 / AC24 | FPGA → MCU |
| CS / P4 | 29 / AD24 | FPGA → MCU |
| MOSI / P6 | 32 / AG24 | FPGA → MCU |
| MISO / P5 | 27 / AA24 | MCU → FPGA |
| GND | 2 or 4 | Common ground |

The operator confirmed the data SPI header is independent of the boot Flash. The legacy internal HDL port name `mcu_ready` now carries MISO on AA24; `radar_sync`/AG25 is unused. Do not connect two masters: upgrade both sides together.

## Tools and dependencies

- Vivado and Xilinx SDK **2018.3**, XC7Z035FFG900-2; tested PL input clock 100 MHz.
- Keil ARM Compiler **5.06 update 7 build 960**; both MCU targets build with zero errors/warnings.
- Python **3.11** for evidence tools; PySerial **3.5** for serial access. COM7 and local tool installation paths are examples for the tested Windows workstation.
- Optional report plots: exact installed dependencies in `../../docs/progress_20261119/requirements-figures.txt`; Windows Microsoft YaHei/Consolas fonts are used by the plot generator.
- No NumPy dependency for normal serial capture. No network dependency on the board.

## Programming and use

1. Download matching release artifacts. Back up the existing W25X40CL Flash. Start with the MCU **selftest SPI_SLAVE W25X40CL 512KiB** image, then switch to real acquisition after verification.
2. SP32: Winbond W25X40CL, 4 Mbit; load buffer and Flash offset **0**, whole Flash erase/program/verify, leave Status Register unselected. Full images include Boot V3 header and FF padding. Root `hardware/*.bin` from a local build are **bare applications**, not programmer images.
3. Program `spi_only_master.bit` through JTAG, initialize PS using `ps7_init.tcl`, download `spi_only_ps.elf` to Cortex-A9 #0 and Resume. Use matching `spi_only_hw.hdf` for a new SDK platform. JTAG loads are volatile.
4. UART: **115200 / 8N1**. Send real CR, not literal backslash-r. `s` = STATUS (`73 0D`); `p` = first capture (`70 0D`); `c` = capture and read previous frame (`63 0D`). Wait for `END RADAR_RAW` before the next command. Reset **both** sessions before repeating `p`.
5. A successful frame prints `SPI FRAME DDR STORED bytes=47136 seq=N CRC16_PER_CHUNK=PASS`, then all bytes. This test is limited by low-speed SPI and HEX UART output and is not continuous high-rate streaming.

## Wire protocol

Mode 0, 16-bit MSB first, initial SCLK **100 kHz**. CS is asserted once **per 16-bit word**, unlike the retired whole-frame-CS protocol. Messages are framed by magic, length, request ID and CRC16/CCITT-FALSE (FFFF initial, polynomial 1021, high byte first).

Request words: `D5A0 opcode id offset_lo offset_hi length_bytes reserved crc16`.
Response: `5AD0 opcode id flags length_bytes offset_lo offset_hi payload... crc16`.

STATUS=1; TRIGGER=2; READ=3 (up to 64 bytes); ACK=4 (offset carries frame sequence). Flags: bit0 busy, bit1 previous frame available, bit2 fault, bit15 command error. Idle words are FFFF; the host clocks for a delayed response. Only one request is outstanding. Duplicate last trigger ID is idempotent; CRC failures stop the host, not silently retry acquisition.

## Build and evidence

- `radar_sync.xpr`: regenerate BD/IP outputs after moving the project; current simulation is `sim/tb_spi_master.v`. Archived slave simulations remain in older projects.
- MCU Keil project: `mcu_frontend/05_application/firmware_app/keil_project/firmware_app.uvprojx`, targets `radar_sync_selftest` and `radar_sync_mcu`.
- `scripts/build_mcu.ps1`, `scripts/build_live.py`, `scripts/build_diagnostic.tcl`, `scripts/test_spi_master.tcl`.
- The checked-in BSP supports the tested PS ELF. Download prebuilt BIT/ELF/HDF and full programmer images from the release; generated binaries and vendor IP caches are not source control inputs.
- `evidence/spi_only_real_20261119_154059/`: complete UART logs, ten BIN/CSV frames, summaries, final state and the last-frame JTAG comparison. `read_last_ddr.tcl` contains the symbol address of the **tested ELF only**; resolve it again after rebuilding.
- `test_spi_only_real.py` expects freshly restarted MCU/PS sessions and the matching board wiring. It triggers real RF acquisition. It is not a generic offline test.

[Chinese full implementation notes](README.zh-CN.md) · [Historical raw-CS investigation](../radar_spi_raw/README.md)
