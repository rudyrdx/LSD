#pragma once

#include "Interfaces.h"
#include "IBaseClientDll.h"
#include "IClientMode.h"
#include "ISurface.h"
#include "IGameEvent.h"
#include "CInput.h"
#include "CModelRender.h"
#include "IVModelInfo.h"
#include "IMaterial.h"
#include "IVRenderView.h"
#include <assert.h>
#include "IEngineTrace.h"
#include <array>
#include <d3dx9.h>

namespace vtable_indexes
{
	constexpr auto view         = 18;
	constexpr auto paint        = 41;
	constexpr auto createMove   = 24;
	constexpr auto hltv			= 93;
	constexpr auto lockCursor   = 67;
	constexpr auto frameStage   = 37;
	constexpr auto dme          = 21;
	constexpr auto extraBonePro	= 197;
	constexpr auto sceneEnd     = 9;
}

class Event : public IGameEventListener
{
public:
	
	void FireGameEvent(IGameEvent * event);

	int  GetEventDebugID = 42;
//	int GetEventDebugID(void);
	void Init()
	{
		g_pEventManager->AddListener(this, "player_hurt", false);
		g_pEventManager->AddListener(this, "start_vote", true);
		g_pEventManager->AddListener(this, "bullet_impact", false);
		g_pEventManager->AddListener(this, "weapon_fire", false);
		g_pEventManager->AddListener(this, "player_death", false);
		g_pEventManager->AddListener(this, "player_connect", false);
		g_pEventManager->AddListener(this, "item_purchase", false);
		g_pEventManager->AddListener(this, "bomb_pickup", false);
		g_pEventManager->AddListener(this, "bomb_beginplant", false);
		g_pEventManager->AddListener(this, "bomb_begindefuse", false);
		g_pEventManager->AddListener(this, "enter_bombzone", false);
		g_pEventManager->AddListener(this, "round_prestart", false);
		g_pEventManager->AddListener(this, "bullet_impact", false);
		g_pEventManager->AddListener(this, "player_footstep", false);
		g_pEventManager->AddListener(this, "player_jump", false);
		g_pEventManager->AddListener(this, "player_falldamage", false);
		g_pEventManager->AddListener(this, "round_start", false);
		g_pEventManager->AddListener(this, "client_disconnect", false);
		g_pEventManager->AddListener(this, "cs_game_disconnected", false);
	}
};


extern Event g_Event;
class IMatRenderContext;
class vfunc_hook;
class CBoneBitList;
class VMTHook;
class IViewRenderBeams;
class ShadowVTManager;
using doextraboneprocessing_fn	= void(__thiscall*)(void*, int, int, int, int, int, int);
typedef void(__thiscall* build_transformations_t)(C_BaseEntity*, studiohdr_t*, Vector*, QAngle*, const matrix3x4_t&, int32_t, byte*);
using standard_blending_rules_fn = void* (__thiscall*)(void*, studiohdr_t*, Vector*, Quaternion*, float, int);
using update_anims_fn			= void(__thiscall*)(void*);
using setup_bones_t = bool(__thiscall*) (void*, matrix3x4_t*, int, int, float);
typedef void(__thiscall* ExtraBoneProcess_t) (void*, void*, void*, void*, void*, CBoneBitList&, void*);
extern setup_bones_t o_setup_bones;
extern ExtraBoneProcess_t o_dpfp;
extern doextraboneprocessing_fn o_doextraboneprocessing;
extern update_anims_fn	o_update_anims;
extern build_transformations_t	o_build_transformations;
extern standard_blending_rules_fn	o_standard_blending_rules;
class Hooks
{
public:
    // Initialization setup, called on injection
    static void Init();
    static void Restore();

    /*---------------------------------------------*/
    /*-------------Hooked functions----------------*/
    /*---------------------------------------------*/

	static void __fastcall update_anims(void* ecx, void* edx);

