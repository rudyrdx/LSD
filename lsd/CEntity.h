#pragma once
#include "Definitions.h"
#include "IClientUnknown.h"
#include "IClientEntityList.h"
#include "CInput.h"
#include "Utils.h"
#include "NetvarManager.h"
#include "IVModelInfo.h"

#define VFUNC( clazz, idx )			( ( *( void*** ) clazz )[ idx ] )
#define FOFFSET( type, ptr, offset ) ( *( type* ) ( ( std::uintptr_t ) (ptr) + ( offset ) ) )
#define FOFFSETPTR( type, ptr, offset ) ( ( type ) ( ( std::uintptr_t ) (ptr) + ( offset ) ) )
#define OFFSETPTR( type, offset )	( ( type ) ( ( std::uintptr_t ) this + ( offset ) ) )
#define OFFSET( type, offset )		( *( type* ) ( ( std::uintptr_t ) this + ( offset ) ) )
struct _CrtMemState;
#define MEMALLOC_VERSION 1
typedef size_t(*MemAllocFailHandler_t)(size_t);
#define HANDLE( type )				void*
class IMemAlloc
{
public:
	virtual ~IMemAlloc();

	// Release versions
	virtual void* Alloc(size_t nSize) = 0;
	virtual void* Realloc(void* pMem, size_t nSize) = 0;
	virtual void Free(void* pMem) = 0;
	virtual void* Expand_NoLongerSupported(void* pMem, size_t nSize) = 0;

	// Debug versions
	//virtual void *Alloc(size_t nSize, const char *pFileName, int nLine) = 0;
	//virtual void *Realloc(void *pMem, size_t nSize, const char *pFileName, int nLine) = 0;
	//virtual void  Free(void *pMem, const char *pFileName, int nLine) = 0;
	//virtual void *Expand_NoLongerSupported(void *pMem, size_t nSize, const char *pFileName, int nLine) = 0;

	// Returns size of a particular allocation
	virtual size_t GetSize(void* pMem) = 0;

	// Force file + line information for an allocation
	virtual void PushAllocDbgInfo(const char* pFileName, int nLine) = 0;
	virtual void PopAllocDbgInfo() = 0;

	// FIXME: Remove when we have our own allocator
	// these methods of the Crt debug code is used in our codebase currently
	virtual long CrtSetBreakAlloc(long lNewBreakAlloc) = 0;
	virtual	int CrtSetReportMode(int nReportType, int nReportMode) = 0;
	virtual int CrtIsValidHeapPointer(const void* pMem) = 0;
	virtual int CrtIsValidPointer(const void* pMem, unsigned int size, int access) = 0;
	virtual int CrtCheckMemory(void) = 0;
	virtual int CrtSetDbgFlag(int nNewFlag) = 0;
	virtual void CrtMemCheckpoint(_CrtMemState* pState) = 0;

	// FIXME: Make a better stats interface
	virtual void DumpStats() = 0;
	virtual void DumpStatsFileBase(char const* pchFileBase) = 0;

	// FIXME: Remove when we have our own allocator
	virtual void* CrtSetReportFile(int nRptType, void* hFile) = 0;
	virtual void* CrtSetReportHook(void* pfnNewHook) = 0;
	virtual int CrtDbgReport(int nRptType, const char* szFile,
		int nLine, const char* szModule, const char* pMsg) = 0;

	virtual int heapchk() = 0;

	virtual bool IsDebugHeap() = 0;

	virtual void GetActualDbgInfo(const char*& pFileName, int& nLine) = 0;
	virtual void RegisterAllocation(const char* pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime) = 0;
	virtual void RegisterDeallocation(const char* pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime) = 0;

	virtual int GetVersion() = 0;

	virtual void CompactHeap() = 0;

	// Function called when malloc fails or memory limits hit to attempt to free up memory (can come in any thread)
	virtual MemAllocFailHandler_t SetAllocFailHandler(MemAllocFailHandler_t pfnMemAllocFailHandler) = 0;

	virtual void DumpBlockStats(void*) = 0;

#if defined( _MEMTEST )	
	virtual void SetStatsExtraInfo(const char* pMapName, const char* pComment) = 0;
#endif

	// Returns 0 if no failure, otherwise the size_t of the last requested chunk
	//  "I'm sure this is completely thread safe!" Brian Deen 7/19/2012.
	virtual size_t MemoryAllocFailed() = 0;

	// handles storing allocation info for coroutines
	virtual int  GetDebugInfoSize() = 0;
	virtual void SaveDebugInfo(void* pvDebugInfo) = 0;
	virtual void RestoreDebugInfo(const void* pvDebugInfo) = 0;
	virtual void InitDebugInfo(void* pvDebugInfo, const char* pchRootFileName, int nLine) = 0;

	// Replacement for ::GlobalMemoryStatus which accounts for unused memory in our system
	virtual void GlobalMemoryStatus(size_t* pUsedMemory, size_t* pFreeMemory) = 0;
}; extern IMemAlloc* g_pMemalloc;
struct animstate_pose_param_cache_t {
	std::uint8_t pad_0x0[0x4]; //0x0
	std::uint32_t m_idx; //0x4 
	char* m_name; //0x8 
};

// class predefinition
class C_BaseCombatWeapon;
template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}
class CCSGOPlayerAnimState
{
public:

