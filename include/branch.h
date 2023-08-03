//
// Created by Max on 8/2/2023.
//

#ifndef SEMI_STATIC_CONDITIONS_BRANCH_H
#define SEMI_STATIC_CONDITIONS_BRANCH_H


#include "utilities.h"
#include "platform.h"


template <typename T>
class branch_changer_aux {};
    /*Unspecialised template class for CRTP. */


template <typename Ret, typename... Args>
class branch_changer_aux<Ret (*)(Args...)>
{
    /* Base class template for static member functions or
	   regular functions as branches. */

protected:
    unsigned char* bytecode_to_edit;
    static uint64_t instances;

public:
    branch_changer_aux() : 
    bytecode_to_edit((unsigned char*) &branch_changer_aux::branch)
    {
        if (instances >= 1)
            throw branch_changer_error(error_codes::MULTIPLE_INSTANCE_ERROR);
        instances++;
    }

    /* Compiler specific attributes used for controlling
		optimisations on branch method. Can be controlled with
		compiler flags for finer granularity. */

    HOT_ATTR
    NOCF_CHECK_ATTR
    OPTIMIZE_ATTR("O3")
    OPTIMIZE_ATTR("no-ipa-cp-clone")

    static Ret branch(Args... args)
    {
        /* Entry point for branch taking, first 5 bytes is always
			jmp (except on clang -O0 where an instruction is overwritten)
			which the proceeding DWORD offsets goes to some branch target
			passed to constructor. Must never be inlined or have ICP on GCC. */

        asm ("jmp 0x0");
        if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};


template <typename Class, typename Ret, typename... Args>
class branch_changer_aux<Ret (Class::*)(Args...)>
{
    /* Base class template for class member functions as branches. */

protected:
    unsigned char* bytecode_to_edit;
    static uint64_t instances;

public:
    branch_changer_aux() : 
    bytecode_to_edit((unsigned char*) &branch_changer_aux::branch)
    {
        if (instances >= 1)
            throw branch_changer_error(error_codes::MULTIPLE_INSTANCE_ERROR);
        instances++;
    }

    HOT_ATTR
    NOCF_CHECK_ATTR
    OPTIMIZE_ATTR("O3")
    OPTIMIZE_ATTR("no-ipa-cp-clone")

    static Ret branch(const Class& inst, Args... args)
    {
        asm ("jmp 0x0");
            if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};


template <typename... Funcs>
class BranchChanger :
public branch_changer_aux<typename std::common_type<Funcs...>::type>
{
    /* Core BranchChanger derived class for semi-static conditions.
    Pointer type (regular or member function) deduced at compile time
	and instantiates the corresponding base class via CRTP to deduce
	argument types and form branch entry point. */

    static_assert(pack_size<Funcs...> > 1);

private:
    uint64_t current_direction;
    unsigned char jump_offsets[pack_size<Funcs...>][DWORD_];


public:
    explicit BranchChanger(const Funcs... funcs) : current_direction(-1)
    {
        std::vector<typename std::common_type<Funcs...>::type> pack = { funcs... };
        for (int i = 0; i < (int)pack.size(); i++)
        {
            intptr_t offset = compute_jump_offset(pack[i], this->bytecode_to_edit);
            if (offset > JMP_DISTANCE_ - 1)
                throw branch_changer_error(error_codes::BRANCH_TARGET_OUT_OF_BOUNDS);
            store_offset_as_bytes(offset, jump_offsets[i]);
        }
        change_permissions(this->bytecode_to_edit);
        *this->bytecode_to_edit++ = JUMP_OPCODE_;
        if (pack.size() == 2)
        {
            std::swap(jump_offsets[0], jump_offsets[1]);
            set_direction(1);
        } else
            set_direction(0);
    }

    void set_direction(const uint64_t condition)
    {
        /* Alters 32-bit offset in branch method prologue to a different
			branch target based on boolean/int indexing. If modified code in
			close proximity it will trigger SMC machine clears, which cost
			about 100 cycles each. */

        if (current_direction != condition)
        {
            std::memcpy(this->bytecode_to_edit, jump_offsets[condition], DWORD_);
            current_direction = condition;
        }
    }
};


template <typename Ret, typename... Args>
uint64_t branch_changer_aux<Ret (*)(Args...)>::instances = 0;
    /* Reference counters used to ensure templates pertaining to the same
    function signature are not instantiated more than once as they will share
    a common entry point, this will result in dangerous behaviour. */


template <typename Class, typename Ret, typename... Args>
uint64_t branch_changer_aux<Ret (Class::*)(Args...)>::instances = 0;


#endif //SEMI_STATIC_CONDITIONS_BRANCH_H
