"""Package this independent project; no source changes or hardware operations."""
from pathlib import Path
import hashlib,json,zipfile
root=Path(__file__).resolve().parent.parent
dest=root.parent/(root.name+'_review.zip')
excluded={'_deps','__pycache__','.Xil','.metadata','.git','_probe',
          'bd_start.runs','bd_start.cache','bd_start.sim',
          'simcheck.runs','simcheck.cache','simcheck.sim'}
manifest={}
with zipfile.ZipFile(dest,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if not p.is_file() or excluded.intersection(rel.parts):continue
        if p.suffix.lower() in {'.wdb','.jou','.str','.lck','.zip'}:continue
        if p.name=='package_result.json':continue
        if '.backup.' in p.name:continue
        z.write(p,root.name+'/'+rel.as_posix())
        manifest[rel.as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
    z.writestr(root.name+'/PACKAGE_SHA256.json',json.dumps(manifest,indent=2))
with zipfile.ZipFile(dest) as z:
    bad=z.testzip()
    if bad:raise RuntimeError('ZIP CRC failure: '+bad)
print(json.dumps({'package':str(dest),'files':len(manifest),'bytes':dest.stat().st_size,
                  'sha256':hashlib.sha256(dest.read_bytes()).hexdigest()},indent=2))
