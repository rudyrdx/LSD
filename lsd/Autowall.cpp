#include "Autowall.h"
#include "Utils.h"
#include "IVEngineClient.h"
#include "PlayerInfo.h"
#include "ISurfaceData.h"
#include "Hitboxes.h"
#include "bspflags.h"
#include "ICvar.h"
#include "Math.h"
#include "ClientClass.h"
#include "IBaseClientDll.h"
#include <algorithm>
// esoterik ftw

Autowall g_Autowall;

Vector aim;
void TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, C_BaseEntity* ignore, C_Trace_Antario* ptr)
{
	C_TraceFilter filter(ignore);
	g_pTrace->TraceRay(C_Ray_Antario(vecAbsStart, vecAbsEnd), mask, &filter, ptr);
}

bool VectortoVectorVisible(Vector src, Vector point)
{
	C_Trace_Antario Trace;
	TraceLine(src, point, mask_solid, g::pLocalEntity, &Trace);

	if (Trace.flFraction == 1.0f)
	{
		return true;
	}

	return false;
}
bool Autowall::VectortoVectorVisibles(Vector src, Vector point, C_BaseEntity* pEnt)
{
	//mask_solid
	C_Trace_Antario TraceInit;
	TraceInit.m_pEnt = NULL;
	C_TraceFilter filter1(g::pLocalEntity);
	filter1.pSkip1 = g::pLocalEntity;
	g_pTrace->TraceRay(C_Ray_Antario(src, point), mask_solid, &filter1, &TraceInit);

	if (TraceInit.flFraction == 1.0f)
		return true;

	if (pEnt != nullptr && TraceInit.m_pEnt == pEnt)
		return true;

	return false;
}
bool HandleBulletPenetration(WeaponInfo_t* wpn_data, FireBulletData& data, bool extracheck);

float GetHitgroupDamageMult(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
		return 4.f;
	case HITGROUP_STOMACH:
		return 1.25f;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return 0.75f;
	}

	return 1.0f;
}

void ScaleDamage(int hitgroup, C_BaseEntity* enemy, float weapon_armor_ratio, float& current_damage)
{
	current_damage *= GetHitgroupDamageMult(hitgroup);

	if (enemy->ArmorValue() > 0.0f && hitgroup < HITGROUP_LEFTLEG)
	{
		if (hitgroup == HITGROUP_HEAD && !enemy->HasHelmet())
			return;

		float armorscaled = (weapon_armor_ratio * 0.5f) * current_damage;
		if ((current_damage - armorscaled) * 0.5f > enemy->ArmorValue())
			armorscaled = current_damage - (enemy->ArmorValue() * 2.0f);
		current_damage = armorscaled;
	}
}

void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, C_Trace_Antario* tr)
{
	static DWORD dwAddress = Utils::FindPattern("client_panorama.dll", (BYTE*)"\x53\x8B\xDC\x83\xEC\x08\x83\xE4\xF0\x83\xC4\x04\x55\x8B\x6B\x04\x89\x6C\x24\x04\x8B\xEC\x81\xEC\x00\x00\x00\x00\x8B\x43\x10", "xxxxxxxxxxxxxxxxxxxxxxxx????xxx");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, vecAbsEnd
		LEA		ECX, vecAbsStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

bool SimulateFireBullet(C_BaseEntity* local, C_BaseCombatWeapon* weapon, FireBulletData& data)
{
	data.penetrate_count = 4; // Max Amount Of Penitration
	data.trace_length = 0.0f; // wow what a meme
	auto* wpn_data = weapon->GetCSWpnData(); // Get Weapon Info
	data.current_damage = (float)wpn_data->damage;// Set Damage Memes
	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->range - data.trace_length;
		Vector End_Point = data.src + data.direction * data.trace_length_remaining;
		TraceLine(data.src, End_Point, 0x4600400B, local, &data.enter_trace);
		UTIL_ClipTraceToPlayers(data.src, End_Point * 40.f, 0x4600400B, &data.filter, &data.enter_trace);
		if (data.enter_trace.flFraction == 1.0f)
			break;
		if ((data.enter_trace.hitGroup <= 7) && (data.enter_trace.hitGroup > 0) && (local->GetTeam() != data.enter_trace.m_pEnt->GetTeam()))
		{
			data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->range_modifier, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitGroup, data.enter_trace.m_pEnt, wpn_data->armor_ratio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration(wpn_data, data, false))
			break;
	}
	return false;
}

