#include "CEntity.h"
#include "IVModelInfo.h"
#include "CGlobalVarsBase.h"
#include "GlobalVars.h"
#include "Math.h"
#include "ICvar.h"
#include <iostream>
#include <algorithm>
Vector  C_BaseEntity::GetHitboxPos(int hitbox)
{
	matrix3x4_t boneMatrix[256];

	if (this->SetupBones(boneMatrix, 128, 0x00000100, this->GetSimulationTime()))
	{
		studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
		mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
		mstudiobbox_t* hit_box = set->GetHitbox(hitbox);
	     	Vector
	     		bbmin = Vector{},
	     		bbmax = Vector{};
	     
	     	g_Math.VectorTransform(hit_box->min, boneMatrix[hit_box->bone], bbmin);
	     	g_Math.VectorTransform(hit_box->max, boneMatrix[hit_box->bone], bbmax);
	     
	     	return (bbmin + bbmax) / 2.0f;
	}

	return Vector{};
}
Vector C_BaseEntity::hitbox_position(int hitbox_id, matrix3x4_t matrix[128]) {
	studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	mstudiobbox_t* hit_box = set->GetHitbox(hitbox_id);
			Vector
				min = Vector{ },
				max = Vector{ };

			g_Math.VectorTransform(hit_box->min, matrix[hit_box->bone], min);
			g_Math.VectorTransform(hit_box->max, matrix[hit_box->bone], max);

			return (min + max) / 2.0f;


	return Vector{ };
}
Vector C_BaseEntity::GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix, float *Radius)
{
	studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	mstudiobbox_t* hitbox = set->GetHitbox(Hitbox);

	if (hitbox)
	{
		Vector vMin, vMax, vCenter, sCenter;
		g_Math.VectorTransform(hitbox->min, Matrix[hitbox->bone], vMin);
		g_Math.VectorTransform(hitbox->max, Matrix[hitbox->bone], vMax);
		vCenter = (vMin + vMax) * 0.5;

		*Radius = hitbox->radius;

		return vCenter;
	}
	
	return Vector(0, 0, 0);
}

