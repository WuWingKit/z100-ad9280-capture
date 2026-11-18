> f1_0 说明：本文只描述 Boot V3/生产镜像打包。运行时的 SPI Flash 读写、槽位/参数持久化已从固件中删除；应用启动后 P4 保持高电平，SPI0 用于原始 ADC 数据输出。

# ADT6101P Flash image and download guide

This project follows the ANDAR Boot V3 format used by the vendor
`adt6101P_resolution_test_v3.5.0` project.

## Memory flow

The ADT6101P has a factory ROM bootloader. Application code is stored in an
external P25Q80H-compatible SPI NOR, then copied to RAMI and executed:

- factory ROM bootloader: CPU address `0x00000000..0x00007FFF`;
- application execution region (RAMI): `0x00008000..0x00017FFF`, 64 KiB;
- application vector table: `0x00008000`;
- external SPI Flash packaged image: starts at physical Flash address `0`.

## Files and their uses

- `output/firmware_f1_0.bin`: raw application payload linked for execution at
  `0x8000`; useful for debugging and as input to packaging.
- `output/firmware_f1_0.hex`: Keil address-bearing application image. Use only
  with the ANDAR Keil Flash algorithm configured for the application mapping.
- `output/firmware_f1_0_andar_flash.bin`: production/OTA image for Boot V3.
  This is the file to write to external SPI Flash address `0` with a universal
  programmer such as the user's SOFI/硕飞 programmer.

## Boot V3 header

The production image starts with 16 bytes:

| Offset | Meaning |
|---:|---|
| `0..1` | raw payload length, little-endian |
| `2..3` | duplicated payload length |
| `4` | valid application marker `0x5A` |
| `5..7` | payload offset, little-endian 24-bit (`0x10`) |
| `8` | SPI clock divider (`0x02`) |
| `9..12` | standard CRC32 of the raw payload, big-endian |
| `13..15` | reserved, zero |

Keil runs `external_scripts/package_firmware.cmd` automatically after a
successful build. It creates and validates the Boot V3 image automatically. The
same script can also be run manually to repackage an existing raw binary. The
standalone header-only entry point is `external_scripts/make_andar_flash_image.cmd`.

## Keil/J-Link direct download

Copy `06_third_party/andar_flash/ANDAR_P25Q80H_8M_FLASH.FLM` to
`Keil_v5/ARM/Flash`, select J-Link SW at no more than 5 MHz, enable Program,
Verify, Reset and Run, and add the ANDAR programming algorithm. The vendor
procedure uses Erase Full Chip; this erases the existing contents of the external
SPI NOR, including any previously packaged application/OTA image.

Do not use a generic MCU-internal-Flash algorithm. The ADT6101P program image is
stored in the external SPI NOR.
