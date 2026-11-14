; RUN: llc -mtriple=powerpc64-unknown-lv2 -mcpu=pwr7 -O0 -verify-machineinstrs < %s | FileCheck %s --check-prefix=PPC64
; RUN: llc -mtriple=powerpc64-unknown-lv2 -mcpu=pwr7 -O2 -ppc-gpr-icmps=all -verify-machineinstrs < %s | FileCheck %s --check-prefix=OPT

; Lv2 has 32-bit pointers and uses the PPC64 instruction selection paths.
define i32 @nonzero_low_word(i64 %x) {
; PPC64-LABEL: nonzero_low_word:
; PPC64-NOT: addic
; PPC64: cmplwi
; PPC64-NOT: subfe
; PPC64: blr
; OPT-LABEL: nonzero_low_word:
; OPT: cntlzw 3, 3
; OPT: srwi 3, 3, 5
; OPT: xori 3, 3, 1
; OPT: blr
  %low = trunc i64 %x to i32
  %test = icmp ne i32 %low, 0
  %result = zext i1 %test to i32
  ret i32 %result
}
