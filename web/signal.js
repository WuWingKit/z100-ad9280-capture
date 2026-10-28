/* Offline DSP and packet checks. No network or hardware access. */
(function(root){
 'use strict';
 function crc32(data){let c=0xffffffff;for(const b of data){c^=b;for(let j=0;j<8;j++)c=(c>>>1)^((c&1)?0xedb88320:0);}return (c^0xffffffff)>>>0;}
 function fields(line){return Object.fromEntries([...line.matchAll(/(\w+)=([^\s]+)/g)].map(m=>[m[1],m[2]]));}
 function parsePacket(text){
  const lines=text.replace(/\r/g,'').split('\n').map(s=>s.trim()).filter(Boolean);
  const begin=lines.findIndex(s=>s.startsWith('BEGIN MEAS '));
  if(begin<0)throw Error('没有 BEGIN MEAS；请使用本版 PS 程序');
  const end=lines.findIndex((s,i)=>i>begin&&s.startsWith('END MEAS '));
  if(end<0)throw Error('数据帧不完整：没有 END');
  const meta=fields(lines[begin]),tail=fields(lines[end]);
  const n=Number(meta.samples);if(![256,512,1024,2048,4096].includes(n))throw Error('采样点数非法');
  if(!['REAL','RAMP'].includes(meta.mode))throw Error('未知采集模式');
  const bytes=[];
  for(const line of lines.slice(begin+1,end)){
   if(!/^[0-9a-f]{2}(\s+[0-9a-f]{2})*$/i.test(line))throw Error('数据区含非法字符或混入其它输出');
   bytes.push(...line.split(/\s+/).map(v=>parseInt(v,16)));
  }
  if(bytes.length!==n)throw Error(`样本数不匹配：收到 ${bytes.length}，应为 ${n}`);
  if(Number(tail.errors)!==0)throw Error('板端报告采集或缓冲区错误');
  if(!/^[0-9a-f]{8}$/i.test(tail.crc||'') || crc32(bytes)!==parseInt(tail.crc,16))throw Error('CRC32 不匹配');
  if(bytes.reduce((a,b)=>a+b,0)!==Number(tail.sum)||Math.min(...bytes)!==Number(tail.min)||Math.max(...bytes)!==Number(tail.max))throw Error('统计校验不一致');
  const success=meta.mode==='RAMP'?'SELFTEST PASS':'MEASUREMENT OK';
  if(!lines.slice(end+1).some(s=>s.startsWith(success)))throw Error('缺少板端成功标志');
  if(meta.mode==='RAMP' && bytes.some((v,i)=>v!==(i&255)))throw Error('RAMP 顺序错误');
  return {samples:Uint8Array.from(bytes),meta,crc:tail.crc.toUpperCase(),verified:true};
 }
 function parseCSV(text){
  const rows=text.trim().split(/\r?\n/),cols=rows.shift().replace(/^\uFEFF/,'').split(','),at=cols.indexOf('adc_code');
  if(at<0)throw Error('CSV 需要 adc_code 列');
  const values=rows.filter(s=>s.trim()).map(s=>Number(s.split(',')[at]));
  if(![256,512,1024,2048,4096].includes(values.length)||values.some(v=>!Number.isInteger(v)||v<0||v>255))throw Error('CSV 点数或码值范围错误');
  return Uint8Array.from(values);
 }
 function fft(re,im){
  const n=re.length;
  for(let i=1,j=0;i<n;i++){let b=n>>1;for(;j&b;b>>=1)j^=b;j^=b;if(i<j){[re[i],re[j]]=[re[j],re[i]];[im[i],im[j]]=[im[j],im[i]];}}
  for(let len=2;len<=n;len<<=1){const a=-2*Math.PI/len,cr=Math.cos(a),ci=Math.sin(a);
   for(let i=0;i<n;i+=len){let wr=1,wi=0;for(let j=0;j<len/2;j++){
    const l=i+j,r=l+len/2,tr=wr*re[r]-wi*im[r],ti=wr*im[r]+wi*re[r];
    re[r]=re[l]-tr;im[r]=im[l]-ti;re[l]+=tr;im[l]+=ti;
    const old=wr;wr=old*cr-wi*ci;wi=old*ci+wi*cr;
   }}
  }
 }
 function fitPhase(x,fs,f){
  // Least squares [sin,cos,DC], solved with pivoted elimination.
  const a=Array.from({length:3},()=>[0,0,0,0]);
  for(let i=0;i<x.length;i++){const p=2*Math.PI*f*i/fs,v=[Math.sin(p),Math.cos(p),1];
   for(let j=0;j<3;j++){for(let k=0;k<3;k++)a[j][k]+=v[j]*v[k];a[j][3]+=v[j]*x[i];}}
  for(let j=0;j<3;j++){let p=j;for(let k=j+1;k<3;k++)if(Math.abs(a[k][j])>Math.abs(a[p][j]))p=k;
   [a[j],a[p]]=[a[p],a[j]];const d=a[j][j];if(Math.abs(d)<1e-10)return null;
   for(let k=j;k<4;k++)a[j][k]/=d;
   for(let r=0;r<3;r++)if(r!==j){const q=a[r][j];for(let k=j;k<4;k++)a[r][k]-=q*a[j][k];}
  }
  return {phaseDeg:Math.atan2(a[1][3],a[0][3])*180/Math.PI,amplitude:Math.hypot(a[0][3],a[1][3]),dc:a[2][3]};
 }
 function analyze(samples,fs,ref){
  const n=samples.length;if(![256,512,1024,2048,4096].includes(n)||!(fs>0)||!(ref>0&&ref<fs/2))throw Error('FFT 参数非法');
  const min=Math.min(...samples),max=Math.max(...samples),mean=samples.reduce((a,b)=>a+b,0)/n;
  if(max-min<2)throw Error('交流变化不足，不能可靠测频；检查回环接线和幅度');
  const re=new Float64Array(n),im=new Float64Array(n);let sum=0;
  for(let i=0;i<n;i++){const w=.5*(1-Math.cos(2*Math.PI*i/(n-1)));sum+=w;re[i]=(samples[i]-mean)*w;}
  fft(re,im);const magnitude=new Float64Array(n/2+1);let peak=1;
  for(let i=0;i<=n/2;i++){magnitude[i]=Math.hypot(re[i],im[i])/sum*((i>0&&i<n/2)?2:1);if(i>0&&i<n/2&&magnitude[i]>magnitude[peak])peak=i;}
  const log=i=>Math.log(Math.max(magnitude[i],1e-15)),a=log(peak-1),b=log(peak),c=log(peak+1),den=a-2*b+c;
  const delta=Math.abs(den)>1e-15?Math.max(-.5,Math.min(.5,.5*(a-c)/den)):0;
  const frequency=(peak+delta)*fs/n,cycles=n*ref/fs;
  return {n,fs,ref,min,max,mean,binHz:fs/n,peakBin:peak,frequency,errorHz:frequency-ref,
    cycles,reliable:cycles>=3,fit:fitPhase(samples,fs,ref),magnitude};
 }
 const api={crc32,parsePacket,parseCSV,fft,fitPhase,analyze};
 if(typeof module!=='undefined'&&module.exports)module.exports=api;else root.Z100DSP=api;
})(typeof globalThis!=='undefined'?globalThis:this);
