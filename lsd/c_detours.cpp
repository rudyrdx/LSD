#include "c_detours.h"
c_detour::c_detour(void* addr, void* dst, std::size_t len) {
	this->m_addr = addr;
	this->m_dst = dst;
	this->m_len = len;
	this->m_trampoline = VirtualAlloc(nullptr, this->m_len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

void* c_detour::hook(bool trampoline) {
	if (this->m_len < 5)
		return nullptr;

	unsigned long prot = 0;
	VirtualProtect(this->m_addr, this->m_len, PAGE_EXECUTE_READWRITE, &prot);

	for (auto i = 0; i < this->m_len; i++) {
		auto& byte = *(std::uint8_t*) ((std::uintptr_t) this->m_addr + i);
		*(std::uint8_t*) ((std::uintptr_t) this->m_trampoline + i) = byte;
		this->m_bytes.push_back(byte);
		byte = 144;
	}

	auto rel32 = (std::uintptr_t) this->m_dst - (std::uint32_t) this->m_addr - 5;

	*(std::uint8_t*) this->m_addr = 233;
	*(std::uint32_t*) ((std::uintptr_t) this->m_addr + 1) = rel32;

	VirtualProtect(this->m_addr, this->m_len, prot, &prot);

	auto target = (std::uintptr_t) this->m_trampoline + this->m_len;
	auto new_src = (std::uintptr_t) this->m_addr + this->m_len;
	auto trampoline_rel32 = (std::uintptr_t) new_src - (std::uint32_t) target - 5;

	*(std::uint8_t*) target = 233;
	*(std::uint32_t*) (target + 1) = trampoline_rel32;

	if (trampoline)
		return this->m_trampoline;

	return (void*)((std::uintptr_t) this->m_addr + this->m_len);
}

bool c_detour::unhook() {
	if (!this->m_bytes.size())
		return false;

	unsigned long prot = 0;
	VirtualProtect(this->m_addr, this->m_len, PAGE_EXECUTE_READWRITE, &prot);

	std::uintptr_t i = 0;

	for (auto& byte : this->m_bytes) {
		*(std::uint8_t*) ((std::uintptr_t) this->m_addr + i) = byte;
		i++;
	}

	VirtualProtect(this->m_addr, this->m_len, prot, &prot);

	VirtualFree(this->m_trampoline, this->m_len + 5, MEM_RELEASE);

	return true;
}

