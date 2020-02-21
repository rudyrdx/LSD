#include <thread>
#include <Windows.h>  
#include <iostream> 
#include "Hooks.h"
#include "c_detours.h"
#include "Utils.h"
#include "Features.h"
#include "Events.h"
#include "ISurface.h"
#include "Vector.h"
#include "Menu.h"
#include "Hitboxes.h"
#include "knifechanger.h"
#include "libloaderapi.h"
#include "BulletTracer.h"
#include "ESP.h"
#include "logs2.h"
#include "hitsound.h"
#include <string>
#include <intrin.h>
#include "IBaseClientDll.h"
#include "IVEngineClient.h"


Misc     g_Misc;
Hooks    g_Hooks;
Event    g_Event;
int gitgrp;
typedef void(__fastcall* ClanTagFn)(const char*, const char*);
ClanTagFn dw_ClanTag;
void SetClanTag(const char* tag, const char* name)
{

if (!dw_ClanTag)
	dw_ClanTag = reinterpret_cast<ClanTagFn>(Utils::FindPattern2("engine.dll", "53 56 57 8B DA 8B F9 FF 15"));

if (dw_ClanTag)
	dw_ClanTag(tag, name);
}

void Clan_Tag()
{
if (!g_Menu.Config.Aimbot) 
return;

static int iLastTime;

float latency = g_pEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + g_pEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
static int timer = 0;
if (int(g_pGlobalVars->curtime * 2 + latency) != iLastTime)
{
	switch (timer)
	{
	case 0: SetClanTag("L S D ", "L S D  "); break;
	case 1: SetClanTag("L S D ", "L S D  "); break;
	}
	timer++;
	iLastTime = int(g_pGlobalVars->curtime * 2 + latency);
}
if (timer > 2)
	timer = 0;

}


void Warning(const char* msg, ...) // wintergang https://www.unknowncheats.me/forum/1923881-post1.html
{
if (msg == nullptr)
	return;
typedef void(__cdecl* MsgFn)(const char* msg, va_list);
static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Warning");
char buffer[989];
va_list list;
va_start(list, msg);
vsprintf(buffer, msg, list);
va_end(list);
fn(buffer, list);
}
template <typename T = uintptr_t>
static auto findPattern(const wchar_t* module, std::string_view pattern, size_t offset = 0) noexcept
{
	MODULEINFO moduleInfo;

	if (GetModuleInformation(GetCurrentProcess(), GetModuleHandleW(module), &moduleInfo, sizeof(moduleInfo))) {
		char* begin = static_cast<char*>(moduleInfo.lpBaseOfDll);
		char* end = begin + moduleInfo.SizeOfImage - pattern.length() + 1;

		for (char* c = begin; c != end; c++) {
			bool matched = true;
			auto it = c;

			if (*(c + pattern.length() - 1) != pattern.back())
				continue;

			for (auto a : pattern) {
				if (a != '?' && *it != a) {
					matched = false;
					break;
				}
				it++;
			}
			if (matched)
				return reinterpret_cast<T>(c + offset);
		}
	}
	MessageBox(NULL, (std::ostringstream{ } << "Failed to find pattern in " << module << '!').str().c_str(), "Error", MB_OK | MB_ICONERROR);
	exit(EXIT_FAILURE);
}



class Memory {
public:
	uintptr_t present;
	uintptr_t reset;
};
Memory memory;
std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)> originalPresent;
std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)> originalReset;
doextraboneprocessing_fn o_doextraboneprocessing = nullptr;
setup_bones_t o_setup_bones = nullptr;
update_anims_fn	o_update_anims = nullptr;
build_transformations_t	o_build_transformations = nullptr;
static uint32_t* get_vtable()
{
	static const auto table = (Utils::FindSignature("client_panorama.dll",
		"55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C")) + 0x47;
	return *reinterpret_cast<uint32_t * *>(table);
}
bool __fastcall get_bool(ConVar*, uint32_t)
{
	return false;
}
C_AnimState* last_process_state = nullptr;
bool __fastcall Hooks::c_mdl_cacheunknown_fn(void* mdl_cache, uint32_t)
{
	static auto return_to_setup_velocity = Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? F3 0F 7E 86");
	static auto return_to_setup_aim_matrix = Utils::Utils::FindSignature("client_panorama.dll", "F3 0F 10 8B ? ? ? ? 0F 57 C0 0F 2F C1 F3 0F 10 2D");

	if (g_pEngine->IsInGame())
	{
		if (_ReturnAddress() == reinterpret_cast<void*>(return_to_setup_velocity) && last_process_state)
			last_process_state->m_last_clientside_anim_update_time_delta = 0.f;

		if (_ReturnAddress() == reinterpret_cast<void*>(return_to_setup_aim_matrix) && last_process_state)
		{
			last_process_state->m_last_clientside_anim_update_time_delta
				= fmaxf(g_pGlobalVars->curtime - last_process_state->m_last_clientside_anim_update, 0.f);

			if (last_process_state->m_speed2d <= 0.f)
			{
				last_process_state->m_time_since_move = 0.f;
				last_process_state->m_time_since_stop
					= last_process_state->m_last_clientside_anim_update_time_delta + last_process_state->m_time_since_stop;
			}
			else
			{
				last_process_state->m_time_since_stop = 0.f;
				last_process_state->m_time_since_move
					= last_process_state->m_time_since_move + last_process_state->m_last_clientside_anim_update_time_delta;
			}
		}
	}

	return _unknown_fn(mdl_cache);
}
void Hooks::Init()
{
	// Get window handle
	while (!(g_Hooks.hCSGOWindow = FindWindowA("Valve001", nullptr)))
	{
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(50ms);
	}
	interfaces::Init();    // Get interfaces
	g_pNetvars = std::make_unique<NetvarTree>();// Get netvars after getting interfaces as we use them
	if (g_Hooks.hCSGOWindow)        // Hook WNDProc to capture mouse / keyboard input
		g_Hooks.pOriginalWNDProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(g_Hooks.WndProc)));
	/*auto bad = Utils::FindSignature("engine.dll", "0F 4F F0 89 5D FC" - 0x6);
	DWORD oldprotecc = 0;
	VirtualProtect((LPVOID)(bad), 5, 0x04, &oldprotecc);
	int fakeval = 62;
	std::memcpy((void*)(bad + 1), &fakeval, 4);
	VirtualProtect((LPVOID)(bad), 5, oldprotecc, &oldprotecc);*/

	//vf_Table xDDDDD
	auto tmp_addr =Utils::FindSignature(("client_panorama.dll"),
		("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C"));

	const auto dw_ccs_player_renderablevftable = *reinterpret_cast<DWORD*> (tmp_addr + 0x4E);
	if (dw_ccs_player_renderablevftable)
		o_setup_bones = vftable_hook::hook_manual< setup_bones_t >(
			reinterpret_cast<uintptr_t*> (dw_ccs_player_renderablevftable), 13, reinterpret_cast<setup_bones_t> (Hooks::hk_setup_bones)
			);

	//c_detours xDD
	auto c_csplayer__doextraboneprocessing = (void*)Utils::find("client_panorama.dll", "55 8B EC 83 E4 F8 81 EC FC 00 00 00 53 56 8B F1 57").get();
	auto update_clientside_animations = (void*)Utils::find("client_panorama.dll", "55 8B EC 51 56 8B F1 80 BE ? ? ? ? 00 74 ? 8B 06 FF").get();
	auto build_transformations = (void*)Utils::find("client_panorama.dll", "E8 ? ? ? ? 80 BF ? ? ? ? ? 0F 84 ? ? ? ? 83 BF").get();
	std::unique_ptr< c_detour > doextraboneprocessing_hk{ };
	std::unique_ptr< c_detour > update_anims_hk{ };
	doextraboneprocessing_hk = std::make_unique< c_detour >(c_csplayer__doextraboneprocessing, Hooks::DoExtraBonesProcessing, 6);
	update_anims_hk = std::make_unique< c_detour >(update_clientside_animations, Hooks::update_anims, 5);
	o_update_anims = (update_anims_fn)update_anims_hk->hook();
	o_doextraboneprocessing = (doextraboneprocessing_fn)doextraboneprocessing_hk->hook();
	

	//c_hook xD

	static c_hook<ConVar> hook(g_pCvar->FindVar("cl_csm_shadows"));
	hook.apply<void(__thiscall*)()>(13, get_bool);

	static c_hook<void> hooks(**reinterpret_cast<uint32_t * **>(uint32_t(Utils::FindSignature("client_panorama.dll", "8B 3D ? ? ? ? 8B D9 8B CF")) + 2));
	_unknown_fn = hooks.apply<unknown_fn_t>(33, c_mdl_cacheunknown_fn);

	/*static const auto state = **reinterpret_cast<IClientState * **>(Utils::FindSignature("engine.dll", "8B 3D ? ? ? ? 8A F9") + 2);
	static c_hook<IClientState> hooks(reinterpret_cast<IClientState*>(uint32_t(state) + 8));
	_packet_start = hooks.apply<packet_start_t>(5, packet_start);*/

	// VMTHooks
	g_Hooks.pClientHook = std::make_unique<VMTHook>(g_pClientDll);
	g_Hooks.pPredictionHook = std::make_unique<VMTHook>(g_pPrediction);
	g_Hooks.pEngineHook = std::make_unique<VMTHook>(g_pEngine);
	g_Hooks.pClientModeHook = std::make_unique<VMTHook>(g_pClientMode);
	g_Hooks.pSurfaceHook = std::make_unique<VMTHook>(g_pSurface);
	g_Hooks.pPanelHook = std::make_unique<VMTHook>(g_pPanel);
	g_Hooks.pModelHook = std::make_unique<VMTHook>(g_pModelRender);
	g_Hooks.pRenderViewHook = std::make_unique<VMTHook>(g_pRenderView);
	
	// Hook the table functions
	g_Hooks.pClientHook->Hook(vtable_indexes::frameStage, Hooks::FrameStageNotify);
	g_Hooks.pPredictionHook->Hook(14, Hooks::inprediction);
	g_Hooks.pClientHook->Hook(24, Hooks::WriteUsercmdDeltaToBuffer_hook);
	
	g_Hooks.pClientModeHook->Hook(vtable_indexes::createMove, Hooks::CreateMove);
	g_Hooks.pClientModeHook->Hook(44, Hooks::DoPostScreenEffects);
	g_Hooks.pEngineHook->Hook(59, Hooks::fire_event);
	//g_Hooks.pEngineHook->Hook(93, Hooks::IsHLTV);
	g_Hooks.pClientModeHook->Hook(vtable_indexes::view, Hooks::OverrideView);
	g_Hooks.pSurfaceHook->Hook(vtable_indexes::lockCursor, Hooks::LockCursor);
	g_Hooks.pPanelHook->Hook(vtable_indexes::paint, Hooks::PaintTraverse);
	g_Hooks.pModelHook->Hook(vtable_indexes::dme, Hooks::DrawModelExecute);
	g_Hooks.pRenderViewHook->Hook(vtable_indexes::sceneEnd, Hooks::SceneEnd);
	g_Event.Init();
	
	g_bullettracer.listener();
	skinchanger.ApplyAAAHooks();
	g::CourierNew = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::CourierNew, "Verdana", 14, 400, 0, 0, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	g::Tahoma = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::Tahoma, "Aaargh", 12, 700, 0, 0, FONTFLAG_DROPSHADOW);
	g::Logs = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::Logs, "BatmanForeverAlternate", 15, 400, 0, 0, FONTFLAG_ITALIC + FONTFLAG_OUTLINE + FONTFLAG_DROPSHADOW);
	g::trajen = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::trajen, "TrajanPro-Regular", 15, 400, 0, 0, FONTFLAG_DROPSHADOW);
	g::logo = g_pSurface->FontCreate();
	g_pSurface->SetFontGlyphSet(g::logo, "cherryfont", 32, 700, 0, 0,0);

	//g_pEngine->ExecuteClientCmd("clear");
	//g_pSurface->play_sound("common\\beep.wav");
	//g_pEngine->ExecuteClientCmd("cam_collision 1");

	g::cheat_start_time = GetTickCount() * 0.001f;
}

