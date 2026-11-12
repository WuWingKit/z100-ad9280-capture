"""Verify recorded evidence matches the delivered bit/ELF and original copy."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
baseline=json.loads((ROOT/'evidence/external_baseline.json').read_text(encoding='utf-8-sig'))
original=Path(baseline['source'])
checks={f'original_unchanged:{f}':sha(original/f)==h for f,h in baseline['sha256'].items()}
for name in ['spi_slave_rx16.v','radar_frame_capture.v','radar_fifo_to_axis.v']:
    checks['receiver_copy_unchanged:'+name]=sha(ROOT/'rtl'/name)==sha(original/'rtl'/name)
for name in ['unit_result.txt','full_wire_result.txt','radar_ddr_result.txt']:
    checks[name]=(ROOT/'evidence'/name).read_text().startswith('PASS ')
checks['hardware_build']='\nFPGA_ONLY_DEBUG_HARDWARE_PASS\n' in (ROOT/'evidence/hardware_build.txt').read_text(errors='replace')
checks['software_build']='FPGA_ONLY_DEBUG_SOFTWARE_PASS' in (ROOT/'evidence/software_build.txt').read_text(errors='replace')
board=json.loads((ROOT/'evidence/board_20261112_170218/results.json').read_text())
checks['actual_board_tests']=board['pass'] and all(t['pass'] for t in board['tests'])
checks['actual_board_ILA']=json.loads((ROOT/'evidence/board_ila_result.json').read_text())['pass']
for name in ['radar_debug.bit','radar_debug_uart.elf','radar_debug.ltx']:
    checks['tested_hash:'+name]=sha(ROOT/'hardware'/name)==board[name+'_sha256']
checks['no_flash_write']=board['flash_written'] is False
checks['timing_pass']='All user specified timing constraints are met.' in (ROOT/'hardware/implemented_timing.rpt').read_text()
files={name:{'bytes':(ROOT/name).stat().st_size,'sha256':sha(ROOT/name)} for name in [
    'radar_debug.xpr','rtl/radar_frontend_sim.v','software/main.c',
    'hardware/radar_debug.bit','hardware/radar_debug.ltx','hardware/radar_debug.hdf',
    'hardware/radar_debug_uart.elf','evidence/board_spi_ila.csv',
    'evidence/board_20261112_170218/frame_seq0021.bin']}
manifest={'date':'2026-11-12','checks':checks,'files':files,'pass':all(checks.values())}
(ROOT/'evidence/delivery_manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
assert manifest['pass'],checks
print('DELIVERY_PASS checks='+str(len(checks)))
