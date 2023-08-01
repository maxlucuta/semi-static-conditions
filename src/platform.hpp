#ifndef PLATFORM_DETECT_HPP
#define PLATFORM_DETECT_HPP

#define _JUMP_OPCODE 0xe9
#define _JMP_DISTANCE 2e32
#define _DWORD 4

#ifndef __cplusplus
    #error "A C++ compiler is required."
#elif __cplusplus < 201703L
   #error "Library supports only C++17 or later."
#endif

#if defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#elif defined(__APPLE__)
    #define PLATFORM_MAC
#else
    #error "Platform not supported!"
#endif


#if defined(_MSC_VER)
    #define COMPILER_MSVC
#elif defined(__GNUC__)
    #define COMPILER_GCC
#elif defined(__clang__)
    #define COMPILER_CLANG
#else
    #error "Compiler not supported!"
#endif

#endif