void Hooks::Restore()
{
  // Unhook every function we hooked and restore original one
	g_Hooks.pClientHook->Unhook(vtable_indexes::frameStage);
	g_Hooks.pPredictionHook->Unhook(14);
	g_Hooks.pClientModeHook->Unhook(vtable_indexes::createMove);
	g_Hooks.pClientModeHook->Unhook(vtable_indexes::view);
	g_Hooks.pClientModeHook->Unhook(44);
	g_Hooks.pSurfaceHook->Unhook(vtable_indexes::lockCursor);
	g_Hooks.pPanelHook->Unhook(vtable_indexes::paint);
	g_Hooks.pModelHook->Unhook(vtable_indexes::dme);
	g_Hooks.pRenderViewHook->Unhook(vtable_indexes::sceneEnd);
	//g_Hooks.pEngineHook->Unhook(vtable_indexes::hltv);
	g_Hooks.pEngineHook->Unhook(59);

	SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.pOriginalWNDProc));

}

std::vector<const char*> vistasmoke_mats = //b1g nosmoke
{
"particle/vistasmokev1/vistasmokev1_smokegrenade",
"particle/vistasmokev1/vistasmokev1_emods",
"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};
auto smoke_count = *(DWORD*)(Utils::FindSignature("client_panorama", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);

typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void NoScopeOverlay()
{
	g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!g::pLocalEntity)
		return;

	C_BaseCombatWeapon* pWeapon = g::pLocalEntity->GetActiveWeapon();
	if (!pWeapon)
		return;

	int screenx;
	int screeny;
	g_pEngine->GetScreenSize(screenx, screeny);

	int centerx = screenx / 2;
	int centery = screeny / 2;

	float spread = pWeapon->GetInaccuracy() * 300;
	int height = std::clamp(spread, 1.f, 25.f);
	int alpha = 255 - (height * 7.5f);

	if (g::pLocalEntity->IsScoped())
	{
		g_pSurface->DrawFilledRect(0, centery - (height / 2), screenx, centery + (height / 2));
		g_pSurface->DrawFilledRect(centerx + (height / 2), 0, centerx - (height / 2), screeny);

		g_pSurface->Line(0, centery - (height / 2), screenx, centery + (height / 2), Color(0, 0, 0, alpha));
		g_pSurface->Line(centerx + (height / 2), 0, centerx - (height / 2), screeny, Color(0, 0, 0, alpha));
		//	csgo->surface()->filled_rect(0, center.y - (height / 2), screen.x, height, color(0, 0, 0, alpha));
		//	csgo->surface()->filled_rect(center.x - (height / 2), 0, height, screen.y, color(0, 0, 0, alpha));
	}
}



void AutoRevolver(CUserCmd* cmd, C_BaseEntity* local_player)
{

	C_BaseCombatWeapon* weapon = local_player->GetActiveWeapon();
	if (!weapon || weapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
		return;

	static float delay = 0.f;
	if (delay < 0.15f)
	{
		delay += g_pGlobalVars->intervalPerTick;
		cmd->buttons |= IN_ATTACK;
	}
	else
		delay = 0.f;
}
inline void ConMsg(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg"); //This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there)
	char buffer[989];
	va_list list; //Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments
	va_start(list, msg);
	vsprintf_s(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}
float network_latency() {
	INetChannelInfo* nci = g_pEngine->GetNetChannelInfo();
	if (nci)
		return nci->GetAvgLatency(FLOW_INCOMING);

	return 0.0f;
}
int GetChokedTicksNetworked(C_BaseEntity* entity)
{
	float flSimulationTime = entity->GetSimulationTime();
	float flSimDiff = g_pGlobalVars->curtime - flSimulationTime;
	float latency = network_latency();
	return (int)(0.5f + (float)(max(0.0f, flSimDiff - latency)) / g_pGlobalVars->intervalPerTick);
}
void ModulateWorld() //credits to my nigga monarch
{
	static bool nightmode_performed = false, nightmode_lastsetting;

	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
	{
		if (nightmode_performed)
			nightmode_performed = false;
		return;
	}

	if (nightmode_performed != g_Menu.Config.Nightmode)
	{
		nightmode_lastsetting = g_Menu.Config.Nightmode;
		nightmode_performed = false;
	}

	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (!local_player)
		return;

	if (!local_player)
	{
		if (nightmode_performed)
			nightmode_performed = false;
		return;
	}

	if (nightmode_lastsetting != g_Menu.Config.Nightmode)
	{
		nightmode_lastsetting = g_Menu.Config.Nightmode;
		nightmode_performed = false;
	}

	if (!nightmode_performed)
	{
		static auto r_DrawSpecificStaticProp = g_pCvar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->nFlags &= ~FCVAR_CHEAT;
		r_DrawSpecificStaticProp->SetValue(1);

		static auto sv_skyname = g_pCvar->FindVar("sv_skyname");
		sv_skyname->nFlags &= ~FCVAR_CHEAT;

		static auto mat_postprocess_enable = g_pCvar->FindVar("mat_postprocess_enable");
		mat_postprocess_enable->SetValue(0);

		for (MaterialHandle_t i = g_pMaterialSys->FirstMaterial(); i != g_pMaterialSys->InvalidMaterial(); i = g_pMaterialSys->NextMaterial(i))
		{
			IMaterial* pMaterial = g_pMaterialSys->GetMaterial(i);

			if (!pMaterial)
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), "World"))
			{
				if (g_Menu.Config.Aimbot)
					pMaterial->ColorModulate(0.1, 0.1, 0.1);
				else
					pMaterial->ColorModulate(1, 1, 1);

				if (g_Menu.Config.Nightmode)
				{
					sv_skyname->SetValue("sky_grimmnight");
					//sky_night_01
					//sky_csgo_night02
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
					pMaterial->ColorModulate(0.05, 0.05, 0.05);
				}
				else
				{
					sv_skyname->SetValue("sky_grimmnight");
					pMaterial->ColorModulate(1.00, 1.00, 1.00);
				}
			}
			else if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
			{
				if (g_Menu.Config.Nightmode)
				{
					//pMaterial->ColorModulate(0.30, 0.30, 0.30);
					pMaterial->AlphaModulate(0.9);
				}
				else
				{
					pMaterial->ColorModulate(1, 1, 1);
					pMaterial->AlphaModulate(1);
				}
			}
		}
		nightmode_performed = true;
	}
}
void extrapolation()
{
	if (g::pLocalEntity->IsAlive())
	{
		for (int i = 1; i <= 64; i++)
		{
			auto ent = g_pEntityList->GetClientEntity(i);
			if (!ent || ent == g::pLocalEntity)
				continue;
		
			float CurrSimtime = ent->GetSimulationTime();
			float OldSimtime = ent->GetOldSimulationTime();
			float SimDelta = CurrSimtime - OldSimtime;
			int ChokedTicks = GetChokedTicksNetworked(ent);
			Vector CurrOrigin = ent->GetOrigin();
			auto origin = CurrOrigin;
			Vector OldOrigin;

			float DistDelta = (CurrOrigin - OldOrigin).LengthSqr();
			
			Vector VelPerTick = ent->GetVelocity() / SimDelta;
			Vector NewOrigin = CurrOrigin + (VelPerTick * (g_pGlobalVars->curtime - DistDelta));
			ent->SetAbsOrigin(NewOrigin);
			CurrOrigin = origin;
			
		}
    }
}
typedef void(*CL_FullUpdate_t) (void);
CL_FullUpdate_t CL_FullUpdate = nullptr;

