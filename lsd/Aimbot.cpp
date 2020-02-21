#include "Aimbot.h"
#include "Autowall.h"
#include "LagComp.h"
#include "AntiAim.h"
#include "Utils.h"
#include "IVEngineClient.h"
#include "PlayerInfo.h"
#include "ICvar.h"
#include "Math.h"
#include "Hitboxes.h"
#include "Menu.h"
#include "IVModelInfo.h"
#include "ClientClass.h"
#include <array>
#include <deque>
#include <algorithm>
#include <DirectXMath.h>

Aimbot g_Aimbot;
static int sinceUse;

enum Hitboxes
{
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX
};

Vector anglevectorsp(Vector& angles) {
	float sp, sy, cp, cy;

	g_AntiAim.sincos(angles.x * (PI / 180.0f), &sp, &cp);
	g_AntiAim.sincos(angles.y * (PI / 180.0f), &sy, &cy);

	auto forward = Vector();

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;

	return forward;
}
inline float FastSqrt222(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}
#define square( x ) ( x * x )
void ClampMov236326263347ement(CUserCmd* pCommand, float fMaxSpeed)
{

	if (fMaxSpeed <= 0.f)
		return;
	float fSpeed = (float)(FastSqrt222(square(pCommand->forwardmove) + square(pCommand->sidemove) + square(pCommand->upmove)));
	if (fSpeed <= 0.f)
		return;
	if (pCommand->buttons & IN_DUCK)
		fMaxSpeed *= 2.94117647f; // TO DO: Maybe look trough the leaked sdk for an exact value since this is straight out of my ass...
	if (fSpeed <= fMaxSpeed)
		return;
	float fRatio = fMaxSpeed / fSpeed;
	pCommand->forwardmove *= fRatio;
	pCommand->sidemove *= fRatio;
	pCommand->upmove *= fRatio;
}

void rotate_movement(float yaw) {
	Vector viewangles;
	g_pEngine->GetViewAngles(viewangles);

	float rotation = DEG2RAD(viewangles.y - yaw);

	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);

	float new_forwardmove = (cos_rot * g::pCmd->forwardmove) - (sin_rot * g::pCmd->sidemove);
	float new_sidemove = (sin_rot * g::pCmd->forwardmove) + (cos_rot * g::pCmd->sidemove);

	g::pCmd->forwardmove = new_forwardmove;
	g::pCmd->sidemove = new_sidemove;
}



int bestHitbox = -1, mostDamage;
Vector multipoints[128];
int multipointCount = 0;

void CAimbot::run_aimbot(CUserCmd* cmd)
{
	Entities.clear();
	SelectTarget();
	shoot_enemy(cmd);
}

void CAimbot::SelectTarget()
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player) return;
	for (int index = 1; index <= 65; index++)
	{
		auto entity = g_pEntityList->GetClientEntity(index);
		if (!entity) continue;
		if (entity->GetTeam() == local_player->GetTeam()) continue;
		if (entity->GetHealth() <= 0) continue;
		if (entity->GetClientClass()->ClassID != EClassIds::CCSPlayer) continue;
		if (entity->GetOrigin() == Vector(0, 0, 0)) continue;
		if (entity->IsImmune()) continue;
		if (entity->IsDormant())	continue;
		/*	Vector ViewOffset = local_player->GetVecOrigin() + local_player->GetViewOffset();
			Vector View; g_pEngine->GetViewAngles(View);
			float fov = FovToPlayer1(ViewOffset, View, entity);
			if (fov > g_Options.Ragebot.fov)
				continue;*/
		AimbotData_t data = AimbotData_t(entity, index);
		Entities.push_back(data);
	}
}



inline float asdasd(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;
}

#define square( x ) ( x * x )
void ClampMovement(CUserCmd* pCommand, float fMaxSpeed)
{
	if (fMaxSpeed <= 0.f)
		return;
	float fSpeed = (float)(asdasd(square(pCommand->forwardmove) + square(pCommand->sidemove) + square(pCommand->upmove)));
	if (fSpeed <= 0.f)
		return;
	if (pCommand->buttons & IN_DUCK)
		fMaxSpeed *= 2.94117647f; // TO DO: Maybe look trough the leaked sdk for an exact value since this is straight out of my ass...
	if (fSpeed <= fMaxSpeed)
		return;
	float fRatio = fMaxSpeed / fSpeed;
	pCommand->forwardmove *= fRatio;
	pCommand->sidemove *= fRatio;
	pCommand->upmove *= fRatio;
}

static ConVar* big_ud_rate = nullptr,
* min_ud_rate = nullptr, * max_ud_rate = nullptr,
* interp_ratio = nullptr, * cl_interp = nullptr,
* cl_min_interp = nullptr, * cl_max_interp = nullptr;

