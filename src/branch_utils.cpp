#include "branch_utils.hpp"


void _change_permissions(const unsigned char* address) 
{
	intptr_t page_size = getpagesize();
    intptr_t relative_addr = reinterpret_cast<intptr_t>(address);
    relative_addr -= relative_addr % page_size;
    void* page_offset = reinterpret_cast<void*>(relative_addr);

	#if defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
	if (mprotect(page_offset, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
		throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
	#endif

	#ifdef PLATFORM_WINDOWS
	DWORD oldProtect;
	if (!VirtualProtect(page_offset, page_size, PAGE_EXECUTE_READWRITE, &oldProtect))
		throw branch_changer_error(error_codes::PAGE_PERMISSIONS_ERROR);
	#endif
}


void _store_offset_as_bytes(const intptr_t& offset, unsigned char* dst) 
{
	unsigned char offset_in_bytes[4] = {
	    static_cast<unsigned char>(offset & 0xff),
	    static_cast<unsigned char>((offset >> 8) & 0xff),
	    static_cast<unsigned char>((offset >> 16) & 0xff),
	    static_cast<unsigned char>((offset >> 24) & 0xff)
	};
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	std::reverse(offset_in_bytes, offset_in_bytes + _DWORD);
	#endif
	std::memcpy(dst, offset_in_bytes, 4);
}

std::string err_to_str(const error_codes& code)
{
	switch (code)
	{
		case error_codes::BRANCH_TARGET_OUT_OF_BOUNDS:
			return R"(Supplied branch targets (as function pointers) exceed a 2GiB displacement
					  from the entry point in the text segment, and cannot be reached with a 32-bit
					  relative jump. Consider moving the entry point to different areas in the text
					  segment by altering hot/cold attributes.)";
		
		case error_codes::MULTIPLE_INSTANCE_ERROR:
			return R"(More than once instance for template specialised semi-static conditions detected.
					  Program terminated as mutliple instances sharing the same entry point is dangerous
					  and results in undefined behaviour (multiple instances write to same function.))";

		case error_codes::PAGE_PERMISSIONS_ERROR:
			return R"("Unable to change page permissions for the given function pointers.)";

		default:
			return "Runtime error.";
	};
}

branch_changer_error::branch_changer_error(const error_codes& code) : 
std::runtime_error(err_to_str(code)) {}

