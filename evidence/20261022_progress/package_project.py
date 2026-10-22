"""Build a clean public source snapshot and a complete handover ZIP.
Does not alter the active Vivado/SDK hardware design or access the board.
"""
from pathlib import Path
from datetime import datetime, timezone, timedelta
import hashlib,json,shutil,zipfile,re

project=Path(__file__).resolve().parents[2]
repo=project.parent/'z100-ad9280-capture'
repo.mkdir(exist_ok=True)
evidence=project/'evidence/20261022_progress'

def copy(rel):
    source=project/rel
    target=repo/rel
    target.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(source,target)

for rel in ['bd_start.xpr','rtl/adc_board_control.v','rtl/z100_adc_top.v',
            'bd_start.srcs/sources_1/new/adc_capture_pack.v',
            'bd_start.srcs/sources_1/new/fifo_to_axis.v',
            'bd_start.srcs/sources_1/bd/capture_bd/capture_bd.bd',
            'bd_start.srcs/sources_1/bd/capture_bd/hdl/capture_bd_wrapper.v',
            'bd_start.srcs/sources_1/ip/adc_clk_wiz/adc_clk_wiz.xci',
            'bd_start.srcs/sim_1/new/tb_capture_bd.v',
            'constraints/z100_adc.xdc','sim/tb_z100_hw.v','software/main.c',
            'POWER_CHECK.md','README_Z100.md','board_reference/apply_board_ps.tcl',
            'board_reference/vendor_ps_parameters.json']:
    copy(rel)
for p in (project/'scripts').glob('*'):
    if p.is_file(): copy(p.relative_to(project))
for name in ['verification_summary.json','offline_preflight.json','timing_routed.rpt',
             'drc_routed.rpt','cdc_routed.rpt','cdc_detailed.rpt','io_routed.rpt','REVIEW_NOTES.md',
             'hw_sim.log','bitstream.log']:
    copy('hardware/'+name)
for p in evidence.iterdir():
    if p.is_file() and p.suffix in {'.png','.svg','.txt','.csv','.json','.log','.py','.tcl','.xml'}:
        copy(p.relative_to(project))

# Preserve the historical report, but add a current report instead of replacing history.
original_checks={}
for path,digest in json.loads((project/'original_sha256.json').read_text()).items():
    original_checks[Path(path).name]=hashlib.sha256(Path(path).read_bytes()).hexdigest()==digest
current={'date':datetime.now(timezone(timedelta(hours=8))).isoformat(),
         'user_board_evidence':'t screenshot: SELFTEST PASS; c reported frame: sum12503 min125 max126',
         'agent_hardware_actions':'NONE',
         'local_reanalysis':'PASS; four parser negative tests rejected; seven prior artifact hashes match; backup CRC pass',
         'simulation_rerun':'BLOCKED: XSim cannot allocate 536870924 bytes, including a no-wave retry; no new PASS',
         'historical_simulation':'PASS frames=2 samples=200 DDR_bytes=200 checks=2676',
         'source_recreate':'PASS: Vivado2018.3 source project created, BD validated and output products generated; not a new synthesis or simulation pass',
         'original_snapshot_comparison':original_checks,
         'note':'Historical offline_preflight.json is dated evidence, not a claim that all original files still match. XPR has changed; original RTL and BD still match.'}
for directory in [evidence,repo/'evidence/20261022_progress']:
    (directory/'progress_status.json').write_text(json.dumps(current,ensure_ascii=False,indent=2),encoding='utf-8')

# Full package includes generated source IP and SDK projects, but no workspace caches.
chosen=[]
for folder in ['bd_start.srcs','rtl','constraints','sim','software','scripts','sdk_workspace','evidence']:
    for p in (project/folder).rglob('*'):
        if not p.is_file(): continue
        rel=p.relative_to(project)
        if any(s in {'.metadata','.Xil','__pycache__','.git'} for s in rel.parts): continue
        if p.suffix.lower() in {'.wdb','.o','.d','.obj','.jou','.pyc','.pdf'}: continue
        if p.name.endswith('.str'): continue
        chosen.append(p)
for name in ['z100_adc.bit','z100_adc.hdf','system.hdf','verification_summary.json',
             'offline_preflight.json','timing_routed.rpt','drc_routed.rpt','cdc_routed.rpt',
             'cdc_detailed.rpt','io_routed.rpt','REVIEW_NOTES.md','hw_sim.log','bitstream.log']:
    chosen.append(project/'hardware'/name)
chosen.extend(project/n for n in ['bd_start.xpr','POWER_CHECK.md','README_Z100.md'])
chosen.extend(project/'board_reference'/n for n in ['apply_board_ps.tcl','vendor_ps_parameters.json'])
chosen=sorted(set(chosen))
release=repo/'release_assets';release.mkdir(exist_ok=True)
archive=release/'z100-ad9280-capture-20261022.zip'
prefix='z100-ad9280-capture-20261022'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in chosen: z.write(p,str(Path(prefix)/p.relative_to(project)))
    z.write(repo/'README.md',prefix+'/README.md')
    z.write(repo/'scripts/recreate_project.tcl',prefix+'/scripts/recreate_project.tcl')
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for rel in ['bd_start.xpr','hardware/z100_adc.bit','sdk_workspace/adc_uart/Debug/adc_uart.elf']:
        assert f'{prefix}/{rel}' in z.namelist()
    for rel in ['hardware/z100_adc.bit','sdk_workspace/adc_uart/Debug/adc_uart.elf']:
        assert hashlib.sha256(z.read(f'{prefix}/{rel}')).digest()==hashlib.sha256((project/rel).read_bytes()).digest()
manifest={'archive':archive.name,'bytes':archive.stat().st_size,
          'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'zip_crc':'PASS',
          'files':len(chosen)+2,'scope':'Current hardware-test project with generated source IP, SDK projects, bit/HDF/ELF, source and evidence; caches and large waveforms excluded'}
(release/'package_manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
(release/'SHA256SUMS.txt').write_text(manifest['sha256']+'  '+archive.name+'\n',encoding='ascii')
print(json.dumps(manifest,ensure_ascii=False,indent=2))
