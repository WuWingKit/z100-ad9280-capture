"""Verify archived physical-board evidence and packaged current source inputs.
This is an offline integrity check, not a fresh hardware run.
"""
from pathlib import Path
import hashlib,json,struct,xml.etree.ElementTree as ET
R=Path(__file__).resolve().parents[1]/'projects/radar_spi_four_wire'
for name,count,pattern in [('spi_only_selftest_20261119_153253',3,True),('spi_only_real_20261119_154059',10,False)]:
    p=R/'evidence'/name;j=json.loads((p/'result.json').read_text());assert j['passed'] and len(j['frames'])==count
    for n,f in enumerate(j['frames']):
        raw=(p/f'frame_{n}.bin').read_bytes();assert len(raw)==47136
        assert hashlib.sha256(raw).hexdigest()==f['sha256']
        h=struct.unpack_from('<16H',raw);x=0
        for word in h:x^=word
        assert h[:10]==(0xa610,3,16,9,0,19,512,23,24,4)
        assert h[12:15]==(47136,32,n) and x==0
        matches=all(i==k+512 and q==(0x8000|n) for k,(i,q) in enumerate(struct.iter_unpack('<HH',raw[32:])))
        assert matches==pattern
        assert f['chunk_crc']=='PASS'
    if not pattern:
        assert len({f['payload_sha256'] for f in j['frames']})==10
        assert (p/'last_frame_ddr.bin').read_bytes()==(p/'frame_9.bin').read_bytes()
        assert json.loads((p/'ddr_uart_comparison.json').read_text())['identical']
for file in ET.parse(R/'radar_sync.xpr').iter('File'):
    path=file.attrib.get('Path','').replace('$PPRDIR',str(R)).replace('$PSRCDIR',str(R/'radar_sync.srcs'))
    assert Path(path).is_file(),path
print('SPI_FOUR_WIRE_EVIDENCE_PASS: archived selftest=3 real=10, CRC result logs, exact patterns, hashes, headers, DDR/UART and Vivado inputs consistent')
print('This does not run the MCU, Vivado implementation or a radar measurement.')
