#ifndef BRANCH_GCC_HPP
#define BRANCH_GCC_HPP


#include "branch_base.hpp"


template <typename Ret, typename... Args>
class branch_changer_aux<Ret (*)(Args...)> {

protected:
    unsigned char* bytecode_to_edit;
    static uint64_t instances;

public:
    branch_changer_aux() : 
    bytecode_to_edit((unsigned char*) &branch_changer_aux::branch) {
        if (instances >= 1)
            throw branch_changer_error(error_codes::MULTIPLE_INSTANCE_ERROR);
        instances++;
    }

    __attribute__((hot,nocf_check,optimize("no-ipa-cp-clone,O3")))
    static Ret branch (Args... args) {
        JUMP_INSTRUCTION
        if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};


template <typename Class, typename Ret, typename... Args>
class branch_changer_aux<Ret (Class::*)(Args...)> {

protected:
    unsigned char* bytecode_to_edit;
    static uint64_t instances;

public:
    branch_changer_aux () : 
    bytecode_to_edit ((unsigned char*) &branch_changer_aux::branch) {
        if (instances >= 1)
            throw branch_changer_error (error_codes::MULTIPLE_INSTANCE_ERROR);
        instances++;
    }

     __attribute__((hot,nocf_check,optimize("no-ipa-cp-clone,O3")))
    static Ret branch (const Class& inst, Args... args) {
        JUMP_INSTRUCTION
        if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};


#endif