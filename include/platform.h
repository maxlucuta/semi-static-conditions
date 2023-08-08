//
// Created by Max on 8/2/2023.
//

#ifndef SEMI_STATIC_CONDITIONS_PLATFORM_H
#define SEMI_STATIC_CONDITIONS_PLATFORM_H


#if defined(__x86_64__) 
    #define JUMP_OPCODE_ 0xE9
    #define JMP_DISTANCE_ 1ULL << 32
#elif defined(__arm__)
    #define JUMP_OPCODE_ 0xEA
    #define JMP_DISTANCE_ 1ULL << 24
#endif

#define DWORD_ 4

#ifndef __cplusplus
#error "A C++ compiler is required."
#elif __cplusplus < 201703L
#error "Library supports only C++17 or later."
#endif

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
    #include <Windows.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #include <sys/mman.h>
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <sys/mman.h>
    #include <unistd.h>
    #define PLATFORM_MAC
#else
    #error "Platform not supported!"
#endif



#if (defined(__clang__) || defined(__GNUC__))

    #if defined(__x86_64__)
        #define ASM_INLINE asm("jmp 0x0");
    #else
        #define ASM_INLINE asm("b 0x0");
    #endif

    #ifdef __has_attribute
        #if __has_attribute(optimize)

            #define USING_GCC
            #if __has_attribute(nocf_check)
                #define ATTRIBUTES __attribute__((hot,nocf_check,optimize("no-ipa-cp-clone,O3")))
            #else
                #define ATTRIBUTES __attribute__((hot,optimize("no-ipa-cp-clone,O3")))
            #endif

        #else
            #define USING_CLANG
            #define ATTRIBUTES __attribute__((hot,noinline))
        #endif
    #endif

#elif defined(_MSC_VER)
    #if defined(__x86_64__)
        #define ASM_INLINE __asm{ jmp $+0x0 };
    #else
        #define ASM_INLINE
    #endif
    #define USING_MSVC
    #define ATTR __delspec(noinline)
#else
    #error "Compiler is not supported."
#endif
                


#endif //SEMI_STATIC_CONDITIONS_PLATFORM_H