bool TraceToExitalt(Vector& end, C_Trace_Antario& tr, Vector start, Vector vEnd, C_Trace_Antario* trace)
{
	typedef bool(__fastcall* TraceToExitFn)(Vector&, C_Trace_Antario&, float, float, float, float, float, float, C_Trace_Antario*);
	static TraceToExitFn TraceToExit = (TraceToExitFn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 EC 30 F3 0F 10 75");

	if (!TraceToExit)
		return false;

	return TraceToExit(end, tr, start.x, start.y, start.z, vEnd.x, vEnd.y, vEnd.z, trace);
}

bool HandleBulletPenetration(WeaponInfo_t* wpn_data, FireBulletData& data, bool extracheck)
{
	surfacedata_t* enter_surface_data = g_pSurfaceData->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->range_modifier, (data.trace_length * 0.002));
	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;
	if (data.penetrate_count <= 0)
		return false;
	static Vector dummy;
	static C_Trace_Antario trace_exit;
	if (!TraceToExitalt(dummy, data.enter_trace, data.enter_trace.end, data.direction, &trace_exit))
		return false;
	surfacedata_t* exit_surface_data = g_pSurfaceData->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if (((data.enter_trace.contents & contents_grate) != 0) || (enter_material == 89) || (enter_material == 71))
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)combined_penetration_modifier = 2.0f;
	}
	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->penetration) * 1.25f);
	float thickness = VectorLength(trace_exit.end - data.enter_trace.end);
	if (extracheck)
		if (!VectortoVectorVisible(trace_exit.end, aim))
			return false;
	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;
	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;
	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f)
		return false;
	data.src = trace_exit.end;
	data.penetrate_count--;

	return true;
}

float Autowall::Damage(const Vector& point)
{
	auto data = FireBulletData(g::pLocalEntity->GetEyePosition(), g::pLocalEntity);

	Vector angles;
	angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)g::pLocalEntity->GetActiveWeapon();

	if (!pWeapon)
		return false;

	WeaponInfo_t* weaponData = pWeapon->GetCSWpnData();

	if (!weaponData)
		return false;
	data.penetrate_count = 1;
	data.trace_length = 0.0f;
	data.current_damage = (float)weaponData->damage;

	data.trace_length_remaining = weaponData->range - data.trace_length;

	Vector end = data.src + data.direction * data.trace_length_remaining;

	TraceLine(data.src, end, mask_shot | contents_hitbox, g::pLocalEntity, &data.enter_trace);

	if (VectortoVectorVisible(data.src, point))
	{
		return true;
	}
	static bool extra_check = true;
	if (HandleBulletPenetration(weaponData, data, extra_check))
	{
		return true;

	}

	if (SimulateFireBullet(g::pLocalEntity, g::pLocalEntity->GetActiveWeapon(), data))
		return data.current_damage;

	return 0.f;
}

bool Autowall::CanHitFloatingPoint(const Vector& point, const Vector& source)
{
	aim = point;

	if (!g::pLocalEntity)
		return false;

	FireBulletData data = FireBulletData(source, g::pLocalEntity);

	Vector angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)g::pLocalEntity->GetActiveWeapon();

	if (!pWeapon)
		return false;

	WeaponInfo_t* weaponData = pWeapon->GetCSWpnData();

	if (!weaponData)
		return false;
	data.penetrate_count = 1;
	data.trace_length = 0.0f;
	data.current_damage = (float)weaponData->damage;

	data.trace_length_remaining = weaponData->range - data.trace_length;

	Vector end = data.src + data.direction * data.trace_length_remaining;

	TraceLine(data.src, end, mask_shot | contents_hitbox, g::pLocalEntity, &data.enter_trace);

	if (VectortoVectorVisible(data.src, point))
	{
		return true;
	}
	static bool extra_check = true;
	if (HandleBulletPenetration(weaponData, data, extra_check))
	{
		return true;

	}
	return false;
}


