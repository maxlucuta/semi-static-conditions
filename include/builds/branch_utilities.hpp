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
    void* src_addr = reinterpret_cast<void*>(src);
    void* dst_addr = reinterpret_cast<void*>(dst);
    return (char*)src_addr - (char*)dst_addr - INSTRUCTION_SIZE;
}


enum class permissions {
    READ_WRITE_EXECUTE,
    READ_EXECUTE
};


void change_permissions(const unsigned char* address, const permissions& config);


void store_offset_as_bytes(const intptr_t& offset, unsigned char* dst);


template<typename... Types>
constexpr auto pack_size = [](auto... args) constexpr {
    return sizeof...(Types);
} (Types{}...);


#endif