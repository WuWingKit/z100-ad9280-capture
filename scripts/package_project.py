# 本文件的执行方式、输入输出及与其它模块的关系见 docs/CODE_GUIDE.md。
"""Package this independent project; no source changes or hardware operations."""

from pathlib import Path
import hashlib, json, zipfile

root = Path(__file__).resolve().parent.parent
dest = root.parent / (root.name + "_review.zip")
# ===== 1. 排除可重建缓存、工具依赖和旧打包输出，避免ZIP递归包含自身 =====
excluded = {
    "_deps",
    "__pycache__",
    ".Xil",
    ".metadata",
    ".git",
    "_probe",
    "bd_start.runs",
    "bd_start.cache",
    "bd_start.sim",
    "simcheck.runs",
    "simcheck.cache",
    "simcheck.sim",
}
manifest = {}
# ===== 2. 收集源码/生成产品，并给每个文件记录SHA256 =====
with zipfile.ZipFile(dest, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for p in sorted(root.rglob("*")):
        rel = p.relative_to(root)
        if not p.is_file() or excluded.intersection(rel.parts):
            continue
        if p.suffix.lower() in {".wdb", ".jou", ".str", ".lck", ".zip"}:
            continue
        if p.name == "package_result.json":
            continue
        # 云文档上传回执属于本机工作记录，不作为工程源码/测试数据分发。
        if p.name == "upload_state.json" or p.name.startswith("response_"):
            continue
        if ".backup." in p.name:
            continue
        z.write(p, root.name + "/" + rel.as_posix())
        manifest[rel.as_posix()] = hashlib.sha256(p.read_bytes()).hexdigest()
    z.writestr(root.name + "/PACKAGE_SHA256.json", json.dumps(manifest, indent=2))
# ===== 3. 检查ZIP自身CRC；完整SHA256清单用于发布和接收方核对 =====
with zipfile.ZipFile(dest) as z:
    bad = z.testzip()
    if bad:
        raise RuntimeError("ZIP CRC failure: " + bad)
print(
    json.dumps(
        {
            "package": str(dest),
            "files": len(manifest),
            "bytes": dest.stat().st_size,
            "sha256": hashlib.sha256(dest.read_bytes()).hexdigest(),
        },
        indent=2,
    )
)