std::uint8_t* pattern_scan(void* module, const char* signature) {
	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else {
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dos_headers = (PIMAGE_DOS_HEADER)module;
	auto nt_headers = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dos_headers->e_lfanew);

	auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	auto pattern_bytes = pattern_to_byte(signature);
	auto scan_bytes = reinterpret_cast<std::uint8_t*>(module);

	auto s = pattern_bytes.size();
	auto d = pattern_bytes.data();

	for (auto i = 0ul; i < size_of_image - s; ++i) {
		bool found = true;
		for (auto j = 0ul; j < s; ++j) {
			if (scan_bytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found) {
			return &scan_bytes[i];
		}
	}
	return nullptr;
}
void NewSkinChanger()
{
	if (!g::pLocalEntity)
		return;

	if (g::pLocalEntity->IsAlive())
	{
		int nLocalPlayerID = g_pEngine->GetLocalPlayer();
		C_BaseEntity* pLocal = (C_BaseEntity*)g_pEntityList->GetClientEntity(nLocalPlayerID);


		auto Weapons = g::pLocalEntity->GetWeapons();

		for (int i = 0; i < 64; i++) {
			if (Weapons[i] == -1)
				continue;

			C_BaseCombatWeapon* Weap = (C_BaseCombatWeapon*)g_pEntityList->GetClientEntityFromHandle(Weapons[i]);

			if (!Weap || Weap == nullptr)
				continue;


			ClientClass* pClass = g_pClientDll->GetAllClasses();

			auto world_model_handle = Weap->m_hWeaponWorldModel_h();

			const auto world_model = (CBaseWeaponWorldModel*)(g_pEntityList->GetClientEntityFromHandle(world_model_handle));


			if (Weap->GetClientClass()->ClassID == EClassIds::CKnife && g_Menu.Config.knife_model > 0)
			{
				*Weap->ItemDefinitionIndex2() = skinchanger.getKnifeItemDefinitionIndex();
				Weap->SetModelIndex(g_pModelInfo->GetModelIndex(skinchanger.getKnifeModel(true)));

				if (world_model) {
					*world_model->GetModelIndex() = g_pModelInfo->GetModelIndex(skinchanger.getKnifeModel(false));
				}
			}

		}
	}
}
bool SetShit = false;
struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};
template<class T>
static T* find_hud_element(const char* name) {
	static auto fn = *reinterpret_cast<DWORD * *>(pattern_scan(GetModuleHandleA("client_panorama.dll"), ("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(pattern_scan(GetModuleHandleA("client_panorama.dll"), ("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));
	return (T*)find_hud_element(fn, name);
}
void KnifeApplyCallbk()
{
	static auto fn = reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(pattern_scan(GetModuleHandleA("client_panorama.dll"), ("55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C")));

	auto element = find_hud_element<std::uintptr_t*>(("CCSGO_HudWeaponSelection"));

	auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xA0);
	if (hud_weapons == nullptr)
		return;

	if (!*hud_weapons->get_weapon_count())
		return;

	for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
		i = fn(hud_weapons, i);

	static auto CL_FullUpdate = reinterpret_cast<CL_FullUpdate_t>(Utils::FindPattern("engine.dll", reinterpret_cast<PBYTE>("\xA1\x00\x00\x00\x00\xB9\x00\x00\x00\x00\x56\xFF\x50\x14\x8B\x34\x85"), "x????x????xxxxxxx"));
	CL_FullUpdate();
}
void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto oFrameStage = g_Hooks.pClientHook->GetOriginal<FrameStageNotify_t>(vtable_indexes::frameStage);

	if (g_Menu.Config.Aimbot && g::pLocalEntity && g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		for (auto matName : vistasmoke_mats)
		{
			*(int*)smoke_count = 0;
			IMaterial* mat = g_pMaterialSys->FindMaterial(matName, "Other textures");
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		}
	}
	//player_log::get().log(curStage);
	static int stored_shit = 0;
	if (stored_shit != g_Menu.Config.knife_model)
	{
		if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && g::pLocalEntity && g::pLocalEntity->IsAlive() && g::pLocalEntity->GetActiveWeapon())
		{
			KnifeApplyCallbk();
			SetShit = true;
		}
		stored_shit = g_Menu.Config.knife_model;
	}
	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		NewSkinChanger();
	}


	if (curStage == FRAME_NET_UPDATE_START)
	{
		/* bullet tracers */
		g_bullettracer.draw();
	}
	
	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
	{
		for (auto i = 1; i <= 64; i++) {
			auto e = (C_BaseEntity*) g_pEntityList->GetClientEntity(i);

			// Entity validation checks
			if (!e || !e->IsAlive() || e->IsDormant() || !e->AnimState() || e == g::pLocalEntity|| !g_pEngine->IsConnected() && !g_pEngine->IsInGame())
				continue;

			e->should_interp(false);//raw data 
		}
	}
	if (curStage == 5)
	{
		//g_Misc.nightmode();
		for (int i = 1; i <= g_pGlobalVars->maxClients; i++)
		{
			if (i == g_pEngine->GetLocalPlayer()) continue;

			auto pCurEntity = g_pEntityList->GetClientEntity(i);
			if (!pCurEntity) continue;

			*(int*)((uintptr_t)pCurEntity + 0xA30) = g_pGlobalVars->framecount; //we'll skip occlusion checks now
			*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;//clear occlusion flags
		}
		if (g_pEngine->IsInGame() && g_pEngine->IsConnected() && g::pLocalEntity && g::pLocalEntity->IsAlive())
		{
			for (int i = 1; i <= 64; i++)
			{
				static auto util_playerbyindex = Utils::FindSignature("server.dll", "85 C9 7E 2A A1");
				static auto draw_server_hitboxes = Utils::FindSignature("server.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE");
				auto t = -1.0f;
				auto idx = i;
				__asm {
					mov ecx, idx
					call util_playerbyindex
					cmp eax, 0
					je gay
					pushad
					movss xmm1, t
					push 0
					mov ecx, eax
					call draw_server_hitboxes
					popad
				}

			gay:
				{

				}
			}

		}
	}
	/*g::shotshagdiab = g::shotfiredb - g::shothitb;
	if (g::shotfiredb > 1 || g::shotshagdiab < 0)
	{
		g::shothitb = 0;
		//g::shotfiredb = 0;
		g::shotshagdiab = 0;
	}*/
	
	
	g_Misc.ThirdPerson(curStage);

	g_Resolver.FrameStage(curStage);

	
	oFrameStage(curStage);
}
int GetTickbase(CUserCmd* ucmd) {

	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;

	if (!ucmd)
		return g_tick;

	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = g::pLocalEntity->GetTickBase();
	}
	else {
		// Required because prediction only runs on frames, not ticks
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick
		++g_tick;
	}

	g_pLastCmd = ucmd;
	return g_tick;
}
void FixWalk(CUserCmd* pCmd)
{
	if (!g_Menu.Config.Aimbot&&!g_pEngine->IsInGame())
		return;

	if (pCmd->buttons & IN_FORWARD)
	{
		pCmd->forwardmove = 450;
		pCmd->buttons &= ~IN_FORWARD;
	}
	else if (pCmd->buttons & IN_BACK)
	{
		pCmd->forwardmove = -450;
		pCmd->buttons &= ~IN_BACK;
	}
	if (pCmd->buttons & IN_MOVELEFT)
	{
		pCmd->sidemove = -450;
		pCmd->buttons &= ~IN_MOVELEFT;
	}
	else if (pCmd->buttons & IN_MOVERIGHT)
	{
		pCmd->sidemove = 450;
		pCmd->buttons &= ~IN_MOVERIGHT;
	}
}
//CUserCmd* lastcmd = nullptr;
float get_estimate_server_time(CUserCmd* cmd) 
{
	//double v0; // st7@0

	const auto v1 = static_cast<INetChannelInfo*> (g_pEngine->GetNetChannelInfo());

	const auto v3 = v1->GetAvgLatency(INetChannelInfo::LOCALPLAYER);
	const auto v4 = v1->GetAvgLatency(INetChannelInfo::GENERIC);

	return v3 + v4 + TICKS_TO_TIME(1) + TICKS_TO_TIME(cmd->tick_count);
}
bool __fastcall Hooks::CreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* pCmd)
{
	// Call original createmove before we start screwing with it
	static auto oCreateMove = g_Hooks.pClientModeHook->GetOriginal<CreateMove_t>(vtable_indexes::createMove);
	oCreateMove(thisptr, edx, sample_frametime, pCmd);

	if (!pCmd || !pCmd->command_number)
		return oCreateMove;

	auto CL_SendMove = []() {
		using CL_SendMove_t = void(__fastcall*)(void);
		static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Utils::FindSignature("engine.dll", "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");

		CL_SendMoveF();
	};

	// Get globals
	g::pCmd = pCmd;
	g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	g::bSendPacket = true;
	g::curtime = get_estimate_server_time(pCmd);
	/*if (g_Menu.Config.doubletap)
	{
		if (GetAsyncKeyState('I'))
		{
			if (g::bSendPacket)
			{
				while (pCmd->buttons & IN_ATTACK)
				{
					CL_SendMove();
					break;
				}
			}
		}
	}*/
	
	if (!g::pLocalEntity)
		return oCreateMove;

	uintptr_t *framePtr;
	__asm mov framePtr, ebp;

	if (g_Menu.Config.Aimbot)
	{
		

		g_pEngine->ExecuteClientCmd("mat_postprocess_enable 0");
	}
	g::OriginalView = g::pCmd->viewangles;
	static float SpawnTime = 0.0f;
	FixWalk(pCmd);
	g_Misc.OnCreateMove();
	if (GetAsyncKeyState(VK_SHIFT))
		g_Misc.do_fakewalk(pCmd, g_Menu.Config.fakeduckspeed);
	GetTickbase(pCmd);
	auto sv_cheats = g_pCvar->FindVar("sv_cheats");

	// spoof
	if (sv_cheats->GetInt() != 1) {
		*reinterpret_cast<int*>((DWORD)& sv_cheats->fnChangeCallback + 0xC) = 0;
		sv_cheats->SetValue(1);
	}

	//if (GetAsyncKeyState('I')) // server paused
	//{
	//	static bool flip = false; flip = !flip;
	//	//g::bSendPacket = g_pEngine->GetNetChannel()->m_nChokedPackets;
	//	//g::pCmd->command_number = flip ? g::bSendPacket :!g::bSendPacket;
	//	g_pClientState->m_bPaused = flip ? true:false;
	//	
	//}
	//else
	//{
		g_pClientState->m_bPaused =false;
	//}
	//if (GetAsyncKeyState('I')) // server paused
	//{
	//	static bool flip = false; flip = !flip;
	//	//g_pClientState->insimulation = true;
	////	pCmd->tick_count = g_pClientState->oldtickcount;
	////	pCmd->command_number = 
	//	g::bSendPacket = flip ? true:false;
	//}
	//extrapolation();
	//if (g_Menu.Config.doubletap)
	//{
	//	if (!g_pClientState->chokedcommands)
	//		g::m_nTickbaseShift = TIME_TO_TICKS(g_Menu.Config.tickbaseshift);
	//}
	
		/*if (g_Menu.Config.doubletap)
		{
			if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
				return;

			if (pCmd->buttons & 1 && g_pGlobalVars->tickcount > g::attacktime)
				g::isattacking = true;
			else
				g::isattacking = false;
			if(g::isattacking)
			g::m_nTickbaseShift = TIME_TO_TICKS(g_Menu.Config.tickbaseshift);
		}*/
		AutoRevolver(pCmd, g::pLocalEntity);
		prediction::get().start();
		{
			g_Misc.DoFakeLag();
			g_AntiAim.predict_lby_update(sample_frametime, pCmd, g::bSendPacket);
			g_AntiAim.OnCreateMove();
			aimbot->run_aimbot(pCmd);
		}
		prediction::get().finish();


	g_Misc.MovementFix(g::OriginalView);
		g_Resolver.manage_local_fake_animstate();

	
	if (g::bSendPacket)
	{
		g::AngleAA = g::pCmd->viewangles;
	}
	else
	{
		g::FakeAngle = g::pCmd->viewangles;
	}
	
	g_Math.Clamp(g::pCmd->viewangles);
	*(bool*)(*framePtr - 0x1C) = g::bSendPacket;
	
	g::pCmd->buttons |= IN_BULLRUSH; // hehe
	
	return false;
}
void __fastcall Hooks::fire_event(IVEngineClient* engine_client, uint32_t)
{
	static auto _fire_event = g_Hooks.pEngineHook->GetOriginal<fire_event_t>(59);
	const auto local =g::pLocalEntity;

	if (!local || !local->IsAlive() || !engine_client->IsInGame())
		return _fire_event(engine_client);

	auto cur_event = *reinterpret_cast<c_event_info * *>(reinterpret_cast<uintptr_t>(g_pClientState) + 0x4E64);
	if (!cur_event)
		return _fire_event(engine_client);

	c_event_info* next = nullptr;
	do
	{
		next = *(c_event_info * *)((uintptr_t)cur_event + 0x38);
		if (cur_event->class_id - 1 == 197)
			cur_event->fire_delay = 0.0f;

		cur_event = next;

	} while (next);

	return _fire_event(engine_client);
}
Do* DoGlow;

