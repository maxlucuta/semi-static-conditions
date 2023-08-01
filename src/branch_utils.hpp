#ifndef BRANCH_UTILS_HPP
#define BRANCH_UTILS_HPP

#include <type_traits>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "os_utils.hpp"


template <typename Func_A, typename Func_B>
intptr_t _compute_jump_offset(const Func_A src, const Func_B dst) {
       /* Computes offset between two methods in memory accounting for
       size of a JMP instruction in bytes. Pragma directives used to
       hide compiler warnings for void* casts which cannot be avoided
       when dealing with member-function pointers. */

       #pragma GCC diagnostic push
       #pragma GCC diagnostic ignored "-Wpmf-conversions"
       void* src_addr = reinterpret_cast<void*>(src);
       void* dst_addr = reinterpret_cast<void*>(dst);
       return (char*)src_addr - (char*)dst_addr - 5;
       #pragma GCC diagnostic pop
}


void _change_permissions(const unsigned char* address);
	/* Locates the page in memory where a address resides and makes
       the page accessible to write operations, allowing for binary
       editing. */

void _store_offset_as_bytes(const intptr_t& offset, unsigned char* dst);
	/* Changes numerical represention of offset to 4-byte hexadecimal
       and stores it in intermediate array which will be indexed when
       changing the direction of the branch. */


template<typename... Types>
constexpr auto pack_size = [](auto... args) constexpr {
       /* Computes the size of a variadic parameter pack using recursive
       unrolling through lambdas (C++20 feature). */

       return sizeof...(Types);
} (Types{}...);

#endif