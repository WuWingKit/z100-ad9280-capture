"""Convert frames emitted by the real portable C serializer into SPI test words."""
import struct
from pathlib import Path
root=Path(__file__).resolve().parent.parent
frames=[(root/'evidence'/f'mcu_frame_{i}.bin').read_bytes() for i in range(2)]
assert all(len(x)==47136 for x in frames)
words=struct.unpack('<47136H',b''.join(frames))
(root/'evidence/mcu_reference_words.hex').write_text(''.join(f'{v:04X}\n' for v in words),encoding='ascii')
print('C_REFERENCE_VECTORS_READY frames=2 bytes=94272')