	std::uint8_t pad_0x0000[0x4]; //0x0000
	bool m_force_update; //0x0005 
	std::uint8_t pad_0x0006[0x5A]; //0x0006
	C_BaseEntity* m_entity; //0x0060 
	C_BaseCombatWeapon* m_weapon; //0x0064 
	C_BaseCombatWeapon* m_last_weapon; //0x0068 
	float m_last_clientside_anim_update; //0x006C 
	std::uint32_t m_last_clientside_anim_framecount; //0x0070 
	float m_last_clientside_anim_update_time_delta; //0x0074 
	float m_eye_yaw; //0x0078 
	float m_pitch; //0x007C 
	float m_goalfeet_yaw; //0x0080 
	float m_feet_yaw; //0x0084 
	float m_body_yaw; //0x0088 
	float m_body_yaw_clamped; //0x008C 
	float m_feet_vel_dir_delta; //0x0090 
	std::uint8_t pad_0x0094[0x4]; //0x0094
	float m_feet_cycle; //0x0098 
	float m_feet_yaw_rate; //0x009C 
	std::uint8_t pad_0x00A0[0x4]; //0x00A0
	float m_duck_amount; //0x00A4 
	float m_landing_duck_additive; //0x00A8 
	std::uint8_t pad_0x00AC[0x4]; //0x00AC
	Vector m_origin; //0x00B0 
	Vector m_old_origin; //0x00BC 
	Vector2D m_vel2d; //0x00C8 
	std::uint8_t pad_0x00D0[0x10]; //0x00D0
	Vector2D m_last_accelerating_vel; //0x00E0 
	std::uint8_t pad_0x00E8[0x4]; //0x00E8
	float m_speed2d; //0x00EC 
	float m_up_vel; //0x00F0 
	float m_speed_normalized; //0x00F4 
	float m_run_speed; //0x00F8 
	float m_unk_feet_speed_ratio; //0x00FC 
	float m_time_since_move; //0x0100 
	float m_time_since_stop; //0x0104 
	bool m_on_ground; //0x0108 
	bool m_hit_ground; //0x0109 
	std::uint8_t pad_0x010A[0x4]; //0x010A
	float m_time_in_air; //0x0110 
	std::uint8_t pad_0x0114[0x6]; //0x0114
	float m_ground_fraction; //0x011C 
	std::uint8_t pad_0x0120[0x2]; //0x0120
	float m_duck_rate; //0x0124 
	std::uint8_t pad_0x0128[0xC]; //0x0128
	bool m_moving; //0x0134
	std::uint8_t pad_0x0135[0x7B]; //0x0135
	animstate_pose_param_cache_t m_lean_yaw_pose; //0x1B0
	animstate_pose_param_cache_t m_speed_pose; //0x01BC
	animstate_pose_param_cache_t m_ladder_speed_pose; //0x01C8
	animstate_pose_param_cache_t m_ladder_yaw_pose; //0x01D4
	animstate_pose_param_cache_t m_move_yaw_pose; //0x01E0
	animstate_pose_param_cache_t m_unk_pose_pose; //0x01EC 
	animstate_pose_param_cache_t m_body_yaw_pose; //0x01F8
	animstate_pose_param_cache_t m_body_pitch_pose; //0x0204
	animstate_pose_param_cache_t m_dead_yaw_pose; //0x0210
	animstate_pose_param_cache_t m_stand_pose; //0x021C
	animstate_pose_param_cache_t m_jump_fall_pose; //0x0228
	animstate_pose_param_cache_t m_aim_blend_stand_idle_pose; //0x0234
	animstate_pose_param_cache_t m_aim_blend_crouch_idle_pose; //0x0240
	animstate_pose_param_cache_t m_strafe_yaw_pose; //0x024C
	animstate_pose_param_cache_t m_aim_blend_stand_walk_pose; //0x0258
	animstate_pose_param_cache_t m_aim_blend_stand_run_pose; //0x0264
	animstate_pose_param_cache_t m_aim_blend_crouch_walk_pose; //0x0270
	animstate_pose_param_cache_t m_move_blend_walk_pose; //0x027C
	animstate_pose_param_cache_t m_move_blend_run_pose; //0x0288
	animstate_pose_param_cache_t m_move_blend_crouch_pose; //0x0294
	std::uint8_t pad_0x02A0[0x4]; //0x02A0
	float m_vel_unk; //0x02A4 
	std::uint8_t pad_0x02A8[0x86]; //0x02A8
	float m_min_yaw; //0x0330 
	float m_max_yaw; //0x0334 
	float m_max_pitch; //0x0338 
	float m_min_pitch; //0x033C

}; //Size=0x344
class C_AnimState
{
public:
	std::uint8_t pad_0x0000[0x4]; //0x0000
	bool m_force_update; //0x0005 
	std::uint8_t pad_0x0006[0x5A]; //0x0006
	C_BaseEntity* m_entity; //0x0060 
	C_BaseCombatWeapon* m_weapon; //0x0064 
	C_BaseCombatWeapon* m_last_weapon; //0x0068 
	float m_last_clientside_anim_update; //0x006C 
	std::uint32_t m_last_clientside_anim_framecount; //0x0070 
	float m_last_clientside_anim_update_time_delta; //0x0074 
	float m_eye_yaw; //0x0078 
	float m_pitch; //0x007C 
	float m_goalfeet_yaw; //0x0080 
	float m_feet_yaw; //0x0084 
	float m_body_yaw; //0x0088 
	float m_body_yaw_clamped; //0x008C 
	float m_feet_vel_dir_delta; //0x0090 
	std::uint8_t pad_0x0094[0x4]; //0x0094
	float m_feet_cycle; //0x0098 
	float m_feet_yaw_rate; //0x009C 
	std::uint8_t pad_0x00A0[0x4]; //0x00A0
	float m_duck_amount; //0x00A4 
	float m_landing_duck_additive; //0x00A8 
	std::uint8_t pad_0x00AC[0x4]; //0x00AC
	Vector m_origin; //0x00B0 
	Vector m_old_origin; //0x00BC 
	Vector2D m_vel2d; //0x00C8 
	std::uint8_t pad_0x00D0[0x10]; //0x00D0
	Vector2D m_last_accelerating_vel; //0x00E0 
	std::uint8_t pad_0x00E8[0x4]; //0x00E8
	float m_speed2d; //0x00EC 
	float m_up_vel; //0x00F0 
	float m_speed_normalized; //0x00F4 
	float m_run_speed; //0x00F8 
	float m_unk_feet_speed_ratio; //0x00FC 
	float m_time_since_move; //0x0100 
	float m_time_since_stop; //0x0104 
	bool m_on_ground; //0x0108 
	bool m_hit_ground; //0x0109 
	std::uint8_t pad_0x010A[0x4]; //0x010A
	float m_time_in_air; //0x0110 
	std::uint8_t pad_0x0114[0x6]; //0x0114
	float m_ground_fraction; //0x011C 
	std::uint8_t pad_0x0120[0x2]; //0x0120
	float m_duck_rate; //0x0124 
	std::uint8_t pad_0x0128[0xC]; //0x0128
	bool m_moving; //0x0134
	std::uint8_t pad_0x0135[0x7B]; //0x0135
	animstate_pose_param_cache_t m_lean_yaw_pose; //0x1B0
	animstate_pose_param_cache_t m_speed_pose; //0x01BC
	animstate_pose_param_cache_t m_ladder_speed_pose; //0x01C8
	animstate_pose_param_cache_t m_ladder_yaw_pose; //0x01D4
	animstate_pose_param_cache_t m_move_yaw_pose; //0x01E0
	animstate_pose_param_cache_t m_unk_pose_pose; //0x01EC 
	animstate_pose_param_cache_t m_body_yaw_pose; //0x01F8
	animstate_pose_param_cache_t m_body_pitch_pose; //0x0204
	animstate_pose_param_cache_t m_dead_yaw_pose; //0x0210
	animstate_pose_param_cache_t m_stand_pose; //0x021C
	animstate_pose_param_cache_t m_jump_fall_pose; //0x0228
	animstate_pose_param_cache_t m_aim_blend_stand_idle_pose; //0x0234
	animstate_pose_param_cache_t m_aim_blend_crouch_idle_pose; //0x0240
	animstate_pose_param_cache_t m_strafe_yaw_pose; //0x024C
	animstate_pose_param_cache_t m_aim_blend_stand_walk_pose; //0x0258
	animstate_pose_param_cache_t m_aim_blend_stand_run_pose; //0x0264
	animstate_pose_param_cache_t m_aim_blend_crouch_walk_pose; //0x0270
	animstate_pose_param_cache_t m_move_blend_walk_pose; //0x027C
	animstate_pose_param_cache_t m_move_blend_run_pose; //0x0288
	animstate_pose_param_cache_t m_move_blend_crouch_pose; //0x0294
	std::uint8_t pad_0x02A0[0x4]; //0x02A0
	float m_vel_unk; //0x02A4 
	std::uint8_t pad_0x02A8[0x86]; //0x02A8
	float m_min_yaw; //0x0330 
	float m_max_yaw; //0x0334 
	float m_max_pitch; //0x0338 
	float m_min_pitch; //0x033C
};