	static void     __stdcall   FrameStageNotify(ClientFrameStage_t curStage);
    static bool     __fastcall  CreateMove(IClientMode*, void*, float, CUserCmd*);
	static void __fastcall fire_event(IVEngineClient* IVEngineClient, uint32_t);
	static bool __fastcall hk_setup_bones(void* ecx, void* edx, matrix3x4_t* bone_to_world_out, int max_bones, int bone_mask, float curtime);
	static void     __fastcall  LockCursor(ISurface*, void*);
	static void		__fastcall  PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
	static void     __fastcall  OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
	static bool __fastcall inprediction(void* ecx, void* edx);
	static void     __fastcall  DrawModelExecute(void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& render_info, matrix3x4_t* matrix);
	static void c_cs_player_hook();
	static void __fastcall packet_start(IClientState* state, uint32_t, int incoming_sequence, int outgoing_acknowledged);
	static bool __fastcall c_mdl_cacheunknown_fn(void* mdl_cache, uint32_t);
	static void __fastcall DoExtraBonesProcessing(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7);
	static int __fastcall DoPostScreenEffects(void* ecx, void* edx, int a1);
	static bool __fastcall WriteUsercmdDeltaToBuffer_hook(void* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd);
	static void     __fastcall  SceneEnd(void *ecx, void *edx);
    static LRESULT  __stdcall   WndProc   (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	inline static std::vector<int32_t> cmds = {};
	bool __fastcall is_hltv_gate(void*, void*);

	static bool __fastcall IsHLTV();

private:
    /*---------------------------------------------*/
    /*-------------VMT Hook pointers---------------*/
    /*---------------------------------------------*/

	std::unique_ptr<VMTHook> pClientHook;
	std::unique_ptr<VMTHook> pDeviceHook;
	std::unique_ptr<VMTHook> pSetupBones;
	std::unique_ptr<VMTHook> pPredictionHook;
	std::unique_ptr<VMTHook> pClientState;
	std::unique_ptr<VMTHook> pEngineHook;
    std::unique_ptr<VMTHook> pClientModeHook;
    std::unique_ptr<VMTHook> pSurfaceHook;
	std::unique_ptr<VMTHook> pPanelHook;
	std::unique_ptr<VMTHook> pEvent;
	std::unique_ptr<VMTHook> pModelHook;
	std::unique_ptr<VMTHook> pBones;
	std::unique_ptr<VMTHook> pRenderViewHook;
	std::unique_ptr<ShadowVTManager> pPlayerHook[65];



    /*---------------------------------------------*/
    /*-------------Hook prototypes-----------------*/
    /*---------------------------------------------*/
	

	using senddatagram_fn			= int(__thiscall*)(void*, void*);
	typedef void(__thiscall* fire_event_t)(void*);
	typedef void (__stdcall*  FrameStageNotify_t) (ClientFrameStage_t);
    typedef bool (__fastcall* CreateMove_t) (IClientMode*, void*, float, CUserCmd*);
	typedef void(__thiscall* packet_start_t)(IClientState*, int, int);
	inline static packet_start_t _packet_start;
	typedef bool(__thiscall* unknown_fn_t)(void*);
	inline static unknown_fn_t _unknown_fn;
	typedef bool(__thiscall *FireEventClientSide)(void*, IGameEvent*);
	typedef int(__fastcall* DoPostScreenEffects_t) (void*, void*, int);
    typedef void (__fastcall* LockCursor_t) (ISurface*, void*);
	typedef void (__thiscall* PaintTraverse_t) (PVOID, unsigned int, bool, bool);
	typedef void (__fastcall* OverrideView_t) (void*, void*, CViewSetup*);
	typedef void (__thiscall* DrawModelExecute_t) (void*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	typedef void (__fastcall* SceneEnd_t) (void*, void*);
	typedef bool(__thiscall* WriteUsercmdDeltaToBufferFn)(void* ecx, int nSlot, void* buf, int from, int to, bool isNewCmd);
	using inprediction_fn			= bool(__thiscall*)(void*);
	using is_hltv_t = bool(__fastcall*) ();
private:
    HWND                           hCSGOWindow             = nullptr; // CSGO window handle
    bool                           bInitializedDrawManager = false;   // Check if we initialized our draw manager
    WNDPROC                        pOriginalWNDProc        = nullptr; // Original CSGO window proc

	
};

extern Hooks g_Hooks;
class vftable_hook /*: public singleton<vftable_hook>*/
{
	vftable_hook(const vftable_hook&) = delete;
public:

	template < class Type >
	static Type hook_manual(uintptr_t* instance, int offset, Type hook)
	{
		DWORD Dummy;
		Type fnOld = reinterpret_cast<Type> (instance[offset]);
		//VirtualProtect ( ( void* ) ( instance + offset * 0x4 ), 0x4, PAGE_EXECUTE_READWRITE, &Dummy );
		VirtualProtect(static_cast<void*> (instance + offset * 0x4), 0x4, PAGE_EXECUTE_READWRITE, &Dummy);
		instance[offset] = (uintptr_t)hook;
		VirtualProtect(static_cast<void*> (instance + offset * 0x4), 0x4, Dummy, &Dummy);
		return fnOld;
	}

private:

	static int unprotect(void* region)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(static_cast<LPCVOID> (region), &mbi, sizeof(mbi));
		VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
		return mbi.Protect;
	}

	static void protect(void* region, int protection)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(static_cast<LPCVOID> (region), &mbi, sizeof(mbi));
		VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
	}
};
class VMTHook
{
public:
	VMTHook(void* ppClass)
	{
		ppBaseClass = static_cast<std::uintptr_t**>(ppClass);

		// loop through all valid class indexes. When it will hit invalid (not existing) it will end the loop
		while (static_cast<std::uintptr_t*>(*ppBaseClass)[indexCount])
			++indexCount;

		const std::size_t kSizeTable = indexCount * sizeof(std::uintptr_t);

		pOriginalVMT = *ppBaseClass;
		pNewVMT = std::make_unique<std::uintptr_t[]>(indexCount);

		// copy original vtable to our local copy of it
		std::memcpy(pNewVMT.get(), pOriginalVMT, kSizeTable);

		// replace original class with our local copy
		*ppBaseClass = pNewVMT.get();
	};
	~VMTHook() { *ppBaseClass = pOriginalVMT; };

