// 测试入口：覆盖CRC/帧完整性、单音FFT与实际仿真CSV，测试不访问串口。
const assert = require('node:assert/strict'),
  fs = require('node:fs'),
  path = require('node:path');
const D = require('./signal.js');
assert.equal(D.crc32(Buffer.from('123456789')), 0xcbf43926);
// 生成与PS格式相同的合法测试帧，作为正例和篡改负例的共同基线。
function packet(values, mode = 'REAL') {
  const x = Array.from(values),
    sum = x.reduce((a, b) => a + b, 0),
    crc = D.crc32(x).toString(16).padStart(8, '0');
  return (
    `BEGIN MEAS job=1 mode=${mode} samples=${x.length} fs=10000000 freq=100000 k=42949673 delay=3 amp=24 otr=0\n` +
    x.map((v) => v.toString(16).padStart(2, '0')).join(' ') +
    `\nEND MEAS sum=${sum} min=${Math.min(...x)} max=${Math.max(...x)} errors=0 crc=${crc}\n` +
    (mode === 'RAMP' ? 'SELFTEST PASS\n' : 'MEASUREMENT OK (analog phase not calibrated)\n')
  );
}
const ramp = Uint8Array.from({ length: 256 }, (_, i) => i);
assert.equal(D.parsePacket(packet(ramp, 'RAMP')).samples.length, 256);
// 故意篡改数据、CRC、错误标志或长度，确保解析器拒绝假成功。
const text = packet(ramp, 'RAMP');
assert.throws(() => D.parsePacket(text.replace('00 01 02', '00 02 01')), /CRC|顺序/);
assert.throws(() => D.parsePacket(text.replace(/crc=[0-9a-f]+/, 'crc=00000000')), /CRC/);
assert.throws(() => D.parsePacket(text.replace('errors=0', 'errors=1')), /板端/);
assert.throws(() => D.parsePacket(text.replace('SELFTEST PASS', '')), /成功/);
assert.throws(() => D.parsePacket(text.replace('00 01 02', '00 01')), /样本数/);
assert.throws(() => D.parseCSV('sample_index,adc_code\n0,999'), /点数|范围/);
assert.throws(() => D.analyze(new Uint8Array(256).fill(125), 1e7, 1e5), /交流/);
// 四组非整格点正弦：同时验证FFT频率和已知参考频率下的相位。
for (const f of [50000, 100000, 250000, 700000]) {
  const x = Uint8Array.from({ length: 4096 }, (_, i) =>
    Math.round(128 + 24 * Math.sin((2 * Math.PI * f * i) / 1e7 + 0.2))
  );
  const a = D.analyze(x, 1e7, f);
  assert.ok(Math.abs(a.frequency - f) < 60);
  assert.ok(Math.abs(a.fit.phaseDeg - (0.2 * 180) / Math.PI) < 1);
}
// 再读实际Xilinx IP仿真CSV，与Python分析的数值结果交叉核对。
const x = D.parseCSV(fs.readFileSync(path.join(__dirname, 'demo_samples.csv'), 'utf8')),
  r = D.analyze(x, 1e7, 1e5);
assert.equal(x.length, 4096);
assert.equal(x[0], 128);
assert.ok(Math.abs(r.frequency - 99992.04430657563) < 0.001);
console.log(
  'WEB_DSP_PASS CRC32 / packet checks / RAMP / flat rejection / 4 frequencies / actual Xilinx-model CSV'
);
console.log(
  JSON.stringify({ n: r.n, frequency: r.frequency, phaseDeg: r.fit.phaseDeg, binHz: r.binHz })
);