class AnimationLayer {
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	unsigned int m_nOrder; //0x0014
	unsigned int m_nSequence; //0x0018
	float m_flPrevCycle; //0x001C
	float m_flWeight; //0x0020
	float m_flWeightDeltaRate; //0x0024
	float m_flPlaybackRate; //0x0028
	float m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = g_pNetvars->GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
	}

#define NETVAR2(table, prop, func_name, type) \
	type& func_name( ) { \
      static uintptr_t offset = 0; \
      if(!offset) \
      { offset = sdk::util::getNetVar( sdk::util::fnv::hash( table ), sdk::util::fnv::hash( prop ) ); } \
	  \
      return *reinterpret_cast< type* >( uintptr_t( this ) + offset ); \
    }
struct datamap_t;
class typedescription_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,

	// Must be last
	TD_OFFSET_COUNT,
};

class typedescription_t
{
public:
	int32_t fieldType; //0x0000
	char* fieldName; //0x0004
	int fieldOffset[TD_OFFSET_COUNT]; //0x0008
	int16_t fieldSize_UNKNWN; //0x0010
	int16_t flags_UNKWN; //0x0012
	char pad_0014[12]; //0x0014
	datamap_t* td; //0x0020
	char pad_0024[24]; //0x0024
}; //Size: 0x003C
struct datamap_t
{
	typedescription_t* dataDesc;
	int                    dataNumFields;
	char const* dataClassName;
	datamap_t* baseMap;

	bool                chains_validated;
	// Have the "packed" offsets been computed
	bool                packed_offsets_computed;
	int                    packed_size;
};
struct varmapentry_t {
	std::uint16_t m_type;
	std::uint16_t m_needstointerpolate;
	void* data;
	void* watcher;
};
class CBoneCache
{
public:
	matrix3x4_t* m_pCachedBones;
	char pad[8];
	unsigned int m_CachedBoneCount;
};
struct varmapping_t {
	varmapentry_t* m_entries;
	int m_interpolatedentries;
	float m_lastinterptime;
};
class C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable
{
private:

	template <class T>
	static T GetFunction(void* instance, int index)
	{
		const auto vtable = *static_cast<void***>(instance);
		return reinterpret_cast<T>(vtable[index]);
	}
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
    return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
	template <typename T>
	T& SetValue(uintptr_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}
	