void Do::DrawGlow()
{
	CGlowObjectManager* GlowObjectManager = (CGlowObjectManager*)g_pGlowManager;
	g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	//for (int i = 0; i < GlowObjectManager->m_GlowObjectDefinitions.size; ++i)
	//{
	//	if (GlowObjectManager->m_GlowObjectDefinitions[i].IsUnused() || !GlowObjectManager->m_GlowObjectDefinitions[i].GetEnt())
	//		continue;
	//	auto* object = &GlowObjectManager->m_GlowObjectDefinitions[i];
	//	auto entity = reinterpret_cast<C_BaseEntity*>(object->GetEnt());
	//	if (!entity || entity->IsDormant())
	//		continue;
	//	//switch (entity->GetClientClass()->ClassID)
	//	//{
	//	//default:
	//	//case 40:
	//	///*	if (entity->GetClientClass()->ClassID == (int)EClassIds::CCSPlayer && (g::pLocalEntity != entity) && g::pLocalEntity->GetTeam() != entity->GetTeam())
	//	//	{
	//	//		if (g_Menu.Config.Aimbot)
	//	//			object->Set(Color(128, 0, 128, 255), .35, 0);
	//	//
	//	//	}
	//	//	if (entity->GetClientClass()->ClassID == (int)EClassIds::CCSPlayer && (g::pLocalEntity == entity))
	//	//	{
	//	//		if (g_Menu.Config.Aimbot)
	//	//			object->Set(Color(96, 28, 255, 255), .35, 1);
	//	//	}*/
	//	//}
	//}
}