	template<class Type>
	Type GetOriginal(const std::size_t index)
	{
		return reinterpret_cast<Type>(pOriginalVMT[index]);
	};

	HRESULT Hook(const std::size_t index, void* fnNew)
	{
		if (index > indexCount)   // check if given index is valid
			return E_INVALIDARG;

		pNewVMT[index] = reinterpret_cast<std::uintptr_t>(fnNew);
		return S_OK;
	};

	HRESULT Unhook(const std::size_t index)
	{
		if (index > indexCount)
			return E_INVALIDARG;

		pNewVMT[index] = pOriginalVMT[index];
		return S_OK;
	};

private:
	std::unique_ptr<std::uintptr_t[]> pNewVMT = nullptr;    // Actual used vtable
	std::uintptr_t**                  ppBaseClass = nullptr;             // Saved pointer to original class
	std::uintptr_t*                   pOriginalVMT = nullptr;             // Saved original pointer to the VMT
	std::size_t                       indexCount = 0;                     // Count of indexes inside out f-ction
};


class ProtectGuard
{
public:

	ProtectGuard(void *base, uint32_t len, uint32_t protect)
	{
		base = base;
		len = len;

		if (!VirtualProtect(base, len, protect, (PDWORD)&old_protect))
			throw std::runtime_error("Failed to protect region!");
	}

	~ProtectGuard()
	{
		VirtualProtect(base, len, old_protect, (PDWORD)&old_protect);
	}

private:

	void *base;
	uint32_t len;
	uint32_t old_protect;
};

class ShadowVTManager // GLAD
{

public:

	ShadowVTManager() : class_base(nullptr), method_count(0), shadow_vtable(nullptr), original_vtable(nullptr) {}
	ShadowVTManager(void *base) : class_base(base), method_count(0), shadow_vtable(nullptr), original_vtable(nullptr) {}
	~ShadowVTManager()
	{
		RestoreTable();

		delete[] shadow_vtable;
	}

	inline void Setup(void *base = nullptr)
	{
		if (base != nullptr)
			class_base = base;

		if (class_base == nullptr)
			return;

		original_vtable = *(uintptr_t**)class_base;
		method_count = GetMethodCount(original_vtable);

		if (method_count == 0)
			return;

		shadow_vtable = new uintptr_t[method_count + 1]();

		shadow_vtable[0] = original_vtable[-1];
		std::memcpy(&shadow_vtable[1], original_vtable, method_count * sizeof(uintptr_t));

		try
		{
			auto guard = ProtectGuard{ class_base, sizeof(uintptr_t), PAGE_READWRITE };
			*(uintptr_t**)class_base = &shadow_vtable[1];
		}
		catch (...)
		{
			delete[] shadow_vtable;
		}
	}

	template<typename T>
	inline void Hook(uint32_t index, T method)
	{
		assert(index < method_count);
		shadow_vtable[index + 1] = reinterpret_cast<uintptr_t>(method);
	}

	inline void Unhook(uint32_t index)
	{
		assert(index < method_count);
		shadow_vtable[index + 1] = original_vtable[index];
	}

	template<typename T>
	inline T GetOriginal(uint32_t index)
	{
		return (T)original_vtable[index];
	}

	inline void RestoreTable()
	{
		try
		{
			if (original_vtable != nullptr)
			{
				auto guard = ProtectGuard{ class_base, sizeof(uintptr_t), PAGE_READWRITE };
				*(uintptr_t**)class_base = original_vtable;
				original_vtable = nullptr;
			}
		}
		catch (...) {}
	}

private:

	inline uint32_t GetMethodCount(uintptr_t *vtable_start)
	{
		uint32_t len = -1;

		do ++len; while (vtable_start[len]);

		return len;
	}

	void *class_base;
	uint32_t method_count;
	uintptr_t *shadow_vtable;
	uintptr_t *original_vtable;
};


namespace random {
	constexpr auto time = __TIME__;
	constexpr auto seed = static_cast<unsigned>(time[7]) + static_cast<unsigned>(time[6]) * 10 + static_cast<unsigned>(time[4]) * 60 + static_cast<unsigned>(time[3]) * 600 + static_cast<unsigned>(time[1]) * 3600 + static_cast<unsigned>(time[0]) * 36000;