	unsigned int FindInDataMap(datamap_t* pMap, const char* name)
	{
		while (pMap)
		{
			for (int i = 0; i < pMap->dataNumFields; i++)
			{
				if (pMap->dataDesc[i].fieldName == NULL)
					continue;

				if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
					return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (pMap->dataDesc[i].fieldType == 10)
				{
					if (pMap->dataDesc[i].td)
					{
						unsigned int offset;

						if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
							return offset;
					}
				}
			}
			pMap = pMap->baseMap;
		}

		return 0;
	}
public:

	bool is_local_player() const
	{
		return g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	}
	void invalidate_physics_recursive(int32_t flags) {
		static const auto invalidate_physics_recursive = reinterpret_cast<void(__thiscall*)(C_BaseEntity*, int32_t)>(
			Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56"));
		invalidate_physics_recursive(this, flags);
	}
	C_AnimState *AnimState()
	{
		return *reinterpret_cast<C_AnimState**>(uintptr_t(this) + 0x3900);
	}
	CCSGOPlayerAnimState *fake_animstate()
	{
		return *reinterpret_cast<CCSGOPlayerAnimState **>(uintptr_t(this) + 0x3900);
	}
	float_t m_flSpawnTime() {
		// 0xA360
		//static auto m_iAddonBits = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_iAddonBits" );
		//return *( float_t* )( ( uintptr_t )this + m_iAddonBits - 0x4 );
		return *(float_t*)((uintptr_t)this + 0xA360);
	}
	int& EFlags()
	{
		return SetValue<int>(0xE4);
	}
	int& effects() {

		static int m_iTeamNum = g_pNetvars->GetOffset("DT_BaseEntity", "m_fEffects");
		return SetValue<int>(m_iTeamNum);
		
	}

	int& flags() {
		return OFFSET(int, 0x104);
	}
	AnimationLayer *AnimOverlays()
	{
		return *reinterpret_cast<AnimationLayer**>(uintptr_t(this) + 0x2990);
	}
	AnimationLayer& GetAnimOverlay(int Index)
	{
		return (*(AnimationLayer**)((DWORD)this + 0x2990))[Index];
	}
	AnimationLayer* get_anim_overlay_index(int i)
	{
		if (i < 15)
			return &GetAnimOverlay(i);
		return nullptr;
	}
	int NumOverlays()
	{
		return 15;
	}

	void UpdateClientAnimation()
	{
		Utils::GetVFunc<void(__thiscall*)(void*)>(this, 223)(this);
	}

	void ClientAnimations(bool value)
	{
		static int m_bClientSideAnimation = g_pNetvars->GetOffset("DT_BaseAnimating", "m_bClientSideAnimation");
		*reinterpret_cast<bool*>(uintptr_t(this) + m_bClientSideAnimation) = value;
	}
	//void ClientAnimationss()
	//{
	//	static int m_bClientSideAnimation = g_pNetvars->GetOffset("DT_BaseAnimating", "m_bClientSideAnimation");
	//	return (uintptr_t(this) + m_bClientSideAnimation);
	//}
	float &GetDuckAmount()
	{
		static int m_flDuckAmount = g_pNetvars->GetOffset("DT_BasePlayer", "m_flDuckAmount");
		return SetValue<float>(m_flDuckAmount);
	}
	int m_nhitboxgayset()
	{
		static int m_nSequence = g_pNetvars->GetOffset("CBasePlayer", "m_nHitboxSet");
		return GetValue<int>(m_nSequence);
	}

	int GetSequence()
	{
		static int m_nSequence = g_pNetvars->GetOffset("DT_BaseAnimating", "m_nSequence");
		return GetValue<int>(m_nSequence);
	}
	int GetSequenceActivity(int sequence)
	{
		auto hdr = g_pModelInfo->GetStudiomodel(GetModel());

		if (hdr == nullptr)
			return -1;

		if (!hdr)
			return -1;

		static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::FindSignature("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 8B F1 83"));

		return get_sequence_activity(this, hdr, sequence);
	}

	int GetActiveWeaponIndex()
	{
		static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
		return *reinterpret_cast<int*>(uintptr_t(this) + m_hActiveWeapon) & 0xFFF; //m_hActiveWeapon
	}