#define XorStr( s ) ( s )
#define _(s) xorstr<sizeof(s), random::_char<__COUNTER__>::value>(s, std::make_index_sequence<sizeof(s)>()).dec().c_str()
void modulate(const Color color, IMaterial* material)
{
	if (material)
	{
		material->modulate(color);

		auto tint = material->find_var(_("$envmaptint"));
		if (tint)
			tint->set_vector(Vector(color.red / 255.f, color.green / 255.f, color.blue / 255.f));
	}

	g_pRenderView->set_color_modulation(color);
}
void modulatess(const Color color, IMaterial* material)
{
	if (material)
	{
		material->modulater(color);

		auto tint = material->find_var(_("$envmaptint"));
		if (tint)
			tint->set_vector(Vector(color.red / 255.f, color.green / 255.f, color.blue / 255.f));
	}

	g_pRenderView->set_color_modulation(color);
}
void modulate_exp(IMaterial* material, const float alpha, const float width)
{
	const auto transform = material->find_var(_("$envmapfresnelminmaxexp"));
	if (transform)
		transform->set_vector_component(width * alpha, 1);
}

int __fastcall Hooks::DoPostScreenEffects(void* ecx, void* edx, int a1)
{
	static auto oDoPostScreenEffects = g_Hooks.pClientModeHook->GetOriginal<DoPostScreenEffects_t>(44);

	if (g_Menu.Config.Aimbot)
		DoGlow->DrawGlow();

	return oDoPostScreenEffects(ecx, edx, a1);
}
#define mins(a,b)            (((a) < (b)) ? (a) : (b))

bool bInSendMove = false, bFirstSendMovePack = false;

bool transmit(bool only_reliable) {
	auto nc = FOFFSET(void*, g_pClientState, 0x9C);

	if (only_reliable) {
		((std::uint32_t*) nc)[24] = 0;
		*((std::uint8_t*) nc + 100) = 0;
	}

	return (*(int(__stdcall * *)(std::uint32_t))(*(std::uint32_t*) nc + 184))(0) != 0;
}
bool inSendMove = false, firstSendMovePack = false;

bool __fastcall Hooks::WriteUsercmdDeltaToBuffer_hook(void* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd)
{
	static DWORD WriteUsercmdDeltaToBufferReturn = (DWORD)Utils::FindSignature("engine.dll", "84 C0 74 04 B0 01 EB 02 32 C0 8B FE 46 3B F3 7E C9 84 C0 0F 84");
	static auto sendmovecall = Utils::FindSignature("engine.dll", "84 C0 74 04 B0 01 EB 02 32 C0 8B FE 46 3B F3 7E C9 84 C0 0F 84 ? ? ? ?");
	static auto ofunct = g_Hooks.pClientHook->GetOriginal<WriteUsercmdDeltaToBufferFn>(24);

	if (_ReturnAddress() != (void*)sendmovecall /*|| !g_Menu.Config.doubletap|| g_Menu.Config.Fakelag >=6*/ ||
		g::m_nTickbaseShift <= 0)
	{
		g::m_nTickbaseShift = 0;
		return ofunct(ECX, nSlot, buf, from, to, isNewCmd);
	}
	if (from != -1)
		return true;
	
	auto CL_SendMove = []() {
		using CL_SendMove_t = void(__fastcall*)(void);
		static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Utils::FindSignature("engine.dll", "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");

		CL_SendMoveF();
	};

	auto WriteUsercmd = [](bf_write* buf, CUserCmd* in, CUserCmd* out) {
		using WriteUsercmd_t = void(__fastcall*)(bf_write*, CUserCmd*, CUserCmd*);
		static WriteUsercmd_t WriteUsercmdF = (WriteUsercmd_t)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");
		WriteUsercmdF(buf, in, out);

	};
	// number of backup and new commands
	int* pNumBackupCommands = (int*)((uintptr_t) buf - 0x30);
	int* pNumNewCommands = (int*)((uintptr_t)buf - 0x2C);
	auto net_channel = *reinterpret_cast<NetChannel * *>(reinterpret_cast<uintptr_t>(g_pClientState) + 0x9C);
	int32_t new_commands = *pNumNewCommands;
	// Manipulate CLC_Move
	auto nextcmdnumber = g_pClientState->lastoutgoingcommand + g_pClientState->chokedcommands + 1;
	auto totalnewcommands = min(g::m_nTickbaseShift, 60);
	g::m_nTickbaseShift -= totalnewcommands;

	from = -1;
	*pNumNewCommands = totalnewcommands;
	*pNumBackupCommands = 0;
	//real cmds
	for (to = nextcmdnumber - new_commands + 1; to <= nextcmdnumber; to++)
	{
		if (!ofunct(ECX, nSlot, buf, from, to, true))
			return false;

		from = to;
	}
	auto lastrealcmd = g_pInput->GetUserCmd(nSlot, from);
	CUserCmd fromcmd;

	if (lastrealcmd)
		fromcmd = *lastrealcmd;

	CUserCmd tocmd = fromcmd;
	tocmd.command_number = nextcmdnumber++;
	tocmd.tick_count += 100;

	//fake cmds
	for (int i = new_commands; i <= totalnewcommands; i++)
	{
		WriteUsercmd(buf, &tocmd, &fromcmd);
		fromcmd = tocmd;
		tocmd.command_number++;
		tocmd.tick_count++;
	}

	return true;
}

