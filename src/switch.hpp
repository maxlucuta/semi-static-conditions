#ifndef SWITCH_HPP
#define SWITCH_HPP

#include <sys/mman.h>
#include <type_traits>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

#define DWORD 4
#define QWORD 8
#define N 4
#define JMP_OPCODE 0xe9


template <typename Ret, typename... Args>
class BranchChangerSwitch
{
    using func = Ret(*)(Args...);

    private:

        unsigned char* bytecodeToEdit;
        unsigned char jumpOffsets[N][DWORD];

        template <typename funcA, typename funcB>
        intptr_t computeJumpOffset(const funcA from, const funcB to) const
        {   
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wpmf-conversions"
            void* fromAddress = reinterpret_cast<void*>(from);
            void* toAddress = reinterpret_cast<void*>(to);
            return (char*)fromAddress - (char*)toAddress - 5;
            #pragma GCC diagnostic pop
        }

        void changePagePermissions(const unsigned char* address)
        {   
            intptr_t pageSize = getpagesize();
            intptr_t relativeAddress = reinterpret_cast<intptr_t>(address);
            relativeAddress -= relativeAddress % pageSize;
            void* pageOffset = reinterpret_cast<void*>(relativeAddress);
            mprotect(pageOffset, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);
        }

        void storeOffsetToBranch(const intptr_t& offset, unsigned char* dst)
        {
            unsigned char offsetInBytes[DWORD] = {
                static_cast<unsigned char>(offset & 0xff),
                static_cast<unsigned char>((offset >> 8) & 0xff),
                static_cast<unsigned char>((offset >> 16) & 0xff),
                static_cast<unsigned char>((offset >> 24) & 0xff)
            };
            #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            std::reverse(offsetInBytes, offsetInBytes + DWORD);
            #endif
            std::memcpy(dst, offsetInBytes, DWORD);
        }

    public:

        template <typename... func>
        BranchChangerSwitch(const func&&... args)
        {   
            const auto self = &BranchChangerSwitch::branch;
            bytecodeToEdit = (unsigned char*)self;
            std::vector<typename std::common_type<func...>::type> vec = {args...};
            for (int i = 0; i < N; i++)
            {
                intptr_t jumpOffsetToBranch = computeJumpOffset(vec[i], self);
                storeOffsetToBranch(jumpOffsetToBranch, jumpOffsets[i]);
            }
            changePagePermissions(bytecodeToEdit);
            *bytecodeToEdit++ = JMP_OPCODE;
            setDirection(1);
        }

        __attribute__((optimize("O0")))
        static Ret branch(Args... args)
        {   
            if constexpr (!std::is_void_v<Ret>)
            {
                return Ret{};
            }
        }

        void setDirection(int condition)
        {
            std::memcpy(bytecodeToEdit, jumpOffsets[condition], DWORD);
        }
};


#endif 