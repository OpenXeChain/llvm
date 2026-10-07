Xbox 360 scalar ABI
===================

The Xbox 360 target uses 32-bit addresses with 64-bit general registers.
It must not use the PPC32 System V argument or va_list conventions.

Implemented scalar contract
---------------------------

* Integer/pointer arguments occupy positional r3-r10; an i64 uses one full GPR.
* Named float/double arguments use sequential f1-f13 while consuming the
  corresponding integer position. ``(float, int)`` uses f1 and r4.
* Parameter homes begin at incoming SP+20 with an eight-byte stride. Argument
  nine starts at SP+84, argument ten at SP+92. Fixed narrow stack arguments
  occupy the beginning of the home, matching the native kernel shim convention.
* Unnamed arguments use full64 GPRs or homes; floating values are double bit
  patterns. va_list is a 32-bit pointer advancing by eight bytes. Narrow
  variadic values are right-adjusted in a big-endian home.
* Integer64 results use r3, floating results f1. Nonvolatile GPRs r14-r31
  are saved/restored at full64 width, including frame and base pointers.
* LR is saved as a 32-bit address at incoming SP-8. Register save areas begin
  below that slot. Stack frames retain the existing 16-byte alignment and
  minimum 96-byte allocation. This is not an implementation of native unwind.
* Tail calls are disabled for this ABI until their stack rules are implemented.

Evidence and limits
-------------------

The scalar layout is checked against Xenia's kernel/util/shim_utils.h and
xboxkrnl/xboxkrnl_strings.cc, AngelScript's as_callfunc_xenon.cpp, and Adisak
Pochanayon's GDC2012 CPU instrumentation handout. See:

* https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/util/shim_utils.h
* https://github.com/xenia-project/xenia/blob/master/src/xenia/kernel/xboxkrnl/xboxkrnl_strings.cc
* https://github.com/anjo76/angelscript/blob/master/sdk/angelscript/source/as_callfunc_xenon.cpp
* https://media.gdcvault.com/gdc2012/slides/Programming%20Track/Adisak_Pochanayon_CPU_Instrumentation_Handouts.pdf

Clang currently retains DefaultABIInfo aggregate classification for internal
compiled C code. Native aggregate argument/return classification, C++ methods,
exceptions, VMX128 and native unwind interoperability are **not verified**.
In particular, small native POD returns may require register classification
that is not implemented here. Do not infer full native C++ compatibility from
the scalar implementation. Unknown split, vector and nested arguments are
rejected rather than assigned System V locations.

CodeGen/PowerPC/xbox-native-abi.ll verifies O0/O2 scalar lowering, preservation,
varargs and internal byval call sequencing. Clang's xbox-varargs.c checks the
pointer va_list representation and access stride. The workspace console test
uses independent assembler in both calling directions and must also be run on
an Xbox; compiling the test alone is not a hardware result.

All target libraries must be rebuilt after changing this ABI. Old objects and
the former temporary XAM argument-nine bridge must not be mixed with it.
