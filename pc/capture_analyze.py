"""Acquire a single DDR frame over UART, validate CRC, FFT and plot.
Offline --input never opens a serial port. All synthetic results are labelled.
"""
from pathlib import Path
import sys
deps=Path(__file__).resolve().parent/'_deps'
if deps.exists():sys.path.insert(0,str(deps))
import argparse,csv,json,re,time,zlib
import numpy as np

def parse_packet(text):
    lines=text.splitlines()
    start=next(i for i,s in enumerate(lines) if s.startswith('BEGIN MEAS '))
    meta=dict(re.findall(r'(\w+)=([^ ]+)',lines[start]))
    end=next(i for i in range(start+1,len(lines)) if lines[i].startswith('END MEAS '))
    samples=[]
    for line in lines[start+1:end]:
        if not re.fullmatch(r'[0-9a-fA-F]{2}( [0-9a-fA-F]{2})*',line.strip()):raise ValueError('Invalid sample line')
        samples.extend(int(v,16) for v in line.split())
    tail=dict(re.findall(r'(\w+)=([^ ]+)',lines[end]))
    if len(samples)!=int(meta['samples']):raise ValueError('Truncated/wrong frame length')
    if sum(samples)!=int(tail['sum']) or int(tail['errors'])!=0:raise ValueError('Frame statistics/board checks failed')
    if (zlib.crc32(bytes(samples))&0xffffffff)!=int(tail['crc'],16):raise ValueError('CRC mismatch')
    if meta['mode']=='RAMP' and samples!=[i&255 for i in range(len(samples))]:raise ValueError('RAMP mismatch')
    if not any(s.startswith('MEASUREMENT OK') or s=='SELFTEST PASS' for s in lines[end+1:]):raise ValueError('Missing successful completion')
    return np.asarray(samples,dtype=np.uint8),meta

def analyze(samples,fs,reference_hz):
    x=np.asarray(samples,dtype=float);n=len(x)
    if n<256 or n&(n-1):raise ValueError('Expected power-of-two frame >=256')
    y=x-x.mean()
    if np.ptp(x)<2:raise ValueError('No usable AC signal; cannot estimate frequency')
    window=np.hanning(n);spec=np.fft.rfft(y*window)
    mag=np.abs(spec)/window.sum();mag[1:-1]*=2
    peak=int(np.argmax(mag[1:-1])+1)
    a,b,c=np.log(np.maximum(mag[peak-1:peak+2],1e-15))
    den=a-2*b+c
    delta=float(np.clip(.5*(a-c)/den,-.5,.5)) if abs(den)>1e-15 else 0.0
    measured=(peak+delta)*fs/n
    t=np.arange(n)/fs
    # Phase calibration uses the known transmitted reference, NOT the FFT
    # frequency; frequency estimation above remains independent of this fit.
    basis=np.column_stack([np.sin(2*np.pi*reference_hz*t),np.cos(2*np.pi*reference_hz*t),np.ones(n)])
    coef=np.linalg.lstsq(basis,x,rcond=None)[0]
    phase=float(np.degrees(np.arctan2(coef[1],coef[0])))
    report={'samples':n,'sample_rate_hz':fs,'fft_bin_hz':fs/n,'peak_bin':peak,
            'fft_peak_bin_hz':peak*fs/n,'fft_interpolated_hz':measured,
            'reference_hz':reference_hz,'absolute_error_hz':abs(measured-reference_hz),
            'relative_error_percent':abs(measured-reference_hz)/reference_hz*100,
            'mean_code':float(x.mean()),'min_code':int(x.min()),'max_code':int(x.max()),
            'phase_reference_hz':reference_hz,'phase_at_first_sample_deg':phase,
            'fitted_amplitude_codes':float(np.hypot(coef[0],coef[1])),
            'fitted_dc_code':float(coef[2]),'first_sample_minus_fitted_dc':float(x[0]-coef[2]),
            'analog_calibration':'NOT VERIFIED by software; confirm actual board delay/gain/polarity'}
    return report,np.fft.rfftfreq(n,1/fs),mag

