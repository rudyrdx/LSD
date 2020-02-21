#include "LagComp.h"
#include "Aimbot.h"
#include "Utils.h"
#include "IVEngineClient.h"
#include "PlayerInfo.h"
#include "ICvar.h"
#include "Math.h"
#include "Hitboxes.h"
#include "Menu.h"
#include "IClientMode.h"
#include "AntiAim.h"
#include "Resolver.h"


//lag_record_t::lag_record_t(C_BaseEntity* player)
//{
//	index = player->EntIndex();
//	valid = true;
//	m_fake_walk = false;
//	//m_mode = RMODE_MOVING;
//	m_dormant = player->IsDormant();
//	m_velocity = player->GetVelocity();
//	m_origin = player->GetOrigin();
//	m_abs_origin = player->GetAbsOrigin();
//	//m_layers = player->AnimOverlays();
//	m_poses = player->m_flPoseParameter();
//	m_anim_time = player->GetOldSimulationTime() + g_pGlobalVars->intervalPerTick;
//	m_sim_time = player->GetSimulationTime();
//	m_duck = player->GetDuckAmount();
//	m_body = player->GetLowerBodyYaw();
//	m_eye_angles = player->GetEyeAngles();
//	m_abs_ang = player->GetAbsAngles();
//	m_flags = player->GetFlags();
//	//m_rotation = player->get_rotation();
//	m_lag = TIME_TO_TICKS(player->GetSimulationTime() - player->GetOldSimulationTime());
//	m_lby_flick = false;
//	m_override = false;
//	const auto collideable = player->GetCollideable();
//	m_obb_maxs = collideable->OBBMaxs();
//	m_obb_mins = collideable->OBBMins();
//	//m_cached_count = player->bone_cache()->m_CachedBoneCount;
//	if (m_cached_count)
//	{
//		//memcpy(matrix, player->bone_cache()->m_pCachedBones, 48 * m_cached_count);
//		m_writable_bones = *(&player->WriteableBones() + 8);
//	}
//	else m_writable_bones = 0;
//}
//
//void lag_record_t::apply(C_BaseEntity* player, bool backup) const
//{
//	player->GetFlags() = m_flags;
//	player->GetSimulationTime() = m_sim_time;
//	player->GetEyeAngles() = m_eye_angles;
//	player->SetAbsAngles(m_abs_ang);
//	player->GetVelocity() = m_velocity;
//	player->SetAbsOrigin(backup ? m_abs_origin : m_origin);
//	player->GetOrigin() = m_origin;
//	player->GetLowerBodyYaw() = m_body;
//	player->GetDuckAmount() = m_duck;
//	const auto collideable = player->GetCollideable();
//	player->Maxs() = m_obb_maxs;
//	player->Mins() = m_obb_mins;
//
//	if (m_cached_count && m_cached_count > 0)
//	{
//		//memcpy(player->bone_cache()->m_pCachedBones, matrix, 48 * m_cached_count);
//		*(&player->WriteableBones() + 8) = m_writable_bones;
//	}
//}
//
//int lagcomp::fix_tickcount(const float& simtime)
//{
//	return TIME_TO_TICKS(simtime + get_lerp_time());
//}
LagComp g_LagComp;
float LagComp::get_lerp_time()
{
	static auto cl_updaterate = g_pCvar->FindVar("cl_updaterate");
	static auto cl_interp = g_pCvar->FindVar("cl_interp");

	const auto update_rate = cl_updaterate->GetInt();
	const auto interp_ratio = cl_interp->GetFloat();

	auto lerp = interp_ratio / update_rate;

	if (lerp <= cl_interp->GetFloat())
		lerp = cl_interp->GetFloat();

	return lerp;
}
bool LagComp::valid_simtime(const float& simtime)
{
	const auto nci = g_pEngine->GetNetChannelInfo();
	if (!nci)
		return false;

	float correct = 0;

	correct += nci->GetLatency(FLOW_OUTGOING);
	correct += nci->GetLatency(FLOW_INCOMING);
	correct += get_lerp_time();

	const auto delta_time = correct - (g_pGlobalVars->curtime - simtime);

	return fabsf(delta_time) <= 0.2f && correct < 1.f;
}
float LagComp::LerpTime() // GLAD https://github.com/sstokic-tgm/Gladiatorcheatz-v2.1/blob/eaa88bbb4eca71f8aebfed32a5b86300df8ce6a3/features/LagCompensation.cpp
{
	int updaterate = g_pCvar->FindVar("cl_updaterate")->GetInt();
	ConVar* minupdate = g_pCvar->FindVar("sv_minupdaterate");
	ConVar* maxupdate = g_pCvar->FindVar("sv_maxupdaterate");

	if (minupdate && maxupdate)
		updaterate = maxupdate->GetInt();

	float ratio = g_pCvar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_pCvar->FindVar("cl_interp")->GetFloat();
	ConVar* cmin = g_pCvar->FindVar("sv_client_min_interp_ratio");
	ConVar* cmax = g_pCvar->FindVar("sv_client_max_interp_ratio");

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratio = std::clamp(ratio, cmin->GetFloat(), cmax->GetFloat());

	return max(lerp, (ratio / updaterate));
}

