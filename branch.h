#ifndef BRANCH_H
#define BRANCH_H


#include <unistd.h>
#include <sys/mman.h>
#include <algorithm>
#include <type_traits>
#include <cstring>


#define PAGE_SIZE 4096
#define JMP_OPCODE 0xE9
#define DWORD 4


template <typename Ret, typename... Args>
class BranchChanger
{   
    /* Changes the direction of a branch programatically at runtime, facilitating
       low-overhead function calls that can be switched by the user. The switching
       process exploits code segment organisation to compute relative offsets 
       between functions, and alters function calls in memory using an adapted
       trampoline method. */

    typedef Ret(*FUNC)( Args... );

    private:

        unsigned char * bytecode_to_edit_;
        unsigned char bytecode_to_insert_[2][DWORD];

        constexpr std::intptr_t compute_offset(const FUNC from, const FUNC to) const
        {
            /* Computes the relative offset between two instructions in memory, this
               will be used the computating the offset for a jump instruction between 
               areas in memory. Note, we need to subtract 5 since the length of a
               relative jmp instruction is 5 bytes. */

            return (char *) from - (char *) to - 5;
        }

        void change_page_permissions(const unsigned char *address)
        {
            /* Changes the permissions on the current page that the address resides on
               to allow it to be read, write and executable. This will allow us to 
               modify the direction of the call instruction at runtime. */

            std::intptr_t relative_address = reinterpret_cast<std::intptr_t>(address);
            relative_address -= relative_address % PAGE_SIZE;
            void * page_offset = reinterpret_cast<void *>(relative_address);
            if (mprotect(page_offset, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC ) == -1) 
            { 
                throw std::runtime_error("Could not change page permissions.");
            }
        }

        void set_address_bytes(const std::intptr_t &offset, unsigned char *destination)
        {
            /* Sets a vector of bytes that correspond to relative offset that the call
               instruction must jump to for the specified branch. We will check for the
               endianess of the current system to ensure correct byte ordering. */

            unsigned char offset_bytes[4] = {
                static_cast<unsigned char>(offset & 0xFF),
                static_cast<unsigned char>((offset >> 8) &0xFF),
                static_cast<unsigned char>((offset >> 16) &0xFF),
                static_cast<unsigned char>((offset >> 24 ) &0xFF)
            };
            #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            std::reverse(offset_bytes, offset_bytes + DWORD);
            #endif
            memcpy(destination, offset_bytes, DWORD);
        }

        void prepare_branch(bool condition)
        {   
            /* Prepares the 'branch' method preemptively to minimise runtime overhead. The
               branch is set to the 'true' direction by default, however this can be changed
               so it can be user defined if desired. The first byte of 'branch' is changed
               to a JMP opcode, so at runtime all that needs to be changed is a DWORD. */

            change_page_permissions(bytecode_to_edit_);
            *bytecode_to_edit_++ = JMP_OPCODE;
            set_direction(condition);
        }

    public:

        BranchChanger(const FUNC if_func, const FUNC else_func, bool condition = true) : 
        bytecode_to_edit_((unsigned char *) &BranchChanger::branch)
        {
            /* Pre-computes relative addresses and bytes that correspond to jump addresses,
               overwrites page permissions with linux system calls to allow modification of
               the executable while the programme runs. */

            
            std::intptr_t offset_to_if = compute_offset(if_func, &BranchChanger::branch);
            std::intptr_t offset_to_else = compute_offset(else_func, &BranchChanger::branch);
            set_address_bytes(offset_to_else, *bytecode_to_insert_);
            set_address_bytes(offset_to_if, *(bytecode_to_insert_ + 1));
            prepare_branch(condition);
        }
        
        __attribute__((optimize("O0")))
        static Ret branch(Args... args)
        {
            /* This is the trampoline function that serves as the entry point in
               in main. The method has to be declared static so it has the same 
               calling convention as a regular non-class member function (rdi 
               pushed onto stack with 'this' pointer). Should have the same signature 
               as the branch methods so the compiler can set up the stack frame 
               appropriately. Note this method will never fully execute, its first 5 
               bytes will be overwritten with a JMP instruction to one of two methods. */

            if constexpr (!std::is_void_v<Ret>) 
            { 
                return Ret{}; 
            }
        }

        void set_direction(bool condition) 
        {
            /* Programatically alters the relative JMP offset in memory depending on
               the runtime condition (DWORD copy into executable code). */
            
            std::memcpy(bytecode_to_edit_, bytecode_to_insert_[condition], DWORD);
        }
};


#endif