float LerpTime() {

	static ConVar* updaterate = g_pCvar->FindVar("cl_updaterate");
	static ConVar* minupdate = g_pCvar->FindVar("sv_minupdaterate");
	static ConVar* maxupdate = g_pCvar->FindVar("sv_maxupdaterate");
	static ConVar* lerp = g_pCvar->FindVar("cl_interp");
	static ConVar* cmin = g_pCvar->FindVar("sv_client_min_interp_ratio");
	static ConVar* cmax = g_pCvar->FindVar("sv_client_max_interp_ratio");
	static ConVar* ratio = g_pCvar->FindVar("cl_interp_ratio");

	float lerpurmom = lerp->GetFloat(), maxupdateurmom = maxupdate->GetFloat(),
		ratiourmom = ratio->GetFloat(), cminurmom = cmin->GetFloat(), cmaxurmom = cmax->GetFloat();
	int updaterateurmom = updaterate->GetInt(),
		sv_maxupdaterate = maxupdate->GetInt(), sv_minupdaterate = minupdate->GetInt();

	if (sv_maxupdaterate && sv_minupdaterate)
		updaterateurmom = maxupdateurmom;

	if (ratiourmom == 0)
		ratiourmom = 1.0f;

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratiourmom = std::clamp(ratiourmom, cminurmom, cmaxurmom);

	return max(lerpurmom, ratiourmom / updaterateurmom);
}

void rotate_movement(float yaw, CUserCmd* cmd)
{
	Vector viewangles;
	g_pEngine->GetViewAngles(viewangles);

	float rotation = DEG2RAD(viewangles.y - yaw);

	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);

	float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
	float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);

	cmd->forwardmove = new_forwardmove;
	cmd->sidemove = new_sidemove;
}
//void quickstop(CUserCmd* cmd) {
//
//	cmd->sidemove = 0;
//	cmd->forwardmove = g::pLocalEntity->GetVelocity().Length2D() > 20.f ? 450.f : 0.f;
//
//	rotate_movement(MATH::CalcAngle(Vector(0, 0, 0), g::pLocalEntity->GetVelocity()).y + 180.f, cmd);
//}

void CAimbot::shoot_enemy(CUserCmd* cmd)
{
	static bool flips = false;
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player || local_player->GetHealth() <= 0) return;

	auto weapon = reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon || weapon->GetAmmo() == 0) return;

	if (weapon->GetCSWpnData()->type == 9) return;
	if (weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_C4 || weapon->is_grenade() || weapon->is_knife()) return;
	if (!can_shoot(cmd))
	{
		cmd->buttons &= ~IN_ATTACK;
		return;
	}
	if (GetAsyncKeyState(VK_LBUTTON)) return;
	for (auto players : Entities)
	{
		auto entity = players.pPlayer;
		auto class_id = entity->GetClientClass()->ClassID;

		if (!entity) continue;
		if (entity->GetTeam() == local_player->GetTeam()) continue;
		if (entity->GetHealth() <= 0) continue;
		if (class_id != EClassIds::CCSPlayer) continue;
		if (!entity->IsAlive())
			continue;
		if (entity->GetOrigin() == Vector(0, 0, 0)) continue;
		if (entity->IsImmune()) continue;
		if (entity->IsDormant()) continue;

		Vector where2Shoot;

		Vector minus_origin = Vector(0, 0, 0);



		where2Shoot = multipoint(entity, g_Menu.Config.hitscan);


		if (where2Shoot == Vector(0, 0, 0)) continue;

		if (g_Menu.Config.auto_scope)
		{
			if (weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AWP || weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SSG08 ||
				weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20 || weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1 ||
				weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AUG || weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SG556)
				if (!local_player->IsScoped())
					cmd->buttons |= IN_ATTACK2;
		}



		g::aimangle = g_Math.CalcAngle(local_player->GetEyePosition(), where2Shoot);
		g_Math.NormalizeAngles(g::aimangle);
		if (g::aimangle == Vector(0, 0, 0)) continue;



		//	where2Shoot = VelocityExtrapolate(local_player, where2Shoot); // get eye pos next tick
			//aim_angles = VelocityExtrapolate(entity, aim_angles); // get target pos next tick

		if (g_Menu.Config.Autostop)
		{
			if (local_player->GetFlags() & FL_ONGROUND)
			{

				if (weapon->is_grenade() || weapon->is_knife() || weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_TASER) //we dont wanna stop if we holdin a knife, grenade or zeus
					return;

				ClampMovement(cmd, 28);
				//g_pClientState->m_bPaused = true;
			}

		}

		if (accepted_inaccuracy(weapon) < g_Menu.Config.HitchanceValue) continue;

		if (g_Menu.Config.Autoshoot)
		cmd->buttons |= IN_ATTACK;

		cmd->tick_count = TIME_TO_TICKS(entity->GetSimulationTime() + LerpTime());
	}
	static bool needtoshift = false;
	static int firstuse = 0;
	if (cmd->buttons & IN_ATTACK)
	{


		float recoil_scale = g_pCvar->FindVar("weapon_recoil_scale")->GetFloat();
		g::bSendPacket = true; //g_pClientState->m_bPaused = false;
		g::aimangle -= local_player->GetAimPunchAngle() * recoil_scale;
		if (g::bSendPacket)
			cmd->viewangles = g::aimangle;
		else
			cmd->viewangles.y = 90;

	}
	if (g_Menu.Config.doubletap)
	{
		static int lastDoubleTapInTickcount = 0;
		
		if (GetAsyncKeyState(g_Menu.Config.doubletapkey) )
		{
			int doubletapTickcountDelta = g_pGlobalVars->tickcount - lastDoubleTapInTickcount;

			if (doubletapTickcountDelta >= TIME_TO_TICKS(0.2f) && g::pCmd->buttons & IN_ATTACK) {

				lastDoubleTapInTickcount = g_pGlobalVars->tickcount;
				g::bSendPacket = true;
				g::m_nTickbaseShift = 14;
			}
		}
	}
}