void __fastcall Hooks::SceneEnd(void *ecx, void *edx)
{
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected()||!g::pLocalEntity->IsAlive())
		return;
	static auto oSceneEnd = g_Hooks.pRenderViewHook->GetOriginal<SceneEnd_t>(vtable_indexes::sceneEnd);

	if (!g::pLocalEntity)
		return;

	static const auto glow = find_mat("models/inventory_items/trophy_majors/crystal_clear");
	if(!glow)
	return;
	static const auto Es0 = find_mat("glowOverlay");
	if (!Es0)
		return;
	static const auto fbi = find_mat("models/gibs/glass/glass");
	if (!fbi)
		return;
	static const auto tags = find_mat("models/inventory_items/dogtags/dogtags_lightray");
	if (!tags)
		return;
	static const auto Mat = find_mat("models/player/ct_fbi/ct_fbi_glass");
	if (!Mat)
		return;
		
		float localcol[3] = { 1.0f,0.0f,0.0f };
		for (int i = 1; i <= 64; i++)
	   	{
			auto local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
			auto ent = g_pEntityList->GetClientEntity(i);
			if (!ent
				|| !ent->IsAlive()
				|| ent->IsDormant())
				continue;
			if (ent==local)
			{
			
			}
			else if(ent->GetTeam() != g::pLocalEntity->GetTeam()&& ent)
			{
				if (g_Menu.Config.fakeduck)
				{
					//fbi->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					//modulate(Color(33, 33, 33), fbi);
				//	g_pModelRender->ForcedMaterialOverride(fbi);
				//	ent->DrawModel(1, 255);
				//	g_pModelRender->ForcedMaterialOverride(nullptr);
				//	tags->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
				//	modulate(Color(0, 255, 21), tags);
				//	g_pModelRender->ForcedMaterialOverride(tags);
				//	ent->DrawModel(1, 255);
				//	g_pModelRender->ForcedMaterialOverride(nullptr);
					Mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					modulate(Color(69, 7, 37), Mat);
					//modulate_exp(fbi, 255, 0.7);
					g_pModelRender->ForcedMaterialOverride(Mat);
					ent->DrawModel(1, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
					Mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					modulate(Color(69, 7, 37), Mat);
					//modulate_exp(fbi, 255, 0.7);
					g_pModelRender->ForcedMaterialOverride(Mat);
					ent->DrawModel(1, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
					Es0->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					modulatess(Color(0, 98, 255), Es0);
					g_pRenderView->SetBlend(0.5f);
				//	modulate_exp(Es0, 100.0f, 1.f);
					g_pModelRender->ForcedMaterialOverride(Es0);
					ent->DrawModel(1, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
					Es0->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
					modulatess(Color(255, 0, 0), Es0);
					g_pRenderView->SetBlend(0.5f);
					//	modulate_exp(Es0, 100.0f, 1.f);
					g_pModelRender->ForcedMaterialOverride(Es0);
					ent->DrawModel(1, 255);
					g_pModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	oSceneEnd(ecx, edx);
}

inline static std::unique_ptr<c_hook<uint32_t>> hk;
typedef void(__thiscall* update_client_side_animation_t)(C_BaseEntity*);

typedef void(__thiscall* do_extra_bone_processing_t)(void*, void*, void*, void*, const matrix3x4_t&, uint8_t*, void*);
inline static do_extra_bone_processing_t _do_extra_bone_processing;
inline static update_client_side_animation_t _update_client_side_animation;
int proxy_is_player(C_BaseEntity* player, void* return_address, void* eax)
{
	static const auto return_to_should_skip_animframe = Utils::FindSignature("client_panorama.dll", "84 C0 75 02 5F C3 8B 0D");

	if (return_address != reinterpret_cast<void*>(return_to_should_skip_animframe))
		return 2;

	const auto local = g::pLocalEntity;

	if (!local || !local->IsAlive() || !player->IsEnemy())
		return 2;

	return !(player->effects() & 8);
}
static uint32_t is_player_retn_address = 0;
bool __declspec(naked) is_player(void* eax, void* edx)
{
	_asm
	{
		push eax

		mov eax, [esp + 4]
		mov is_player_retn_address, eax

		push is_player_retn_address
		push ecx
		call proxy_is_player

		cmp eax, 1
		je _retn1

		cmp eax, 0
		je _retn0

		mov al, 1
		retn

		_retn0 :
		mov al, 0
			retn

			_retn1 :
		pop eax
			mov eax, is_player_retn_address
			add eax, 0x6B
			push eax
			retn
	}
}

void Hooks::update_anims(void* ecx, void* edx)
{
	C_BaseEntity* player = (C_BaseEntity*)ecx;
	const auto local = g::pLocalEntity;
	if (!local || !local->IsAlive() || !player->is_local_player())
		return o_update_anims(player);

	if (g::enable_bones)	
		o_update_anims(player);
}

void __fastcall Hooks::packet_start(IClientState* state, uint32_t, int incoming_sequence, int outgoing_acknowledged)
{
	const auto local =g::pLocalEntity;
	if (!local || !local->IsAlive() || !g_Menu.Config.Aimbot || !g_pEngine->IsInGame())
		return _packet_start(state, incoming_sequence, outgoing_acknowledged);

	for (auto it = cmds.begin(); it != cmds.end(); it++)
		if (*it == outgoing_acknowledged)
		{
			cmds.erase(it);
			return _packet_start(state, incoming_sequence, outgoing_acknowledged);
		}
}

void __fastcall Hooks::DoExtraBonesProcessing(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7)
{
	C_BaseEntity* player = (C_BaseEntity*)ecx;
	if (player->effects() & 8)
		return;

	const auto state = player->AnimState();

	if (!state || !state->m_entity)
		return o_doextraboneprocessing(ecx, a2, a3, a4, a5, a6, a7);

	auto o_on_ground = state->m_on_ground;
	state->m_on_ground = false;
	o_doextraboneprocessing(ecx, a2, a3, a4, a5, a6, a7);
	state->m_on_ground = o_on_ground;
}

constexpr uint32_t seed = 0x45C3370D;
uint32_t runtime_basis = seed;
constexpr uint32_t prime = 0x1000193;
inline uint32_t fnv1a_rt(const char* key) {
	const char* data = const_cast<char*>(key);
	auto hash = runtime_basis;

#ifdef RELEASE
	hash ^= __readfsdword(0x20);
#endif

	for (size_t i = 0; i < strlen(key); ++i) {
		const uint8_t value = data[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;
}
#define ALPHA_FREQUENCY 255.0f / 5.0f		
#define ALPHA_FREQUENCY_RETURN 10.0f / 5.0f 


void __fastcall Hooks::DrawModelExecute(void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	static auto oDrawModelExecute = g_Hooks.pModelHook->GetOriginal<DrawModelExecute_t>(vtable_indexes::dme);
	
	const char* ModelName = g_pModelInfo->GetModelName((model_t*)info.pModel);
	std::string ModelNames = g_pModelInfo->GetModelName(info.pModel);

	C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(info.index);
	static const auto Es0 = find_mat("glowOverlay");
	if (!Es0)
		return;
	static const auto glow = find_mat("dev/glow_armsrace");
	if (!glow)
		return;
	static const auto fbi = find_mat("chamsAnimated");
	if (!fbi)
		return;
	static const auto tags = find_mat("models/inventory_items/dogtags/dogtags_lightray");
	if (!tags)
		return;
	static const auto Mat = find_mat("models/player/ct_fbi/ct_fbi_glass");
	static const auto Mats = find_mat("models/inventory_items/music_kit/darude_01/mp3_detail");
	if (!Mat)
		return;
	//static const auto simple = find_mat("simple_regular");
	static auto Material = g_pMaterialSys->FindMaterial("models/inventory_items/music_kit/darude_01/mp3_detail", "Model textures");
	static float r; r += 0.001f; if (r > 1) r = 0;
	Color color = color.FromHSB(r, 1, 1);
	if (pPlayerEntity && pPlayerEntity->IsAlive() && !pPlayerEntity->IsDormant() && g_Aimbot.Matrix[info.index] && strstr(ModelName, "models/player")
		&& strstr(ModelName, "arms"))
		oDrawModelExecute(ecx, context, state, info, matrix);
	else
		oDrawModelExecute(ecx, context, state, info, matrix);

	if (strstr(ModelName, "models/player") != nullptr)
	{

		const auto player = dynamic_cast<C_BaseEntity*> (pPlayerEntity);

		if (!player)
			return;

	
		if (g::pLocalEntity && player == g::pLocalEntity && g::pLocalEntity->IsAlive() && g_pGlobalVars->curtime - g::pLocalEntity->m_flSpawnTime() > 3.f)
		{
			if (g_Menu.Config.local_chams)
			{
				if (g_Resolver.m_got_fake_matrix && g::intp)
				{
					for (auto& i : /*g_Resolver.m_fake_matrix*/g::fakematrix)
					{
						i[0][3] += info.origin.x;
						i[1][3] += info.origin.y;
						i[2][3] += info.origin.z;
					}
					glow->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					modulate(Color(0, 255, 68),glow);
					//g_pRenderView->SetBlend(0.25f + sin(g_pGlobalVars->curtime * 5.f) * .2f);
					g_pModelRender->ForcedMaterialOverride(glow);
					oDrawModelExecute(ecx, context, state, info,/* g_Resolver.m_fake_matrix*/g::fakematrix);
					g_pModelRender->ForcedMaterialOverride(nullptr);
					//g_pRenderView->SetBlend(1.f);

					for (auto& i : /*g_Resolver.m_fake_matrix*/g::fakematrix)
					{
						i[0][3] -= info.origin.x;
						i[1][3] -= info.origin.y;
						i[2][3] -= info.origin.z;
					}
				}
			}
		}

	}
		
	//}
	

	if (strstr(ModelName, "weapons/v_") && g_Menu.Config.weapon_chams && !g::intp) {
		if (!g::pLocalEntity->IsAlive())
			return;
		tags->AlphaModulate(0);
		g_pModelRender->ForcedMaterialOverride(tags);
		oDrawModelExecute(ecx, context, state, info, matrix);
		//modulate(Color(255, 0, 0), Es0);
		//Es0->AlphaModulate(0.1f);
	//	modulate_exp(Es0, 70.0f, 0.1f);
		Es0->AlphaModulate(0.25f);
		g_pRenderView->SetBlend(0.6);
		g_pModelRender->ForcedMaterialOverride(Es0);
		oDrawModelExecute(ecx, context, state, info, matrix);
		modulate(Color(33,33,33), Mat);
		Mat->AlphaModulate(0.6f);
		g_pModelRender->ForcedMaterialOverride(Mat);
		oDrawModelExecute(ecx, context, state, info, matrix);
		
	}
	if (g_Menu.Config.hand_chams && strstr(ModelName, _("arms")) != nullptr && !g::intp /*|| fnv1a_rt(ModelName) == 2565982016*/ )
	{
		/*fbi->AlphaModulate(255.0f);
		fbi->ColorModulate(33, 33, 33);*/
		modulate(Color(
			196, 37, 85
		), fbi);
		g_pModelRender->ForcedMaterialOverride(fbi);
		oDrawModelExecute(ecx, context, state, info, matrix);
		tags->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
		modulate(Color(0, 255, 21), tags);
		g_pModelRender->ForcedMaterialOverride(tags);
		oDrawModelExecute(ecx, context, state, info, matrix);
		//glow->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
		modulate(Color(232, 16, 164), glow);
		modulate_exp(glow, 100.0f,0.10f);
		g_pModelRender->ForcedMaterialOverride(glow);
		oDrawModelExecute(ecx, context, state, info, matrix);
	}
}


void __fastcall Hooks::PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	static auto oPaintTraverse = g_Hooks.pPanelHook->GetOriginal<PaintTraverse_t>(vtable_indexes::paint);
	static unsigned int panelID, panelHudID;
	std::string panel_name = g_pPanel->GetName(vguiPanel);
	if (!panelHudID)
		if (!strcmp("HudZoom", g_pPanel->GetName(vguiPanel)))
		{
			panelHudID = vguiPanel;
		}
	
	
	int screen_width, screen_height;
	g_pEngine->GetScreenSize(screen_width, screen_height);

	int centerX = screen_width / 2;
	int CenterY = screen_height / 2;
	auto DoBamewareInto = [screen_width, screen_height]() // failed pasting
{
		
	static constexpr float intro_length_time = 2.f, intro_max_height_offset = -25.f, intro_max_width_offset = -15.f;

	const float current_time = GetTickCount() * 0.001f, time_delta = current_time - g::cheat_start_time;
	if (time_delta < intro_length_time)
	{
			const char bameware_text[] = "Allah hu Akbar";
		    const int  text_width = g_pSurface->GetTextSize2(g::Logs, bameware_text), text_len = strlen(bameware_text);
		const int center_width = screen_width * 0.5, center_height = screen_height * 0.5;
		const int current_letter = ((time_delta / intro_length_time) * text_len) + 1;
		const float time_per_letter = intro_length_time / static_cast<float>(text_len);
		for (int i = 0; i < text_len; i++)
		{
			if (i >= current_letter)
				continue;

			const float frac = ((fmod(time_delta, time_per_letter) * 0.5f + (i == current_letter - 2 ? time_per_letter * 0.5f : 0)) / time_per_letter);

			int alpha = 0;
			if (current_letter > i + 2)
				alpha = 255;
			else
				alpha = frac * 255.f;
			const float frac_2 = (frac > 0.5f ? 1.f - frac : frac) * 2.f;

			auto cur_width = center_width + (text_width * 0.5f) - (text_width / static_cast<float>(text_len)) * static_cast<float>(text_len - i);
			if (i == current_letter - 1 || i == current_letter - 2)
			{
					
				g_pSurface->DrawT(cur_width + (frac_2 * intro_max_width_offset), center_height + (frac_2 * intro_max_height_offset), Color(255, 20, 147, alpha), g::CourierNew, false, bameware_text);
			}
			else
				g_pSurface->DrawT(cur_width + (frac_2 * intro_max_width_offset), center_height + (frac_2 * intro_max_height_offset), Color(255, 20, 147, alpha), g::CourierNew, false, bameware_text);
		}
	}
};
	
	DoBamewareInto();
		
	
	if (panelHudID == vguiPanel && g::pLocalEntity && g::pLocalEntity->IsAlive() && g_Menu.Config.Aimbot)
	{
		if (g::pLocalEntity->IsScoped())
			return;
	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
	
	if (!panelID)
		if (!strcmp("MatSystemTopPanel", g_pPanel->GetName(vguiPanel)))
		{
			panelID = vguiPanel;
			g_Hooks.bInitializedDrawManager = true;
	
			if (g::pLocalEntity && g::pLocalEntity->IsAlive())
			{
				g_pCvar->FindVar("mat_postprocess_enable")->SetValue(0);
			}
		
		}
	
	if (panelID == vguiPanel)
	{
		NoScopeOverlay();
		g_ESP.Render();
		g_Menu.Render();
		g_bullettracer.listener();
		FEATURES::MISC::in_game_logger.Do();
	}
	if (panel_name == "FocusOverlayPanel")
	{
		//ModulateWorld();
	}
}


void Display_damage()
{

}
template<class T2>
static T2* FindHudElement2(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::FindSignature("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::FindSignature("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T2*)find_hud_element(pThis, name);
}
char* HitgroupToName(int hitgroup)
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_LEFTLEG:
		return "left toe";
	case HITGROUP_RIGHTLEG:
		return "right toe";
	case HITGROUP_STOMACH:
		return "stomach";
	case HITGROUP_CHEST:
		return "chest";
	case HITGROUP_LEFTARM:
		return "left arm";
	case HITGROUP_RIGHTARM:
		return "right arm";
	default:
		return "body";
	}
}

void Event::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;
	
	if (!g::pLocalEntity)
		return;
	
	auto attacker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("attacker")));
	auto event_name = event->GetName();
	
	if (!attacker)
		return;
	
	if (attacker != g::pLocalEntity)
		return;
	int index = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));
	
		
	g::Hit[index] = true;
	if (strcmp(event_name, "round_prestart"))
	{
		
	}  
	if (strcmp(event_name, "round_freeze_end"))
	{
		

	}


	const char* fuckthattrash;
	static const char* team[]{ " (Terrorist)"," (Counter-Terrorist)" };
	if (strstr(event->GetName(), "player_hurt"))
	{

		if (!event)
			return;

		if (!g::pLocalEntity)
			return;

		auto* attacker = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("attacker")));
		if (!attacker)
			return;

		if (attacker != g::pLocalEntity)
			return;

		PlayerInfo_s player_info;
		g_pEngine->GetPlayerInfo(g_pEngine->GetPlayerForUserID(event->GetInt("userid")), &player_info);

		FEATURES::MISC::InGameLogger::Log log;


		log.color_line.ChangeAlpha(255);
		log.color_line.PushBack("hit ", Color(255, 255, 255, 255));
		log.color_line.PushBack(player_info.szName, Color(255, 255, 255, 255));


		int hitgroup = event->GetInt("hitgroup");
		if (hitgroup > 0 && hitgroup <= 7)
		{
			log.color_line.PushBack(" in the ", Color(255, 255, 255, 255));
			log.color_line.PushBack(HitgroupToName(hitgroup), Color(255, 255, 255, 255));
			gitgrp = hitgroup;
			fuckthattrash = HitgroupToName(hitgroup);
		}

		log.color_line.PushBack(" for ", Color(255, 255, 255, 255));
		log.color_line.PushBack(std::to_string(event->GetInt("dmg_health")), Color(255, 255, 255, 255));
		log.color_line.PushBack(" damage", Color(255, 255, 255, 255));
		//log.color_line.PushBack(std::to_string(event->GetInt("health")), Color(255, 255, 255, 255));
		

		FEATURES::MISC::in_game_logger.AddLog(log);
	}
	if (!strcmp(event->GetName(), "player_death"))
	{
		auto userid = event->GetInt("userid");
		auto attacker = event->GetInt("attacker");

		if (!userid || !attacker)
			return;


		auto userid_id = g_pEngine->GetPlayerForUserID(userid);
		auto attacker_id = g_pEngine->GetPlayerForUserID(attacker);

		PlayerInfo_t userid_info, attacker_info;
		if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
			return;

		if (!g_pEngine->GetPlayerInfo(attacker_id, &attacker_info))
			return;
		FEATURES::MISC::InGameLogger::Log log;

		log.color_line.ChangeAlpha(255);
		log.color_line.PushBack(userid_info.szName, Color(255, 255, 255, 255));
		log.color_line.PushBack(team[event->GetInt("team")], Color(255, 255, 255, 255));
		log.color_line.PushBack(" was ", Color(255, 255, 255, 255));
		log.color_line.PushBack(" killed by ", Color(255, 255, 255, 255));
		log.color_line.PushBack(attacker_info.szName, Color(255, 255, 255, 255));
		FEATURES::MISC::in_game_logger.AddLog(log);
	}
	if (strstr(event->GetName(), "item_purchase") && g_Menu.Config.eventlogs)
	{
		if (!event)
			return;


		if (!g::pLocalEntity)
			return;

		auto* entity = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));
		if (!entity)
			return;

		if (entity->GetTeam() == g::pLocalEntity->GetTeam())
			return;

		PlayerInfo_s player_info;
		if (!g_pEngine->GetPlayerInfo(entity->EntIndex(), &player_info))
			return;

		FEATURES::MISC::InGameLogger::Log log;

		log.color_line.ChangeAlpha(255);
		log.color_line.PushBack(player_info.szName, Color(255, 255, 255, 255));
		log.color_line.PushBack(" bought a ", Color(255, 255, 255, 255));
		log.color_line.PushBack(event->GetString("weapon"), Color(255, 255, 255, 255));
		//g::Msg("%s bought a  %s \n", player_info.szName, event->GetString("weapon"));

		FEATURES::MISC::in_game_logger.AddLog(log);


	}

	if (strstr(event->GetName(), "bomb_beginplant") && g_Menu.Config.eventlogs) {

		auto userid = event->GetInt("userid");

		if (!userid) {
			return;
		}

		auto userid_id = g_pEngine->GetPlayerForUserID(userid);

		PlayerInfo_s userid_info;
		if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info)) {
			return;
		}

		auto m_player = static_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(userid_id));

		if (!m_player) {
			return;
		}



		FEATURES::MISC::InGameLogger::Log log;

		log.color_line.ChangeAlpha(255);
		log.color_line.PushBack(userid_info.szName, Color(255, 255, 255, 255));
		log.color_line.PushBack(" has began planting the bomb at ", Color(255, 255, 255, 255));
		log.color_line.PushBack(std::to_string(event->GetInt("site")), Color(255, 255, 255, 255));
		//	g::Msg("%s has began planting the bomb at  %s \n", userid_info.szName, std::to_string(event->GetInt("site")));
		FEATURES::MISC::in_game_logger.AddLog(log);
		//Color(209, 10, 70, 255)
	}

	if (strstr(event->GetName(), "bomb_begindefuse") && g_Menu.Config.eventlogs) {
		auto userid = event->GetInt("userid");

		if (!userid) {
			return;
		}

		auto userid_id = g_pEngine->GetPlayerForUserID(userid);

		PlayerInfo_s userid_info;
		if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info)) {
			return;
		}

		auto m_player = static_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(userid_id));

		if (!m_player) {
			return;
		}

		constexpr char* hasdefusekit[2] = { "without defuse kit.","with defuse kit." };
		FEATURES::MISC::InGameLogger::Log log;



		log.color_line.ChangeAlpha(255);
		log.color_line.PushBack(" has began defusing the bomb ", Color(255, 255, 255, 255));
		log.color_line.PushBack(hasdefusekit[event->GetBool("haskit")], Color(255, 255, 255, 255));
		FEATURES::MISC::in_game_logger.AddLog(log);
		//g::Msg("%s has began defusing the bomb  %c \n", userid_info.szName, hasdefusekit[event->GetBool("haskit")]);
	}
	

	if (strstr(event->GetName(), "player_death"))
	{
		if (!event)
			return;

		if (!g::pLocalEntity)
			return;

		auto* attacker = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("attacker")));
		if (!attacker)
			return;

		if (attacker != g::pLocalEntity)
			return;

	
	}
	eventmanager.onEvent(event);

	if (!strcmp(event->GetName(), "weapon_fire"))
	{
		auto index = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));
	
		if (index != g_pEngine->GetLocalPlayer())
			return;
		if (index == g_pEngine->GetLocalPlayer())
		{
			g::shotfiredb++;
		}
		
	}

}
void aspectration(float aspect_ratio_multiplier)
{
	int screen_width, screen_height;
	g_pEngine->GetScreenSize(screen_width, screen_height);

	float aspectratio_value = (screen_width * aspect_ratio_multiplier) / screen_height;

	static ConVar* aspectratio = g_pCvar->FindVar("r_aspectratio");

	aspectratio->SetValue(aspectratio_value);
}

