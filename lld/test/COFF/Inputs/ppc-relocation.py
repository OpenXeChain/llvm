"""Build a minimal big-endian PowerPC COFF object with a chosen relocation.

The COFF metadata is little endian; the PowerPC instruction bytes are big
endian. No Microsoft SDK or target compiler is needed to generate the fixture.
"""
from pathlib import Path
import struct
import sys

output = Path(sys.argv[1])
relocation = int(sys.argv[2], 0)
header = struct.pack("<HHIIIHH", 0x1F2, 1, 0, 74, 1, 0, 0)
section = struct.pack("<8sIIIIIIHHI", b".text", 0, 0, 4, 60, 64, 0, 1, 0,
                      0x60000020)
instruction = bytes.fromhex("4e800020")  # blr
reloc = struct.pack("<IIH", 0, 0, relocation)
symbol = struct.pack("<8sIhHBB", b"main", 0, 1, 0x20, 2, 0)
strings = struct.pack("<I", 4)
output.write_bytes(header + section + instruction + reloc + symbol + strings)
