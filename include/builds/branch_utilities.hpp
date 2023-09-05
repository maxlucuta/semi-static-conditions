#ifndef BRANCH_UTILITIES_HPP
#define BRANCH_UTILITIES_HPP

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>

#include "branch_arch.hpp"
#include "branch_misc.hpp"


template <typename Func_A, typename Func_B>
intptr_t compute_jump_offset(Func_A src, Func_B dst) {

    /**
     * Args: Function pointers of type Func_A and Func_B (templated).
     * 
     * Ret: intptr_t representing a signed offset.
     * 
     * Computes the offset for an unconditional jump between two
     * functions in memory.
    */

    void* src_addr = reinterpret_cast<void*>(src);
    void* dst_addr = reinterpret_cast<void*>(dst);
    return (char*)src_addr - (char*)dst_addr - INSTRUCTION_SIZE;
}


enum class permissions {
    READ_WRITE_EXECUTE,
    READ_EXECUTE
};


void change_permissions(const unsigned char* address, const permissions& config);

    /**
     * Args: address is a function pointer cast to unsigned chars
     *       config is enum class above denoting the desired permissions
     * 
     * Changes the page permissions for the page which a specific function
     * lies on.
    */


void store_offset_as_bytes(const intptr_t& offset, unsigned char* dst);

    /**
     * Args: intptr_t represents a signed offset
     *       dst represents an array where the converted offset is stored
     * 
     * Converts a numerical offset to a signed 4-byte hex representation
     * and stores it in a specified array. Accounts for architectual byte
     * ordering.
    */

template<typename... Types>
constexpr auto pack_size = [](auto... args) constexpr {

    /**
     * Returns the size of a parameter pack.
    */
   
    return sizeof...(Types);
} (Types{}...);


#endif