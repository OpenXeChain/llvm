# REQUIRES: ppc
# RUN: split-file %s %t
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/main.s -o %t/main.obj
# RUN: llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/data.s -o %t/data.obj
# RUN: lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /base:0x82000000 /align:65536 /opt:ref,icf /export:cb1 /export:cb2 /out:%t.exe %t/main.obj %t/data.obj
# RUN: llvm-readobj --sections --section-data %t.exe | FileCheck %s
# RUN: llvm-readobj --coff-exports %t.exe | FileCheck %s --check-prefix=ICF
# RUN: %python %t/bad-pair.py %t/main.obj %t/bad.obj
# RUN: not lld-link /entry:main /subsystem:xbox360 /nodefaultlib /fixed /out:%t-bad.exe %t/bad.obj %t/data.obj 2>&1 | FileCheck %s --check-prefix=PAIR
# RUN: not llvm-mc -triple=ppc32-unknown-xbox360 -filetype=obj %t/high.s -o %t/high.obj 2>&1 | FileCheck %s --check-prefix=HIGH
# External and temporary-symbol addends, adjusted-high carry, and resolved
# symbol differences must affect immediate fields without changing registers.
# CHECK: Name: .text
# CHECK: 0000: 38630004 3884FFFC 3CA08203 38A58000
# CHECK: 0010: 3CC08204 38C68000 3CE08202 38E78000
# CHECK: 0020: 3D008205 39088004 3D208204 39297FFC
# CHECK: 0030: 394A0004
# ICF: Name: cb1
# ICF-NEXT: RVA: 0x10038
# ICF: Name: cb2
# ICF-NEXT: RVA: 0x10040
# PAIR: error: missing PowerPC REFHI PAIR relocation
# HIGH: error: unsupported PowerPC COFF unadjusted high relocation
#--- main.s
.text
.globl main
main:
addi 3,3,target+4@l
addi 4,4,target-4@l
lis 5,target+0x8000@ha
addi 5,5,target+0x8000@l
lis 6,target+0x18000@ha
addi 6,6,target+0x18000@l
lis 7,target-0x8000@ha
addi 7,7,target-0x8000@l
lis 8,1f+4@ha
addi 8,8,1f+4@l
lis 9,1f-4@ha
addi 9,9,1f-4@l
.Lstart:
addi 10,10,(.Lend-.Lstart)
.Lend:
blr
.data
.space 0x18000
1:
.long 0
.section .text$A,"xr",discard,cb1
.globl cb1
cb1:
lis 3,target+4@ha
blr
.section .text$B,"xr",discard,cb2
.globl cb2
cb2:
lis 3,target-4@ha
blr
#--- data.s
.globl target
target=0x82020000
#--- high.s
.text
lis 3,target@h
#--- bad-pair.py
import struct,sys
data=bytearray(open(sys.argv[1],'rb').read())
count=struct.unpack_from('<H',data,2)[0]
for i in range(count):
    section=20+i*40
    relocs=struct.unpack_from('<I',data,section+24)[0]
    countrel=struct.unpack_from('<H',data,section+32)[0]
    for j in range(countrel):
        offset=relocs+j*10+8
        if struct.unpack_from('<H',data,offset)[0]==0x12:
            struct.pack_into('<H',data,offset,0)
            struct.pack_into('<I',data,offset-4,0)
            open(sys.argv[2],'wb').write(data)
            sys.exit()
raise RuntimeError('No PAIR found')