Vector C_BaseEntity::GetHitboxPosition(int Hitbox, matrix3x4_t *Matrix) // any public source
{
	studiohdr_t* hdr = g_pModelInfo->GetStudiomodel(this->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	mstudiobbox_t* hitbox = set->GetHitbox(Hitbox);

	if (hitbox)
	{
		Vector vMin, vMax, vCenter, sCenter;
		g_Math.VectorTransform(hitbox->min, Matrix[hitbox->bone], vMin);
		g_Math.VectorTransform(hitbox->max, Matrix[hitbox->bone], vMax);
		vCenter = (vMin + vMax) * 0.5;

		return vCenter;
	}

	return Vector(0, 0, 0);
}
Vector C_BaseEntity::GetBonePosition(int bone)
{
	matrix3x4_t matrix[128];
	if (this->SetupBones(matrix, 128, 0x00000100, GetTickCount64()))
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	}
	return Vector(0, 0, 0);
}
float C_BaseEntity::desync_delta()
{
	if (!this)
		return 0.f;
	auto state = this-> AnimState();

	auto v42 = -360.0f;
	auto v43 = state->m_goalfeet_yaw;
	auto v129 = -360.0f;

	if (v43 >= -360.0f) {
		v42 = std::fminf(v43, 360.0f);
		v129 = v42;
	}

	auto v44 = state->m_eye_yaw - v42;
	auto v45 = state->m_eye_yaw;
	auto v135 = std::fmodf(v44, 360.0f);
	auto v46 = v135;

	if (v45 <= v129) {
		if (v135 <= -180.0f)
			v46 = v135 + 360.0f;
	}
	else if (v135 >= 180.0f) {
		v46 = v135 - 360.0f;
	}

	auto v48 = 0.0f;
	auto v47 = state->m_unk_feet_speed_ratio;

	if (v47 >= 0.0f)
		v48 = std::fmodf(v47, 1.0f);
	else
		v48 = 0.0f;

	auto v49 = (float)((float)(state->m_ground_fraction * -0.3f) - 0.2f) * v48;
	auto v50 = state->m_duck_amount;
	auto v51 = v49 + 1.0f;

	if (v50 > 0.0f) {
		auto v52 = state->m_unk_feet_speed_ratio;
		auto v53 = 0.0f;

		if (v52 >= 0.0f)
			v53 = std::fmodf(v52, 1.0f);
		else
			v53 = 0.0f;

		v51 = v51 + (float)((float)(v50 * v53) * (float)(0.5f - v51));
	}

	return *(float*)((std::uintptr_t) state + 0x334) * v51;
}
void C_BaseEntity::FixSetupBones(matrix3x4_t *Matrix)
{
		int Backup = *(int*)((uintptr_t)this + 0x274);
		*(int*)((uintptr_t)this + 0x274) = 0;
		Vector absOriginBackupLocal = this->GetAbsOrigin();
		this->SetAbsOrigin(this->GetOrigin());
		this->SetupBones(Matrix, 128, 0x00000100,g_pGlobalVars->curtime);
		this->SetAbsOrigin(absOriginBackupLocal);
		*(int*)((uintptr_t)this + 0x274) = Backup;
}
void C_BaseEntity::russianfix(C_BaseEntity* player, matrix3x4_t*mat)
{
	static auto bInterpolateAnimations = *(intptr_t*)(Utils::FindSignature("client_panorama.dll", "80 BF ? ? ? ? ? 0F 84 ? ? ? ? 83 BF ? ? ? ? ? 74 7C") + 2);

	if (!player)
		return;

	DWORD AnimFixTarget = (DWORD)player;
	*(DWORD*)(AnimFixTarget + 0xA24) = 0;
	*(DWORD*)(AnimFixTarget + 0xA30) = g_pGlobalVars->framecount;
	*(DWORD*)(AnimFixTarget + 0xF0) &= ~8;
	*(DWORD*)(AnimFixTarget + bInterpolateAnimations) = false;
	player->InvalidateBoneCache();
	player->SetupBones(mat, 126, 0x0007FF00,g_pGlobalVars->curtime);

}
void C_BaseEntity::setupbonesfix(matrix3x4_t* Matrix, C_BaseEntity* e,float time)
{
	*reinterpret_cast<int*>(uintptr_t(e) + 0x104) |= 0xF0;
	int Backup = *(int*)((uintptr_t)this + 0x274);
	*(int*)((uintptr_t)this + 0x274) = 0;

	*reinterpret_cast<int*>(e->AnimState() + 112) = *reinterpret_cast<int*>(uintptr_t(e) + 0x104);
	e->SetupBones(Matrix, 126, 0x0007FF00, time/* g_pGlobalVars->curtime + 3.5f*/);

	*reinterpret_cast<int*>(uintptr_t(e) + 0x104) &= ~(0xF0);
	// set new flags
	*reinterpret_cast<int*>(uintptr_t(e) + 0x104) |= 0x98;

	Matrix[3].Base()[2] *= 0.6431f;
	Matrix[1].Base()[1] /= 0.2117f;
	*(int*)((uintptr_t)this + 0x274) = Backup;
}

 void  C_BaseEntity::setup_bone_matrix(C_BaseEntity* entity, matrix3x4_t* mat) {
	// must be set to retrive right matrix
	entity->ClientAnimations(false);
	*reinterpret_cast<int*>(uintptr_t(entity) + 0x104) |= 0xF0;

	*reinterpret_cast<int*>(entity->AnimState() + 112) = *reinterpret_cast<int*>(uintptr_t(entity) + 0x104);

	// now we setup the matrix
	entity->SetupBones(mat, 128 /*yes.. you dont want these 2 index's. they are fucked up */,
		0x100 | 0x200, g_pGlobalVars->curtime + 3.5f
	/* this is needed to be on the right time with the server time value of interpolation of player models on client*/);

	// strip flags
	*reinterpret_cast<int*>(uintptr_t(entity) + 0x104) &= ~(0xF0);

	// set new flags
	*reinterpret_cast<int*>(uintptr_t(entity) + 0x104) |= 0x98;

	// clear our bone cache so we dont look retarded when dead, should be easy to find. index is getabsangles + 0x2
	/*entity->ClearCSRagdollBoneCache();*/

	// correct our bone matrix ( values are from server loggings, can share method in private ). legs are now matching
	mat[3].Base()[2] *= 0.6431f;
	mat[1].Base()[1] /= 0.2117f;
	entity->ClientAnimations(true);
}
//SanekGame https://www.unknowncheats.me/forum/1798568-post2.html

