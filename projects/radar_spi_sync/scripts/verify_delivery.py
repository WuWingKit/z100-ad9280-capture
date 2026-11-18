"""Read-only delivery audit; writes a local evidence manifest, never programs HW."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parent.parent
required = [
    "radar_sync.xpr", "README.md", "docs/PROTOCOL.md", "docs/VALIDATION.md",
    "radar_sync.srcs/sources_1/bd/capture_bd/capture_bd.bd",
    "hardware/radar_sync.hdf", "hardware/radar_uart.elf",
    "hardware/implemented_utilization.rpt", "hardware/implemented_timing.rpt",
    "hardware/implemented_drc.rpt", "hardware/implemented_cdc.rpt", "hardware/radar_sync.bit",
    "hardware/radar_sync_mcu.axf", "hardware/radar_sync_selftest.axf",
    "mcu_frontend/05_application/firmware_app/keil_project/firmware_app.uvprojx",
]
checks = {f"exists:{name}": (root / name).is_file() for name in required}
for name in ("radar_sync_unit_result.txt", "radar_ddr_result.txt", "radar_full_wire_result.txt", "mcu_host_result.txt"):
    checks[f"pass:{name}"] = (root / "evidence" / name).read_text().startswith("PASS ")
checks["complete_project_synthesis"] = "RADAR_SYNC_SYNTHESIS_PASS_NO_BITSTREAM" in (root / "evidence/synthesis_console.txt").read_text(errors="replace")
checks["latest_hdf_software_built"] = "RADAR_LATEST_HDF_AND_ELF_BUILT" in (root / "evidence/sdk_latest_20261112.txt").read_text(errors="replace")
checks["implementation_bitstream"] = "RADAR_IMPLEMENTATION_AND_BITSTREAM_PASS" in (root / "evidence/implementation_20261112.txt").read_text(errors="replace")
timing=(root/'hardware/implemented_timing.rpt').read_text()
checks["routed_timing_met"] = 'All user specified timing constraints are met.' in timing
drc=(root/'hardware/implemented_drc.rpt').read_text()
summary=drc.split('Violations found:',1)[1].split('2. REPORT DETAILS',1)[0]
checks["no_drc_errors_or_warnings"] = not any(s in summary for s in ('| Error','| Critical Warning','| Warning'))
checks["ram_async_control_fixed"] = 'REQP-1839' not in drc
cdc_rows=[line for line in (root/'hardware/implemented_cdc.rpt').read_text().splitlines() if line.startswith(('Info ','Warning ','Critical '))]
checks['cdc_all_safe']=bool(cdc_rows) and all(line.split()[-3:]==['0','0','0'] for line in cdc_rows)
for name in ('radar_sync_mcu','radar_sync_selftest'):
    checks[f'keil_clean:{name}']='0 Error(s), 0 Warning(s)' in (root/'evidence'/f'build_{name}.txt').read_text(errors='replace')
checks['uart_parser_tests']='\nOK' in (root/'evidence/uart_export_test.txt').read_text(errors='replace')
latest_rtl = max(p.stat().st_mtime for p in (root / "rtl").glob("*.v"))
for name in ("radar_sync_unit_result.txt", "radar_ddr_result.txt", "radar_full_wire_result.txt"):
    checks[f"fresh:{name}"] = (root / "evidence" / name).stat().st_mtime >= latest_rtl
checks["fresh:elf"] = (root / "hardware/radar_uart.elf").stat().st_mtime >= (root / "software/main.c").stat().st_mtime
baseline = json.loads((root / "evidence/baseline.json").read_text(encoding="utf-8"))
for rel, expected in baseline["sha256"].items():
    actual = hashlib.sha256((Path(baseline["original_project"]) / rel).read_bytes()).hexdigest()
    checks[f"unchanged_original:{rel}"] = actual == expected
mcu_baseline=json.loads((root/'evidence/mcu_original_sha256.json').read_text(encoding='utf-8'))
checks['teacher_mcu_unchanged']=all(hashlib.sha256((Path(mcu_baseline['source'])/rel).read_bytes()).hexdigest()==h for rel,h in mcu_baseline['sha256'].items())
checks['fresh:bit']= (root/'hardware/radar_sync.bit').stat().st_mtime>=latest_rtl
checks['sdk_matches_exported_hdf']=hashlib.sha256((root/'hardware/radar_sync.hdf').read_bytes()).digest()==hashlib.sha256(Path('E:/FPGAWork/radar_sync_sdk_20261111/radar_hw/system.hdf').read_bytes()).digest()
source_hashes = {}
for name in required:
    source_hashes[name]=hashlib.sha256((root/name).read_bytes()).hexdigest()
for folder in ("rtl", "sim", "software", "mcu_sync", "scripts", "constraints", "docs", "hardware"):
    for p in sorted((root / folder).glob("*")):
        if p.is_file():
            source_hashes[p.relative_to(root).as_posix()] = hashlib.sha256(p.read_bytes()).hexdigest()
for p in (root/'mcu_frontend').rglob('*'):
    if p.is_file() and 'output' not in p.relative_to(root/'mcu_frontend').parts:
        source_hashes[p.relative_to(root).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
result = {
    "date": "2026-11-12", "all_pass": all(checks.values()), "checks": checks,
    "sha256": source_hashes,
    "hardware_tested": False,
    "hardware_boundary": "MCU binding and J37 pin constraints implemented; bitstream generated with routed timing met. No board download, RF measurement or Flash programming.",
    "ddr_test_boundary": "Full C-serialized frames injected after SPI decoder for PS7 DDR test; the same full frames tested separately through actual SPI wire receiver and AXIS with FIFO model.",
}
(root / "evidence/delivery_manifest.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
print("DELIVERY_AUDIT_PASS" if result["all_pass"] else "DELIVERY_AUDIT_FAILED")
for key, value in checks.items():
    if not value:
        print("FAIL", key)
raise SystemExit(0 if result["all_pass"] else 1)
