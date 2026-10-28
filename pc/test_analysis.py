# 本文件的执行方式、输入输出及与其它模块的关系见 docs/CODE_GUIDE.md。
from pathlib import Path
import sys, zlib

sys.path.insert(0, str(Path(__file__).resolve().parent))
from capture_analyze import analyze, parse_packet, np

fs = 10000000
n = 4096
# 1. 四种频率的正弦，检查FFT频率与已知参考相位。
for hz in [50000, 100000, 250000, 700000]:
    x = np.clip(
        np.rint(125 + 24 * np.sin(2 * np.pi * hz * np.arange(n) / fs)), 0, 255
    ).astype(np.uint8)
    result, _, _ = analyze(x, fs, hz)
    assert abs(result["fft_interpolated_hz"] - hz) < 60, result
    assert abs(result["phase_at_first_sample_deg"]) < 1, result
# 2. 构造PS格式的合法RAMP帧，再故意篡改字节/长度/errors检查拒绝路径。
x = np.arange(256, dtype=np.uint8)
raw = bytes(x)
header = "BEGIN MEAS job=1 mode=RAMP samples=256 fs=10000000 freq=100000 k=42949673 delay=3 amp=24 otr=0\n"
body = (
    "\n".join(" ".join(f"{v:02x}" for v in x[i : i + 16]) for i in range(0, 256, 16))
    + "\n"
)
tail = f"END MEAS sum={sum(map(int,x))} min=0 max=255 errors=0 crc={zlib.crc32(raw):08x}\nSELFTEST PASS\n"
packet = header + body + tail
assert len(parse_packet(packet)[0]) == 256
for changed in [
    packet.replace("00 01 02", "00 01 03"),
    packet.replace("samples=256", "samples=512"),
    packet.replace("errors=0", "errors=1"),
]:
    try:
        parse_packet(changed)
    except ValueError:
        pass
    else:
        raise AssertionError("Corrupted packet accepted")
# 3. 平坦输入不能被误报成有意义的主频。
try:
    analyze(np.full(4096, 125), fs, 100000)
except ValueError:
    pass
else:
    raise AssertionError("Flat input accepted as a measured frequency")
print(
    "PC_ANALYSIS_PASS frequencies=4 CRC/truncation/error_rejection=PASS flat_input_rejection=PASS"
)