	void SetSequence(int Sequence)
	{
		static int m_nSequence = g_pNetvars->GetOffset("DT_BaseAnimating", "m_nSequence");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nSequence) = Sequence;
	}
	const char* classname(void) {
		return ((const char*(__thiscall*)(C_BaseEntity*)) *(std::uintptr_t*) (*(std::uintptr_t*) this + 568))(this);
	}
	void SimulatedEveryTick(bool value)
	{
		static int m_bSimulatedEveryTick = g_pNetvars->GetOffset("DT_BaseEntity", "m_bSimulatedEveryTick");
		*reinterpret_cast<bool*>(uintptr_t(this) + m_bSimulatedEveryTick) = value;
	}
	Vector* GetVAngles() {
		static auto deadflag = g_pNetvars->GetOffset("DT_BasePlayer", "deadflag");
		return (Vector*)((uintptr_t)this + deadflag + 0x4);
	}
	void SetAbsAngles(Vector angles);
	void InvalidateBoneCache();
	void SetAbsOrigin(Vector origin);

	Vector GetAbsOrigin()
	{
		return Utils::GetVFunc<Vector&(__thiscall*)(void*)>(this, 10)(this);
	}
	Vector& GetAbsAngles()
	{
		typedef Vector& (__thiscall * OriginalFn)(void*);
		return ((OriginalFn)Utils::GetVFunc<OriginalFn>(this, 11))(this);
	}
	void SetAbsVelocity(Vector velocity);

	void ResetAnimationState(C_AnimState* state);

	void CreateAnimstate(C_AnimState* state);

	float get_max_desync_delta_aimware();

    C_BaseCombatWeapon* GetActiveWeapon()
    {
        static int m_hActiveWeapon = g_pNetvars->GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
        const auto weaponData      = GetValue<CBaseHandle>(m_hActiveWeapon);
        return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntityFromHandle(weaponData));
    }

	int GetAmmo()
	{
		static int m_iClip1 = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
		return GetValue<int>(m_iClip1);
	}

    int GetTeam()
    {
        static int m_iTeamNum = g_pNetvars->GetOffset("DT_BaseEntity", "m_iTeamNum");
        return GetValue<int>(m_iTeamNum);
    }
	CBaseHandle* GetWeapons()
	{
		return (CBaseHandle*)((DWORD)this + 0x2DE8);
	}
    int& GetFlags()
    {
        static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
        return SetValue<int>(m_fFlags);
    }

	void SetFlags(int offset)
	{
		static int m_fFlags = g_pNetvars->GetOffset("DT_BasePlayer", "m_fFlags");
		*reinterpret_cast<int*>(uintptr_t(this) + m_fFlags) = offset;
	}

    MoveType_t GetMoveType()
    {
        static int m_Movetype = g_pNetvars->GetOffset("DT_BaseEntity", "m_nRenderMode") + 1;
        return GetValue<MoveType_t>(m_Movetype);
    }

	float& GetSimulationTime()
	{
		static int m_flSimulationTime = g_pNetvars->GetOffset("DT_BaseEntity", "m_flSimulationTime");
		return SetValue<float>(m_flSimulationTime);
	}

	float GetOldSimulationTime()
	{
		static int m_flOldSimulationTime = g_pNetvars->GetOffset("DT_BaseEntity", "m_flSimulationTime") + 4;
		return GetValue<float>(m_flOldSimulationTime);
	}

	float& GetLowerBodyYaw()
	{
		static int m_flLowerBodyYawTarget = g_pNetvars->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		return SetValue<float>(m_flLowerBodyYawTarget);
	}

	float GetZoomLevel()
	{
		static int m_zoomlevel = g_pNetvars->GetOffset("DT_WeaponCSBaseGun", "m_zoomLevel");
		return GetValue<float>(m_zoomlevel);
	}

	void SetLowerBodyYaw(float value)
	{
		static int m_flLowerBodyYawTarget = g_pNetvars->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		*reinterpret_cast<float*>(uintptr_t(this) + m_flLowerBodyYawTarget) = value;
	}

	bool GetHeavyArmor()
	{
		static int m_bHasHeavyArmor = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHeavyArmor");
		return GetValue<bool>(m_bHasHeavyArmor);
	}

	int ArmorValue()
	{
		static int m_ArmorValue = g_pNetvars->GetOffset("DT_CSPlayer", "m_ArmorValue");
		return GetValue<int>(m_ArmorValue);
	}

	bool HasHelmet()
	{
		static int m_bHasHelmet = g_pNetvars->GetOffset("DT_CSPlayer", "m_bHasHelmet");
		return GetValue<bool>(m_bHasHelmet);
	}

    bool GetLifeState()
    {
        static int m_lifeState = g_pNetvars->GetOffset("DT_BasePlayer", "m_lifeState");
        return GetValue<bool>(m_lifeState);
    }

	bool IsScoped()
	{
		static int m_bIsScoped = g_pNetvars->GetOffset("DT_CSPlayer", "m_bIsScoped");
		return GetValue<bool>(m_bIsScoped);
	}
	
    int GetHealth()
    {
        static int m_iHealth = g_pNetvars->GetOffset("DT_BasePlayer", "m_iHealth");
        return GetValue<int>(m_iHealth);
    }

	varmapping_t* varmap(void) {
		return OFFSETPTR(varmapping_t*, 0x24);
	}

	void should_interp(bool interp) {
		auto map = this->varmap();

		if (!map)
			return;

		for (auto i = 0; i < map->m_interpolatedentries; i++)
			map->m_entries[i].m_needstointerpolate = interp;
	}
	bool IsSniper();

	bool IsKnifeorNade();

	bool IsNade();

    bool IsAlive() { return this->GetHealth() > 0 && this->GetLifeState() == 0; }

	bool IsEnemy();

	void SetupBonesFix();


    bool IsImmune()
    {
        static int m_bGunGameImmunity = g_pNetvars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");
        return GetValue<bool>(m_bGunGameImmunity);
    }

    int& GetTickBase()
    {
        static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
        return SetValue<int>(m_nTickBase);
    }

	int GetShotsFired()
	{
		static int m_iShotsFired = g_pNetvars->GetOffset("DT_CSPlayer", "cslocaldata", "m_iShotsFired");
		return GetValue<int>(m_iShotsFired);
	}

	void SetTickBase(int TickBase)
	{
		static int m_nTickBase = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_nTickBase");
		*reinterpret_cast<int*>(uintptr_t(this) + m_nTickBase) = TickBase;
	}
	

	Vector& GetEyeAngles()
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles");
		return GetValue<Vector>(m_angEyeAngles);
	}
	Vector GetViewOffset()
	{
		static int GetViewOffset = g_pNetvars->GetOffset("DT_CSPlayer", "m_vecViewOffset_0");
		return GetValue<Vector>(GetViewOffset);
	}
	QAngle* GetEyeAnglesXY()
	{
		return (QAngle*)((DWORD)this + g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles"));
	}
	void SetEyeAngles(Vector Angle)
	{
		static int m_angEyeAngles = g_pNetvars->GetOffset("DT_CSPlayer", "m_angEyeAngles");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_angEyeAngles) = Angle;
	}
    Vector GetOrigin()
    {
        static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
        return GetValue<Vector>(m_vecOrigin);
    }

	Vector GetOldOrigin()
	{
		static int m_vecOldOrigin = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_flFriction") + 12;
		return GetValue<Vector>(m_vecOldOrigin);
	}

	Vector GetNetworkOrigin()
	{
		static int m_vecNetworkOrigin = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_flFriction") + 40;
		return GetValue<Vector>(m_vecNetworkOrigin);
	}

	void SetOrigin(Vector Origin)
	{
		static int m_vecOrigin = g_pNetvars->GetOffset("DT_BaseEntity", "m_vecOrigin");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_vecOrigin) = Origin;
	}
	
	Vector GetVelocity()
	{
		static int m_vecVelocity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecVelocity[0]");
		return GetValue<Vector>(m_vecVelocity);
	}
	Vector WeaponShootPos(Vector& pos)
	{
		return Utils::GetVFunc<Vector & (__thiscall*)(void*)>(this, 281)(this);
		
	}
	bool IsPlayer()
	{
		return Utils::GetVFunc<bool & (__thiscall*)(void*)>(this, 157)(this);
		
	}
	std::array<float, 24> & m_flPoseParameter()
	{
		static int _m_flPoseParameter = g_pNetvars->GetOffset("DT_BaseAnimating", "m_flPoseParameter");
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}
	Vector GetShootPos()
	{
		static bool in_call_to_weapon_shoot = false;
		Vector pos;
		in_call_to_weapon_shoot = true;
		WeaponShootPos(pos);
		in_call_to_weapon_shoot = false;
		// correct intrinsic server client desync.
		pos.z -= GetViewOffset().z - floor(GetViewOffset().z);
		return pos;
	}