	template <int n>
	struct gen {
	private:
		static constexpr unsigned a = 16807;
		static constexpr unsigned m = 2147483647;

		static constexpr unsigned s = gen<n - 1>::value;
		static constexpr unsigned lo = a * (s & 0xFFFFu);
		static constexpr unsigned hi = a * (s >> 16u);
		static constexpr unsigned lo2 = lo + ((hi & 0x7FFFu) << 16u);
		static constexpr unsigned hi2 = hi >> 15u;
		static constexpr unsigned lo3 = lo2 + hi;

	public:
		static constexpr unsigned max = m;
		static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
	};

	template <>
	struct gen<0> {
		static constexpr unsigned value = seed;
	};

	template <int n, int m>
	struct _int {
		static constexpr auto value = gen<n + 1>::value % m;
	};

	template <int n>
	struct _char {
		static const char value = static_cast<char>(1 + _int<n, 0x7F - 1>::value);
	};
}

// ReSharper disable once CppUnusedIncludeDirective


template <size_t n, char k>
struct xorstr
{
private:
	static constexpr char enc(const char c)
	{
		return c ^ k;
	}

public:
	template <size_t... s>
	constexpr __forceinline xorstr(const char* str, std::index_sequence<s...>) : encrypted{ enc(str[s])... } { }

	__forceinline std::string dec()
	{
		std::string dec;
		dec.resize(n);

		for (auto i = 0; i < n; i++)
			dec[i] = encrypted[i] ^ k;

		return dec;
	}

	__forceinline std::string ot(bool decrypt = true)
	{
		std::string dec;
		dec.resize(n);

		for (auto i = 0; i < n; i++)
		{
			dec[i] = decrypt ? (encrypted[i] ^ k) : encrypted[i];
			encrypted[i] = '\0';
		}

		return dec;
	}

	std::array<char, n> encrypted{};
};

#define _(s) xorstr<sizeof(s), random::_char<__COUNTER__>::value>(s, std::make_index_sequence<sizeof(s)>()).dec().c_str()
#define _ot(s) xorstr<sizeof(s), random::_char<__COUNTER__>::value>(s, std::make_index_sequence<sizeof(s)>()).ot().c_str()
#define __(s) []() -> std::pair<std::string, char> { \
	constexpr auto key = random::_char<__COUNTER__>::value; \
	return std::make_pair(xorstr<sizeof(s), key>(s, std::make_index_sequence<sizeof(s)>()).ot(false), key); \
}()
#define _rt(n, s) auto (n) = reinterpret_cast<char*>(alloca(((s).first.size() + 1) * sizeof(char))); \
	for (size_t i = 0; i < (s).first.size(); i++) \
        (n)[i] = (s).first[i] ^ (s).second; \
    (n)[(s).first.size()] = '\0'


template<class entity>
class c_hook
{
public:
	explicit c_hook(entity* ent)
	{
		base = reinterpret_cast<uintptr_t*>(ent);
		original = *base;

		const auto l = length() + 1;
		current = std::make_unique<uint32_t[]>(l);
		std::memcpy(current.get(), reinterpret_cast<void*>(original - sizeof(uint32_t)), l * sizeof(uint32_t));
		oldbase = base;
		patch_pointer(base);
	}

	template<typename function, typename original_function>
	function apply(const uint32_t index, original_function func)
	{
		auto old = reinterpret_cast<uintptr_t*>(original)[index];
		current.get()[index + 1] = reinterpret_cast<uintptr_t>(func);
		return reinterpret_cast<function>(old);
	}
	//template<typename function, typename original_function>
	void unapply(const uint32_t index) {
		current.get()[index + 1] = reinterpret_cast<uintptr_t*>(original)[index];
		DWORD old;
		VirtualProtect(&base, sizeof(uintptr_t), PAGE_READWRITE, &old);


		*base = *oldbase;
		VirtualProtect(&base, sizeof(uintptr_t), old, &old);
	}
	void patch_pointer(uintptr_t* location) const
	{
		if (!location)
			return;

		DWORD old;
		PVOID address = location;
		ULONG size = sizeof(uintptr_t);
		VirtualProtect(&address, sizeof(uintptr_t), PAGE_READWRITE, &old);

		*location = reinterpret_cast<uint32_t>(current.get()) + sizeof(uint32_t);
		VirtualProtect(&address, sizeof(uintptr_t), old, &old);

	}

private:
	uint32_t length() const
	{
		uint32_t index;
		const auto vmt = reinterpret_cast<uint32_t*>(original);

		for (index = 0; vmt[index]; index++)
			if (IS_INTRESOURCE(vmt[index]))
				break;

		return index;
	}

	std::uintptr_t* base;
	std::uintptr_t* oldbase;
	std::uintptr_t original;

	std::unique_ptr<uint32_t[]> current;
};