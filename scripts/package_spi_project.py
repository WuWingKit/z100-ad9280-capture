from pathlib import Path
import zipfile,hashlib,json
root=Path(__file__).resolve().parents[1]
out=root.parent/(root.name+'.zip')
excluded={'.git','.metadata','.Xil','__pycache__','_deps','_probe','baseline_artifacts_adc','archive','spi_unit','spi_burst','sim_project'}
manifest={}
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(root.rglob('*')):
  rel=p.relative_to(root)
  if not p.is_file() or excluded.intersection(rel.parts):continue
  if any(x.endswith(('.runs','.cache','.sim')) for x in rel.parts):continue
  if p.suffix.lower() in {'.zip','.wdb','.jou','.str','.lck'}:continue
  if p.name=='upload_state.json' or p.name.startswith('response_'):continue
  manifest[rel.as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
  z.write(p,root.name+'/'+rel.as_posix())
 z.writestr(root.name+'/SPI_PACKAGE_SHA256.json',json.dumps(manifest,indent=2))
with zipfile.ZipFile(out) as z:assert z.testzip() is None
print(json.dumps({'zip':str(out),'files':len(manifest),'bytes':out.stat().st_size,'sha256':hashlib.sha256(out.read_bytes()).hexdigest()},indent=2))
