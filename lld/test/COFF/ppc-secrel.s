# REQUIRES: ppc
# RUN: split-file %s %t
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/main.s -o %t/main.obj
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/data.s -o %t/data.obj
# RUN: lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /base:0x82000000 /debug:dwarf /out:%t.exe %t/main.obj %t/data.obj
# RUN: llvm-readobj --sections --section-data %t.exe | FileCheck %s
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/absolute.s -o %t/absolute.obj
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/absolute-definition.s -o %t/absolute-definition.obj
# RUN: not lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /debug:dwarf /out:%t-absolute.exe %t/absolute.obj %t/absolute-definition.obj 2>&1 | FileCheck %s --check-prefix=ABS
# Section offsets must retain big-endian addends, without adding imageBase.
# CHECK: Name: .debug_info
# CHECK: SectionData (
# CHECK-NEXT: 0000: 01020304 0102030C 55667788
# ABS: error: SECREL relocation cannot be applied to absolute symbols
#--- main.s
.text
.globl main
main:
blr
.data
.long 0
Ltarget:
.long 7
.section .debug_info,"dr"
.secrel32 Ltarget + 0x01020300
.secrel32 target + 0x01020300
.long 0x55667788
#--- data.s
.data
.long 0
.globl target
target:
.long 9
#--- absolute.s
.text
.globl main
main:
blr
.globl absolute
.section .debug_info,"dr"
.secrel32 absolute
#--- absolute-definition.s
.globl absolute
absolute = 0x1234
