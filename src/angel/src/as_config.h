/*
   AngelCode Scripting Library
   Copyright (c) 2003-2021 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/

// this file is used for configuring the compilation of the library

#ifndef AS_CONFIG_H
#define AS_CONFIG_H

//
// Features
//-----------------------------------------

// AS_WINDOWS_THREADS
// If the library should be compiled using windows threads.

// AS_POSIX_THREADS
// If the library should be compiled using posix threads.

// AS_NO_ATOMIC
// If the compiler/platform doesn't support atomic instructions
// then this should be defined to use critical sections instead.

// AS_DEBUG
// This flag can be defined to make the library write some extra output when
// compiling and executing scripts.

// AS_DEPRECATED
// If this flag is defined then some backwards compatibility is maintained.
// There is no guarantee for how well deprecated functionality will work though
// so it is best to exchange it for the new functionality as soon as possible.

// AS_DOUBLEBYTE_CHARSET
// When this flag is defined, the parser will treat all characters in strings
// that are greater than 127 as lead characters and automatically include the
// next character in the script without checking its value. This should be
// compatible with common encoding schemes, e.g. Big5. Shift-JIS is not compatible
// though as it encodes some single byte characters above 127.
//
// If support for international text is desired, it is recommended that UTF-8
// is used as this is supported natively by the compiler without the use for this
// preprocessor flag.

// AS_NO_COMPILER
// Compiles the library without support for compiling scripts. This is intended
// for those applications that will load pre-compiled bytecode and wants to decrease
// the size of the executable.

// AS_NO_EXCEPTIONS
// Define this if exception handling is turned off or not available on the target platform.

// AS_NO_MEMBER_INIT
// Disable the support for initialization of class members directly in the declaration.
// This was as a form to maintain backwards compatibility with versions before 2.26.0
// if the new order of the member initialization caused null pointer exceptions in older
// scripts (e.g. if a base class accessed members of a derived class through a virtual method).

// AS_USE_NAMESPACE
// Adds the AngelScript namespace on the declarations.



//
// Library usage
//------------------------------------------

// ANGELSCRIPT_EXPORT
// This flag should be defined when compiling the library as a lib or dll.

// ANGELSCRIPT_DLL_LIBRARY_IMPORT
// This flag should be defined when using AngelScript as a dll with automatic
// library import.

// ANGELSCRIPT_DLL_MANUAL_IMPORT
// This flag should be defined when using AngelScript as a dll with manual
// loading of the library.




//
// Compiler differences
//-----------------------------------------

// asVSNPRINTF(a,b,c,d)
// Some compilers use different names for this function. You must
// define this macro to map to the proper function.

// ASM_AT_N_T or ASM_INTEL
// You should choose what inline assembly syntax to use when compiling.

// VALUE_OF_BOOLEAN_TRUE
// This flag allows to customize the exact value of boolean true.

// AS_SIZEOF_BOOL
// On some target platforms the sizeof(bool) is 4, but on most it is 1.

// STDCALL
// This is used to declare a function to use the stdcall calling convention.

// AS_NO_MEMORY_H
// Some compilers don't come with the memory.h header file.

// AS_NO_THISCALL_FUNCTOR_METHOD
// Defined if the support for functor methods hasn't been implemented on the platform.



//
// How to identify different compilers
//-----------------------------------------
// Ref: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros

// MS Visual C++
//  _MSC_VER   is defined
//  __MWERKS__ is not defined

// Metrowerks
//  _MSC_VER   is defined
//  __MWERKS__ is defined

// GNU C based compilers
//  __GNUC__   is defined

// CLang/LLVM 
// __clang__ is defined

// Embarcadero C++Builder
//  __BORLANDC__ is defined

// Oracle Solaris Studio (previously known as Sun CC compiler)
// __SUNPRO_CC is defined

// Local (or Little) C Compiler
// __LCC__ is defined
// __e2k__ is not defined

// MCST eLbrus C Compiler
// __LCC__ is defined
// __e2k__ is defined



//
// CPU differences
//---------------------------------------

// AS_USE_DOUBLE_AS_FLOAT
// If there is no 64 bit floating point type, then this constant can be defined
// to treat double like normal floats.

// AS_X86
// Use assembler code for the x86 CPU family

// AS_SH4
// Use assembler code for the SH4 CPU family

// AS_MIPS
// Use assembler code for the MIPS CPU family

// AS_PPC
// Use assembler code for the 32bit PowerPC CPU family

// AS_PPC_64
// Use assembler code for the 64bit PowerPC CPU family

// AS_XENON
// Use assembler code for the Xenon (XBOX360) CPU family

// AS_ARM
// Use assembler code for the ARM CPU family

// AS_ARM64
// Use assembler code for the ARM64/AArch64 CPU family

// AS_SOFTFP
// Use to tell compiler that ARM soft-float ABI
// should be used instead of ARM hard-float ABI

// AS_X64_GCC
// Use GCC assembler code for the X64 AMD/Intel CPU family

// AS_X64_MSVC
// Use MSVC assembler code for the X64 AMD/Intel CPU family

// AS_64BIT_PTR
// Define this to make the engine store all pointers in 64bit words.

// AS_BIG_ENDIAN
// Define this for CPUs that use big endian memory layout, e.g. PPC

// AS_SPARC
// Define this for SPARC CPU family

// AS_E2K
// Define this for MCST Elbrus 2000 CPU family




//
// Target systems
//--------------------------------
// This group shows a few of the flags used to identify different target systems.
// Sometimes there are differences on different target systems, while both CPU and
// compiler is the same for both, when this is so these flags are used to produce the
// right code.

// AS_WIN            - Microsoft Windows
// AS_LINUX          - Linux
// AS_MAC            - Apple Macintosh
// AS_BSD            - BSD based OS (FreeBSD, DragonFly, OpenBSD, etc)
// AS_XBOX           - Microsoft XBox
// AS_XBOX360        - Microsoft XBox 360
// AS_PSP            - Sony Playstation Portable
// AS_PSVITA         - Sony Playstation Vita
// AS_PS2            - Sony Playstation 2
// AS_PS3            - Sony Playstation 3
// AS_DC             - Sega Dreamcast
// AS_GC             - Nintendo GameCube
// AS_WII            - Nintendo Wii
// AS_WIIU           - Nintendo Wii U
// AS_NINTENDOSWITCH - Nintendo Switch
// AS_IPHONE         - Apple IPhone
// AS_ANDROID        - Android
// AS_HAIKU          - Haiku
// AS_ILLUMOS        - Illumos like (OpenSolaris, OpenIndiana, NCP, etc)
// AS_MARMALADE      - Marmalade cross platform SDK (a layer on top of the OS)
// AS_SUN            - Sun UNIX




//
// Calling conventions
//-----------------------------------------

// GNU_STYLE_VIRTUAL_METHOD
// This constant should be defined if method pointers store index for virtual
// functions in the same location as the function pointer. In such cases the method
// is identified as virtual if the least significant bit is set.

// MULTI_BASE_OFFSET(x)
// This macro is used to retrieve the offset added to the object pointer in order to
// implicitly cast the object to the base object. x is the method pointer received by
// the register function.

// HAVE_VIRTUAL_BASE_OFFSET
// Define this constant if the compiler stores the virtual base offset in the method
// pointers. If it is not stored in the pointers then AngelScript have no way of
// identifying a method as coming from a class with virtual inheritance.

// VIRTUAL_BASE_OFFSET(x)
// This macro is used to retrieve the offset added to the object pointer in order to
// find the virtual base object. x is the method pointer received by the register
// function;

// COMPLEX_RETURN_MASK
// This constant shows what attributes determine if an object is returned in memory
// or in the registers as normal structures

// COMPLEX_MASK
// This constant shows what attributes determine if an object is implicitly passed
// by reference or not, even if the argument is declared by value

// THISCALL_RETURN_SIMPLE_IN_MEMORY
// CDECL_RETURN_SIMPLE_IN_MEMORY
// STDCALL_RETURN_SIMPLE_IN_MEMORY
// When these constants are defined then the corresponding calling convention always
// return classes/structs in memory regardless of size or complexity.

// THISCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE
// STDCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE
// CDECL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE
// Specifies the minimum size in dwords a class/struct needs to be to be passed in memory

// CALLEE_POPS_HIDDEN_RETURN_POINTER
// This constant should be defined if the callee pops the hidden return pointer,
// used when returning an object in memory.

// THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER
// This constant should be defined if the callee pops the hidden return pointer
// for thiscall functions; used when returning an object in memory.

// THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
// With this constant defined AngelScript will pass the object pointer on the stack

// THISCALL_CALLEE_POPS_ARGUMENTS
// If the callee pops arguments for class methods then define this constant

// COMPLEX_OBJS_PASSED_BY_REF
// Some compilers always pass certain objects by reference. GNUC for example does
// this if the the class has a defined destructor.

// AS_LARGE_OBJS_PASSED_BY_REF
// If this is defined large objects are passed by reference, whether they are complex or not

// AS_LARGE_OBJ_MIN_SIZE
// This is the size of objects determined as large ones

// AS_CALLEE_DESTROY_OBJ_BY_VAL
// When an object is passed by value the called function is the one responsible
// for calling the destructor before returning.

// HAS_128_BIT_PRIMITIVES
// 64bit processors often support 128bit primitives. These may require special
// treatment when passed in function arguments or returned by functions.

// SPLIT_OBJS_BY_MEMBER_TYPES
// On some platforms objects with primitive members are split over different
// register types when passed by value to functions.



//
// Detect compiler
//------------------------------------------------
#define VALUE_OF_BOOLEAN_TRUE  1
#define STDCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 0
#define CDECL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 0
#define THISCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 0
#define THISCALL_CALLEE_POPS_HIDDEN_RETURN_POINTER

// Not implemented by default. Undefined with tested platforms.
#define AS_NO_THISCALL_FUNCTOR_METHOD


// GNU C (and MinGW or Cygwin on Windows)
// Use the following command to determine predefined macros: echo . | g++ -dM -E -
// MSVC2015 can now use CLang too, but it shouldn't go in here
#if (defined(__GNUC__) && !defined(__SNC__) && !defined(_MSC_VER)) || defined(EPPC) || defined(__CYGWIN__) // JWC -- use this instead for Wii
    #define GNU_STYLE_VIRTUAL_METHOD
    #define MULTI_BASE_OFFSET(x) (*((asPWORD*)(&x)+1))
    #define asVSNPRINTF(a, b, c, d) vsnprintf(a, b, c, d)
    #define CALLEE_POPS_HIDDEN_RETURN_POINTER
    #define COMPLEX_OBJS_PASSED_BY_REF
    #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_ARRAY)
    #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_ARRAY)
    #define AS_NO_MEMORY_H
    #define AS_SIZEOF_BOOL 1
    #define STDCALL __attribute__((stdcall))
    #define ASM_AT_N_T
// MacOSX and IPhone
    #if defined(__APPLE__)

        #include <TargetConditionals.h>

        // Is this a Mac or an IPhone (or other iOS device)?
        #if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1
            #define AS_IPHONE
        #else
            #define AS_MAC
        #endif

        // The sizeof bool is different depending on the target CPU
        #undef AS_SIZEOF_BOOL
        #if defined(__ppc__)
            #define AS_SIZEOF_BOOL 4
            // STDCALL is not available on PPC
            #undef STDCALL
            #define STDCALL
        #else
            #define AS_SIZEOF_BOOL 1
        #endif

        #if (defined(_ARM_) || defined(__arm__))
            // iOS use ARM processor
            #define AS_ARM
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
            #define CDECL_RETURN_SIMPLE_IN_MEMORY
            #define STDCALL_RETURN_SIMPLE_IN_MEMORY
            #define THISCALL_RETURN_SIMPLE_IN_MEMORY

            #undef GNU_STYLE_VIRTUAL_METHOD

            #undef THISCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE
            #undef CDECL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE
            #undef STDCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE

            #define THISCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 2
            #define CDECL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 2
            #define STDCALL_RETURN_SIMPLE_IN_MEMORY_MIN_SIZE 2
            #define COMPLEX_OBJS_PASSED_BY_REF
            #undef COMPLEX_MASK
            #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
            #undef COMPLEX_RETURN_MASK
            #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)

            // iOS uses soft-float ABI
            #define AS_SOFTFP

            // STDCALL is not available on ARM
            #undef STDCALL
            #define STDCALL

        #elif (defined(__aarch64__))
            // The IPhone 5S+ uses an ARM64 processor

            // AngelScript currently doesn't support native calling
            // for 64bit ARM processors so it's necessary to turn on
            // portability mode
            #define AS_MAX_PORTABILITY

            // STDCALL is not available on ARM
            #undef STDCALL
            #define STDCALL

        #elif (defined(i386) || defined(__i386) || defined(__i386__)) && !defined(__LP64__)
            // Support native calling conventions on Mac OS X + Intel 32bit CPU
            #define AS_X86
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
            #define THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
            #undef COMPLEX_MASK
            #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
            #undef COMPLEX_RETURN_MASK
            #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)

        #elif defined(__LP64__) && !defined(__ppc__) && !defined(__PPC__) && !defined(__aarch64__)
            // http://developer.apple.com/library/mac/#documentation/DeveloperTools/Conceptual/LowLevelABI/140-x86-64_Function_Calling_Conventions/x86_64.html#//apple_ref/doc/uid/TP40005035-SW1
            #define AS_X64_GCC
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
            #define HAS_128_BIT_PRIMITIVES
            #define SPLIT_OBJS_BY_MEMBER_TYPES
            #undef COMPLEX_MASK
            #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
            #undef COMPLEX_RETURN_MASK
            #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
            #define AS_LARGE_OBJS_PASSED_BY_REF
            #define AS_LARGE_OBJ_MIN_SIZE 5
            // STDCALL is not available on 64bit Mac
            #undef STDCALL
            #define STDCALL

        #elif (defined(__ppc__) || defined(__PPC__)) && !defined(__LP64__)
            // Support native calling conventions on Mac OS X + PPC 32bit CPU
            #define AS_PPC
            #define THISCALL_RETURN_SIMPLE_IN_MEMORY
            #define CDECL_RETURN_SIMPLE_IN_MEMORY
            #define STDCALL_RETURN_SIMPLE_IN_MEMORY
            #undef COMPLEX_MASK
            #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
            #undef COMPLEX_RETURN_MASK
            #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)

        #elif (defined(__ppc__) || defined(__PPC__)) && defined(__LP64__)
            #define AS_PPC_64
        #else
            // Unknown CPU type
            #define AS_MAX_PORTABILITY
        #endif
        #define AS_POSIX_THREADS

    // Windows
    #elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        // On Windows the simple classes are returned in the EAX:EDX registers
        //#define THISCALL_RETURN_SIMPLE_IN_MEMORY
        //#define CDECL_RETURN_SIMPLE_IN_MEMORY
        //#define STDCALL_RETURN_SIMPLE_IN_MEMORY

        #undef COMPLEX_MASK
        #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
        #undef COMPLEX_RETURN_MASK
        #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)

        #if (defined(i386) || defined(__i386) || defined(__i386__)) && !defined(__LP64__)
            // Support native calling conventions on Intel 32bit CPU
            #define AS_X86
            #undef AS_NO_THISCALL_FUNCTOR_METHOD

            #undef  CALLEE_POPS_HIDDEN_RETURN_POINTER
            #define THISCALL_CALLEE_POPS_ARGUMENTS

        #elif defined(__x86_64__)
            #define AS_X64_MINGW
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
            #define AS_LARGE_OBJS_PASSED_BY_REF
            #define AS_LARGE_OBJ_MIN_SIZE 3
            #define COMPLEX_OBJS_PASSED_BY_REF
        #else
            #define AS_MAX_PORTABILITY
        #endif
        #define AS_WIN
        #define AS_WINDOWS_THREADS
    // Linux
    #elif defined(__linux__) && !defined(ANDROID) && !defined(__ANDROID__)

        #undef COMPLEX_MASK
        #define COMPLEX_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)
        #undef COMPLEX_RETURN_MASK
        #define COMPLEX_RETURN_MASK (asOBJ_APP_CLASS_DESTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_ARRAY)

        #if (defined(i386) || defined(__i386) || defined(__i386__)) && !defined(__LP64__)
            // x86 32bit
            #define THISCALL_RETURN_SIMPLE_IN_MEMORY
            #define CDECL_RETURN_SIMPLE_IN_MEMORY
            #define STDCALL_RETURN_SIMPLE_IN_MEMORY

            // Support native calling conventions on Intel 32bit CPU
            #define THISCALL_PASS_OBJECT_POINTER_ON_THE_STACK
            #define AS_X86
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
        #elif defined(__x86_64__)
            // x86 64bit
            #define AS_X64_GCC
            #undef AS_NO_THISCALL_FUNCTOR_METHOD
            #define HAS_128_BIT_PRIMITIVES
            #define SPLIT_OBJS_BY_MEMBER_TYPES
            #define AS_LARGE_OBJS_PASSED_BY_REF
            #define AS_LARGE_OBJ_MIN_SIZE 5
            // STDCALL is not available on 64bit Linux
            #undef STDCALL
            #define STDCALL
        #endif
        #define AS_LINUX
        #define AS_POSIX_THREADS
    #endif

    #define UNREACHABLE_RETURN
#endif


//
// Detect target hardware
//------------------------------------------------

// Big endian CPU target?
// see: http://sourceforge.net/p/predef/wiki/Endianness/
#if !defined(AS_BIG_ENDIAN) && \
    defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
        #define AS_BIG_ENDIAN
#endif

// Dreamcast and Gamecube use only 32bit floats, so treat doubles as floats
#if defined(__SH4_SINGLE_ONLY__) || defined(_GC)
    #define AS_USE_DOUBLE_AS_FLOAT    // use 32bit floats instead of doubles
#endif

// The assert macro
    #include <assert.h>
    #define asASSERT(x) assert(x)


//
// Internal defines (do not change these)
//----------------------------------------------------------------

#define ARG_W(b)     ((asWORD*)&b)
#define ARG_DW(b)    ((asDWORD*)&b)
#define ARG_QW(b)    ((asQWORD*)&b)
#define ARG_PTR(b)   ((asPWORD*)&b)
#define BCARG_W(b)   ((asWORD*)&(b)[1])
#define BCARG_DW(b)  ((asDWORD*)&(b)[1])
#define BCARG_QW(b)  ((asQWORD*)&(b)[1])
#define BCARG_PTR(b) ((asPWORD*)&(b)[1])

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)

#include "../include/angelscript.h"
#include "as_memory.h"

#ifdef AS_USE_NAMESPACE
using namespace AngelScript;
#endif

#endif
