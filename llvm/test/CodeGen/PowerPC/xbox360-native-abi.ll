; RUN: llc -mtriple=ppc32-unknown-xbox360 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=ppc32-unknown-xbox360 -O2 -verify-machineinstrs < %s | FileCheck %s
; Native expectations are independent of LLVM's former PPC32 ELF ABI.
; Xenia kernel/util/shim_utils.h: stack arguments SP+0x54+(position-8)*8.
; AngelScript as_callfunc_xenon.cpp: i64 in one GPR, float shadows a GPR.

declare i32 @native10(i32, i32, i32, i32, i32, i32, i32, i32, i32, i32)

define i32 @call10() {
; CHECK-LABEL: call10:
; CHECK-DAG: stw {{[0-9]+}}, 84(1)
; CHECK-DAG: stw {{[0-9]+}}, 92(1)
  %r = call i32 @native10(i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10)
  ret i32 %r
}

define i32 @get9(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h, i32 %ninth) {
; CHECK-LABEL: get9:
; CHECK: lwz 3, 84(1)
  ret i32 %ninth
}

define i32 @mixed(float %f, i32 %i) {
; CHECK-LABEL: mixed:
; CHECK: mr 3, 4
  ret i32 %i
}

define i32 @wide_high(i32 %pad, i64 %x, i32 %suffix) {
; CHECK-LABEL: wide_high:
; CHECK: {{srdi|rldicl}} 3, 4, 32
  %high = lshr i64 %x, 32
  %r = trunc i64 %high to i32
  ret i32 %r
}

declare i32 @native_wide(i32, i64, i32)
define i32 @call_wide(i64 %x) {
; CHECK-LABEL: call_wide:
; CHECK-DAG: li 5, 99
  %r = call i32 @native_wide(i32 1, i64 %x, i32 99)
  ret i32 %r
}

define void @preserve_gpr() {
; CHECK-LABEL: preserve_gpr:
; CHECK: std 31,
; CHECK: li 31, 0
; CHECK: ld 31,
  call void asm sideeffect "li 31, 0", "~{r31}"()
  ret void
}

define void @frame_pointer() "frame-pointer"="all" {
; CHECK-LABEL: frame_pointer:
; CHECK: std 31,
; CHECK: ld 31,
  ret void
}

declare void @consume(ptr)
define void @dynamic_frame(i32 %n) {
; CHECK-LABEL: dynamic_frame:
; CHECK: std 31,
; CHECK: ld {{[0-9]+}},
  %p = alloca i8, i32 %n, align 16
  call void @consume(ptr %p)
  ret void
}

define i32 @variadic(i32 %first, ...) {
; CHECK-LABEL: variadic:
; CHECK-DAG: std 5,
; CHECK-DAG: std 10,
  %ap = alloca ptr, align 4
  call void @llvm.va_start(ptr %ap)
  %value = va_arg ptr %ap, i32
  call void @llvm.va_end(ptr %ap)
  ret i32 %value
}
declare void @llvm.va_start(ptr)
declare void @llvm.va_end(ptr)

declare void @native_varargs(i32, ...)
define void @call_varargs(i64 %x, double %d) {
; CHECK-LABEL: call_varargs:
  call void (i32, ...) @native_varargs(i32 1, i32 -7, i64 %x, double %d, i32 5, i32 6, i32 7, i32 8, i32 9)
  ret void
}

declare void @take_byval(ptr byval([16 x i8]) align 4)
define void @call_byval(ptr %p) {
; CHECK-LABEL: call_byval:
  call void @take_byval(ptr byval([16 x i8]) align 4 %p)
  ret void
}

declare void @take_two_byval(i32,i32,i32,i32,i32,i32,i32,i32,i32,ptr byval([16 x i8]) align 4,ptr byval([16 x i8]) align 4)
define void @call_two_byval(ptr %p, ptr %q) {
; CHECK-LABEL: call_two_byval:
; CHECK-DAG: stw {{[0-9]+}}, 84(1)
; CHECK-DAG: stw {{[0-9]+}}, 92(1)
; CHECK-DAG: stw {{[0-9]+}}, 100(1)
  call void @take_two_byval(i32 1,i32 2,i32 3,i32 4,i32 5,i32 6,i32 7,i32 8,i32 9,ptr byval([16 x i8]) align 4 %p,ptr byval([16 x i8]) align 4 %q)
  ret void
}

define float @float13(float %a,float %b,float %c,float %d,float %e,float %f,float %g,float %h,float %i,float %j,float %k,float %l,float %m) {
; CHECK-LABEL: float13:
; CHECK: fmr 1, 13
  ret float %m
}

define float @float14(float %a,float %b,float %c,float %d,float %e,float %f,float %g,float %h,float %i,float %j,float %k,float %l,float %m,float %n) {
; CHECK-LABEL: float14:
; CHECK: lfs 1, 124(1)
  ret float %n
}

define i64 @wide9(i64 %a,i64 %b,i64 %c,i64 %d,i64 %e,i64 %f,i64 %g,i64 %h,i64 %n) {
; CHECK-LABEL: wide9:
; CHECK: ld 3, 84(1)
  ret i64 %n
}