float CAimbot::accepted_inaccuracy(C_BaseCombatWeapon* weapon)
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player) return 0;

	if (!weapon) return 0;
	if (weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_TASER) return 0;

	float inaccuracy = weapon->GetInaccuracy();
	if (inaccuracy == 0) inaccuracy = 0.0000001;
	inaccuracy = 1 / inaccuracy;
	return inaccuracy;
}

std::vector<Vector> CAimbot::GetMultiplePointsForHitbox(C_BaseEntity* local, C_BaseEntity* entity, int iHitbox, matrix3x4_t BoneMatrix[128])
{
	auto VectorTransform_Wrapper = [](const Vector& in1, const matrix3x4_t& in2, Vector& out)
	{
		auto VectorTransform = [](const float* in1, const matrix3x4_t& in2, float* out)
		{
			auto DotProducts = [](const float* v1, const float* v2)
			{
				return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
			};
			out[0] = DotProducts(in1, in2[0]) + in2[0][3];
			out[1] = DotProducts(in1, in2[1]) + in2[1][3];
			out[2] = DotProducts(in1, in2[2]) + in2[2][3];
		};
		VectorTransform(&in1.x, in2, &out.x);
	};

	studiohdr_t* pStudioModel = g_pModelInfo->GetStudiomodel(entity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet(0);
	mstudiobbox_t* hitbox = set->GetHitbox(iHitbox);

	std::vector<Vector> vecArray;

	Vector max;
	Vector min;
	VectorTransform_Wrapper(hitbox->max, BoneMatrix[hitbox->bone], max);
	VectorTransform_Wrapper(hitbox->min, BoneMatrix[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	Vector CurrentAngles = g_Math.CalcAngle(center, local->GetEyePosition());

	Vector Forward;
	g_Math.AngleVectors(CurrentAngles, &Forward);

	Vector Right = Forward.Cross(Vector(0, 0, 1));
	Vector Left = Vector(-Right.x, -Right.y, Right.z);

	Vector Top = Vector(0, 0, 1);
	Vector Bot = Vector(0, 0, -1);

	switch (iHitbox) {
	case 0://head
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);

		vecArray[1] += Top * (hitbox->radius * g_Menu.Config.HeadScale * 0.01);
		vecArray[2] += Right * (hitbox->radius * g_Menu.Config.HeadScale * 0.01);
		vecArray[3] += Left * (hitbox->radius * g_Menu.Config.HeadScale * 0.01);
		break;
	default://rest

		for (auto i = 0; i < 3; ++i)
			vecArray.emplace_back(center);

		vecArray[1] += Right * (hitbox->radius * g_Menu.Config.BodyScale * 0.01);
		vecArray[2] += Left * (hitbox->radius * g_Menu.Config.BodyScale * 0.01);
		break;
	}
	return vecArray;
}
Vector CAimbot::get_hitbox_pos(C_BaseEntity* entity, int hitbox_id)
{
	auto getHitbox = [](C_BaseEntity* entity, int hitboxIndex) -> mstudiobbox_t*
	{
		if (entity->IsDormant() || entity->GetHealth() <= 0) return NULL;

		const auto pModel = entity->GetModel();
		if (!pModel) return NULL;

		auto pStudioHdr = g_pModelInfo->GetStudiomodel(pModel);
		if (!pStudioHdr) return NULL;

		auto pSet = pStudioHdr->GetHitboxSet(0);
		if (!pSet) return NULL;

		if (hitboxIndex >= pSet->numhitboxes || hitboxIndex < 0) return NULL;

		return pSet->GetHitbox(hitboxIndex);
	};

	auto hitbox = getHitbox(entity, hitbox_id);
	if (!hitbox) return Vector(0, 0, 0);

	auto bone_matrix = entity->GetBoneMatrix(hitbox->bone);

	Vector bbmin, bbmax;
	g_Math.VectorTransform(hitbox->min, bone_matrix, bbmin);
	g_Math.VectorTransform(hitbox->max, bone_matrix, bbmax);

	return (bbmin + bbmax) * 0.5f;
}

Vector CAimbot::multipoint(C_BaseEntity* entity, int option)
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player) return Vector(0, 0, 0);

	Vector vector_best_point = Vector(0, 0, 0);
	int maxDamage = g_Menu.Config.Mindmg;

	auto matrix = g_Aimbot.Matrix[entity->EntIndex()];
	//auto matrixsssss = g_LagComp->Matrix[entity->GetIndex()];

	matrix = entity->bone_cache();

	if (GetAsyncKeyState(g_Menu.Config.baimkey)) //baim key
	{
		int hitboxes[] =
		{
		Hitboxes::HITBOX_PELVIS,
		Hitboxes::HITBOX_STOMACH,
		Hitboxes::HITBOX_UPPER_CHEST

		};// mixed hitscan
		for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
		{
			for (auto point : GetMultiplePointsForHitbox(local_player, entity, hitboxes[i], matrix))
			{
				int damage = Autowall::Get().CalculateDamage(local_player->GetEyePosition(), point, local_player, entity).damage;
				if (damage > maxDamage)
				{
					bestHitbox = hitboxes[i];
					maxDamage = damage;
					vector_best_point = point;

					if (maxDamage >= entity->GetHealth())
						return vector_best_point;
				}
			}
		}
	}
	else
	{
		switch (option)
		{
		case 0:
		{
			int hitboxes[] = { Hitboxes::HITBOX_HEAD,Hitboxes::HITBOX_NECK };  //head hitscan
			for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
			{
				for (auto point : GetMultiplePointsForHitbox(local_player, entity, hitboxes[i], matrix))
				{
					int damage = Autowall::Get().CalculateDamage(local_player->GetEyePosition(), point, local_player, entity).damage;
					if (damage > maxDamage)
					{
						bestHitbox = hitboxes[i];
						maxDamage = damage;
						vector_best_point = point;

						if (maxDamage >= entity->GetHealth())
							return vector_best_point;
					}
				}
			}
		}
		break;
		case 1:
		{
			int hitboxes[] = { Hitboxes::HITBOX_PELVIS, Hitboxes::HITBOX_STOMACH, Hitboxes::HITBOX_CHEST, Hitboxes::HITBOX_LOWER_CHEST }; // body hitscan
			for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
			{
				for (auto point : GetMultiplePointsForHitbox(local_player, entity, hitboxes[i], matrix))
				{
					int damage = Autowall::Get().CalculateDamage(local_player->GetEyePosition(), point, local_player, entity).damage;
					if (damage > maxDamage)
					{
						bestHitbox = hitboxes[i];
						maxDamage = damage;
						vector_best_point = point;

						if (maxDamage >= entity->GetHealth())
							return vector_best_point;
					}
				}
			}
		}
		break;
		case 2:
		{
			int hitboxes[] = { Hitboxes::HITBOX_NECK, Hitboxes::HITBOX_HEAD,Hitboxes::HITBOX_PELVIS, Hitboxes::HITBOX_STOMACH, Hitboxes::HITBOX_UPPER_CHEST, Hitboxes::HITBOX_CHEST };// mixed hitscan
			for (int i = 0; i < ARRAYSIZE(hitboxes); i++)
			{
				for (auto point : GetMultiplePointsForHitbox(local_player, entity, hitboxes[i], matrix))
				{
					int damage = Autowall::Get().CalculateDamage(local_player->GetEyePosition(), point, local_player, entity).damage;
					if (damage > maxDamage)
					{
						bestHitbox = hitboxes[i];
						maxDamage = damage;
						vector_best_point = point;

						if (maxDamage >= entity->GetHealth())
							return vector_best_point;
					}
				}
			}
		}
		}

	}
	return vector_best_point;
}
float GetCurtime()
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player)
		return -1;

	return static_cast<float>(local_player->GetTickBase())* g_pGlobalVars->intervalPerTick;
}
bool CAimbot::can_shoot(CUserCmd* cmd)
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!local_player) return false;
	if (local_player->GetHealth() <= 0) return false;

	auto weapon = reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));
	if (!weapon || weapon->GetAmmo() == 0) return false;

	return (weapon->GetNextPrimaryAttack() < GetCurtime()) && (local_player->GetNextAttack() < GetCurtime());
}
CAimbot* aimbot = new CAimbot();