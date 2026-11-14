# REQUIRES: ppc
# RUN: split-file %s %t
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -mcpu=ppc64 -filetype=obj %t/main.s -o %t/main.obj
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/data.s -o %t/data.obj
# RUN: lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /base:0x82000000 /align:65536 /out:%t.exe %t/main.obj %t/data.obj
# RUN: llvm-readobj --sections --section-data %t.exe | FileCheck %s
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/misaligned.s -o %t/misaligned.obj
# RUN: not lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /out:%t-bad.exe %t/main.obj %t/misaligned.obj 2>&1 | FileCheck %s --check-prefix=BAD
# RUN: not llvm-mc -triple=ppc32-unknown-xbox360 -mcpu=ppc64 -filetype=obj %t/modifier.s -o %t/modifier.obj 2>&1 | FileCheck %s --check-prefix=MOD
# RUN: not llvm-mc -triple=ppc32-unknown-xbox360 -mcpu=ppc64 -filetype=obj %t/addend.s -o %t/addend.obj 2>&1 | FileCheck %s --check-prefix=ADDEND
# DS relocations preserve the two low opcode bits and signed addends.
# CHECK: Name: .text
# CHECK: 0000: E8620004 E8820009 F8A2FFFC F8C20001
# BAD: error: misaligned PowerPC ADDR14 relocation
# ADDEND-COUNT-2: error: misaligned PowerPC COFF DS addend
# MOD: error: unsupported PowerPC COFF DS modifier
#--- main.s
.text
.globl main
main:
ld 3, target+4@l(2)
ldu 4, target+8@l(2)
std 5, target-4@l(2)
stdu 6, target@l(2)
blr
#--- data.s
.data
.p2align 3
.globl target
target:
.quad 0
#--- misaligned.s
.data
.byte 0
.globl target
target:
.quad 0
#--- modifier.s
.text
ld 3, target@ha(2)

#--- addend.s
.text
ld 3,target+2@l(2)
std 3,target-2@l(2)
