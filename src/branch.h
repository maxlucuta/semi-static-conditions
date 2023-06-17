#ifndef BRANCH_H
#define BRANCH_H

#include <sys/mman.h>
#include <type_traits>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <algorithm>

#define DWORD 4
#define QWORD 8
#define JMP_OPCODE 0xe9


class BranchChangerBase
{
    /* Base class for branch changing funcionality, contains all key methods
       used to facilitate binary editing. Contains no functionality regarding
       branch-taking as it requires different template specialisations which
       cannot be achieved via CRTP or other methods. */

    protected:

        unsigned char* bytecodeToEdit;
        unsigned char jumpOffsets[QWORD];

        template <typename funcA, typename funcB>
        intptr_t computeJumpOffset(const funcA from, const funcB to) const
        {   
            /* Computes offset between two methods in memory accounting for
               size of a JMP instruction in bytes. Pragma directives used to
               hide compiler warnings for void* casts which cannot be avoided
               when dealing with member-function pointers. */

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wpmf-conversions"
            void* fromAddress = reinterpret_cast<void*>(from);
            void* toAddress = reinterpret_cast<void*>(to);
            return (char*)fromAddress - (char*)toAddress - 5;
            #pragma GCC diagnostic pop
        }

        void changePagePermissions(const unsigned char* address)
        {   
            /* Locates the page in memory where a address resides and makes
               the page accessible to write operations, allowing for binary
               editing. */

            intptr_t pageSize = getpagesize();
            intptr_t relativeAddress = reinterpret_cast<intptr_t>(address);
            relativeAddress -= relativeAddress % pageSize;
            void* pageOffset = reinterpret_cast<void*>(relativeAddress);
            mprotect(pageOffset, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC);
        }

        void storeOffsetToBranch(const intptr_t& offset, unsigned char* dst)
        {
            /* Changes numerical represention of offset to 4-byte hexadecimal
               and stores it in intermediate array which will be indexed when
               changing the direction of the branch. */

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

        template <typename funcA, typename funcB>
        void prepareBranchChanger(
            const funcA ifBranch, const funcA elseBranch, const funcB self
        )
        {
            /* Essentially the constructor, however extracted to minimise code
               duplication. Takes pointers to if and else branch, as well as a
               pointer to the 'branch' method and does all pre-computations so
               class is ready to be used. Method is only used by base classes. */

            intptr_t jumpOffsetIfBranch = computeJumpOffset(ifBranch, self);
            intptr_t jumpOffsetElseBranch = computeJumpOffset(elseBranch, self);
            storeOffsetToBranch(jumpOffsetIfBranch, (jumpOffsets + DWORD));
            storeOffsetToBranch(jumpOffsetElseBranch, jumpOffsets);
            changePagePermissions(bytecodeToEdit);
            *bytecodeToEdit++ = JMP_OPCODE;
            setDirection(true);
        }

    public:

        void setDirection(bool condition)
        {
            /* Changes direction of the current branch by overwriting relative
               JMP offset with another 4-byte offset. */

            std::memcpy(bytecodeToEdit, jumpOffsets + (condition * DWORD), DWORD);
        }
};


template <typename Ret, typename... Args>
class BranchChanger : public BranchChangerBase
{
    /* Core BranchChanger construct compatible with regular functions and static
       member functions. */

    using func = Ret(*)(Args...);

    public:

        BranchChanger(const func ifBranch, const func elseBranch)
        {   
            const auto self = &BranchChanger::branch;
            bytecodeToEdit = (unsigned char*)self;
            prepareBranchChanger(ifBranch, elseBranch, self);
        }

        __attribute__((optimize("O0")))
        static Ret branch(Args... args)
        {   
            /* Entry point for all branch taking associated with this template. 
               In reality the function never fully executes since the first 5
               bytes are overwritten with a JMP instruction, effectively acting
               as a trampoline. Compile time type checking and return type are
               just syntactic sugar to allow for compilation. */

            if constexpr (!std::is_void_v<Ret>)
            {
                return Ret{};
            }
        }
};


template <typename Class, typename Ret, typename... Args>
class BranchChangerClass : public BranchChangerBase
{
    /* Variation of BranchChanger that allows for the use of non-static class
       member functions only. */

    using func = Ret(Class::*)(Args...);

    public:

        BranchChangerClass(const func ifBranch, const func elseBranch)
        {   
            const auto self = &BranchChangerClass::branch;
            bytecodeToEdit = (unsigned char*)self;
            prepareBranchChanger(ifBranch, elseBranch, self);
        }

        __attribute__((optimize("O0")))
        static Ret branch(const Class& instance, Args... args)
        {   
            /* In case member functions use internal data when called, instance
               of the class needs to be pushed on the stack. */
            
            if constexpr (!std::is_void_v<Ret>)
            {
                return Ret{};
            }
        }
};


#endif