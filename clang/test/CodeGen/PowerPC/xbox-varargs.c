// RUN: %clang_cc1 -triple ppc32-unknown-xbox360 -emit-llvm -o - %s | FileCheck %s
// Xbox has a pointer va_list and positional eight-byte homes, not ELF's struct.
typedef __builtin_va_list va_list;
_Static_assert(sizeof(va_list) == 4, "Xbox va_list is a 32-bit pointer");
_Static_assert(sizeof(void *) == 4, "Xbox pointers remain 32-bit");
_Static_assert(sizeof(long long) == 8, "Xbox integers may be 64-bit");

// CHECK-LABEL: define{{.*}} i32 @read_int
// CHECK: getelementptr inbounds i8, ptr {{.*}}, i32 8
// CHECK: getelementptr inbounds i8, ptr {{.*}}, i32 4
int read_int(int first, ...) {
  va_list ap;
  __builtin_va_start(ap, first);
  int result = __builtin_va_arg(ap, int);
  __builtin_va_end(ap);
  return result;
}

// CHECK-LABEL: define{{.*}} i64 @read_wide
// CHECK: getelementptr inbounds i8, ptr {{.*}}, i32 8
long long read_wide(int first, ...) {
  va_list ap;
  __builtin_va_start(ap, first);
  long long result = __builtin_va_arg(ap, long long);
  __builtin_va_end(ap);
  return result;
}
