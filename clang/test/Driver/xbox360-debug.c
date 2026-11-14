// RUN: %clang --target=ppc32-unknown-xbox360 -### -g %s 2>&1 | FileCheck %s --check-prefix=LINK
// RUN: %clang --target=ppc32-unknown-xbox360 -### -gdwarf-4 %s 2>&1 | FileCheck %s --check-prefix=LINK
// RUN: %clang --target=ppc32-unknown-xbox360 -### -g -g0 %s 2>&1 | FileCheck %s --check-prefix=NONE
// RUN: %clang --target=ppc32-unknown-xbox360 -### -g -ggdb0 %s 2>&1 | FileCheck %s --check-prefix=NONE
// RUN: %clang --target=ppc32-unknown-xbox360 -### -ggdb0 -g %s 2>&1 | FileCheck %s --check-prefix=LINK
// LINK: "/DEBUG:DWARF"
// NONE-NOT: "/DEBUG:DWARF"
// NONE: "/SUBSYSTEM:xbox360"
// NONE-NOT: "/DEBUG:DWARF"
int main(void) { return 0; }
