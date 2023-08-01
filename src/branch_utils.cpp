#include "branch_utils.hpp"


void _change_permissions(const unsigned char* address) {
	intptr_t page_size = getpagesize();
    intptr_t relative_addr = reinterpret_cast<intptr_t>(address);
    relative_addr -= relative_addr % page_size;
    void* page_offset = reinterpret_cast<void*>(relative_addr);

	#if defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
    mprotect(page_offset, page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
	#endif

	#ifdef PLATFORM_WINDOWS
	DWORD oldProtect;
    VirtualProtect(page_offset, page_size, PAGE_EXECUTE_READWRITE, &oldProtect);
	#endif
}


void _store_offset_as_bytes(const intptr_t& offset, unsigned char* dst) {
	unsigned char offset_in_bytes[4] = {
	    static_cast<unsigned char>(offset & 0xff),
	    static_cast<unsigned char>((offset >> 8) & 0xff),
	    static_cast<unsigned char>((offset >> 16) & 0xff),
	    static_cast<unsigned char>((offset >> 24) & 0xff)
	};
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	std::reverse(offset_in_bytes, offset_in_bytes + 4);
	#endif
	std::memcpy(dst, offset_in_bytes, 4);
}

