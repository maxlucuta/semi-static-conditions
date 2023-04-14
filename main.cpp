#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

typedef int(*func)(int, int);


class BranchChanger {
    /* Changes the direction of a branch programatically at runtime, facilitating
       low overhead function calls that can be switched by the user. The switching
       process exploits code segment organisation to compute relative offsets 
       between functions, and alters function calls in memory using an adapted
       trampoline method. */

    private:

        const unsigned char CALL_OPCODE = 0xe8;
        const func if_branch;
        const func else_branch;
        const void* call_instr_ptr;
        std::array<unsigned char, 4> if_branch_offset_bytecode;
        std::array<unsigned char, 4> else_branch_offset_bytecode;
    
        
        static int _placeholder_func(int a, int b) { 
            /* Placeholder method to force compiler to set up stack
               frame for when we intercept the call. This will not
               ever be used for funtional purposes. */
               return 0;
        }


        const void* _get_branch_call_ptr(void* branch_ptr, const void* placeholder_ptr) {
            /* Finds the location of the call instruction within 'branch' that calls
               the placeholder function, returning a const pointer to it. In x86
               architecture, the call instruction opcode is 0xe8, however it is not
               guaranteed that an operand doesn't contain this as well. This will need
               to be handled... */

            intptr_t offset = _compute_relative_offset(placeholder_ptr, branch_ptr);
            for (int i = 0; i < (int)offset; i++) {
                unsigned char byte = *(reinterpret_cast<unsigned char*>(branch_ptr) + i);
                if (byte == CALL_OPCODE) { return branch_ptr + i; }
            }
            /* TODO */
            return nullptr;
        }

        intptr_t _compute_relative_offset(const void* from_ptr, const void* to_ptr) {
            /* Computes the relative offset between two instructions in memory, this
               will be used for facilitating the computation of jump offsets between 
               areas in memory. */

            return reinterpret_cast<intptr_t>(from_ptr) - reinterpret_cast<intptr_t>(to_ptr);
        }

        void _change_page_permissions(void* address) {
            /* Changes the permissions on the current page that the address resides on
               to allow it to be read, write and executable. This will allow us to 
               modify the direction of the call instruction at runtime. */

            int page_size = getpagesize();
            address -= (unsigned long)address % page_size;
            if(mprotect(address, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) { 
                throw std::runtime_error("Could not change permissions.");
            }
        }

        std::array<unsigned char, 4> _get_address_bytes(intptr_t num) {
            /* Returns a vector of bytes that correspond to relative offset that the call
               instruction must jump to for the specified branch. */

            std::array<unsigned char, 4> arr;
            arr[3] = static_cast<unsigned char>((num >> 24) & 0xFF);
            arr[2] = static_cast<unsigned char>((num >> 16) & 0xFF);
            arr[1] = static_cast<unsigned char>((num >> 8) & 0xFF);
            arr[0] = static_cast<unsigned char>(num & 0xFF);
            return arr;
        }


    public:
        BranchChanger(const func& if_branch, const func& else_branch) :
        if_branch(if_branch), else_branch(else_branch) {
            /* Pre-computes relative addresses and bytes that correspond to jump addresses,
               overwrites page permissions with linux system calls to allow modification of
               the executable while the programme runs. */

            void* ptr_to_branch = reinterpret_cast<void*>(&BranchChanger::branch);
            const void* ptr_to_set_dir = reinterpret_cast<void*>(&BranchChanger::set_direction);
            call_instr_ptr = _get_branch_call_ptr(ptr_to_branch, ptr_to_set_dir);
            intptr_t offset_to_if = _compute_relative_offset(reinterpret_cast<void*>(if_branch), call_instr_ptr + 5);
            intptr_t offset_to_else = _compute_relative_offset(reinterpret_cast<void*>(else_branch), call_instr_ptr + 5);
            if_branch_offset_bytecode = _get_address_bytes(offset_to_if);
            else_branch_offset_bytecode = _get_address_bytes(offset_to_else);
            _change_page_permissions(ptr_to_branch); 
        }

        int branch(int a, int b) {
            /* Here, we allow the compiler to optimise a direct function call.
            We will use the address of this call to locate the addresses of
            the two branch functions, to which we will compute their address
            and edit the call instruction in memory. */

            return _placeholder_func(a, b);
        }

        void set_direction(bool condition) {
            /* Here, we will programatically edit the call instruction in 'branch'
            based on a runtime condition. This method call is strategically
            placed below 'branch' as the compiler will organise the code segment
            such that this method will always be directly below 'branch'. This
            will allow us to easier locate the instruction to edit. */
            
            unsigned char* address = (unsigned char*)call_instr_ptr + 1;
            for (int i = 0; i < 4; i++) {
                if (condition) { address[i] = if_branch_offset_bytecode[i]; }
                else { address[i] = else_branch_offset_bytecode[i]; }
            }
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





