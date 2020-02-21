#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include "Singleton.h"
#include <deque>

class Aimbot
{
public:
	
	matrix3x4_t Matrix[65][126];
	

private:

};
extern Aimbot g_Aimbot;

struct AimbotData_t
{
	AimbotData_t(C_BaseEntity* player, const int& idx)
	{
		this->pPlayer = player;
		this->index = idx;
	}
	C_BaseEntity* pPlayer;
	int					index;
};

class CAimbot
{
public:
	void shoot_enemy(CUserCmd* cmd);

	void run_aimbot(CUserCmd* cmd);

	void SelectTarget();

	float accepted_inaccuracy(C_BaseCombatWeapon* weapon);

	bool can_shoot(CUserCmd* cmd);

	std::vector<Vector> GetMultiplePointsForHitbox(C_BaseEntity* local, C_BaseEntity* entity, int iHitbox, matrix3x4_t BoneMatrix[128]);

	Vector get_hitbox_pos(C_BaseEntity* entity, int hitbox_id);

	Vector multipoint(C_BaseEntity* entity, int option);

	std::vector<AimbotData_t>	Entities;
private:


};

extern CAimbot* aimbot;