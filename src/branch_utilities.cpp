
#include "builds/branch_utilities.hpp"


#ifdef PLATFORM_WINDOWS_BRANCH

#include <Windows.h>

void change_permissions(const unsigned char* address, const permissions& config) {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    intptr_t page_size = systemInfo.dwPageSize;
    auto relative_addr = reinterpret_cast<intptr_t>(address);
    relative_addr -= relative_addr % page_size;
    void* page_offset = reinterpret_cast<void*>(relative_addr);
    DWORD oldProtect;
    if (config == permissions::READ_WRITE_EXECUTE) {
        if (!VirtualProtect(page_offset, page_size, PAGE_EXECUTE_READWRITE, &oldProtect))
            throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
    } else {
        if (!VirtualProtect(page_offset, page_size, PAGE_EXECUTE_READ, &oldProtect))
            throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
    }
}

#elif defined(PLATFORM_LINUX_BRANCH)

#include <sys/mman.h>
#include <unistd.h>

void change_permissions(const unsigned char* address, const permissions& config) {
    intptr_t page_size = getpagesize();
    auto relative_addr = reinterpret_cast<intptr_t>(address);
    relative_addr -= relative_addr % page_size;
    void* page_offset = reinterpret_cast<void*>(relative_addr);
    if (config == permissions::READ_WRITE_EXECUTE) {
        if (mprotect(page_offset, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
            throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
    } else {
        if (mprotect(page_offset, page_size, PROT_READ | PROT_EXEC) == -1)
            throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
    }
}

#endif


#ifdef X86_BUILD_BRANCH

void store_offset_as_bytes(const intptr_t& offset, unsigned char* dst) {

    #ifdef LITTLE_ENDIAN_BRANCH
    unsigned char offset_in_bytes[OFFSET_] = {
            static_cast<unsigned char>(offset & 0xff),
            static_cast<unsigned char>((offset >> 8) & 0xff),
            static_cast<unsigned char>((offset >> 16) & 0xff),
            static_cast<unsigned char>((offset >> 24) & 0xff)
    };

    #elif defined(BIG_ENDIAN_BRANCH)
    unsigned char offset_in_bytes[OFFSET_] = {
        static_cast<unsigned char>((offset >> 24) & 0xff),
        static_cast<unsigned char>((offset >> 16) & 0xff),
        static_cast<unsigned char>((offset >> 8) & 0xff),
        static_cast<unsigned char>(offset & 0xff)
    };

    #endif
    std::memcpy(dst, offset_in_bytes, OFFSET_);
}

#elif defined(ARM_BUILD_BRANCH)

void store_offset_as_bytes(const intptr_t& offset, unsigned char* dst) {

    #ifdef LITTLE_ENDIAN_BRANCH
    unsigned char offset_in_bytes[OFFSET_] = {
            static_cast<unsigned char>(offset & 0xff),
            static_cast<unsigned char>((offset >> 8) & 0xff),
            static_cast<unsigned char>((offset >> 16) & 0xff),
    };

    #elif defined(BIG_ENDIAN_BRANCH)
    unsigned char offset_in_bytes[OFFSET_] = {
        static_cast<unsigned char>((offset >> 16) & 0xff),
        static_cast<unsigned char>((offset >> 8) & 0xff),
        static_cast<unsigned char>(offset & 0xff)
    };

    #endif
    std::memcpy(dst, offset_in_bytes, OFFSET_);
}

#endif


