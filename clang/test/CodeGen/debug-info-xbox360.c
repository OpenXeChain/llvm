// REQUIRES: powerpc-registered-target
// RUN: %clang -target ppc32-unknown-xbox360 -g -O0 -c %s -o %t
// RUN: llvm-dwarfdump --verify %t
// RUN: llvm-dwarfdump --debug-info %t | FileCheck %s
// RUN: %clang -target ppc32-unknown-xbox360 -gdwarf-4 -O2 -c %s -o %t
// RUN: llvm-dwarfdump --verify %t
// RUN: llvm-dwarfdump --debug-info %t | FileCheck %s
// CHECK: DW_TAG_compile_unit
// CHECK: DW_AT_name{{.*}}("add")
int global = 7;
int add(int value) { return value + global; }
int main(void) { return add(3); }
