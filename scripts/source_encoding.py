# -*- coding: utf-8 -*-
"""Convert handwritten IDE sources only; Python/web/Markdown remain UTF-8.

Windows Vivado 2018.3 compatibility: --target gbk
GitHub/web source publication:      --target utf8
"""
from pathlib import Path
import argparse
import hashlib
import json
import zipfile
from datetime import datetime


# ===== 1. 严格识别文本；不能用替换字符掩盖原文件已损坏 =====
def decode_source(data):
    """Strictly decode original text; never replace undecodable characters."""
    try:
        return data.decode("utf-8-sig"), "utf-8"
    except UnicodeDecodeError:
        return data.decode("gbk"), "gbk"


# ===== 2. 只处理旧版IDE手写源码，不转换网页/Python/Markdown/厂商IP =====
def files_for_ide(root):
    # Only custom sources. Do not rewrite vendor IP, generated BD products,
    # Python/JS/HTML or evidence files, whose encodings follow their own tools.
    for directory, pattern in [
        ("rtl", "*.v"),
        ("sim", "*.v"),
        ("software", "*.c"),
        ("scripts", "*.tcl"),
        ("constraints", "*.xdc"),
        ("web", "*.cmd"),
        ("sdk_workspace/adc_uart/src", "*.c"),
    ]:
        yield from sorted((root / directory).glob(pattern))


# ===== 3. 先预检查整批，再备份、转换、设置SDK属性并保存记录 =====
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--target", choices=["gbk", "utf8"], required=True)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    root = args.root.resolve()
    target = "utf-8" if args.target == "utf8" else "gbk"
    changes = []
    # 严格往返转换保证Unicode文字完全一致；遇到GBK无法表示的字立即停止。
    # Preflight every file before changing any: strict encode + round trip.
    for path in files_for_ide(root):
        before = path.read_bytes()
        text, origin = decode_source(before)
        after = text.encode(target)
        assert after.decode(target) == text, path
        if after != before:
            changes.append((path, before, after, origin, text))

    backup = None
    if changes:
        backup = root.parent / (
            root.name + "_encoding_backup_" + datetime.now().strftime("%Y%m%d_%H%M%S_%f") + ".zip"
        )
        with zipfile.ZipFile(backup, "w", zipfile.ZIP_DEFLATED) as archive:
            for path, before, *_ in changes:
                archive.writestr(path.relative_to(root).as_posix(), before)
        for path, _, after, _, _ in changes:
            path.write_bytes(after)

    # Eclipse/SDK stores an explicit per-file charset independently of Windows.
    # SDK另有文件级编码设置，单改字节而不改属性仍可能显示乱码。
    sdk_project = root / "sdk_workspace/adc_uart"
    if sdk_project.exists():
        settings = sdk_project / ".settings"
        settings.mkdir(exist_ok=True)
        prefs = settings / "org.eclipse.core.resources.prefs"
        lines = prefs.read_text(encoding="ascii").splitlines() if prefs.exists() else []
        lines = [line for line in lines if not line.startswith("encoding/src/main.c=")]
        if "eclipse.preferences.version=1" not in lines:
            lines.insert(0, "eclipse.preferences.version=1")
        lines.append("encoding/src/main.c=" + ("GBK" if target == "gbk" else "UTF-8"))
        prefs.write_text("\n".join(lines) + "\n", encoding="ascii")

    report = {
        "target_encoding": target,
        "backup": str(backup) if backup else None,
        "changed_files": [
            {
                "path": path.relative_to(root).as_posix(),
                "from": origin,
                "to": target,
                "unicode_content_unchanged": True,
                "unicode_utf8_sha256": hashlib.sha256(text.encode("utf-8")).hexdigest(),
            }
            for path, _, _, origin, text in changes
        ],
        "web_python_markdown": "left as UTF-8",
        "hardware_operations": False,
    }
    evidence = root / "evidence"
    evidence.mkdir(exist_ok=True)
    (evidence / "source_encoding.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print("SOURCE_ENCODING_PASS target=" + target + " files=" + str(len(changes)))


if __name__ == "__main__":
    main()