void on_aspect_ratio_changed()
{
	float aspect_ratio = (128) * 0.01;
	aspect_ratio = 2 - aspect_ratio;
	if (g_Menu.Config.Aimbot)
		aspectration(aspect_ratio);
	else
		aspectration(0);
}
void __fastcall Hooks::OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	static auto oOverrideView = g_Hooks.pClientModeHook->GetOriginal<OverrideView_t>(vtable_indexes::view);
	
	if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
	
		if (!g::pLocalEntity)
			return;
	
		if (!g::pLocalEntity->IsAlive())
			return;
	
		if (g_Menu.Config.Aimbot)
		{
			Vector viewPunch = g::pLocalEntity->GetViewPunchAngle();
			Vector aimPunch = g::pLocalEntity->GetAimPunchAngle();
	
			pSetup->angles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
			pSetup->angles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
			pSetup->angles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		}
		*reinterpret_cast<int*>((DWORD)& g_pCvar->FindVar("viewmodel_fov")->fnChangeCallback + 0xC) = 0;
		*reinterpret_cast<int*>((DWORD)& g_pCvar->FindVar("viewmodel_offset_x")->fnChangeCallback + 0xC) = 0;
		*reinterpret_cast<int*>((DWORD)& g_pCvar->FindVar("viewmodel_offset_y")->fnChangeCallback + 0xC) = 0;
		*reinterpret_cast<int*>((DWORD)& g_pCvar->FindVar("viewmodel_offset_z")->fnChangeCallback + 0xC) = 0;
		g_pCvar->FindVar("viewmodel_fov")->SetValue((float)g_Menu.Config.vFov);
	//	g_pCvar->FindVar("viewmodel_offset_x")->SetValue((float)Config.visuals.viewmodel_x);
	//	g_pCvar->FindVar("viewmodel_offset_y")->SetValue((float)Config.visuals.viewmodel_y);
	//	g_pCvar->FindVar("viewmodel_offset_z")->SetValue((float)Config.visuals.viewmodel_z - 10);
		if (g_Menu.Config.Fov != 0 && !g::pLocalEntity->IsScoped())
			pSetup->fov = g_Menu.Config.Fov;
	
		if (g_Menu.Config.NoZoom && g::pLocalEntity->IsScoped())
			pSetup->fov = (g_Menu.Config.Fov == 0) ? 90 : g_Menu.Config.Fov;
			
		on_aspect_ratio_changed();
	}

	oOverrideView(ecx, edx, pSetup);
}

