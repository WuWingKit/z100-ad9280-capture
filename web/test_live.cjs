// Offline controller test: fake serial transport, no board I/O.
const vm = require('node:vm');
const fs = require('node:fs');
const assert = require('node:assert/strict');
const elements = new Map();
const jobs = new Map();
let token = 0;
const context = vm.createContext({
  document: { getElementById(id) {
    if (!elements.has(id)) elements.set(id, { checked: true, value: '0' });
    return elements.get(id);
  } },
  window: { Z100DSP: { parsePacket() { return {
    meta: { samples: 256, freq: 100000, delay: 3, amp: 8, mode: 'REAL', fs: 10000000 }
  }; } } },
  navigator: { serial: {} }, location: { search: '' },
  URLSearchParams, TextEncoder, TextDecoder, Date,
  setTimeout(fn, delay) { const id = ++token; jobs.set(id, { fn, delay }); return id; },
  clearTimeout(id) { jobs.delete(id); },
  ResizeObserver: class { observe() {} },
  console
});
// Initialization below section 9 is drawing/import-only, tested by test_signal.cjs.
const source = fs.readFileSync(__dirname + '/app.js', 'utf8');
vm.runInContext(source.slice(0, source.indexOf('// 优先用K')), context);
vm.runInContext(`
  let sent = [], renders = 0;
  send = async command => { sent.push(command); };
  render = () => { renders++; };
  port = {}; readActive = true;
  $('frequency').value = '100000'; $('samples').value = '256';
  $('delay').value = '3'; $('amplitude').value = '8';
  $('live').onclick();
`, context);
const run = code => vm.runInContext(code, context);
assert.equal(run('sent.length'), 1);
run("$('live').onclick()");
assert.equal(run('sent.length'), 1, 'double click must not send twice');
run("receive('BEGIN MEAS test'); receive('MEASUREMENT OK')");
assert.equal(run('renders'), 1);
const scheduledEntry = [...jobs.entries()].find(([, j]) => j.delay === 150);
const scheduled = scheduledEntry?.[1];
assert.ok(scheduled, 'successful frame schedules another request');
jobs.delete(scheduledEntry[0]); // Real timers are removed before their callback runs.
scheduled.fn();
assert.equal(run('sent.length'), 2);
run("$('stop').onclick()");
assert.equal(run('continuous'), false);
assert.equal(run('sent.at(-1)'), 'q');
run("receive('BEGIN MEAS test'); receive('MEASUREMENT OK')");
assert.equal(run('continuous'), false, 'late frame must not restart capture');
assert.equal([...jobs.values()].filter(j => j.delay === 150).length, 0);
run("busy = false; $('live').onclick(); receive('ERROR test')");
assert.equal(run('continuous'), false, 'board error stops loop');
assert.equal(run('busy'), false);
console.log('WEB_LIVE_PASS double-start / sequential frames / stop / late frame / device error');
