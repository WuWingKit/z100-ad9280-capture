"""Evidence recheck only: no COM/JTAG/hardware access.
RAMP is manually transcribed from user screenshot; not a new board capture.
REAL record retains only the footer/statistics supplied in chat, not invented samples.
"""
from pathlib import Path
from datetime import datetime, timezone, timedelta
import csv, hashlib, json, re, statistics, zipfile

root = Path(__file__).resolve().parent
project = root.parents[1]
ramp_text = '''BEGIN ADC100 mode=RAMP bytes=100
00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13
14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27
28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B
3C 3D 3E 3F 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F
50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63
END ADC100 sum=4950 min=0 max=99 errors=0
SELFTEST PASS
'''
(root/'ramp_screenshot_transcript.txt').write_text(ramp_text,encoding='utf-8')

def validate_ramp(text):
    lines=text.splitlines()
    assert lines[0] == 'BEGIN ADC100 mode=RAMP bytes=100'
    pos=next(i for i,s in enumerate(lines) if s.startswith('END ADC100'))
    assert all(re.fullmatch(r'[0-9A-F]{2}( [0-9A-F]{2})*',s) for s in lines[1:pos])
    samples=[int(s,16) for s in ' '.join(lines[1:pos]).split()]
    assert len(samples)==100, 'length'
    assert samples==list(range(100)), 'sequence'
    assert lines[pos]=='END ADC100 sum=4950 min=0 max=99 errors=0'
    assert sum(samples)==4950 and min(samples)==0 and max(samples)==99
    assert lines[-1]=='SELFTEST PASS'
    return samples

samples=validate_ramp(ramp_text)
with (root/'ramp_screenshot_transcribed.csv').open('w',newline='',encoding='utf-8-sig') as f:
    w=csv.writer(f); w.writerow(['sample_index','adc_code_decimal','adc_code_hex','source'])
    w.writerows((i,x,f'{x:02X}','user_screenshot_manual_transcription') for i,x in enumerate(samples))

# Negative checks exercise the evidence parser; they are not hardware fault injections.
negative={}
for name,text in {
    'missing_sample':ramp_text.replace('00 01 02 03','00 01 03'),
    'duplicate_sample':ramp_text.replace('00 01 02 03','00 01 01 03'),
    'swapped_order':ramp_text.replace('00 01 02 03','00 02 01 03'),
    'changed_footer':ramp_text.replace('sum=4950','sum=4951'),
}.items():
    try: validate_ramp(text)
    except AssertionError: negative[name]='REJECTED_AS_EXPECTED'
    else: raise AssertionError('corruption accepted: '+name)

real={'source':'User pasted c-command UART hex stream in conversation; footer decoded from ASCII. No new hardware capture.',
      'input_condition':'ADC module connected; analog input not connected to a signal source, per user. Not a calibrated zero input.',
      'frame_samples':100,'reported_sum':12503,'reported_min':125,'reported_max':126,'reported_errors':0,
      'decoded_footer':'END ADC100 sum=12503 min=125 max=126 errors=0',
      'decoded_result':'CAPTURE OK (signal quality not assessed)',
      'inferred_distribution_from_count_min_max_sum':{'125':97,'126':3},
      'mean_code':125.03,'peak_to_peak_codes':1,
      'limitation':'Full original UART byte stream was not exported to a local raw file; only reported statistics are retained here. Analog accuracy not validated.'}
assert 97*125+3*126==12503
assert bytes([0x37,0x44]).decode('ascii')=='7D'
assert int(bytes([0x37,0x44]).decode('ascii'),16)==125
(root/'real_adc_reported_summary.json').write_text(json.dumps(real,ensure_ascii=False,indent=2),encoding='utf-8')

manifest=json.loads((project/'hardware/verification_summary.json').read_text())
artifact_checks={}
for name,info in manifest['files'].items():
    digest=hashlib.sha256((project/name).read_bytes()).hexdigest()
    artifact_checks[name]=(digest==info['sha256'])
assert all(artifact_checks.values()), 'artifact differs from prior verified build'
backup=Path('E:/WorkSpace/Phased-array radar/fpga-learning/backups/bd_start_before_z100_20261022.zip')
with zipfile.ZipFile(backup) as z:
    assert z.testzip() is None
    backup_entries=len(z.infolist())
report={'time':datetime.now(timezone(timedelta(hours=8))).isoformat(),
        'operator':'Codex local evidence analysis; no hardware connection',
        'ramp':{'source':'user_screenshot_manual_transcription','count':len(samples),'sum':sum(samples),'min':min(samples),'max':max(samples),'mean':statistics.mean(samples),'sequence_matches_0_to_99':True},
        'negative_parser_tests':negative,'real_adc_statistics_consistent':True,
        'prior_build_artifact_hashes_match':artifact_checks,
        'original_backup_crc':'PASS','original_backup_entries':backup_entries,
        'hardware_retested_by_agent':False,'result':'PASS'}
(root/'evidence_analysis.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