static auto ptr_setupbones =Utils::FindSignature("client_panorama.dll","8B 40 ? FF D0 84 C0 74 ? F3 0F 10 05 ? ? ? ? EB ?");
bool __fastcall  Hooks::inprediction(void* ecx, void* edx) {
	auto o_inprediction = g_Hooks.pPredictionHook->GetOriginal<inprediction_fn>(14);
	if (!g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		return o_inprediction(ecx) ;
	if (g_Menu.Config.Aimbot) {
		if (*reinterpret_cast<uintptr_t*> (_ReturnAddress()) == *reinterpret_cast<uintptr_t*> (ptr_setupbones) + 5)
			return false; // we don't want to predict in setupbones
	}

	return o_inprediction(ecx);
}




bool __fastcall Hooks::hk_setup_bones(void* ecx, void* edx, matrix3x4_t* bone_to_world_out, int max_bones, int bone_mask, float curtime)
{
	static auto setupbones_ret_addr_2 = Utils::FindSignature("client_panorama.dll", "FF 75 08 E8 ? ? ? ? 5F 5E 5D C2 10 00") + 8; // sets up 

	if (std::uintptr_t(_ReturnAddress()) == setupbones_ret_addr_2)
		return false;

	return o_setup_bones(ecx, bone_to_world_out, max_bones, bone_mask, curtime);
}


void __fastcall Hooks::LockCursor(ISurface* thisptr, void* edx)
{
static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>(vtable_indexes::lockCursor);

if (!g_Menu.menuOpened)
	return oLockCursor(thisptr, edx);

	g_pSurface->UnLockCursor();
}


LRESULT Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	const auto getButtonHeld = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;

		if (uMsg == WM_KEYDOWN)
			bButton = true;
		else if (uMsg == WM_KEYUP)
			bButton = false;
	};

	const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;

		if (uMsg == WM_KEYUP)
			bButton = !bButton;
	};

	if (g_Hooks.bInitializedDrawManager)
	{
		// our wndproc capture fn
		if (g_Menu.menuOpened)
		{
			return true;
		}
	}


	return CallWindowProcA(g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam);
}

auto setupbones = Utils::FindSignature("client_panorama.dll", "E8 ? ? ? ? F3 0F 10 45 ? 51") + 5;
auto evalanimlod = reinterpret_cast<void*>(Utils::FindSignature("client_panorama.dll", "84 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B B7"));
__declspec(naked) bool __fastcall Hooks::is_hltv_gate(void*, void*) {
	__asm {
		mov 	edx, edi
		jmp		IsHLTV
	}
}
bool __fastcall Hooks::IsHLTV()
{
	
	const auto org_f = g_Hooks.pEngineHook->GetOriginal< is_hltv_t >(vtable_indexes::hltv);
	 const auto return_to_setup_velocity = Utils::FindSignature("client_panorama.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80");
	 const auto return_to_accumulate_layers = Utils::FindSignature("client_panorama.dll", "84 C0 75 0D F6 87");

	if (_ReturnAddress() == reinterpret_cast<void*>(return_to_setup_velocity) && g::enable_bones && g_pEngine->IsInGame())
		return true;

	if (_ReturnAddress() == reinterpret_cast<void*>(return_to_accumulate_layers) && g_pEngine->IsInGame())
		return true;

	return org_f();
}


std::uintptr_t pattern_t::get() {
	return this->m_addr;
}
pattern_t::pattern_t(void* addr) {
	this->m_addr = (std::uintptr_t) addr;
}
pattern_t pattern_t::sub(std::uintptr_t bytes) {
	return pattern_t((void*)(this->m_addr - bytes));
}

pattern_t pattern_t::add(std::uintptr_t bytes) {
	return pattern_t((void*)(this->m_addr + bytes));
}

pattern_t pattern_t::deref() {
	return pattern_t((void*)(*(std::uintptr_t*) this->m_addr));
}
