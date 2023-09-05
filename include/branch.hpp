#ifndef BRANCH_HPP
#define BRANCH_HPP

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

    /**
     * BranchChanger represents the semi-static conditions language construct,
     * derived from branch_changer_aux which is used to deduce the return and
     * argument types of the supplied function pointers through CRTP.
    */

    static_assert(pack_size<Funcs...> > 1);

private:
    uint64_t current_direction;
    unsigned char jump_offsets[pack_size<Funcs...>][OFFSET_];

    #if defined(GCC_BUILD_BRANCH) || defined(CLANG_BUILD_BRANCH)

    /**
     * SMC machine clears tend to propogate outside of set_direction which is 
     * not ideal, surrounding code will be effected by penalites that are not
     * associated with it. To isolate SMC penalites within set_direction 
     * exclusively, we can insert a ret instruction after the jump, and once
     * we edit, we can execute that ret through casting the bytes to a void*().
     * This forces the CPU to execute part of the function that has been edited,
     * which is sufficient in triggering the SMC clear as a one time penalty so
     * it does not propogate outside of set_direction.
    */

    using functor = void(*)();
    functor force_smc_clear;

    void _initilise_smc_functor() {

        /**
         * Initialises a void(*)() which executes a single ret instruction within
         * the branch method. The functor is called within set_direction.
        */

        unsigned char* branch_copy = this->bytecode_to_edit;
        for (int i = 0; i < OFFSET_; i++)
            branch_copy++;
        #ifdef X86_BUILD_BRANCH
        *branch_copy = RET_OPCODE_;
        #endif
        force_smc_clear = (functor)branch_copy;
    }
    #endif

public:
    explicit BranchChanger(const Funcs... funcs) : current_direction(-1) {
        std::vector<typename std::common_type<Funcs...>::type> pack = { funcs... };
        for (int i = 0; i < (int)pack.size(); i++) {
            intptr_t offset = compute_jump_offset(pack[i], this->bytecode_to_edit);
            if (abs(offset) > (JUMP_DISTANCE_) - 1)
                throw branch_changer_error(error_codes::BRANCH_TARGET_OUT_OF_BOUNDS);
            store_offset_as_bytes(offset, jump_offsets[i]);
        }
        change_permissions(this->bytecode_to_edit, permissions::READ_WRITE_EXECUTE);
        *this->bytecode_to_edit++ = JUMP_OPCODE_;
        #if defined(GCC_BUILD_BRANCH) || defined(CLANG_BUILD_BRANCH) 
        _initilise_smc_functor();
        #endif
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
    void set_direction(const uint64_t condition) {

        /**
         * Args: a runtime condition which can either be a bool or int.
         * 
         * Peforms assembly modification, changing the offset pertaining to an
         * unconditional jump in memory. Assembly modification is only performed
         * if the condition is different to the current branch direction, this is
         * to avoid unecessary SMC penalties.
         * 
         * Cost: ~110-120 cycles.
        */

        if (current_direction != condition) {
            std::memcpy(this->bytecode_to_edit, jump_offsets[condition], OFFSET_);
            current_direction = condition;
            #if defined(GCC_BUILD_BRANCH) || defined(CLANG_BUILD_BRANCH) 
            force_smc_clear();
            #endif
        }
    }

    #else
    void set_direction(const uint64_t condition) {

        /**
         * Args: a runtime condition which can either be a bool or int.
         * 
         * Same semantics as above method, but will alter page permissions before
         * and after modification for additional security. This can be activated
         * using the -DSAFE_MODE flag. Note: much more expensive than above method.
         * 
        */

        if (current_direction != condition) {
            change_permissions(this->bytecode_to_edit, permissions::READ_WRITE_EXECUTE);
            std::memcpy(this->bytecode_to_edit, jump_offsets[condition], OFFSET_);
            current_direction = condition;
            #if defined(GCC_BUILD_BRANCH) || defined(CLANG_BUILD_BRANCH) 
            force_smc_clear();
            #endif
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
