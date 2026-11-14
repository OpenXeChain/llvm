"""Corrupt an emitted PAIR to exercise the linker's missing-PAIR diagnostic.

Use IMAGE_REL_PPC_ABSOLUTE with a valid symbol index so the input remains a
readable COFF file and reaches REFHI validation.
"""
import struct
import sys
from pathlib import Path

data = bytearray(Path(sys.argv[1]).read_bytes())
section_count = struct.unpack_from('<H', data, 2)[0]
for index in range(section_count):
    section = 20 + index * 40
    relocations = struct.unpack_from('<I', data, section + 24)[0]
    relocation_count = struct.unpack_from('<H', data, section + 32)[0]
    for relocation in range(relocation_count):
        offset = relocations + relocation * 10
        if struct.unpack_from('<H', data, offset + 8)[0] == 0x12:
            struct.pack_into('<H', data, offset + 8, 0)
            struct.pack_into('<I', data, offset + 4, 0)
            Path(sys.argv[2]).write_bytes(data)
            sys.exit(0)
raise RuntimeError('No PAIR found')
