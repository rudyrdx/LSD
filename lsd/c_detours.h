#pragma once
#include <vector>
#include <Windows.h>
class c_detour{
	void* m_addr = nullptr, *m_dst = nullptr, *m_trampoline = nullptr;
	std::size_t m_len;
	std::vector< std::uint8_t > m_bytes{ };

public:
	c_detour(void* addr, void* dst, std::size_t len);

	void* hook(bool trampoline = true);
	bool unhook();
};
