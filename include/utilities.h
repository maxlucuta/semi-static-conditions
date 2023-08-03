//
// Created by Max on 8/2/2023.
//

#ifndef SEMI_STATIC_CONDITIONS_UTILITIES_H
#define SEMI_STATIC_CONDITIONS_UTILITIES_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <algorithm>

template <typename Func_A, typename Func_B>
intptr_t compute_jump_offset(Func_A src, Func_B dst)
{
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


void change_permissions(const unsigned char* address);
    /* Locates the page in memory where a address resides and makes
    the page accessible to write operations, allowing for binary
    editing. */


void store_offset_as_bytes(const intptr_t& offset, unsigned char* dst);
    /* Changes numerical represention of offset to 4-byte hexadecimal
       and stores it in intermediate array which will be indexed when
       changing the direction of the branch. */


template<typename... Types>
constexpr auto pack_size = [](auto... args) constexpr
{
    /* Computes the size of a variadic parameter pack using recursive
       unrolling through lambdas (C++17 feature). */

    return sizeof...(Types);
} (Types{}...);


enum class error_codes
{
    /* Error handling codes for BranchChanger. */

    BRANCH_TARGET_OUT_OF_BOUNDS,
    MULTIPLE_INSTANCE_ERROR,
    PAGE_PERMISSIONS_ERROR
};


std::string err_to_str(const error_codes& code);
    /* Error code conversion for exceptions. */


class branch_changer_error : public std::runtime_error
{
    /* Runtime exceptions used only, any error that occurs with branch
       changer is non-recoverable and execution must be stopped. */
       
public:
    explicit branch_changer_error(const error_codes& code);
};


#endif //SEMI_STATIC_CONDITIONS_UTILITIES_H
