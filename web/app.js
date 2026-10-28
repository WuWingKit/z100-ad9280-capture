'use strict';
// ===== 1. 页面元素与连接/任务状态 =====
// port/reader只在用户点击连接后创建；加载演示不会访问串口。
// pending保存本次请求，用于核对返回参数；packet收集设备返回的一帧文本。
const $ = (id) => document.getElementById(id),
  D = window.Z100DSP;
let port = null,
  reader = null,
  readTask = null,
  readActive = false,
  busy = false,
  pending = null,
  timer = null;
let current = null,
  rawLog = '',
  packet = [],
  lineBuffer = '',
  timeWindow = 0;
const stamp = () => new Date().toISOString().replace(/[:.]/g, '-');
// ===== 2. 提示、日志与按钮互锁 =====
// DOM文字全部用textContent写入，不把串口数据当HTML执行。
function message(text, kind = '') {
  $('message').textContent = text;
  $('message').className = 'banner ' + kind;
}
function log(text) {
  rawLog += text + '\n';
  $('log').textContent = rawLog.slice(-18000);
  $('log').scrollTop = $('log').scrollHeight;
}
// 真实采集需要串口已连接、当前空闲，并勾选模拟接线/电压确认。
// 忙时禁止修改本次参数，页面不会自动循环触发测量。
function buttons() {
  const active = !!port && readActive;
  $('connect').disabled = !!port;
  $('disconnect').disabled = !port;
  $('capture').disabled = !active || busy || !$('physicalReady').checked;
  $('ramp').disabled = !active || busy;
  $('stop').disabled = !active;
  $('status').disabled = !active || busy;
  for (const id of ['frequency', 'samples', 'delay', 'amplitude', 'file', 'demo'])
    $(id).disabled = busy;
}
// 无论成功、错误或中止，都需清前端计时器并恢复参数编辑。
function endJob() {
  busy = false;
  pending = null;
  clearTimeout(timer);
  buttons();
}
function fail(error) {
  log('! ' + error.message);
  message(error.message, 'error');
  endJob();
}
// ===== 3. 发送命令 =====
// Web Serial写入时取得writer锁，finally释放；命令以换行结束，与PS组行匹配。
async function send(command) {
  if (!port || !readActive) throw Error('请先连接 Z100 PS 串口');
  const writer = port.writable.getWriter();
  try {
    await writer.write(new TextEncoder().encode(command + '\n'));
    log('> ' + command);
  } finally {
    writer.releaseLock();
  }
}
// 从输入框读取整数参数；范围与PS端一致，避免错误命令进入硬件。
function config() {
  const f = Number($('frequency').value),
    n = Number($('samples').value),
    d = Number($('delay').value),
    a = Number($('amplitude').value);
  if (
    !Number.isInteger(f) ||
    f < 1000 ||
    f > 1000000 ||
    ![256, 512, 1024, 2048, 4096].includes(n) ||
    !Number.isInteger(d) ||
    d < 0 ||
    d > 255 ||
    !Number.isInteger(a) ||
    a < 0 ||
    a > 96
  )
    throw Error('参数范围错误，请检查频率、点数、延迟及幅度');
  return { f, n, d, a };
}
// ===== 4. 用户触发单帧任务 =====
// 前端12秒超时覆盖采集与ASCII回传时间；PS内部仍有独立2秒硬件等待超时。
async function start(ramp) {
  try {
    if (busy) return;
    if (!ramp && !$('physicalReady').checked) throw Error('真实采集前请确认模拟回环和电压');
    const c = config();
    busy = true;
    pending = { ...c, mode: ramp ? 'RAMP' : 'REAL' };
    packet = [];
    buttons();
    message(
      ramp
        ? '正在执行板内递增码自检，DA 保持中点。'
        : '正在发波并采集；等待 DMA 完成和完整串口数据帧。'
    );
    timer = setTimeout(
      () => fail(Error('12 秒未收到完整帧；保留日志，检查 Debug 程序/串口，必要时点击停止/复位。')),
      12000
    );
    await send(`${ramp ? 't' : 'm'} ${c.f} ${c.n} ${c.d} ${c.a}`);
  } catch (e) {
    fail(e);
  }
}
// ===== 5. 按行接收并组帧 =====
// BEGIN开始收集，END之后还要等成功标志；设备错误/中止则停止本次任务。
// render只接受通过完整性检查且与pending请求一致的数据。
function receive(line) {
  if (!line) return;
  log(line);
  if (line.startsWith('ERROR') || line.startsWith('MEASUREMENT FAILED')) {
    packet = [];
    fail(Error(line));
    return;
  }
  if (
    line.startsWith('MEASUREMENT ABORTED') ||
    line.startsWith('STOPPED') ||
    line.startsWith('RESET OK')
  ) {
    packet = [];
    endJob();
    message('采集路径已复位；DAC 保持名义中点。', 'warn');
    return;
  }
  if (line.startsWith('BEGIN MEAS ')) packet = [line];
  else if (packet.length) packet.push(line);
  if (packet.length && (line.startsWith('MEASUREMENT OK') || line === 'SELFTEST PASS')) {
    try {
      const frame = D.parsePacket(packet.join('\n'));
      if (
        pending &&
        (Number(frame.meta.samples) !== pending.n ||
          Number(frame.meta.freq) !== pending.f ||
          Number(frame.meta.delay) !== pending.d ||
          Number(frame.meta.amp) !== pending.a ||
          frame.meta.mode !== pending.mode)
      )
        throw Error('返回参数与本次命令不符，请确认板上运行的是本版 ELF');
      if (Number(frame.meta.fs) !== 10000000) throw Error('返回采样率与本版 10 MHz 时钟配置不符');
      render(frame, `串口实测 · ${frame.meta.mode}`, 'live');
      endJob();
      packet = [];
    } catch (e) {
      packet = [];
      fail(e);
    }
  }
}
// ===== 6. 串口字节流 → 文本行 =====
// 一次read可能只含半行，也可能包含多行；必须用lineBuffer拼接后按换行切分。
// TextDecoder的stream模式可处理跨read边界的字符；断开时释放reader锁。
async function readLoop() {
  const decoder = new TextDecoder();
  reader = port.readable.getReader();
  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      lineBuffer += decoder.decode(value, { stream: true });
      if (lineBuffer.length > 1000000) throw Error('串口没有正确换行，检查波特率/固件');
      let at;
      while ((at = lineBuffer.indexOf('\n')) >= 0) {
        const line = lineBuffer.slice(0, at).replace(/\r/g, '').trim();
        lineBuffer = lineBuffer.slice(at + 1);
        receive(line);
      }
    }
  } catch (e) {
    if (readActive) fail(Error('串口读取中断：' + e.message));
  } finally {
    reader.releaseLock();
    reader = null;
    readActive = false;
    buttons();
  }
}
// ===== 7. 连接与断开 =====
// requestPort必须由用户点击产生的浏览器手势触发；网页不能悄悄选串口。
// 115200/8N1、无流控，与PS UART0一致。浏览器通常只提供USB VID/PID而非COM名称。
$('connect').onclick = async () => {
  try {
    if (!navigator.serial)
      throw Error('此浏览器未提供 Web Serial。请用桌面 Chrome/Edge 打开 http://127.0.0.1:8765');
    port = await navigator.serial.requestPort();
    await port.open({
      baudRate: 115200,
      dataBits: 8,
      stopBits: 1,
      parity: 'none',
      flowControl: 'none'
    });
    lineBuffer = '';
    packet = [];
    readActive = true;
    readTask = readLoop();
    buttons();
    const info = port.getInfo();
    $('portInfo').textContent =
      `已连接 · USB VID ${info.usbVendorId?.toString(16) || '未知'} / PID ${info.usbProductId?.toString(16) || '未知'} · 115200 / 8N1`;
    $('sourceBadge').textContent = '串口已连接 · 尚未采集';
    $('sourceBadge').className = 'pill live';
    message('串口连接成功。请确认新版程序已在 SDK Debug 中继续运行，再点击递增码自检。', 'ok');
    log('[串口已打开，未自动发出采集命令]');
  } catch (e) {
    if (port) {
      try {
        await port.close();
      } catch {}
      port = null;
    }
    fail(e);
  }
};
$('disconnect').onclick = async () => {
  try {
    readActive = false;
    if (reader) await reader.cancel();
    if (readTask) await readTask;
    if (port) await port.close();
  } catch (e) {
    log('! ' + e.message);
  } finally {
    port = null;
    readTask = null;
    endJob();
    $('portInfo').textContent = '串口已断开';
    log('[串口已断开]');
    message('串口已断开。现有图像保持，来源标签不变。');
  }
};
// ===== 8. 控件事件：测量、自检、停止、状态 =====
// q由PS处理整条路径复位；关闭网页/串口本身不是硬件停止命令。
$('capture').onclick = () => start(false);
$('ramp').onclick = () => start(true);
$('stop').onclick = async () => {
  try {
    await send('q');
    message('已发送 q，等待板端停止/复位响应；串口打印阶段会在打印结束后处理。', 'warn');
  } catch (e) {
    fail(e);
  }
};
$('status').onclick = () => send('s').catch(fail);
$('physicalReady').onchange = buttons;
// 优先用K*fs/2^32计算实际设置频率，减少频率控制字量化造成的参考误差。
function reference(meta) {
  return Number(meta.k) > 0 ? (Number(meta.k) * Number(meta.fs)) / 4294967296 : Number(meta.freq);
}
// ===== 9. 更新统计与来源标签 =====
// 串口CRC通过、CSV导入、数字仿真是三种来源，必须分别标注。
// RAMP不测单音；真实数据平坦时仍显示码值与CRC，但不编造主频。
function render(frame, source, kind) {
  const x = frame.samples,
    meta = frame.meta,
    fs = Number(meta.fs),
    ref = reference(meta);
  let result = null,
    analysisError = '';
  if (meta.mode !== 'RAMP') {
    try {
      result = D.analyze(x, fs, ref);
    } catch (e) {
      analysisError = e.message;
    }
  }
  current = { ...frame, source, kind, result, analysisError };
  $('sourceBadge').textContent = source;
  $('sourceBadge').className = 'pill ' + kind;
  $('nStat').textContent = x.length.toLocaleString() + ' 点';
  $('duration').textContent =
    `${((x.length / fs) * 1e6).toFixed(1)} μs · ${(fs / 1e6).toFixed(1)} MSPS`;
  const mean = x.reduce((a, b) => a + b, 0) / x.length;
  $('range').textContent = Math.min(...x) + '–' + Math.max(...x);
  $('mean').textContent = `均值 ${mean.toFixed(3)} code · 首点 ${x[0]}`;
  $('integrity').textContent = frame.verified
    ? '校验通过'
    : kind === 'demo'
      ? '仿真数据'
      : 'CSV 导入';
  $('crc').textContent = frame.verified ? `CRC32 ${frame.crc}` : '不具有板端 CRC 来源校验';
  $('peak').textContent = result ? (result.frequency / 1000).toFixed(3) + ' kHz' : '—';
  $('error').textContent = result
    ? `参考 ${(ref / 1000).toFixed(3)} kHz · 偏差 ${result.errorHz.toFixed(2)} Hz`
    : meta.mode === 'RAMP'
      ? 'RAMP 自检不作单音测频'
      : '没有可用交流信号';
  $('spectrumNote').textContent = result
    ? `去直流 · Hann · 格点 ${result.binHz.toFixed(3)} Hz · 插值不代表同等频率分辨率`
    : 'RAMP/平坦信号不报告单音频率';
  $('phaseNote').textContent = result?.fit
    ? `已知参考频率下拟合首点相位 ${result.fit.phaseDeg.toFixed(3)}°，幅度 ${result.fit.amplitude.toFixed(3)} code，DC ${result.fit.dc.toFixed(3)}。真实模拟延迟尚需标定；首点标红。`
    : '参考相位拟合只适用于具有足够幅度的真实正弦采样。';
  const origin =
    kind === 'demo'
      ? '这是 Xilinx IP 数字模型的仿真数据，未访问开发板。'
      : kind === 'live'
        ? '已收到完整串口帧，点数/CRC/统计/成功标志均通过。'
        : '已导入文件；请自行确认数据来源及采样率。';
  message(
    origin +
      (meta.mode === 'RAMP'
        ? ' 递增码逐点顺序通过。'
        : analysisError
          ? ' ' + analysisError
          : result && !result.reliable
            ? ' 采集窗口不足 3 个参考周期，频率估计不可靠；请增加点数或提高频率。'
            : '') +
      (Number(meta.otr) ? ' 注意：板端 OTR 曾置位，检查过量程。' : ''),
    analysisError || (result && !result.reliable) || Number(meta.otr)
      ? 'warn'
      : kind === 'live'
        ? 'ok'
        : ''
  );
  for (const id of ['csv', 'json', 'png']) $(id).disabled = false;
  draw();
}
// ===== 10. 离线演示与文件导入 =====
// 演示数据来自实际Xilinx IP数字模型，不来自开发板，也不发送任何串口命令。
async function demo() {
  try {
    const r = await fetch('demo_samples.csv');
    if (!r.ok) throw Error('无法载入演示 CSV');
    render(
      {
        samples: D.parseCSV(await r.text()),
        meta: {
          fs: '10000000',
          freq: '100000',
          samples: '4096',
          delay: '3',
          amp: '24',
          mode: 'REAL'
        },
        verified: false
      },
      '数字仿真演示 · 非板上实测',
      'demo'
    );
    log('[已载入已验证的 Xilinx CORDIC 数字回环数据]');
  } catch (e) {
    fail(Error(e.message + '；请使用 start_dashboard.ps1 启动本地 HTTP 服务。'));
  }
}
$('demo').onclick = demo;
$('file').onchange = async (e) => {
  const f = e.target.files[0];
  if (!f) return;
  try {
    const text = await f.text();
    if (f.name.toLowerCase().endsWith('.csv')) {
      const c = config();
      render(
        {
          samples: D.parseCSV(text),
          meta: { fs: '10000000', freq: String(c.f), mode: 'REAL' },
          verified: false
        },
        '导入 CSV · 来源由操作者确认',
        ''
      );
    } else render(D.parsePacket(text), '导入串口日志 · CRC 已校验', '');
    log('[导入文件 ' + f.name + ']');
  } catch (err) {
    fail(err);
  }
};
// ===== 11. Canvas通用绘图 =====
// 使用devicePixelRatio匹配高分辨率屏幕；sx/sy将实际坐标映射为像素。
// 裁剪绘图区，先画网格/轴，再画曲线；红点标记第0个样本。
function drawPlot(canvas, points, xMax, yMin, yMax, xLabel, yLabel, color, marker) {
  const w = canvas.clientWidth || 800,
    h = canvas.clientHeight || 265,
    dpr = window.devicePixelRatio || 1;
  canvas.width = Math.round(w * dpr);
  canvas.height = Math.round(h * dpr);
  const ctx = canvas.getContext('2d');
  ctx.scale(dpr, dpr);
  ctx.fillStyle = 'white';
  ctx.fillRect(0, 0, w, h);
  const m = { l: 57, r: 18, t: 14, b: 40 },
    pw = w - m.l - m.r,
    ph = h - m.t - m.b;
  const sx = (x) => m.l + (x / xMax) * pw,
    sy = (y) => m.t + ((yMax - y) / (yMax - yMin)) * ph;
  ctx.font = '11px Segoe UI, Microsoft YaHei';
  ctx.lineWidth = 1;
  for (let i = 0; i <= 5; i++) {
    const x = m.l + (i * pw) / 5,
      y = m.t + (i * ph) / 5;
    ctx.strokeStyle = '#e8edf4';
    ctx.beginPath();
    ctx.moveTo(x, m.t);
    ctx.lineTo(x, h - m.b);
    ctx.moveTo(m.l, y);
    ctx.lineTo(w - m.r, y);
    ctx.stroke();
    ctx.fillStyle = '#687a92';
    ctx.textAlign = 'center';
    ctx.fillText(((i * xMax) / 5).toFixed(xMax < 10 ? 2 : 1), x, h - 22);
    ctx.textAlign = 'right';
    ctx.fillText((yMax - (i * (yMax - yMin)) / 5).toFixed(0), m.l - 8, y + 4);
  }
  ctx.strokeStyle = '#9aaac0';
  ctx.beginPath();
  ctx.moveTo(m.l, m.t);
  ctx.lineTo(m.l, h - m.b);
  ctx.lineTo(w - m.r, h - m.b);
  ctx.stroke();
  ctx.save();
  ctx.beginPath();
  ctx.rect(m.l, m.t, pw, ph);
  ctx.clip();
  ctx.strokeStyle = color;
  ctx.lineWidth = 1.5;
  ctx.beginPath();
  let started = false;
  for (const [x, y] of points) {
    if (x > xMax) break;
    if (!started) {
      ctx.moveTo(sx(x), sy(y));
      started = true;
    } else ctx.lineTo(sx(x), sy(y));
  }
  ctx.stroke();
  if (marker) {
    ctx.fillStyle = '#d74a47';
    ctx.beginPath();
    ctx.arc(sx(marker[0]), sy(marker[1]), 4, 0, 2 * Math.PI);
    ctx.fill();
  }
  ctx.restore();
  ctx.fillStyle = '#52647d';
  ctx.textAlign = 'right';
  ctx.fillText(xLabel, w - m.r, h - 4);
  ctx.textAlign = 'left';
  ctx.fillText(yLabel, 4, 10);
}
// 按用户选择显示完整帧或前三周期；频谱可显示主频附近或完整Nyquist范围。
// 原始samples不被绘图更改，导出的CSV仍保留全部样本。
function draw() {
  if (!current) return;
  const { samples: x, meta, result } = current,
    fs = Number(meta.fs),
    ref = reference(meta);
  const count =
    $('timeView').value === 'all'
      ? x.length
      : Math.min(x.length, Math.max(40, Math.ceil((3 * fs) / ref)));
  timeWindow = count;
  const low = Math.max(0, Math.min(...x) - 5),
    high = Math.min(255, Math.max(...x) + 5);
  drawPlot(
    $('timeCanvas'),
    Array.from(x.slice(0, count), (y, i) => [(i / fs) * 1e6, y]),
    Math.max(1, ((count - 1) / fs) * 1e6),
    low,
    Math.max(low + 1, high),
    '时间 / μs',
    'ADC code',
    '#2258ce',
    [0, x[0]]
  );
  if (result) {
    const maxHz =
      $('freqView').value === 'all'
        ? fs / 2
        : Math.min(fs / 2, Math.max(ref * 4, 10 * result.binHz));
    drawPlot(
      $('freqCanvas'),
      Array.from(result.magnitude, (v, i) => [
        (i * fs) / x.length / 1000,
        20 * Math.log10(Math.max(v, 1e-9))
      ]),
      maxHz / 1000,
      -120,
      Math.max(10, 20 * Math.log10(Math.max(...result.magnitude)) + 8),
      '频率 / kHz',
      'dB re 1 code',
      '#0a968e'
    );
  } else
    drawPlot(
      $('freqCanvas'),
      [],
      Number(meta.fs) / 2000,
      -120,
      20,
      '频率 / kHz',
      '不作单音测频',
      '#0a968e'
    );
}
$('timeView').onchange = draw;
$('freqView').onchange = draw;
window.addEventListener('resize', draw);
$('timeCanvas').onmousemove = (e) => {
  if (!current) return;
  const rect = e.currentTarget.getBoundingClientRect(),
    ratio = Math.max(0, Math.min(1, (e.clientX - rect.left - 57) / (rect.width - 75))),
    i = Math.round(ratio * (timeWindow - 1));
  $('hover').textContent =
    `样本 #${i} · ${((i / Number(current.meta.fs)) * 1e6).toFixed(3)} μs · ${current.samples[i]} code`;
};
// ===== 12. 本地导出 =====
// Blob下载只保存到用户电脑；没有云端上传和外部CDN依赖。
function save(name, data, type = 'text/plain') {
  const url = URL.createObjectURL(new Blob([data], { type })),
    a = document.createElement('a');
  a.href = url;
  a.download = name;
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
$('csv').onclick = () => {
  if (current)
    save(
      'z100_' + stamp() + '.csv',
      'sample_index,adc_code,time_us\n' +
        Array.from(
          current.samples,
          (v, i) => `${i},${v},${(i / Number(current.meta.fs)) * 1e6}`
        ).join('\n'),
      'text/csv'
    );
};
$('json').onclick = () => {
  if (current) {
    const r = current.result ? { ...current.result, magnitude: undefined } : null;
    save(
      'z100_' + stamp() + '.json',
      JSON.stringify(
        {
          source: current.source,
          metadata: current.meta,
          crcVerified: current.verified,
          crc32: current.crc || null,
          analogCalibration: 'NOT VERIFIED',
          analysis: r
        },
        null,
        2
      ),
      'application/json'
    );
  }
};
$('raw').onclick = () => save('z100_serial_' + stamp() + '.txt', rawLog);
$('png').onclick = () => {
  if (!current) return;
  const a = $('timeCanvas'),
    b = $('freqCanvas'),
    out = document.createElement('canvas');
  out.width = Math.max(a.width, b.width);
  out.height = a.height + b.height + 110;
  const c = out.getContext('2d');
  c.fillStyle = 'white';
  c.fillRect(0, 0, out.width, out.height);
  c.fillStyle = '#182a41';
  c.font = '20px sans-serif';
  c.fillText('Z100 | ' + current.source, 20, 30);
  c.font = '14px sans-serif';
  c.fillText(
    'N=' +
      current.samples.length +
      ' | fs=' +
      current.meta.fs +
      ' Hz | analog phase NOT calibrated',
    20,
    60
  );
  c.drawImage(a, 0, 90);
  c.drawImage(b, 0, a.height + 100);
  out.toBlob((blob) => {
    if (blob) save('z100_plot_' + stamp() + '.png', blob, 'image/png');
  });
};
// ===== 13. 首次加载 =====
// 只有URL带?demo才自动载入数字演示，绝不自动连接串口或触发真实采集。
buttons();
if (!navigator.serial)
  $('portInfo').textContent =
    '当前浏览器未提供 Web Serial；可先使用仿真演示，实测请用桌面 Chrome/Edge。';
if (new URLSearchParams(location.search).has('demo')) demo();
