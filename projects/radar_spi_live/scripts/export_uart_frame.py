"""Decode a saved automatic `c` UART dump (or a repeated `d` dump); no serial port or hardware is opened.
Usage: python scripts/export_uart_frame.py capture.txt --out capture
Outputs one .bin, .json and .csv for each complete header/payload frame.
"""
import argparse
import csv
import json
import re
import struct
from pathlib import Path

def decode(text):
    for match in re.finditer(r"BEGIN RADAR_RAW bytes=(\d+)\s+(.*?)END RADAR_RAW", text, re.S):
        declared = int(match[1])
        tokens = re.findall(r"\b[0-9a-fA-F]{2}\b", match[2])
        raw = bytes(int(v, 16) for v in tokens)
        if len(raw) != declared or len(raw) != 47136:
            raise ValueError("Incomplete frame or unexpected byte count")
        h = struct.unpack_from("<16H", raw)
        xor = 0
        for word in h:
            xor ^= word
        if h[0] != 0xA610 or h[1] != 3 or h[2] != 16 or h[12] != len(raw) or h[13] != 32 or xor:
            raise ValueError("Invalid frame header")
        if h[3] >= 19 or struct.unpack_from('<h',raw,8)[0] != -4500+h[3]*500:
            raise ValueError("Beam index/angle mismatch")
        if (h[5],h[6],h[7],h[8],h[9]) != (19,512,23,24,4):
            raise ValueError("Unexpected sample layout")
        meta = dict(version=h[1], sequence=h[14], beam_index=h[3], angle_cdeg=struct.unpack_from('<h',raw,8)[0],
                    samples_per_chirp=h[6], payload_chirps=h[7], capture_chirps=h[8],
                    resolution_um=h[10] | (h[11]<<16), bytes=len(raw),
                    integrity="Header XOR and length only; payload has no wire CRC")
        yield raw, meta

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('input',type=Path)
    ap.add_argument('--out',type=Path,default=Path('capture'))
    args = ap.parse_args()
    frames = list(decode(args.input.read_text(encoding='utf-8',errors='replace')))
    if not frames:
        raise ValueError("No complete BEGIN RADAR_RAW / END RADAR_RAW frame found")
    for n,(raw,meta) in enumerate(frames):
        prefix=Path(str(args.out)+f'_{n:03d}_seq{meta["sequence"]}')
        prefix.parent.mkdir(parents=True,exist_ok=True)
        prefix.with_suffix('.bin').write_bytes(raw)
        prefix.with_suffix('.json').write_text(json.dumps(meta,indent=2),encoding='utf-8')
        with prefix.with_suffix('.csv').open('w',newline='',encoding='utf-8') as stream:
            writer=csv.writer(stream)
            writer.writerow(['payload_chirp_0based','capture_chirp_0based','sample','i_u16','q_u16','i_s16_interpretation','q_s16_interpretation'])
            for k,offset in enumerate(range(32,len(raw),4)):
                iu,qu=struct.unpack_from('<HH',raw,offset)
                iv,qv=struct.unpack_from('<hh',raw,offset)
                writer.writerow([k//512,k//512+1,k%512,iu,qu,iv,qv])
        print(prefix, 'PASS',meta)

if __name__=='__main__':main()
