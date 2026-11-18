"""Wire-format decoder tests using bytes emitted by the actual MCU C module."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent.parent/'scripts'))
from export_uart_frame import decode
root=Path(__file__).resolve().parent.parent

def text(raw):return f'BEGIN RADAR_RAW bytes={len(raw)}\r\n'+raw.hex(' ')+'\r\nEND RADAR_RAW\r\n'

class ExportTests(unittest.TestCase):
    def setUp(self):self.raw=(root/'evidence/mcu_frame_0.bin').read_bytes()
    def test_c_frame(self):
        raw,meta=next(decode(text(self.raw)))
        self.assertEqual(raw,self.raw);self.assertEqual((meta['sequence'],meta['beam_index'],meta['angle_cdeg']),(0,9,0))
    def test_truncation(self):
        with self.assertRaises(ValueError):list(decode(text(self.raw[:-1])))
    def test_bad_xor(self):
        raw=bytearray(self.raw);raw[20]^=1
        with self.assertRaises(ValueError):list(decode(text(raw)))
    def test_angle_with_valid_xor(self):
        raw=bytearray(self.raw);raw[8]^=1;raw[30]^=1
        with self.assertRaises(ValueError):list(decode(text(raw)))

if __name__=='__main__':unittest.main()