void C_BaseEntity::SetAbsAngles(Vector angles)
{
	using Fn = void(__thiscall*)(C_BaseEntity*, const Vector& angles);
	static Fn AbsAngles = (Fn)(Utils::FindPattern("client_panorama.dll", (BYTE*)"\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x64\x53\x56\x57\x8B\xF1\xE8", "xxxxxxxxxxxxxxx"));

	AbsAngles(this, angles);
}
void C_BaseEntity::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::FindSignature("client_panorama.dll", "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}
void C_BaseEntity::SetAbsOrigin(Vector origin)
{
	using Fn = void(__thiscall*)(void*, const Vector &origin);
	static Fn AbsOrigin = (Fn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

	AbsOrigin(this, origin);
}

void C_BaseEntity::SetAbsVelocity(Vector velocity) // i dont remember
{
	using Fn = void(__thiscall*)(void*, const Vector &velocity);
	static Fn AbsVelocity = (Fn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3");

	AbsVelocity(this, velocity);
}
void C_BaseEntity::ResetAnimationState(C_AnimState* state) {
	using ResetAnimState_t = void(__thiscall*)(C_AnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::FindSignature(("client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BaseEntity::CreateAnimstate(C_AnimState* state) {
	using CreateAnimState_t = void(__thiscall*)(C_AnimState*, void*);
	static auto CreateAnimstate = (CreateAnimState_t)Utils::FindSignature(("client_panorama.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimstate)
		return;

	CreateAnimstate(state, this);
}
float C_BaseEntity::get_max_desync_delta_aimware()
{

	auto thisptr = uintptr_t(this->AnimState());


	auto v1 = ((float*)thisptr)[62];
	auto v2 = 1.0f;
	auto v3 = 0.0f;
	auto v4 = 0.f;
	auto v7 = 0.0f;
	auto v8 = 0.f;
	auto v10 = 0.0f;
	auto v11 = v1;

	if (v1 <= 1.0f) {
		v4 = v10;

		if (v1 >= 0.0)
			v4 = v11;

		v3 = v4;
	}
	else
		v3 = 1.0f;

	auto v5 = ((float*)thisptr)[41];
	auto v6 = (float)((float)((float)(((float*)thisptr)[71] * -0.3f) - 2.0f) * v3) + 1.0f;

	if (v5 > 0.0) {
		v7 = ((float*)thisptr)[63];
		v11 = 0.0;
		v10 = v7;

		if (v7 <= 1.0f) {
			v8 = v11;

			if (v7 >= 0.0)
				v8 = v10;

			//v2 = v8;
		}

		v6 = v6 + (float)((float)(v2 * v5) * (float)(0.5f - v6));
	}

	return ((float*)thisptr)[205] * v6;
};
bool C_BaseEntity::IsSniper()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_awp")
		return true;
	else if (WeaponName == "weapon_g3sg1")
		return true;
	else if (WeaponName == "weapon_scar20")
		return true;
	else if (WeaponName == "weapon_ssg08")
		return true;

	return false;
}
bool C_BaseEntity::IsKnifeorNade()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_knife")
		return true;
	else if (WeaponName == "weapon_incgrenade")
		return true;
	else if (WeaponName == "weapon_decoy")
		return true;
	else if (WeaponName == "weapon_flashbang")
		return true;
	else if (WeaponName == "weapon_hegrenade")
		return true;
	else if (WeaponName == "weapon_smokegrenade")
		return true;
	else if (WeaponName == "weapon_molotov")
		return true;

	return false;
}

bool C_BaseEntity::IsNade()
{
	if (!this)
		return false;
	if (!this->IsAlive())
		return false;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	 if (WeaponName == "weapon_incgrenade")
		return true;
	else if (WeaponName == "weapon_decoy")
		return true;
	else if (WeaponName == "weapon_flashbang")
		return true;
	else if (WeaponName == "weapon_hegrenade")
		return true;
	else if (WeaponName == "weapon_smokegrenade")
		return true;
	else if (WeaponName == "weapon_molotov")
		return true;

	return false;
}
float C_BaseEntity::FireRate()
{
	if (!this)
		return 0.f;
	if (!this->IsAlive())
		return 0.f;
	if (this->IsKnifeorNade())
		return 0.f;

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)this->GetActiveWeapon();

	if (!pWeapon)
		return false;

	std::string WeaponName = pWeapon->GetName();

	if (WeaponName == "weapon_glock")
		return 0.15f;
	else if (WeaponName == "weapon_hkp2000")
		return 0.169f;
	else if (WeaponName == "weapon_p250")//the cz and p250 have the same name idky same with other guns
		return 0.15f;
	else if (WeaponName == "weapon_tec9")
		return 0.12f;
	else if (WeaponName == "weapon_elite")
		return 0.12f;
	else if (WeaponName == "weapon_fiveseven")
		return 0.15f;
	else if (WeaponName == "weapon_deagle")
		return 0.224f;
	else if (WeaponName == "weapon_nova")
		return 0.882f;
	else if (WeaponName == "weapon_sawedoff")
		return 0.845f;
	else if (WeaponName == "weapon_mag7")
		return 0.845f;
	else if (WeaponName == "weapon_xm1014")
		return 0.35f;
	else if (WeaponName == "weapon_mac10")
		return 0.075f;
	else if (WeaponName == "weapon_ump45")
		return 0.089f;
	else if (WeaponName == "weapon_mp9")
		return 0.070f;
	else if (WeaponName == "weapon_bizon")
		return 0.08f;
	else if (WeaponName == "weapon_mp7")
		return 0.08f;
	else if (WeaponName == "weapon_p90")
		return 0.070f;
	else if (WeaponName == "weapon_galilar")
		return 0.089f;
	else if (WeaponName == "weapon_ak47")
		return 0.1f;
	else if (WeaponName == "weapon_sg556")
		return 0.089f;
	else if (WeaponName == "weapon_m4a1")
		return 0.089f;
	else if (WeaponName == "weapon_aug")
		return 0.089f;
	else if (WeaponName == "weapon_m249")
		return 0.08f;
	else if (WeaponName == "weapon_negev")
		return 0.0008f;
	else if (WeaponName == "weapon_ssg08")
		return 1.25f;
	else if (WeaponName == "weapon_awp")
		return 1.463f;
	else if (WeaponName == "weapon_g3sg1")
		return 0.25f;
	else if (WeaponName == "weapon_scar20")
		return 0.25f;
	else if (WeaponName == "weapon_mp5sd")
		return 0.08f;
	else
		return .0f;
	
}

bool C_BaseEntity::IsEnemy()
{
	return this->GetTeam() != g::pLocalEntity->GetTeam();
}
void C_BaseEntity::SetupBonesFix() {
	static auto r_jiggle_bones = g_pCvar->FindVar("r_jiggle_bones");

	auto animstate = AnimState();
	//fixvelocity
	//animstate->m_vVelocityX = 0.f;
	//animstate->m_vVelocityY = 0.f;
	animstate->m_speed2d = 0.001f; // 0.0 will break more than fix so use 0.001 or just hook this field

	auto parent_cvar = r_jiggle_bones->pParent;
	if (parent_cvar == r_jiggle_bones)
		parent_cvar = 0;
	//else
		//parent_cvar = 1;

	// fixing goal feet yaw for better interpolating bones
	animstate->m_goalfeet_yaw = GetLowerBodyYaw() + animstate->m_max_yaw;

	AnimationLayer* pOverlay = get_anim_overlay_index(6);
	//adjust the value to correctly calculate the position of the bones
	pOverlay->m_flWeight = pOverlay->m_flCycle = pOverlay->m_flPlaybackRate = 0.f;

	if (pOverlay->m_nSequence > 0)
		pOverlay->m_nSequence += 1; // Fixing sequence delay before setup bones calc

	  //skip extrapolating velocity
	//m_bHasDefuser() = false;
	this->GetVelocity() = this->AbsVelocity1() * animstate->m_vel_unk;
	//trigger lby update
	//m_nSequence() = 979;
	animstate->m_last_clientside_anim_update = g_pGlobalVars->curtime + 1.32f; // set new body yaw update with delay

	*(int*)(((uintptr_t)this) + 0xA30) = 0;

	auto v5 = *(int*)(((uintptr_t)this) + 0xF0);
	*(int*)(((uintptr_t)this) + 0xF0) |= 8u;
	auto v6 = *(uint8_t*)(((uintptr_t)this) + 0x274);
	*(uint8_t*)(((uintptr_t)this) + 0x274) &= 0xFEu;
	*(int*)(g_pGlobalVars + 0x001C) = -1; // fix interpolation
	*(bool*)(((uintptr_t)this) + 0xF3C) = true; // SetupVelocity call for calculation right angles
	// ref: DoExtraBonesProcessing
	(*(BYTE*)(uintptr_t(this) + 0xA28)) |= 0xE;
	*(short*)(uintptr_t(this) + 0x272) = -1;


	int result = 1;
	__asm push 0x54f12f43
	this->SetupBones(0, -1, 0x7FF00, g_pGlobalVars->curtime);
	__asm pop result

	this->AbsVelocity1() = this->AbsVelocity1(); // restore velocity

	*(uint8_t*)(((uintptr_t)this) + 0x274) = v6;
	*(int*)(((uintptr_t)this) + 0xF0) = v5;

	r_jiggle_bones->SetValue(result);
}
#include <intrin.h>
// sse min.
template< typename t = float >
t minimum(const t &a, const t &b) {
	// check type.
	static_assert(std::is_arithmetic< t >::value, "Math::min only supports integral types.");
	return (t)_mm_cvtss_f32(
		_mm_min_ss(_mm_set_ss((float)a),
			_mm_set_ss((float)b))
	);
}

// sse max.
template< typename t = float >
t maximum(const t &a, const t &b) {
	// check type.
	static_assert(std::is_arithmetic< t >::value, "Math::max only supports integral types.");
	return (t)_mm_cvtss_f32(
		_mm_max_ss(_mm_set_ss((float)a),
			_mm_set_ss((float)b))
	);
}