bool LagComp::ValidTick(int tick) // gucci i think cant remember
{
	auto nci = g_pEngine->GetNetChannelInfo();

	if (!nci)
		return false;

	auto PredictedCmdArrivalTick = g::pCmd->tick_count + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));
	auto Correct = std::clamp(LerpTime() + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(PredictedCmdArrivalTick + TIME_TO_TICKS(LerpTime()) - (tick + TIME_TO_TICKS(LerpTime())));

	return (abs(Correct) <= 0.2f);
}

void LagComp::StoreRecord(C_BaseEntity* pEnt) // best lag comp in the world
{
	PlayerRecords Setup;

	static float ShotTime[65];
	static float OldSimtime[65];

	if (PlayerRecord[pEnt->EntIndex()].size() == 0)
	{
		Setup.Velocity = abs(pEnt->GetVelocity().Length2D());

		Setup.SimTime = pEnt->GetSimulationTime();

		Setup.m_flLowerBodyYawTarget = pEnt->GetLowerBodyYaw();

		Setup.Entindex = pEnt->EntIndex();

		memcpy(Setup.Matrix, g_Aimbot.Matrix[pEnt->EntIndex()], (sizeof(matrix3x4_t) * 128));

		Setup.Shot = false;

		PlayerRecord[pEnt->EntIndex()].push_back(Setup);
	}

	if (OldSimtime[pEnt->EntIndex()] != pEnt->GetSimulationTime())
	{
		Setup.Velocity = abs(pEnt->GetVelocity().Length2D());

		Setup.SimTime = pEnt->GetSimulationTime();

		Setup.m_flLowerBodyYawTarget = pEnt->GetLowerBodyYaw();

		Setup.m_vecAbsOrigin = pEnt->GetAbsOrigin();

		Setup.mins = pEnt->Mins();

		Setup.maxs = pEnt->Maxs();

		Setup.m_angEyeAngles = pEnt->GetEyeAngles();

		Setup.Entindex = pEnt->EntIndex();

		memcpy(Setup.Matrix, g_Aimbot.Matrix[pEnt->EntIndex()], (sizeof(matrix3x4_t) * 128));

		memcpy(&Setup.m_layer, pEnt->AnimOverlays(), (sizeof(std::array< AnimationLayer, 13 >)));
		
		if (pEnt->GetActiveWeapon() && !pEnt->IsKnifeorNade())
		{
			if (ShotTime[pEnt->EntIndex()] != pEnt->GetActiveWeapon()->GetLastShotTime())
			{
				Setup.Shot = true;
				ShotTime[pEnt->EntIndex()] = pEnt->GetActiveWeapon()->GetLastShotTime();
			}
			else
				Setup.Shot = false;
		}
		else
		{
			Setup.Shot = false;
			ShotTime[pEnt->EntIndex()] = 0.f;
		}

		PlayerRecord[pEnt->EntIndex()].push_back(Setup);

		OldSimtime[pEnt->EntIndex()] = pEnt->GetSimulationTime();
	}


	ShotTick[pEnt->EntIndex()] = -1;

	if (PlayerRecord[pEnt->EntIndex()].size() > 0)
		for (int tick = 0; tick < PlayerRecord[pEnt->EntIndex()].size(); tick++)
			if (!valid_simtime(TIME_TO_TICKS(PlayerRecord[pEnt->EntIndex()].at(tick).SimTime)))
				PlayerRecord[pEnt->EntIndex()].erase(PlayerRecord[pEnt->EntIndex()].begin() + tick);
			else if (PlayerRecord[pEnt->EntIndex()].at(tick).Shot)
				ShotTick[pEnt->EntIndex()] = tick; // gets the newest shot tick
}

void LagComp::ClearRecords(int i)
{
	if (PlayerRecord[i].size() > 0)
	{
		for (int tick = 0; tick < PlayerRecord[i].size(); tick++)
		{
			PlayerRecord[i].erase(PlayerRecord[i].begin() + tick);
		}
	}
}
