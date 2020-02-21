#include "Resolver.h"
#include "Aimbot.h"
#include "Autowall.h"
#include "LagComp.h"
#include "Utils.h"
#include "IVEngineClient.h"
#include "Hitboxes.h"
#include "PlayerInfo.h"
#include "Math.h"
#include "ISurface.h"
#include "Menu.h"
#include "ICvar.h"
#include "AntiAim.h"
#include <random>

Resolver g_Resolver;


typedef void(__cdecl* MsgFn)(const char* msg, va_list);
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

void HandleHits(C_BaseEntity* pEnt)
{
	auto NetChannel = g_pEngine->GetNetChannelInfo();

	if (!NetChannel)
		return;

	static float predTime[65];
	static bool init[65];

	if (g::Shot[pEnt->EntIndex()])
	{
		if (init[pEnt->EntIndex()])
		{
			g_Resolver.pitchHit[pEnt->EntIndex()] = pEnt->GetEyeAngles().x;
			predTime[pEnt->EntIndex()] = g_pGlobalVars->curtime + NetChannel->GetAvgLatency(FLOW_INCOMING) + NetChannel->GetAvgLatency(FLOW_OUTGOING) + TICKS_TO_TIME(1) + TICKS_TO_TIME(g_pEngine->GetNetChannel()->m_nChokedPackets);
			init[pEnt->EntIndex()] = false;
		}

		if (g_pGlobalVars->curtime > predTime[pEnt->EntIndex()] && !g::Hit[pEnt->EntIndex()])
		{
			g::MissedShots[pEnt->EntIndex()] += 1;
			g::missed_shots.at(pEnt->EntIndex())++;
			g::Shot[pEnt->EntIndex()] = false;
		}
		else if (g_pGlobalVars->curtime <= predTime[pEnt->EntIndex()] && g::Hit[pEnt->EntIndex()])
			g::Shot[pEnt->EntIndex()] = false;

	}
	else
		init[pEnt->EntIndex()] = true;

	g::Hit[pEnt->EntIndex()] = false;
}

