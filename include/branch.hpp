#ifndef BRANCH_HPP
#define BRANCH_HPP

#include <iostream>
#include "builds/branch_arch.hpp"

#ifdef GCC_BUILD_BRANCH
#include "builds/branch_gcc.hpp"
#elif defined(CLANG_BUILD_BRANCH)
#include "builds/branch_clang.hpp"
#elif defined(MSVC_BUILD_BRANCH)
#include "builds/branch_msvc.hpp"
#endif


template <typename... Funcs>
class BranchChanger :
public branch_changer_aux<typename std::common_type<Funcs...>::type> {

    static_assert(pack_size<Funcs...> > 1);

private:
    uint64_t current_direction;
    unsigned char jump_offsets[pack_size<Funcs...>][OFFSET_];


public:
    explicit BranchChanger(const Funcs... funcs) : current_direction(-1) {
        std::vector<typename std::common_type<Funcs...>::type> pack = { funcs... };
        for (int i = 0; i < (int)pack.size(); i++) {
            intptr_t offset = compute_jump_offset(pack[i], this->bytecode_to_edit);
            std::cout << "offset: " << offset << std::endl;
            if (abs(offset) > (JUMP_DISTANCE_) - 1)
                throw branch_changer_error(error_codes::BRANCH_TARGET_OUT_OF_BOUNDS);
            store_offset_as_bytes(offset, jump_offsets[i]);
        }
        change_permissions(this->bytecode_to_edit, permissions::READ_WRITE_EXECUTE);
        *this->bytecode_to_edit++ = JUMP_OPCODE_;
        #ifdef SAFE_MODE
        change_permissions(this->bytecode_to_edit, permissions::READ_EXECUTE);
        #endif
        if (pack.size() == 2) {
            std::swap(jump_offsets[0], jump_offsets[1]);
            set_direction(1);
        } else
            set_direction(0);
    }

    #ifndef SAFE_MODE
    void set_direction(const uint64_t condition){
        if (current_direction != condition) {
            std::memcpy(this->bytecode_to_edit, jump_offsets[condition], OFFSET_);
            current_direction = condition;
        }
    }

    #else
    void set_direction(const uint64_t condition){
        if (current_direction != condition) {
            change_permissions(this->bytecode_to_edit, permissions::READ_WRITE_EXECUTE);
            std::memcpy(this->bytecode_to_edit, jump_offsets[condition], OFFSET_);
            current_direction = condition;
            change_permissions(this->bytecode_to_edit, permissions::READ_EXECUTE);
        }
    }
    #endif
};


template <typename Ret, typename... Args>
uint64_t branch_changer_aux<Ret (*)(Args...)>::instances = 0;


template <typename Class, typename Ret, typename... Args>
uint64_t branch_changer_aux<Ret (Class::*)(Args...)>::instances = 0;


#endif