#define offset(func, type, offset) type& func { return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); }

	Vector& AbsVelocity() {
		return OFFSET(Vector, 0x94);
	}
	Vector& AbsVelocity1() {
		return OFFSET(Vector, 0x110);
	}
	Vector& ragdoll_angles() {
		return OFFSET(Vector, 0x31D8);
	}
	std::uint8_t& UnkByte() {
		return OFFSET(std::uint8_t, 0x35F8);
	}
	float& airtime() {
		return OFFSET(float, 0x3A10);
	}
	Vector& Mins() {
		return OFFSET(Vector, 0x324);
	}
	uint32_t& OccusionFlags() {
		return OFFSET(uint32_t, 0xA28);
	}
	uint32_t& OccusionFrameCount() {
		return OFFSET(uint32_t, 0xA30);
	}
	uint32_t& ReadableBones() {
		return OFFSET(uint32_t, 0x269C);
	}
	uint32_t& WriteableBones() {
		return OFFSET(uint32_t, 0x26A0);
	}
	uint32_t& BoneCounter() {
		return OFFSET(uint32_t, 0x2690);
	}
	int& m_iShotsFired() {
		return OFFSET(int, 0xA380);
	}
	float& LastBoneSetupTime() {
		return OFFSET(float, 0x2914);
	}
	offset(get_bone_array_for_write(), matrix3x4_t*, 0x26A8)
	Vector& Maxs() {
		return OFFSET(Vector, 0x330);
	}
	void SetVelocity(Vector velocity)
	{
		static int m_vecVelocity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_vecVelocity[0]");
		*reinterpret_cast<Vector*>(uintptr_t(this) + m_vecVelocity) = velocity;
	}
	bool is_player()
	{

		return CallVFunction< bool(__thiscall*) (C_BaseEntity*) >(this, 157) (this);
	}
	Vector GetAimPunchAngle()
	{
		return *reinterpret_cast<Vector*>(uintptr_t(this) + uintptr_t(0x302C));
	}

	Vector GetViewPunchAngle()
	{
		return *reinterpret_cast<Vector*>(uintptr_t(this) + uintptr_t(0x3020));
	}

    Vector GetEyePosition() 
	{
		Vector ret;
		typedef void(__thiscall *OrigFn)(void *, Vector&);
		Utils::GetVFunc<OrigFn>(this, 284)(this, ret);
		return ret;
	}

	ICollideable* GetCollideable()
	{
		return (ICollideable*)((DWORD)this + 0x31C);
	}

	VMatrix& GetCollisionBoundTrans()
	{
		return *reinterpret_cast<VMatrix*>((DWORD)this + 0x0440);
	}
	float GetNextAttack()
	{
		return *reinterpret_cast<float*>(uint32_t(this) + 0x2D70);
	}
	void SetCurrentCommand(CUserCmd *cmd)
	{
		static int m_hConstraintEntity = g_pNetvars->GetOffset("DT_BasePlayer", "localdata", "m_hConstraintEntity");
		*reinterpret_cast<CUserCmd**>(uintptr_t(this) + m_hConstraintEntity - 0xC) = cmd;
	}
	matrix3x4_t* bone_cache() {
		return FOFFSET(matrix3x4_t*, GetClientRenderable(), 0x290C);
	}