def save_results(samples,meta,out,source):
    out.mkdir(parents=True,exist_ok=True)
    fs=float(meta.get('fs',10000000));ref=int(meta.get('k',0))*fs/2**32
    if not ref:ref=float(meta.get('freq',100000))
    with (out/'samples.csv').open('w',newline='',encoding='utf-8') as f:
        w=csv.writer(f);w.writerow(['sample_index','adc_code','time_us'])
        w.writerows((i,int(v),i/fs*1e6) for i,v in enumerate(samples))
    if meta.get('mode')=='RAMP':
        result={'source':source,'mode':'RAMP','samples':len(samples),'selftest':'PASS','fft':'Not applied to ramp data'}
        (out/'result.json').write_text(json.dumps(result,indent=2),encoding='utf-8');return result
    result,freq,mag=analyze(samples,fs,ref);result.update({'source':source,'metadata':meta})
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig,axes=plt.subplots(3,1,figsize=(11,10),constrained_layout=True)
    times=np.arange(len(samples))/fs*1e6
    axes[0].plot(times,samples,lw=.9);axes[0].set(xlabel='Time (us)',ylabel='ADC code',title='Full captured frame')
    view=min(len(samples),max(40,int(fs/ref*3)))
    axes[1].plot(times[:view],samples[:view],'.-',ms=3);axes[1].axhline(result['fitted_dc_code'],color='gray',ls='--')
    axes[1].plot(times[0],samples[0],'ro');axes[1].set(xlabel='Time (us)',ylabel='ADC code',title=f"First samples: fitted phase {result['phase_at_first_sample_deg']:.2f} deg (reference fit)")
    axes[2].plot(freq/1000,20*np.log10(np.maximum(mag,1e-9)));axes[2].axvline(result['fft_interpolated_hz']/1000,color='r',ls='--')
    axes[2].set(xlabel='Frequency (kHz)',ylabel='dB re 1 ADC code',title=f"FFT estimated {result['fft_interpolated_hz']:.3f} Hz; reference {ref:.3f} Hz")
    for ax in axes:ax.grid(alpha=.25)
    fig.suptitle(f'Z100 CORDIC loop | {source} | NOT analog-calibrated',fontsize=12)
    fig.savefig(out/'waveform_spectrum.png',dpi=150);fig.savefig(out/'waveform_spectrum.svg');plt.close(fig)
    (out/'result.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__)
    g=p.add_mutually_exclusive_group(required=True);g.add_argument('--input',type=Path);g.add_argument('--port')
    p.add_argument('--frequency',type=int,default=100000);p.add_argument('--samples',type=int,default=4096)
    p.add_argument('--delay',type=int,default=3);p.add_argument('--amplitude',type=int,default=24)
    p.add_argument('--ramp',action='store_true');p.add_argument('--source',default='file (origin supplied by operator)')
    p.add_argument('--output',type=Path,default=Path('capture_output'))
    a=p.parse_args()
    if a.port:
        import serial
        command=f"{'t' if a.ramp else 'm'} {a.frequency} {a.samples} {a.delay} {a.amplitude}\n"
        lines=[];started=False;deadline=time.monotonic()+30
        with serial.Serial(a.port,115200,timeout=.5,dsrdtr=False,rtscts=False) as s:
            s.reset_input_buffer();s.write(command.encode('ascii'))
            while time.monotonic()<deadline:
                line=s.readline().decode('ascii',errors='strict').strip()
                if not line:continue
                print(line);lines.append(line)
                a.output.mkdir(parents=True,exist_ok=True)
                (a.output/'uart_raw.txt').write_text('\n'.join(lines)+'\n',encoding='ascii')
                if line.startswith('ERROR') or line.startswith('MEASUREMENT FAILED'):raise RuntimeError(line)
                if line.startswith('BEGIN MEAS '):started=True
                if started and (line.startswith('MEASUREMENT OK') or line=='SELFTEST PASS'):break
            else:raise TimeoutError('No complete measurement; verify Debug app/COM/command and preserve board state')
        text='\n'.join(lines)+'\n';samples,meta=parse_packet(text)
        a.output.mkdir(parents=True,exist_ok=True);(a.output/'uart_raw.txt').write_text(text,encoding='ascii')
        source='hardware UART '+meta['mode']
    else:
        source=a.source
        if a.input.suffix.lower()=='.csv':
            with a.input.open(encoding='utf-8-sig') as f:samples=np.asarray([int(row['adc_code']) for row in csv.DictReader(f)],dtype=np.uint8)
            meta={'fs':'10000000','freq':str(a.frequency),'mode':'REAL'}
        else:samples,meta=parse_packet(a.input.read_text(encoding='ascii'))
    result=save_results(samples,meta,a.output,source)
    print(json.dumps(result,indent=2))

if __name__=='__main__':main()

