#ifndef OS_UTILS_H
#define OS_UTILS_H

#include "platform.hpp"

#ifdef PLATFORM_WINDOWS

    #include <Windows.h>

    size_t getpagesize() {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        return systemInfo.dwPageSize;
    }

#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
    #include <sys/mman.h>
    #include <unistd.h>
#endif


#endif 