#define FUNCARGS(...) ( __VA_ARGS__ ); }
#define FUNC(func, sig, offset) auto func { return reinterpret_cast< sig >( offset ) FUNCARGS
	FUNC(standard_blending_rules(studiohdr_t* hdr, Vector* pos, Quaternion* q, float curTime, int boneMask), void(__thiscall*)(void*, studiohdr_t*, Vector*, Quaternion*, float, int), (void*)Utils::find("client_panorama.dll", "55 8B EC FF 75 18 F3 0F 10 45 ? 51").get())(this, hdr, pos, q, curTime, boneMask)
	matrix3x4_t GetBoneMatrix(int BoneID)
	{
		matrix3x4_t matrix;

		auto offset = *reinterpret_cast<uintptr_t*>(uintptr_t(this) + 0x26A8);
		if (offset)
			matrix = *reinterpret_cast<matrix3x4_t*>(offset + 0x30 * BoneID);

		return matrix;
	}
	float FireRate();

	void FixSetupBones(matrix3x4_t *Matrix);
	void russianfix(C_BaseEntity* player, matrix3x4_t* mat);
	void setupbonesfix(matrix3x4_t* Matrix, C_BaseEntity* e, float time);
	Vector GetHitboxPos(int hitbox);
	Vector hitbox_position(int hitbox_id, matrix3x4_t matrix[128]);
	Vector GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix, float *Radius);
	Vector GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix);
	Vector GetBonePosition(int Hitbox);
	float desync_delta();
	void FixSetupBones(matrix3x4_t* Matrix, C_BaseEntity* e);
	void setup_bone_matrix(C_BaseEntity* entity, matrix3x4_t* mat);
};
class C_BaseViewModel
{
public:

	inline DWORD GetOwner() {

		return *(PDWORD)((DWORD)this + 0x29CC);
	}

	inline int GetModelIndex() {

		return *(int*)((DWORD)this + 0x258);
	}
};
#define netvar(type, name, table, netvar)                         \
   type& name##() const {                                        \
        static int _##name = g_pNetvars->GetOffset(table, netvar);   \
        return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + _##name); \
    }
