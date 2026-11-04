"""Portable source/evidence checks; never accesses a board or serial port."""
from pathlib import Path
import json
import zlib

root = Path(__file__).resolve().parents[1]
bd = json.loads((root / 'spi_capture.srcs/sources_1/bd/capture_bd/capture_bd.bd').read_text())['design']
assert 'spi_capture_core_0' in bd['components']
assert not any('cordic' in name.lower() for name in bd['components'])
assert not {'adc_data_0', 'ad_otr_0', 'dac_data', 'adc_clk_0'}.intersection(bd['ports'])
ps = bd['components']['processing_system7_0']['parameters']
assert ps['PCW_SPI0_PERIPHERAL_ENABLE']['value'] == '1'
assert ps['PCW_SPI0_SPI0_IO']['value'] == 'EMIO'
analysis = json.loads((root / 'evidence/board_test_20261104/serial_log_analysis.json').read_text())
assert len(analysis['frames']) == 8
for frame in analysis['frames']:
    data = bytes.fromhex(frame['payload_hex'])
    assert len(data) == 8
    assert (sum(data), min(data), max(data)) == (frame['sum'], frame['min'], frame['max'])
    assert f'{zlib.crc32(data) & 0xffffffff:08X}' == frame['crc32']
    assert all(frame['checks'].values())
for name in ['spi_burst8_result.txt', 'spi_capture_result.txt', 'ddr_result.txt']:
    assert (root / 'evidence' / name).read_text().startswith('PASS')
print('SPI_REPOSITORY_PASS: active BD / PS EMIO / 8 logged frames / statistics / CRC / saved simulation results')
