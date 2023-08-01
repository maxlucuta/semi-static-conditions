#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include "platform.hpp"

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

#endif