class C_EnvTonemapController : public C_BaseEntity
{
	template <typename T>
	T& SetValue(uintptr_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}
public:
	bool& use_custom_auto_exposure_min()
	{
		static int m_lifeState = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
		return SetValue<bool>(m_lifeState);
	}
	bool& use_custom_auto_exposure_max()
	{
		static int m_lifeState = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");
		return SetValue<bool>(m_lifeState);
	}
	float& custom_auto_exposure_min()
	{
		static int m_lifeState = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMin");
		return SetValue<float>(m_lifeState);

	}	
	float& custom_auto_exposure_max()
	{
		static int m_lifeState = g_pNetvars->GetOffset("DT_EnvTonemapController", "m_flCustomAutoExposureMax");
		return SetValue<float>(m_lifeState);

	}

};
class CBaseWeaponWorldModel : public C_BaseEntity
{
public:
	inline int* GetModelIndex() {

		return (int*)((DWORD)this + 0x258);
	}
};
class C_BaseCombatWeapon : public C_BaseEntity
{
private:
    template<class T>
    T GetPointer(const int offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
    // To get value from the pointer itself
    template<class T>
    T GetValue(const int offset)
    {
        return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

public:
	CBaseHandle GetWeaponWorldModel()
	{
		return *(CBaseHandle*)((uintptr_t)this + g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_hWeaponWorldModel")); //m_hWeaponWorldModel
	}

#define VirtualFn( cast ) typedef cast( __thiscall* OriginalFn )
	void SetModelIndex(int nModelIndex)
	{
		VirtualFn(void)(PVOID, int);
		Utils::GetVFunc< OriginalFn >(this, 75)(this, nModelIndex);
	}
	CBaseHandle m_hWeaponWorldModel_h()
	{
		return *(CBaseHandle*)((uintptr_t)this + 0x3234); //m_hWeaponWorldModel
	}
	HANDLE m_hWeaponWorldModel_c()
	{
		return *(HANDLE*)((uintptr_t)this + 0x3234); //m_hWeaponWorldModel
	}
	short* ItemDefinitionIndex2()
	{
		return (short*)((DWORD)this + g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex"));
	}
	int* ModelIndex()
	{
		return (int*)((uintptr_t)this + 0x258);
	}
	int* WorldModelIndex()
	{
		return (int*)((uintptr_t)this + g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iWorldModelIndex"));
	}
    ItemDefinitionIndex GetItemDefinitionIndex()
    {
        static int m_iItemDefinitionIndex = g_pNetvars->GetOffset("DT_BaseAttributableItem", "m_AttributeManager", "m_Item", "m_iItemDefinitionIndex");
        return GetValue<ItemDefinitionIndex>(m_iItemDefinitionIndex);
    }
	short GetItemDefenitionIndex()
	{
		static short m_iItemDefinitionIndex = g_pNetvars->GetOffset(("DT_BaseAttributableItem"), ("m_iItemDefinitionIndex"));
		return *reinterpret_cast<short*>(uintptr_t(this) + m_iItemDefinitionIndex);
	}
	bool is_knife()
	{
		auto iWeaponID = this->GetItemDefinitionIndex();
		return (iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_BAYONET
			|| iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_M9_BAYONET || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_FLIP
			|| iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_KARAMBIT || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_GUT
			|| iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_FALCHION || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_FLIP || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_PUSH
			|| iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_TACTICAL || iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_WIDOWMAKER 
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_STILETTO
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_SKELETON
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_OUTDOOR
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_URSUS
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_CANIS
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_CORD
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_PUSH
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_CSS
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFEGG
			||iWeaponID == ItemDefinitionIndex::WEAPON_KNIFE_T);
	}
	bool is_grenade()
	{
		if (!this)
			return false;

		auto WeaponId = this->GetItemDefinitionIndex();

		
		if (WeaponId == ItemDefinitionIndex::WEAPON_DECOY || WeaponId == ItemDefinitionIndex::WEAPON_INCGRENADE || WeaponId == ItemDefinitionIndex::WEAPON_MOLOTOV || WeaponId == ItemDefinitionIndex::WEAPON_HEGRENADE || WeaponId == ItemDefinitionIndex::WEAPON_SMOKEGRENADE || WeaponId == ItemDefinitionIndex::WEAPON_FLASHBANG)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	float GetInaccuracy()
	{
		typedef float(__thiscall* oInaccuracy)(PVOID);
		return Utils::GetVFunc< oInaccuracy >(this, 482)(this);
	}

	float GetSpread()
	{
		typedef float(__thiscall* oWeaponSpread)(PVOID);
		return Utils::GetVFunc< oWeaponSpread >(this, 452)(this);
	}

	float GetAccuracyPenalty()
	{
		static int m_fAccuracyPenalty = g_pNetvars->GetOffset("DT_WeaponCSBase", "m_fAccuracyPenalty");
		return GetValue<float>(m_fAccuracyPenalty);
	}

	float GetLastShotTime()
	{
		static int m_fLastShotTime = g_pNetvars->GetOffset("DT_WeaponCSBase", "m_fLastShotTime");
		return GetValue<float>(m_fLastShotTime);
	}
	
	void AccuracyPenalty()
	{
		typedef void(__thiscall *OrigFn)(void *);
		return Utils::GetVFunc<OrigFn>(this, 483)(this);
	} //472

    float GetNextPrimaryAttack()
    {
        static int m_flNextPrimaryAttack = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
        return GetValue<float>(m_flNextPrimaryAttack);
    }

    int GetAmmo()
    {
        static int m_iClip1 = g_pNetvars->GetOffset("DT_BaseCombatWeapon", "m_iClip1");
        return GetValue<int>(m_iClip1);
    }

    WeaponInfo_t* GetCSWpnData()
    {
        return Utils::CallVFunc<460, WeaponInfo_t*>(this);
    }

    std::string GetName()
    {
        ///TODO: Test if szWeaponName returns proper value for m4a4 / m4a1-s or it doesnt recognize them.
        return std::string(this->GetCSWpnData()->weapon_name);
    }
};


class OverlayText_t;

class IVDebugOverlay
{
public:
	virtual void            __unkn() = 0;
	virtual void            AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
	virtual void            AddBoxOverlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
	virtual void            AddSphereOverlay(const Vector& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
	virtual void            AddTriangleOverlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void            AddLineOverlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
	virtual void            AddTextOverlay(const Vector& origin, float duration, const char* format, ...) = 0;
	virtual void            AddTextOverlay(const Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
	virtual void            AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
	virtual void            AddSweptBoxOverlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
	virtual void            AddGridOverlay(const Vector& origin) = 0;
	virtual void            AddCoordFrameOverlay(const matrix3x4_t& frame, float flScale, int vColorTable[3][3] = NULL) = 0;
	virtual int             ScreenPosition(const Vector& point, Vector& screen) = 0;
	virtual int             ScreenPosition(float flXPos, float flYPos, Vector& screen) = 0;
	virtual OverlayText_t*  GetFirst(void) = 0;
	virtual OverlayText_t*  GetNext(OverlayText_t* current) = 0;
	virtual void            ClearDeadOverlays(void) = 0;
	virtual void            ClearAllOverlays() = 0;
	virtual void            AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
	virtual void            AddTextOverlayRGB(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
	virtual void            AddLineOverlayAlpha(const Vector& origin, const Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void            AddBoxOverlay2(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) = 0;
	virtual void            PurgeTextOverlays() = 0;
	virtual void            DrawPill(const Vector& mins, const Vector& max, float& diameter, int r, int g, int b, int a, float duration) = 0;

public:
	void AddCapsuleOverlay(Vector& mins, Vector& maxs, float pillradius, int r, int g, int b, int a, float duration)
	{
		Utils::GetVFunc<void(__thiscall*)(void*, Vector&, Vector&, float&, int, int, int, int, float)>(this, 24)(this, mins, maxs, pillradius, r, g, b, a, duration);
	};
};
extern IVDebugOverlay* g_pIVDebugOverlay;

class CGlowObjectManager
{
public:
	class GlowObjectDefinition_t {
	public:
		void Set(Color color, float bloom, int style)
		{
			m_vGlowColor = Vector(color.red, color.green, color.blue);
			m_flGlowAlpha = color.alpha;
			m_bRenderWhenOccluded = true;
			m_bRenderWhenUnoccluded = false;
			m_flBloomAmount = bloom;
			m_nGlowStyle = style;
		}

		C_BaseEntity* GetEnt() {
			return m_hEntity;
		}

		bool IsUnused() const { return m_nNextFreeSlot != GlowObjectDefinition_t::ENTRY_IN_USE; }

	public:
		C_BaseEntity* m_hEntity;
		Vector            m_vGlowColor;
		float            m_flGlowAlpha;

		char            unknown[4]; //pad 
		float            flUnk; //confirmed to be treated as a float while reversing glow functions 
		float            m_flBloomAmount;
		float            localplayeriszeropoint3;


		bool            m_bRenderWhenOccluded;
		bool            m_bRenderWhenUnoccluded;
		bool            m_bFullBloomRender;
		char            unknown1[1]; //pad 


		int                m_nFullBloomStencilTestValue; // 0x28 
		int                m_nGlowStyle;
		int                m_nSplitScreenSlot; //Should be -1 

											   // Linked list of free slots 
		int                m_nNextFreeSlot;

		// Special values for GlowObjectDefinition_t::m_nNextFreeSlot 
		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	};


	UtlVector< GlowObjectDefinition_t > m_GlowObjectDefinitions;
	int m_nFirstFreeSlot;
};

class Do
{
public:
	void DrawGlow();
};

extern Do* DoGlow;
extern CGlowObjectManager* g_pGlowManager;
