//
// Created by Max on 8/2/2023.
//

#ifndef SEMI_STATIC_CONDITIONS_PLATFORM_H
#define SEMI_STATIC_CONDITIONS_PLATFORM_H


#ifdef __x86_64__ 
#define JUMP_OPCODE_ 0xe9
#endif
#define JMP_DISTANCE_ 2e32
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

#ifdef COMPILER_MSVC
    #define HOT_ATTR __declspec(allocate(".text"))
    #define NOCF_CHECK_ATTR __declspec(guard(nocf))
    #define OPTIMIZE_ATTR(value) __pragma(optimize(value))
#endif

#ifdef COMPILER_GCC
    #define HOT_ATTR __attribute__((hot))
    #define NOCF_CHECK_ATTR __attribute__((nocf_check))
    #define OPTIMIZE_ATTR(value) __attribute__((optimize(value)))
#endif

#ifdef COMPILER_CLANG
    #define HOT_ATTR __attribute__((hot))
    #define NOCF_CHECK_ATTR __attribute__((noinline))
    #define OPTIMIZE_ATTR(value)
#endif


#endif //SEMI_STATIC_CONDITIONS_PLATFORM_H
