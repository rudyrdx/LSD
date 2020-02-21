#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include <deque>
#include <array>
#include "Singleton.h"
#include "IClientMode.h"

#define TIME_TO_TICKS( dt )		( (int)( 0.5 + (float)(dt) / g_pGlobalVars->intervalPerTick ) )
#define TICKS_TO_TIME( t )		( g_pGlobalVars->intervalPerTick *( t ) )


struct lag_record_t
{
	lag_record_t() = default;
	lag_record_t(C_BaseEntity* player);
	void apply(C_BaseEntity* player, bool backup = false) const;

	int index;
	bool valid;
	matrix3x4_t matrix[128];

	bool m_fake_walk;
	int m_mode;
	bool m_dormant;
	Vector m_velocity;
	Vector m_origin;
	Vector m_abs_origin;
	Vector m_anim_velocity;
	Vector m_obb_mins;
	Vector m_obb_maxs;
	static std::array<AnimationLayer, 13> m_layers;
	std::array<float, 24> m_poses;
	float m_anim_time;
	float m_sim_time;
	float m_duck;
	float m_body;
	Vector m_eye_angles;
	Vector m_abs_ang;
	int m_flags;
	int m_lag;
	Vector m_rotation;

	int m_cached_count;
	int m_writable_bones;

	bool m_lby_flick;
	bool m_shot;
	bool m_override;
};
struct PlayerRecords
{
	matrix3x4_t Matrix[128];
	float Velocity;
	float SimTime;
	float m_flLowerBodyYawTarget;
	float Entindex;
	Vector mins;
	Vector maxs;
	bool Shot;
	Vector m_vecAbsOrigin;
	Vector m_angEyeAngles;
	std::array<AnimationLayer, 13> m_layer;
};

class LagComp
{
public:
	int ShotTick[65];
	std::deque<PlayerRecords> PlayerRecord[65] = {  };
	void StoreRecord(C_BaseEntity* pEnt);
	void ClearRecords(int i);
	float get_lerp_time();
	bool valid_simtime(const float& simtime);
	float LerpTime();
	bool ValidTick(int tick);

private:
};
extern LagComp g_LagComp;