#define enc_str(s) std::string(s)
#define enc_char(s) enc_str(s).c_str()
#define enc_wstr(s) std::wstring(enc_str(s).begin(), enc_str(s).end())
#define enc_wchar(s) enc_wstr(s).c_str()

#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
//#define CHAR_TEX_UNUSED		'J'
//#define CHAR_TEX_UNUSED		'K'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
//#define CHAR_TEX_UNUSED		'Q'
//#define CHAR_TEX_UNUSED		'R'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
//#define CHAR_TEX_UNUSED		'X'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.


#pragma region MASKS

#define   DISPSURF_FLAG_SURFACE           (1<<0)
#define   DISPSURF_FLAG_WALKABLE          (1<<1)
#define   DISPSURF_FLAG_BUILDABLE         (1<<2)
#define   DISPSURF_FLAG_SURFPROP1         (1<<3)
#define   DISPSURF_FLAG_SURFPROP2         (1<<4)

#define   CONTENTS_EMPTY                0

#define   CONTENTS_SOLID                0x1       
#define   CONTENTS_WINDOW               0x2
#define   CONTENTS_AUX                  0x4
#define   CONTENTS_GRATE                0x8
#define   CONTENTS_SLIME                0x10
#define   CONTENTS_WATER                0x20
#define   CONTENTS_BLOCKLOS             0x40 
#define   CONTENTS_OPAQUE               0x80 
#define   LAST_VISIBLE_CONTENTS         CONTENTS_OPAQUE

#define   ALL_VISIBLE_CONTENTS            (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define   CONTENTS_TESTFOGVOLUME        0x100
#define   CONTENTS_UNUSED               0x200     
#define   CONTENTS_BLOCKLIGHT           0x400
#define   CONTENTS_TEAM1                0x800 
#define   CONTENTS_TEAM2                0x1000 
#define   CONTENTS_IGNORE_NODRAW_OPAQUE 0x2000
#define   CONTENTS_MOVEABLE             0x4000
#define   CONTENTS_AREAPORTAL           0x8000
#define   CONTENTS_PLAYERCLIP           0x10000
#define   CONTENTS_MONSTERCLIP          0x20000
#define   CONTENTS_CURRENT_0            0x40000
#define   CONTENTS_CURRENT_90           0x80000
#define   CONTENTS_CURRENT_180          0x100000
#define   CONTENTS_CURRENT_270          0x200000
#define   CONTENTS_CURRENT_UP           0x400000
#define   CONTENTS_CURRENT_DOWN         0x800000

#define   CONTENTS_ORIGIN               0x1000000 

#define   CONTENTS_MONSTER              0x2000000 
#define   CONTENTS_DEBRIS               0x4000000
#define   CONTENTS_DETAIL               0x8000000 
#define   CONTENTS_TRANSLUCENT          0x10000000
#define   CONTENTS_LADDER               0x20000000
#define   CONTENTS_HITBOX               0x40000000

#define   SURF_LIGHT                    0x0001 
#define   SURF_SKY2D                    0x0002 
#define   SURF_SKY                      0x0004 
#define   SURF_WARP                     0x0008 
#define   SURF_TRANS                    0x0010
#define   SURF_NOPORTAL                 0x0020 
#define   SURF_TRIGGER                  0x0040 
#define   SURF_NODRAW                   0x0080 

#define   SURF_HINT                     0x0100 

#define   SURF_SKIP                     0x0200   
#define   SURF_NOLIGHT                  0x0400   
#define   SURF_BUMPLIGHT                0x0800   
#define   SURF_NOSHADOWS                0x1000   
#define   SURF_NODECALS                 0x2000   
#define   SURF_NOPAINT                  SURF_NODECALS
#define   SURF_NOCHOP                   0x4000   
#define   SURF_HITBOX                   0x8000   

// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define   MASK_ALL                      (0xFFFFFFFF)
#define   MASK_SOLID                    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCSOLID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCFLUID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_WATER                    (CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
#define   MASK_OPAQUE                   (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
#define   MASK_OPAQUE_AND_NPCS          (MASK_OPAQUE|CONTENTS_MONSTER)
#define   MASK_BLOCKLOS                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
#define   MASK_BLOCKLOS_AND_NPCS        (MASK_BLOCKLOS|CONTENTS_MONSTER)
#define   MASK_VISIBLE                  (MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_VISIBLE_AND_NPCS         (MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_SHOT                     (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
#define   MASK_SHOT_BRUSHONLY           (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
#define   MASK_SHOT_HULL                (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
#define   MASK_SHOT_PORTAL              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_SOLID_BRUSHONLY          (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID_BRUSHONLY    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
#define   MASK_NPCSOLID_BRUSHONLY       (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC           (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC_FLUID     (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
#define   MASK_SPLITAREAPORTAL          (CONTENTS_WATER|CONTENTS_SLIME)
#define   MASK_CURRENT                  (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define   MASK_DEADSOLID                (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)
#pragma endregion


void Autowall::trace_line(Vector& start, Vector& end, unsigned int mask, C_BaseEntity* ignore, C_Trace_Antario* trace) {

	//ray.Init(start, end);

	C_TraceFilter filter(ignore);
	filter.pSkip1 = ignore;

	g_pTrace->TraceRay(C_Ray_Antario(start, end), mask, &filter, trace);
}
#define DAMAGE_YES			2
bool is_breakable_entity(C_BaseEntity* e) {
	using Fn = bool(__fastcall*)(C_BaseEntity*);
	static auto fn = reinterpret_cast<Fn>(Utils::FindSignature("client_panorama.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE"));

	if (!fn)
		return 0;

	if (!e || !e->EntIndex())
		return false;

	auto take_damage{ (char*)((uintptr_t)e + *(size_t*)((uintptr_t)fn + 38)) };
	auto take_damage_backup{ *take_damage };

	auto* cclass = g_pClientDll->GetAllClasses();

	if ((cclass->pNetworkName[1]) != 'F'
		|| (cclass->pNetworkName[4]) != 'c'
		|| (cclass->pNetworkName[5]) != 'B'
		|| (cclass->pNetworkName[9]) != 'h')
		*take_damage = DAMAGE_YES;

	bool breakable = fn(e);
	*take_damage = take_damage_backup;

	return breakable;
}

