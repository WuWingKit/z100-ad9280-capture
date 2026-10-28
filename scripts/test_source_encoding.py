# -*- coding: utf-8 -*-
"""Regression test: IDE GBK compatibility without losing Unicode comments."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile

script = Path(__file__).with_name("source_encoding.py")
with tempfile.TemporaryDirectory(prefix="z100_encoding_test_") as temporary:
    root = Path(temporary) / "project"
    for folder in ["rtl", "software", "web", "sdk_workspace/adc_uart/src"]:
        (root / folder).mkdir(parents=True, exist_ok=True)
    rtl = "// 组合运算：取样来源、定点正弦、DA码值\nmodule sample; endmodule\n"
    c = "// 主程序：复位与采集\nint main(void) { return 0; }\n"
    (root / "rtl/sample.v").write_bytes(rtl.encode("utf-8"))
    (root / "software/main.c").write_bytes(c.encode("utf-8"))
    (root / "sdk_workspace/adc_uart/src/main.c").write_bytes(c.encode("utf-8"))
    page = "<!doctype html><meta charset='utf-8'>波形与频谱"
    (root / "web/index.html").write_bytes(page.encode("utf-8"))

    def run(target):
        return subprocess.run(
            [sys.executable, str(script), "--target", target, "--root", str(root)],
            capture_output=True,
        )

    assert run("gbk").returncode == 0
    assert (root / "rtl/sample.v").read_bytes().decode("gbk") == rtl
    assert (root / "software/main.c").read_bytes().decode("gbk") == c
    assert (root / "web/index.html").read_bytes() == page.encode("utf-8")
    prefs = root / "sdk_workspace/adc_uart/.settings/org.eclipse.core.resources.prefs"
    assert "encoding/src/main.c=GBK" in prefs.read_text(encoding="ascii")
    report = json.loads((root / "evidence/source_encoding.json").read_text(encoding="utf-8"))
    assert len(report["changed_files"]) == 3
    assert all(item["unicode_content_unchanged"] for item in report["changed_files"])
    assert Path(report["backup"]).is_file()

    assert run("utf8").returncode == 0
    assert (root / "rtl/sample.v").read_bytes() == rtl.encode("utf-8")
    assert "encoding/src/main.c=UTF-8" in prefs.read_text(encoding="ascii")
    assert run("utf8").returncode == 0
    assert not json.loads((root / "evidence/source_encoding.json").read_text(encoding="utf-8"))["changed_files"]

    # GBK cannot represent every Unicode character. Refuse the whole batch
    # before any write, rather than replacing characters with question marks.
    (root / "rtl/unrepresentable.v").write_text("// 😀\n", encoding="utf-8")
    assert run("gbk").returncode != 0
    assert (root / "rtl/sample.v").read_bytes() == rtl.encode("utf-8")

print("SOURCE_ENCODING_TEST_PASS GBK/UTF8 round-trip, SDK preference, HTML unchanged, strict no-loss preflight")
