# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %s -o %t
# RUN: llvm-readobj --sections --section-data --relocations --expand-relocs %t | FileCheck %s
# A nonzero local-symbol addend used to crash in getFixupKindNumBytes.
# CHECK: Name: .debug_info
# CHECK: SectionData (
# CHECK-NEXT: 0000: 01020304 55667788
# CHECK: Type: Unknown (11)
# CHECK: Symbol: .data
.data
.long 0
Ltarget:
.long 7
.section .debug_info,"dr"
.secrel32 Ltarget + 0x01020300
.long 0x55667788
