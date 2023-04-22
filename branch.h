#ifndef BRANCH_H
#define BRANCH_H


#include <unistd.h>
#include <sys/mman.h>
#include <algorithm>
#include <type_traits>
#include <cstring>


#define PAGE_SIZE 4096
#define JMP_OPCODE 0xE9


template <typename Ret, typename... Args>
class BranchChanger
{   
    /* Changes the direction of a branch programatically at runtime, facilitating
       low-overhead function calls that can be switched by the user. The switching
       process exploits code segment organisation to compute relative offsets 
       between functions, and alters function calls in memory using an adapted
       trampoline method. */

    typedef Ret(*FUNC)( Args... );
    typedef std::array<unsigned char, 4> BYTE_ARR;

    private:

        const void * if_branch_;
        const void * else_branch_;
        const void * branch_ptr_;
        unsigned char * branch_bytecode_;
        std::array<BYTE_ARR, 2> bytecode_;

        constexpr std::ptrdiff_t ComputeOffset( const void * from, const void * to ) const 
        {
            /* Computes the relative offset between two instructions in memory, this
               will be used the computating the offset for a jump instruction between 
               areas in memory. Note, we need to subtract 5 since the length of a
               relative jmp instruction is 5 bytes. */

            const std::ptrdiff_t address_from = reinterpret_cast<const std::ptrdiff_t>( from );
            const std::ptrdiff_t address_to = reinterpret_cast<const std::ptrdiff_t>( to );
            return address_from - address_to - 5;
        }

        constexpr void ChangePagePermissions( const void * address ) const
        {
            /* Changes the permissions on the current page that the address resides on
               to allow it to be read, write and executable. This will allow us to 
               modify the direction of the call instruction at runtime. */

            std::ptrdiff_t relative_address = reinterpret_cast<uintptr_t>(address);
            relative_address -= relative_address % PAGE_SIZE;
            void * page_offset = reinterpret_cast<void *>(relative_address);
            if ( mprotect( page_offset, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC ) == -1 ) 
            { 
                throw std::runtime_error("Could not change page permissions.");
            }
        }

        constexpr BranchChanger::BYTE_ARR GetAddressBytes( const std::ptrdiff_t & num ) const 
        {
            /* Returns a vector of bytes that correspond to relative offset that the call
               instruction must jump to for the specified branch. We will check for the
               endianess of the current system to ensure correct byte ordering. */

            BranchChanger::BYTE_ARR array = {
                static_cast<unsigned char>( num & 0xFF ),
                static_cast<unsigned char>( ( num >> 8 ) & 0xFF ),
                static_cast<unsigned char>( ( num >> 16 ) & 0xFF ),
                static_cast<unsigned char>( ( num >> 24 ) & 0xFF )
            };
            #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            std::reverse( array.begin(), array.end() );
            #endif
            return array;
        }

        constexpr void PrepareBranch( bool condition = true )
        {   
            /* Prepares the 'branch' method preemptively to minimise runtime overhead. The
               branch is set to the 'true' direction by default, however this can be changed
               so it can be user defined if desired. The first byte of 'branch' is changed
               to a JMP opcode, so at runtime all that needs to be changed is a DWORD. */

            ChangePagePermissions( branch_ptr_ );
            branch_bytecode_++[0] = JMP_OPCODE;
            set_direction( condition );
        }

    public:

        BranchChanger(const FUNC if_func, const FUNC else_func) :
        if_branch_( reinterpret_cast<const void*>( if_func ) ), 
        else_branch_( reinterpret_cast<const void*>( else_func ) ),
        branch_ptr_( reinterpret_cast<const void*>( &BranchChanger::branch ) ),
        branch_bytecode_( ( unsigned char * ) branch_ptr_ )
        {
            /* Pre-computes relative addresses and bytes that correspond to jump addresses,
               overwrites page permissions with linux system calls to allow modification of
               the executable while the programme runs. */

            const std::ptrdiff_t offset_to_if = ComputeOffset( if_branch_, branch_ptr_ );
            const std::ptrdiff_t offset_to_else = ComputeOffset( else_branch_, branch_ptr_ );
            bytecode_ = { GetAddressBytes( offset_to_if ), GetAddressBytes( offset_to_else ) };
            PrepareBranch();
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
            
            memcpy(branch_bytecode_, bytecode_[condition].data(), 4);
        }
};


#endif