bool Autowall::trace_to_exit(C_Trace_Antario* enter_trace, Vector start, Vector dir, C_Trace_Antario* exit_trace) {
	Vector end;
	float distance = 0.f;
	signed int distance_check = 23;
	int first_contents = 0;

	do {
		distance += 4.f;
		end = start + dir * distance;

		if (!first_contents)
			first_contents = g_pTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		int point_contents = g_pTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(point_contents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || point_contents & CONTENTS_HITBOX && point_contents != first_contents) {
			Vector new_end = end - (dir * 4.f);


			//ray.Init(end, new_end);

			g_pTrace->TraceRay(C_Ray_Antario(end, new_end), MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->startSolid && exit_trace->surface.flags & SURF_HITBOX) {
				trace_line(end, start, MASK_SHOT_HULL | CONTENTS_HITBOX, exit_trace->m_pEnt, exit_trace);

				if (exit_trace->DidHit() && !exit_trace->startSolid)
					return true;

				continue;
			}

			if (exit_trace->DidHit() && !exit_trace->startSolid) {
				if (enter_trace->surface.flags & SURF_NODRAW || !(exit_trace->surface.flags & SURF_NODRAW)) {
					if (exit_trace->plane.normal.Dot(dir) <= 1.f)
						return true;

					continue;
				}

				if (is_breakable_entity(enter_trace->m_pEnt) && is_breakable_entity(exit_trace->m_pEnt))
					return true;

				continue;
			}

			if (exit_trace->surface.flags & SURF_NODRAW) {
				if (is_breakable_entity(enter_trace->m_pEnt) && is_breakable_entity(exit_trace->m_pEnt))
					return true;
				else if (!(enter_trace->surface.flags & SURF_NODRAW))
					continue;
			}

			if ((!enter_trace->m_pEnt || enter_trace->m_pEnt->EntIndex() == 0) && (is_breakable_entity(enter_trace->m_pEnt))) {
				exit_trace = enter_trace;
				exit_trace->end = start + dir;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}

bool Autowall::handle_bullet_penetration(WeaponInfo_t* info, Autowall_Info& data) {
	C_Trace_Antario trace_exit;
	surfacedata_t* enter_surface_data = g_pSurfaceData->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;

	float enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float compined_penetration_modifier = 0.f;
	bool solid_surf = ((data.enter_trace.contents >> 3)& CONTENTS_SOLID);
	bool light_surf = ((data.enter_trace.surface.flags >> 7)& SURF_LIGHT);

	if (data.penetration_count <= 0
		|| (!data.penetration_count && !light_surf && !solid_surf && enter_material != CHAR_TEX_GLASS && enter_material != CHAR_TEX_GRATE)
		|| info->penetration <= 0.f
		|| !trace_to_exit(&data.enter_trace, data.enter_trace.end, data.direction, &trace_exit)
		&& !(g_pTrace->GetPointContents(data.enter_trace.end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX)))
		return false;

	surfacedata_t* exit_surface_data = g_pSurfaceData->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		compined_penetration_modifier = 3.f;
		final_damage_modifier = 0.05f;
	}

	else if (light_surf || solid_surf) {
		compined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else {
		compined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material) {
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			compined_penetration_modifier = 3.f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			compined_penetration_modifier = 2.0f;
	}

	float thickness = (trace_exit.end - data.enter_trace.end).LengthSqr();
	float modifier = fmaxf(0.f, 1.f / compined_penetration_modifier);

	float lost_damage = fmaxf(
		((modifier * thickness) / 24.f) //* 0.041666668
		+ ((data.current_damage * final_damage_modifier)
			+ (fmaxf(3.75 / info->penetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage > 0.f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.f)
		return false;

	data.current_position = trace_exit.end;
	data.penetration_count--;

	return true;
}

void Autowall::ScaleDamage(C_BaseEntity* entity, WeaponInfo_t* weapon_info, int hitgroup, float& current_damage)
{
	//Cred. to N0xius for reversing this.
	//TODO: _xAE^; look into reversing this yourself sometime

	bool hasHeavyArmor = false;
	int armorValue = entity->ArmorValue();

	//Fuck making a new function, lambda beste. ~ Does the person have armor on for the hitbox checked?
	auto IsArmored = [&entity, &hitgroup]()-> bool
	{
		C_BaseEntity* targetEntity = entity;
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return targetEntity->HasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage *= hasHeavyArmor ? 2.f : 4.f; //Heavy Armor does 1/2 damage
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weapon_info->armor_ratio / 2.f;

		//Damage gets modified for heavy armor users
		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = current_damage * armorRatio;

		if (hasHeavyArmor)
			NewDamage *= 0.85f;

		if (((current_damage - (current_damage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
			NewDamage = current_damage - (armorValue / armorBonusRatio);

		current_damage = NewDamage;
	}
}

Autowall::Autowall_Return_Info Autowall::CalculateDamage(Vector start, Vector end, C_BaseEntity* from_entity, C_BaseEntity* to_entity, int specific_hitgroup)
{
	// default values for return info, in case we need to return abruptly
	Autowall_Return_Info return_info;
	return_info.damage = -1;
	return_info.hitgroup = -1;
	return_info.hit_entity = nullptr;
	return_info.penetration_count = 4;
	return_info.thickness = 0.f;
	return_info.did_penetrate_wall = false;

	Autowall_Info autowall_info;
	autowall_info.penetration_count = 4;
	autowall_info.start = start;
	autowall_info.end = end;
	autowall_info.current_position = start;
	autowall_info.thickness = 0.f;

	// direction 
	g_Math.AngleVectors(g_Math.CalcAngle(start, end), &autowall_info.direction);

	// attacking entity
	if (!from_entity)
		from_entity = g::pLocalEntity;
	if (!from_entity)
		return return_info;

	auto filter_player = cTraceFilterOneEntity();
	filter_player.pEntity = to_entity;



	auto filter_local = cTraceFilter();
	filter_local.pSkip1 = from_entity;

	// setup filters
	if (to_entity)
		autowall_info.filter = &filter_player;
	else
		autowall_info.filter = &filter_player;

	// weapon
	auto weapon = reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntity(from_entity->GetActiveWeaponIndex()));
	if (!weapon)
		return return_info;

	// weapon data
	auto weapon_info = weapon->GetCSWpnData();
	if (!weapon_info)
		return return_info;

	// client class
	auto weapon_client_class = reinterpret_cast<C_BaseEntity*>(weapon)->GetClientClass();
	if (!weapon_client_class)
		return return_info;

	// weapon range
	float range = min(weapon_info->range, (start - end).Length());
	end = start + (autowall_info.direction * range);
	autowall_info.current_damage = weapon_info->damage;

	while (autowall_info.current_damage > 0 && autowall_info.penetration_count > 0)
	{
		return_info.penetration_count = autowall_info.penetration_count;
		//g_pIVDebugOverlay->AddLineOverlay(g::pLocalEntity->GetEyePosition(), end, 0, 255, 0, true, 0.1);
		trace_line(autowall_info.current_position, end, MASK_SHOT | CONTENTS_GRATE, from_entity, &autowall_info.enter_trace);
		UTIL_ClipTraceToPlayers(autowall_info.current_position, autowall_info.current_position + autowall_info.direction * 40.f, MASK_SHOT | CONTENTS_GRATE, autowall_info.filter, &autowall_info.enter_trace);

		const float distance_traced = (autowall_info.enter_trace.end - start).Length();
		autowall_info.current_damage *= pow(weapon_info->range_modifier, (distance_traced / 500.f));

		/// if reached the end
		if (autowall_info.enter_trace.flFraction == 1.f)
		{
			if (to_entity && specific_hitgroup != 0)
			{
				ScaleDamage(to_entity, weapon_info, specific_hitgroup, autowall_info.current_damage);

				return_info.damage = autowall_info.current_damage;
				return_info.hitgroup = specific_hitgroup;
				return_info.end = autowall_info.enter_trace.end;
				return_info.hit_entity = to_entity;
			}
			else
			{
				return_info.damage = autowall_info.current_damage;
				return_info.hitgroup = -1;
				return_info.end = autowall_info.enter_trace.end;
				return_info.hit_entity = nullptr;
			}

			break;
		}
		// if hit an entity
		if (autowall_info.enter_trace.hitGroup > 0 && autowall_info.enter_trace.hitGroup <= 7 && autowall_info.enter_trace.m_pEnt)
		{
			// checkles gg
			if ((to_entity && autowall_info.enter_trace.m_pEnt != to_entity) ||
				(autowall_info.enter_trace.m_pEnt->GetTeam() == from_entity->GetTeam()))
			{
				return_info.damage = -1;
				return return_info;
			}

			if (specific_hitgroup != -1)
				ScaleDamage(autowall_info.enter_trace.m_pEnt, weapon_info, specific_hitgroup, autowall_info.current_damage);
			else
				ScaleDamage(autowall_info.enter_trace.m_pEnt, weapon_info, autowall_info.enter_trace.hitGroup, autowall_info.current_damage);

			// fill the return info
			return_info.damage = autowall_info.current_damage;
			return_info.hitgroup = autowall_info.enter_trace.hitGroup;
			return_info.end = autowall_info.enter_trace.end;
			return_info.hit_entity = autowall_info.enter_trace.m_pEnt;

			break;
		}

		// break out of the loop retard

		if (!handle_bullet_penetration(weapon_info, autowall_info))
			break;

		return_info.did_penetrate_wall = true;
	}

	return_info.penetration_count = autowall_info.penetration_count;

	return return_info;
}