void Resolver::UpdateAnimationState(C_AnimState* state, Vector angle)
{
	if (!state)
		return;

	static auto UpdateAnimState = Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");//sigchange
	if (!UpdateAnimState)
		return;

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}
void Resolver::CreateAnimationState(CCSGOPlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BaseEntity*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::FindSignature("client_panorama.dll", "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, g::pLocalEntity);
}
void Resolver::ResetAnimationState(CCSGOPlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::FindSignature("client_panorama.dll", "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
void Resolver::ResetAnimationStatereal(C_AnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(C_AnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::FindSignature("client_panorama.dll", "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
//void Resolver::local_player_run(C_BaseEntity*e)
//{
//	auto local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
//	const auto state = e->AnimState();
//
//	if (!state)
//		return;
//	float* PosParams = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(local) + 0x2774);
//	static std::array< AnimationLayer, 13 > o_layers;
//	static auto proper_abs = state->m_goalfeet_yaw;
//	static auto sent_pose_params = PosParams;
//	static auto last_tick = g_pGlobalVars->tickcount;
//	static auto last_simtime = e->GetOldSimulationTime();
//
//	if (g_pGlobalVars->tickcount != last_tick) {
//		/*if (e->GetSimulationTime() != last_simtime) {
//			update_fake();
//		}*/
//
//		e->SetAbsVelocity(e->GetVelocity());
//
//		const float o_curtime = g_pGlobalVars->curtime;
//		const float o_frametime = g_pGlobalVars->frametime;
//		const float o_framecount = g_pGlobalVars->framecount;
//		const float o_tickcount = g_pGlobalVars->tickcount;
//		const float o_interp = g_pGlobalVars->interpolationAmount;
//
//		g_pGlobalVars->frametime = g_pGlobalVars->intervalPerTick;
//		g_pGlobalVars->curtime = e->GetSimulationTime();
//		g_pGlobalVars->framecount = static_cast<int>(e->GetSimulationTime() / g_pGlobalVars->intervalPerTick + 0.5f);
//		g_pGlobalVars->tickcount = static_cast<int>(e->GetSimulationTime() / g_pGlobalVars->intervalPerTick + 0.5f);
//		g_pGlobalVars->interpolationAmount = 0.0f;
//
//		if (state->m_last_clientside_anim_framecount == g_pGlobalVars->framecount)
//			state->m_last_clientside_anim_framecount=0;
//
//		auto lby_delta = g::FakeAngle.y - g::RealAngle.y;
//		//lby_delta = std::remainderf(lby_delta, 360.0f);
//		lby_delta = std::clamp(lby_delta, -50.0f, 50.0f);
//
//		auto feet_yaw = std::remainderf(g::RealAngle.y + lby_delta, 360.0f);
//
//		if (feet_yaw < 0.0f)
//			feet_yaw += 360.0f;
//
//		state->m_goalfeet_yaw = state->m_feet_yaw = feet_yaw;
//		state->m_feet_yaw_rate = 0.0f;
//
//		local->ClientAnimations(true);
//		e->UpdateClientAnimation();
//		local->ClientAnimations(false);
//		
//		g_pGlobalVars->curtime = o_curtime;
//		g_pGlobalVars->frametime = o_frametime;
//		g_pGlobalVars->framecount = o_framecount;
//		g_pGlobalVars->tickcount = o_tickcount;
//		g_pGlobalVars->interpolationAmount = o_interp;
//
//		if (e->GetSimulationTime() != last_simtime) {
//			proper_abs = state->m_goalfeet_yaw;
//			sent_pose_params = PosParams;
//			std::memcpy(&o_layers, local->AnimOverlays(), sizeof(AnimationLayer) * 13);
//			last_simtime = e->GetSimulationTime();
//		}
//		else
//		{
//			proper_abs = state->m_goalfeet_yaw;
//			sent_pose_params = PosParams;
//			std::memcpy(&o_layers, local->AnimOverlays(), sizeof(AnimationLayer) * 13);
//		}
//
//		last_tick = g_pGlobalVars->tickcount;
//	}
//	local->SetAbsAngles(Vector(0.f,proper_abs,0.f));
//	std::memcpy(local->AnimOverlays(), &o_layers, sizeof(AnimationLayer) * 13);
//	PosParams = sent_pose_params;
//}

void update_state(C_AnimState* state, Vector ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.y, ang.x, NULL);
}
void update_Fake_state(CCSGOPlayerAnimState* state, Vector ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.y, ang.x, NULL);
}
bool fresh_tick()
{
	static int old_tick_count;

	if (old_tick_count != g_pGlobalVars->tickcount)
	{
		old_tick_count = g_pGlobalVars->tickcount;
		return true;
	}

	return false;
}
void Resolver::update_local_ghetto(C_BaseEntity* local)
{
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;
	const auto state = local->AnimState();

	if (!state)
		return;
	static std::array< infos, 13 > animation_frames;
	std::memcpy(&animation_frames.at(local->EntIndex()).m_overlays, local->AnimOverlays(), (sizeof(AnimationLayer) * 13));
	animation_frames.at(local->EntIndex()).m_poses = local->m_flPoseParameter();
	static float proper_abs = state->m_goalfeet_yaw;
	static std::array<float, 24> sent_pose_params = local->m_flPoseParameter();
	static std::array< AnimationLayer, 13 > o_layers;
	if (fresh_tick()) // Only update animations each tick, though we are doing this each frame.
	{
		local->ClientAnimations(true); //just does stuff like set m_bClientSideAnimation and m_iLastAnimUpdateFrameCount
		update_state(state, g::pCmd->viewangles);
		g::should_setup_local_bones = true;
		g::enable_bones = true;
		local->UpdateClientAnimation();
		g::enable_bones = false;
		std::memcpy(&o_layers, local->AnimOverlays(), sizeof(AnimationLayer) * 13);
		if (g::bSendPacket)
		{
			m_server_abs_rotation = state->m_goalfeet_yaw;
			proper_abs = state->m_goalfeet_yaw;
			sent_pose_params = local->m_flPoseParameter();
		}
	}
	local->ClientAnimations(false);
	local->SetAbsAngles(Vector(0, proper_abs, 0));
	std::memcpy(local->AnimOverlays(), &o_layers, sizeof(AnimationLayer) * 13);
	std::memcpy(local->AnimOverlays(), &animation_frames.at(local->EntIndex()).m_overlays, (sizeof(AnimationLayer) * 13));
	local->m_flPoseParameter() = animation_frames.at(local->EntIndex()).m_poses;
	local->m_flPoseParameter() = sent_pose_params;
}

void Resolver::manage_local_fake_animstate()
{
	if (!g::pLocalEntity || !g::pLocalEntity->IsAlive())
		return;

	if (!g_Menu.Config.local_chams)
		return;

	if (m_fake_spawntime != g::pLocalEntity->m_flSpawnTime() || m_should_update_fake)
	{
		init_fake_anim = false;
		m_fake_spawntime = g::pLocalEntity->m_flSpawnTime();
		m_should_update_fake = false;
	}

	if (!init_fake_anim)
	{
		m_fake_state = static_cast<CCSGOPlayerAnimState*> (g_pMemalloc->Alloc(sizeof(CCSGOPlayerAnimState)));

		if (m_fake_state != nullptr)
			CreateAnimationState(m_fake_state);

		init_fake_anim = true;
	}
	float frametime = g_pGlobalVars->frametime;

	if (g::bSendPacket)
	{
		std::array<AnimationLayer, 13> networked_layers;
		std::memcpy(&networked_layers, g::pLocalEntity->AnimOverlays(), sizeof(AnimationLayer) * 13);

		auto backup_abs_angles = g::pLocalEntity->GetAbsAngles();
		auto backup_poses = g::pLocalEntity->m_flPoseParameter();
		if (g::pLocalEntity->GetFlags() & FL_ONGROUND)
			g::pLocalEntity->GetFlags() |= FL_ONGROUND;
		else
		{
			if (g::pLocalEntity->AnimOverlays()[4].m_flWeight != 1.f && g::pLocalEntity->AnimOverlays()[5].m_flWeight != 0.f)
				g::pLocalEntity->GetFlags() |= FL_ONGROUND;

			if (g::pLocalEntity->GetFlags() & FL_ONGROUND)
				g::pLocalEntity->GetFlags() &= ~FL_ONGROUND;
		}




		*reinterpret_cast<int*>(g::pLocalEntity + 0xA68) = g_pGlobalVars->frametime;
		g_pGlobalVars->frametime = 23.91753135f; // :^)

		update_Fake_state(m_fake_state, g::pCmd->viewangles);
		g::should_setup_local_bones = true;
		m_got_fake_matrix = g::pLocalEntity->SetupBones(g::fakematrix, 128, 524032 - 66666/*g_Menu.Config.nightmodeval*/, false);
		const auto org_tmp = g::pLocalEntity->GetRenderOrigin();
		if (m_got_fake_matrix)
		{
			for (auto& i : g::fakematrix)
			{
				i[0][3] -= org_tmp.x;
				i[1][3] -= org_tmp.y;
				i[2][3] -= org_tmp.z;
			}
		}
		std::memcpy(g::pLocalEntity->AnimOverlays(), &networked_layers, sizeof(AnimationLayer) * 13);

		g::pLocalEntity->m_flPoseParameter() = backup_poses;
		g::pLocalEntity->GetAbsAngles() = backup_abs_angles;
	}
	g_pGlobalVars->frametime = frametime;

}

template<class T, class U>
T clampsss(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

void Resolver::switch_side(C_BaseEntity* e)
{
	if (!g::pLocalEntity->IsAlive() || !e->IsAlive() || !g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;
	float yaw = 0.0f;
	auto prevLage = g_LagComp.PlayerRecord[e->EntIndex()].at(1);
	auto layer = e->get_anim_overlay_index(3);
	if (e->AnimOverlays()[13].m_flWeight + 14.25092419f > 0.54)
	{
		if (e->AnimOverlays()[3].m_flCycle > 0.12)
		{
			if (e->AnimOverlays()[13].m_flCycle > 0.43)
			{
				yaw = e->GetEyeAngles().y; //no fake angles
			}
		}
	}
	else if (e->AnimOverlays()[7].m_nOrder == prevLage.m_layer[7].m_nOrder) // slow walk detect
	{
		if (e->AnimOverlays()[7].m_flCycle > 0.5f)
		{
			yaw =g_Math.NormalizeYaw(prevLage.m_flLowerBodyYawTarget) + e->get_max_desync_delta_aimware();
			return;
		}
		else //the speed is too high for the desyncing angles
			yaw = e->GetEyeAngles().y;
	}
	else if (e->GetSequenceActivity(layer->m_nSequence) == 979 &&
		abs(prevLage.m_layer[3].m_flWeight - e->AnimOverlays()[7].m_flWeight) >= 1.1f) // fix desync with lby breaker (polak it's you.)
	{
		yaw = g_Math.NormalizeYaw(prevLage.m_flLowerBodyYawTarget);
	}
	else if (abs(e->AnimOverlays()[7].m_flWeight + -10.0429) < 0.0020000001 &&
			abs(prevLage.m_layer[7].m_flWeight + -2.6812999) < 0.0020000001)
		 {
			auto v24 = e->GetEyeAngles().y;
			auto lby_delta = g_Math.NormalizeYaw(e->GetLowerBodyYaw() - v24);
			if (std::remainderf(lby_delta,360.0f) > -e->get_max_desync_delta_aimware())
			{
				v24 = g_Math.NormalizeYaw(v24 + -e->get_max_desync_delta_aimware());
				yaw = v24;
			}
		 }

		else if (
			abs(e->AnimOverlays()[7].m_flWeight + -2.6812999) < 0.0020000001 &&
			abs(prevLage.m_layer[7].m_flWeight + -2.6812999) < 0.0020000001)
		{
			auto v36 =  g_Math.NormalizeYaw(e->GetLowerBodyYaw() - e->GetEyeAngles().y);
			auto v42 =  g_Math.NormalizeYaw(prevLage.m_flLowerBodyYawTarget - prevLage.m_angEyeAngles.y);
			if (std::remainderf(v36 - v42, 360.0f) < -50.0f)
			{
				yaw = g_Math.NormalizeYaw(e->GetEyeAngles().y + -e->get_max_desync_delta_aimware());
				
			}
		}
	
	else // player abusing tickbase
	{
		e->GetTickBase() = (g_pGlobalVars->intervalPerTick * g_pGlobalVars->frametime) + TIME_TO_TICKS(e->GetSimulationTime()); //Fixing player tickbase
		return; // skip this tick
	}
	yaw = e->AnimState()->m_goalfeet_yaw;
	
}

void Resolver::build_bones(C_BaseEntity* e, matrix3x4_t* mat) {
	// last_animation_framecount
	*(std::uint32_t*) ((std::uintptr_t) e + 0xA30) = g_pGlobalVars->framecount;
	*(std::uint32_t*) ((std::uintptr_t) e + 0xA28) = 0;

	// prev_bone_mask
	*(std::uint32_t*) ((std::uintptr_t) e + 0x269C) = 0;
	//int Backup = *(int*)((uintptr_t)this + 0x274);
	//*(int*)((uintptr_t)this + 0x274) &=~ 1;
	const auto iks = *(std::uint8_t*) ((std::uintptr_t) e + 0x64);
	const auto eflags = *(std::uint32_t*) ((std::uintptr_t) e + 0xE4);
	// const auto old_readable_bones = *( std::uint32_t* ) ( ( std::uintptr_t ) e + 0x26A8 );
	const auto mask = *(std::uint32_t*) ((std::uintptr_t) e + 0x2698);
	const auto prev_bone_mask = *(std::uint32_t*) ((std::uintptr_t) e + 0x269C);
	const auto effects = *(std::uint32_t*) ((std::uintptr_t) e + 0xEC);

	// use inverse kinematics
	*(std::uint8_t*) ((std::uintptr_t) e + 0x64) &= ~2;

	// removes interp
	*(std::uint32_t*) ((std::uintptr_t) e + 0xE4) |= 8; // AddFlag( EFL_SETTING_UP_BONES );
	*(std::uint32_t*) ((std::uintptr_t) e + 0xEC) |= 8; // m_Effects |= EF_NOINTERP;

	// call attachment helpers so the arms dont go flying
	// old readable bones
	// *( std::uint32_t* ) ( ( std::uintptr_t ) e + 0x26A8 ) &= ~512;
	// bone mask
	*(std::uint32_t*) ((std::uintptr_t) e + 0x2698) |= 512;
	// previous bone mask
	*(std::uint32_t*) ((std::uintptr_t) e + 0x269C) = 0;

	e->InvalidateBoneCache();
	//	hooks::in_setupbones = true;

	e->SetupBones(mat, 128, 0x7FF00, g_pGlobalVars->curtime);
	//	hooks::in_setupbones = false;

	// *( std::uint32_t* ) ( ( std::uintptr_t ) e + 0x26A8 ) = old_readable_bones;
	//* (int*)((uintptr_t)this + 0x274) = Backup;
	*(std::uint32_t*) ((std::uintptr_t) e + 0x2698) = mask;
	*(std::uint8_t*) ((std::uintptr_t) e + 0x64) = iks;
	*(std::uint32_t*) ((std::uintptr_t) e + 0xE4) = eflags;
	*(std::uint32_t*) ((std::uintptr_t) e + 0xEC) = effects;
}

static Vector interpolate(const Vector from, const Vector to, const float percent)
{
	return to * percent + from * (1.f - percent);
}
static float interpolate(const float from, const float to, const float percent)
{
	return to * percent + from * (1.f - percent);
}

void Resolver::finalFixForAnimations(C_BaseEntity* e)
{
	if (!e->AnimState())
		return;
	float* PosParams = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(e) + 0x2774);
	if (!e || !e->IsAlive() || e->IsDormant() || !e->AnimState())
		return;
	auto spawntime = e->m_flSpawnTime();
	if (e->m_flSpawnTime() != spawntime)
	{
		// reset animation state.
		ResetAnimationStatereal(e->AnimState());

		// note new spawn time.
		spawntime = e->m_flSpawnTime();
	}

	auto local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	//bool didshot = false;
	static auto enable_bone_cache_invalidation =Utils::FindSignature("client_panorama.dll", "C6 05 ? ? ? ? ? 89 47 70" + 2);
	const auto state = e->AnimState();
	// make a backup of globals
	const auto backup_frametime = g_pGlobalVars->frametime;
	const auto backup_curtime = g_pGlobalVars->curtime;
	//auto last_shot_time = e->GetActiveWeapon()->GetLastShotTime();
	//auto shot = last_shot_time > e->GetSimulationTime() &&last_shot_time <= e->GetOldSimulationTime();
	static auto sent_pose_params = PosParams;

	//if (shot)
	//	didshot = true;

	g_pGlobalVars->frametime = g_pGlobalVars->intervalPerTick;
	g_pGlobalVars->curtime = e->GetSimulationTime() + g_pGlobalVars->intervalPerTick;

	o_origin = e->GetOrigin();
	o_absangle = e->GetAbsAngles();
	o_absorigin = e->GetAbsOrigin();
	o_absvel = e->AbsVelocity();
	o_vel = e->GetVelocity();
	o_duckamt = e->GetDuckAmount();
	o_lby = e->GetLowerBodyYaw();
	o_eflags = e->EFlags();
	o_flags = e->flags();
	o_eyeangle = e->GetEyeAngles();
	
	std::memcpy(&o_animlayers.at(0), e->AnimOverlays(), sizeof(std::array< AnimationLayer, 13 >));

	if (e == local) {
	
	}
	// fixes for networked players
	else {

		auto lby_delta = e->GetLowerBodyYaw() - e->GetEyeAnglesXY()->y;
		lby_delta = std::remainderf(lby_delta, 360.0f);
		clampsss(lby_delta, -60.0f, 60.0f);
		//switch_side(e);
		auto feet_yaw = std::remainderf(e->GetEyeAnglesXY()->y + lby_delta, 360.0f);

		if (feet_yaw < 0.0f)
			feet_yaw += 360.0f;
		//v  e->AnimState()->m_goalfeet_yaw = e->AnimState()->m_feet_yaw = feet_yaw;

		updated = true;
		float flTimeDelta = TICKS_TO_TIME(e->GetSimulationTime() - e->GetOldSimulationTime());
		if (flTimeDelta > 0 && flTimeDelta<=17)
		{
			for (auto i = 0; i < flTimeDelta; i++)
			{
				const auto time = e->GetSimulationTime() + TICKS_TO_TIME(i + 1);
				const auto lerp = 1.f - (e->GetOldSimulationTime() - time) / (e->GetOldSimulationTime() - e->GetSimulationTime());

				// lerp eye angles.
				auto eye_angles = interpolate(e->GetEyeAngles(), o_eyeangle, lerp);
				g_Math.NormalizeAngles(eye_angles);
				e->GetEyeAngles() = eye_angles;
				// lerp duck amount.
				e->GetDuckAmount() = interpolate(e->GetDuckAmount(), o_duckamt, lerp);
				if (flTimeDelta - 1 == i)
				{
					Vector newVelo = (e->GetNetworkOrigin() - e->GetOldOrigin()) / flTimeDelta;
					e->SetAbsVelocity(newVelo);
					e->GetVelocity() = /*o_vel*/newVelo;
					e->flags() = o_flags;
					if (flTimeDelta > 1) 
					{
						switch_side(e);
					}
				}
				// correct shot desync.
				if (g::Shot[e->EntIndex()])
				{
					//if (last_shot_time <= time)
						e->GetEyeAngles() = o_eyeangle;
				}
				e->AnimState()->m_feet_yaw_rate = 0;
				// backup simtime.
				const auto backup_simtime = e->GetSimulationTime();
				// set new simtime.
				e->GetSimulationTime() = time;

				// restore old simtime.
				e->GetSimulationTime() = backup_simtime;
				if (o_flags & FL_ONGROUND)
					e->GetFlags() |= FL_ONGROUND;
				else
				{
					if (e->AnimOverlays()[4].m_flWeight != 1.f && e->AnimOverlays()[5].m_flWeight != 0.f)
						e->GetFlags() |= FL_ONGROUND;

					if (o_flags & FL_ONGROUND)
						e->GetFlags() &= ~FL_ONGROUND;
				}
				e->AbsVelocity() = e->GetVelocity();
				e->AbsVelocity1() = e->GetVelocity();
				e->UnkByte() = 1;
				e->EFlags() &= ~0x1000;
			}

			
		}
		if (o_flags & FL_ONGROUND)
			e->GetFlags() |= FL_ONGROUND;
		else
		{
			if (e->AnimOverlays()[4].m_flWeight != 1.f && e->AnimOverlays()[5].m_flWeight != 0.f)
				e->GetFlags() |= FL_ONGROUND;

			if (o_flags & FL_ONGROUND)
				e->GetFlags() &= ~FL_ONGROUND;
		}
		g_pGlobalVars->frametime = g_pGlobalVars->intervalPerTick;
		g_pGlobalVars->curtime = e->GetSimulationTime()+ g_pGlobalVars->intervalPerTick;
		e->AbsVelocity() = e->GetVelocity();
		e->AbsVelocity1() = e->GetVelocity();
		e->UnkByte() = 1;
		e->EFlags() &= ~0x1000;
		e->AnimState()->m_feet_yaw_rate = 0.0f;
		//resolve_freestand(e);
	}
	const auto old_invalidation = enable_bone_cache_invalidation;
	
	// allow reanimating in the same frame
	if (state->m_last_clientside_anim_framecount == g_pGlobalVars->framecount)
		state->m_last_clientside_anim_framecount -= 1;

	// make sure we keep track of the original invalidation state


	// notify the other hooks to instruct animations and pvs fix
	e->ClientAnimations(true);
	update_state(state, e->GetEyeAngles());
	g::enable_bones = true;
	e->UpdateClientAnimation();
	g::enable_bones = false;
	e->ClientAnimations(false);
	e->GetEyeAngles() = o_eyeangle;
	//instance()->last_process_state = nullptr;

	e->GetOrigin() = o_origin;
	e->GetAbsOrigin() = o_absorigin;
	e->GetVelocity() = o_vel;
	e->GetDuckAmount() = o_duckamt;
	e->GetLowerBodyYaw() = o_lby;
	e->AbsVelocity() = o_absvel;
	e->EFlags() = o_eflags;
	e->flags() = o_flags;
	std::memcpy(e->AnimOverlays(), &o_animlayers.at(0), sizeof(std::array< AnimationLayer, 13 >));
	sent_pose_params = PosParams;
	if (updated) {
		e->AbsVelocity() = o_absvel;
		e->AbsVelocity1() = o_absvel1;
		e->UnkByte() = o_unk_byte;
		e->EFlags() = o_eflags;
		updated = false;
	}

	if (FOFFSET(float, state, 0x164) < 0.0f)
		e->airtime() = 0.0f;
	
	// invalidate physics.
	if (!e->is_local_player())
		e->invalidate_physics_recursive(angles_changed
			| animation_changed
			| sequence_changed);

	// we don't want to enable cache invalidation by accident
	enable_bone_cache_invalidation = old_invalidation;

	// restore globals
	g_pGlobalVars->frametime = backup_frametime;
	g_pGlobalVars->curtime = backup_curtime;
	//build matrix
	build_bones(e,nullptr);
	

}

void Resolver::FrameStage(ClientFrameStage_t stage)
{
	if (!g::pLocalEntity || !g_pEngine->IsInGame())
		return;

	static bool  wasDormant[65];
	for (int i = 1; i < g_pEngine->GetMaxClients(); ++i)
	{
		auto local = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive())
			continue;
		if (pPlayerEntity->IsDormant())
		{
			wasDormant[i] = true;
			continue;
		}
		HandleHits(pPlayerEntity);
		if (stage == FRAME_RENDER_START )
		{
			if (pPlayerEntity != g::pLocalEntity)
			{
				finalFixForAnimations(pPlayerEntity);
			}
			else
			{
				update_local_ghetto(local);
			}
			
		}

		wasDormant[i] = false;
	}
}
