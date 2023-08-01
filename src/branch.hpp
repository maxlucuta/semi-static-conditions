#ifndef BRANCH_HPP
#define BRANCH_HPP

#include "branch_utils.hpp"
#include "compiler_utils.hpp"

template <typename T>
class branch_changer_aux {};

template <typename Ret, typename... Args>
class branch_changer_aux<Ret(*)(Args...)> {
protected:
	unsigned char* bytecode_to_edit;

public:
	branch_changer_aux() : 
	bytecode_to_edit((unsigned char*)&branch_changer_aux::branch) {}

	HOT_ATTR 
	NOCF_CHECK_ATTR 
	OPTIMIZE_ATTR("O3")
	OPTIMIZE_ATTR("no-ipa-cp-clone")
    static Ret branch(Args... args) {
        asm ("jmp 0x00000000");
        if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};

template <typename Class, typename Ret, typename... Args>
class branch_changer_aux<Ret(Class::*)(Args...)> {
protected:
	unsigned char* bytecode_to_edit;

public:
	branch_changer_aux() : 
	bytecode_to_edit((unsigned char*)&branch_changer_aux::branch) {}

	HOT_ATTR 
	NOCF_CHECK_ATTR 
	OPTIMIZE_ATTR("O3")
	OPTIMIZE_ATTR("no-ipa-cp-clone")
    static Ret branch(const Class& inst, Args... args) {
        asm ("jmp 0x00000000");
        if constexpr (!std::is_void_v<Ret>)
            return Ret{};
    }
};


template <typename... Funcs>
class BranchChanger : 
public branch_changer_aux<typename std::common_type<Funcs...>::type> {

#define BRANCHES pack_size<Funcs...>
#define JUMP_OPCODE 0xe9
#define DWORD 4

static_assert(BRANCHES > 1);

private:
	uint64_t current_direction;
	unsigned char jump_offsets[BRANCHES][DWORD];

public:
	BranchChanger(const Funcs... funcs) : current_direction(-1) {
		std::vector<typename std::common_type<Funcs...>::type> pack = {funcs...};
		for (int i = 0; i < (int)BRANCHES; i++) {
			intptr_t offset = _compute_jump_offset(pack[i], this->bytecode_to_edit);
			_store_offset_as_bytes(offset, jump_offsets[i]);
		}
		_change_permissions(this->bytecode_to_edit);
		*this->bytecode_to_edit++ = JUMP_OPCODE;
		if (BRANCHES == 2) {
			std::swap(jump_offsets[0], jump_offsets[1]);
			set_direction(1);
		} else set_direction(0);
	}

	void set_direction(uint64_t condition) {
		if (current_direction != condition) {
			std::memcpy(this->bytecode_to_edit, jump_offsets[condition], DWORD);
			current_direction = condition;
		}
	}
};


#endif