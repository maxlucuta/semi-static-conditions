#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define cast(T, expr) reinterpret_cast<T>(expr)

typedef int(*func)(int, int);

class BranchChanger {
    /* Changes the direction of a branch programatically at runtime, facilitating
       low overhead function calls that can be switched by the user. The switching
       process exploits code segment organisation to compute relative offsets 
       between functions, and alters function calls in memory using an adapted
       trampoline method. */

    private:

        const func if_branch;
        const func else_branch;
        unsigned char * branch_method_bytecode;
        const unsigned char JMP_OPCODE = 0xe9;
        const int JMP_INSTRUCTION_LENGTH = 4;
        std::array<unsigned char, 4> if_branch_bytecode;
        std::array<unsigned char, 4> else_branch_bytecode;
    
        intptr_t _compute_offset(const void * from_ptr, const void * to_ptr) const {
            /* Computes the relative offset between two instructions in memory, this
               will be used for facilitating the computation of jump offsets between 
               areas in memory. Note, we need to subtract 5 since the length of a
               relative jmp instruction is 5 bytes. */

            return cast(intptr_t, from_ptr) - cast(intptr_t, to_ptr) - 5;
        }

        void _change_page_permissions(const void * address) const {
            /* Changes the permissions on the current page that the address resides on
               to allow it to be read, write and executable. This will allow us to 
               modify the direction of the call instruction at runtime. */

            int page_size = getpagesize();
            uintptr_t uintptr_address = cast(uintptr_t, address);
            uintptr_address -= uintptr_address % page_size;
            void* offset = cast(void*, uintptr_address);
            if(mprotect(offset, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) { 
                throw std::runtime_error("Could not change page permissions.");
            }
        }

        std::array<unsigned char, 4> _get_address_bytes(const intptr_t & num) const {
            /* Returns a vector of bytes that correspond to relative offset that the call
               instruction must jump to for the specified branch. We will check for the
               endianess of the current system to ensure correct byte ordering. */

            std::array<unsigned char, 4> arr;
            arr[3] = static_cast<unsigned char>((num >> 24) & 0xFF);
            arr[2] = static_cast<unsigned char>((num >> 16) & 0xFF);
            arr[1] = static_cast<unsigned char>((num >> 8) & 0xFF);
            arr[0] = static_cast<unsigned char>(num & 0xFF);

            #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
                std::reverse(arr.begin(), arr.end());
            #endif

            return arr;
        }

    public:

        BranchChanger(const func & if_branch, const func & else_branch) :
        if_branch(if_branch), else_branch(else_branch) {
            /* Pre-computes relative addresses and bytes that correspond to jump addresses,
               overwrites page permissions with linux system calls to allow modification of
               the executable while the programme runs. */

            const void * branch_ptr = cast(const void *, &BranchChanger::branch);
            intptr_t offset_to_if = _compute_offset(cast(void *, if_branch), branch_ptr);
            intptr_t offset_to_else = _compute_offset(cast(void *, else_branch), branch_ptr);
            if_branch_bytecode = _get_address_bytes(offset_to_if);
            else_branch_bytecode = _get_address_bytes(offset_to_else);
            branch_method_bytecode = (unsigned char *)branch_ptr;
            _change_page_permissions(branch_ptr);
            branch_method_bytecode[0] = JMP_OPCODE;
            branch_method_bytecode++; 
        }

        static int branch(int a, int b) {
            /* This is the trampoline function that serves as the entry point in
               in main. The method has to be declared static (for now) so it
               has the same calling convention as a regular non-class member 
               function (rdi pushed onto stack with 'this' pointer). Should have
               the same signature as the branch methods so the compiler can set
               up the stack frame appropriately. Note this method will never
               fully execute, its first 5 bytes will be overwritten with a JMP
               instruction to one of two methods. */

            /* TODO: find a way to make non-static while obeying regular function
               calling convention. */

            return 0;
        }

        void set_direction(bool condition) {
            /* Here, we will programatically edit the first 5 bytes of the branch
               method to be a JMP instruction to either of the functions, based on
               the runtime condition. */

            for (int i = 0; i < JMP_INSTRUCTION_LENGTH; i++) {
                if (condition) { branch_method_bytecode[i] = if_branch_bytecode[i]; }
                else { branch_method_bytecode[i] = else_branch_bytecode[i]; }
            }

            /* TODO: Optimise with bit manipulation to reduce overhead. */
        }
};


int add(int a, int b) {
    /* Placeholder testing function: if_branch. */
    return a + b;   
}

int sub(int a, int b) {
    /* Placeholder testing function: else_branch. */
    return a - b;
}


int main() {

    BranchChanger branch = BranchChanger(add, sub);
    branch.set_direction(true);
    std::cout << branch.branch(1,2) << std::endl;
    branch.set_direction(false);
    std::cout << branch.branch(1,2) << std::endl;
    return 0;

}





