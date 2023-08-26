#ifndef BRANCH_ARCH_HPP
#define BRANCH_ARCH_HPP


#if defined(__clang__)
#define CLANG_BUILD_BRANCH
#elif defined(__GNUC__) || defined(__GNUG__)
#define GCC_BUILD_BRANCH
#elif defined(_MSC_VER)
#define MSVC_BUILD_BRANCH
#else
#error "Compiler build not supported."
#endif


#if defined(GCC_BUILD_BRANCH)  || defined(CLANG_BUILD_BRANCH)
    #ifdef __x86_64__
    #define X86_BUILD_BRANCH
    #elif defined(__aarch64__)
    #define ARM_BUILD_BRANCH
    #else
    #error "Architecture not supported."
    #endif
#else
    #ifdef _M_IX86
    #define X86_BUILD_BRANCH
    #elif defined(_M_ARM64)
    #define ARM_BUILD_BRANCH
    #else
    #error "Architecture not supported."
    #endif
#endif


#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS_BRANCH
#elif defined(__linux__) 
#define PLATFORM_LINUX_BRANCH
#error "Operating system not supported."
#endif


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(PLATFORM_WINDOWS_BRANCH)
#define LITTLE_ENDIAN_BRANCH
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BIG_ENDIAN_BRANCH
#else
#error "Unable to determine byte ordering."
#endif


#ifdef X86_BUILD_BRANCH
#define JUMP_INSTRUCTION asm ("jmp 0x0");
#define INSTRUCTION_SIZE 5
#define JUMP_OPCODE_ 0xE9
#define JUMP_DISTANCE_ 1ULL << 32
#define OFFSET_ 4
#elif defined(ARM_BUILD_BRANCH)
#define JUMP_INSTRUCTION asm ("b 0x0");
#define JUMP_OPCODE_ 0xEA
#define JUMP_DISTANCE_ 1ULL << 24
#define INSTRUCTION_SIZE 4
#define OFFSET_ 3
#endif


#endif