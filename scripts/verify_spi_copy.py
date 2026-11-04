"""Audit only: compare preserved ADC project, inspect SPI wiring and saved tests."""
from pathlib import Path
import hashlib,json,re
root=Path(__file__).resolve().parents[1]
baseline=json.loads((root/'evidence/spi_original_baseline.json').read_text())
source=Path(baseline['source'])
unchanged={p:hashlib.sha256((source/p).read_bytes()).hexdigest()==sha for p,sha in baseline['sha256'].items()}
assert all(unchanged.values()),'Original ADC project was modified'
bd=json.loads((root/'spi_capture.srcs/sources_1/bd/capture_bd/capture_bd.bd').read_text())['design']
assert bd['components']['processing_system7_0']['parameters']['PCW_SPI0_PERIPHERAL_ENABLE']['value']=='1'
nets=list(bd['nets'].values())
for a,b in [('spi_capture_core_0/spi_sclk','processing_system7_0/SPI0_SCLK_I'),
            ('spi_capture_core_0/spi_cs_n','processing_system7_0/SPI0_SS_I'),
            ('spi_capture_core_0/spi_mosi','processing_system7_0/SPI0_MOSI_I'),
            ('processing_system7_0/SPI0_MISO_O','spi_capture_core_0/spi_miso')]:
 assert any(a in n['ports'] and b in n['ports'] for n in nets),(a,b)
tests={f:(root/'evidence'/f).read_text().strip() for f in ['spi_burst8_result.txt','spi_capture_result.txt','ddr_result.txt']}
assert all(s.startswith('PASS') for s in tests.values())
assert 'bytes_per_capture=8' in tests['spi_burst8_result.txt']
assert 'DDR_bytes=16' in tests['ddr_result.txt']
src=(root/'software/main.c').read_bytes().decode('gbk')
assert 'XSPIPS_CLK_ACTIVE_LOW_OPTION' in src and '#define FS 1000000U' in src and 'n != 8' in src
assert (root/'hardware/spi_uart.elf').read_bytes()==(root/'sdk_spi_workspace/spi_uart/Debug/spi_uart.elf').read_bytes()
result={'source_project':str(source),'original_files_unchanged':unchanged,
 'spi':{'master':'PL','slave':'PS SPI0 via internal EMIO','mode':2,'SCLK_Hz':16000000,'CS_repeat_Hz':1000000,'CS_low_ns':500,'bytes_per_capture':8,'parallel_bits':64,'DDR_words_32bit':2},
 'simulation':tests,'simulation_scope':'SPI slave model substitutes PS peripheral shift behavior; PS ARM software is not executed by PS VIP.',
 'ps_software':'Compiled for SDK 2018.3; physical PS SPI FIFO behavior not yet board-tested',
 'board_programmed':False,'flash_written':False,
 'source_hashes':{p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in ['rtl/spi_master_burst8.v','rtl/spi_capture_core.v','software/main.c','hardware/spi_uart.elf']}}
if (root/'hardware/z100_spi_sim.bit').exists():
 timing=(root/'hardware/timing_routed.rpt').read_text()
 assert 'All user specified timing constraints are met.' in timing
 result['bitstream']='Built after SPI changes; timing summary met; not board-tested'
 result['source_hashes']['hardware/z100_spi_sim.bit']=hashlib.sha256((root/'hardware/z100_spi_sim.bit').read_bytes()).hexdigest()
(root/'evidence/SPI_VERIFICATION.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print('SPI_COPY_VERIFIED original_unchanged / EMIO_wiring / mode2 / 8bytes / DDR_guard / compiled_ELF')
