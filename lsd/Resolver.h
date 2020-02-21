#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include "IClientMode.h"
#include <deque>
#include <iostream>
#include "LagComp.h"
#include <array>
enum PoseParam_t {
	STRAFE_YAW,
	STAND,
	LEAN_YAW,
	SPEED,
	LADDER_YAW,
	LADDER_SPEED,
	JUMP_FALL,
	MOVE_YAW,
	MOVE_BLEND_CROUCH,
	MOVE_BLEND_WALK,
	MOVE_BLEND_RUN,
	BODY_YAW,
	BODY_PITCH,
	AIM_BLEND_STAND_IDLE,
	AIM_BLEND_STAND_WALK,
	AIM_BLEND_STAND_RUN,
	AIM_BLEND_COURCH_IDLE,
	AIM_BLEND_CROUCH_WALK,
	DEATH_YAW
};
struct AnimationInfo
{
	AnimationInfo()
	{
		m_flSpawnTime = 0.f;
		m_ulEntHandle = nullptr;
		m_playerAnimState = nullptr;
	}

	std::array<float_t, 24> m_flPoseParameters;
	AnimationLayer m_AnimationLayer[13];

	float_t m_flSpawnTime;
	CBaseHandle* m_ulEntHandle;

	C_AnimState* m_playerAnimState;
};
struct AnimationBackup_t
{
	Vector m_origin;
	Vector m_abs_origin;
	Vector m_velocity;
	Vector m_abs_velocity;
	int m_flags;
	int m_eflags;
	float m_duck;
	float m_body;
	std::array<AnimationLayer, 15> m_layers;
};
struct infos
{
	std::array<float, 24> m_poses;
	AnimationLayer m_overlays;

};
class Resolver
{
public:
public: // fake state
	bool m_should_update_fake = false;
	std::array< AnimationLayer, 13 > m_fake_layers;
	std::array< float, 20 > m_fake_poses;
	C_AnimState* m_fake_states = nullptr;
	CCSGOPlayerAnimState* m_fake_state = nullptr;
	float m_fake_rotation = 0.f;
	bool init_fake_anim = false;
	float m_fake_spawntime = 0.f;
	float m_fake_delta = 0.f;
	matrix3x4_t m_fake_matrix[128];
	matrix3x4_t m_fake_position_matrix[128];
	//std::array< matrix3x4_t, 128 > m_fake_matrix;
	//std::array< matrix3x4_t, 128 > m_fake_position_matrix;
	bool m_got_fake_matrix = false;
	float m_real_yaw_ang = 0.f;
	bool m_should_update_entity_animstate = true;


public: // real state
	bool m_should_update_real = false;
	float m_real_spawntime = 0.f;
	bool init_real_anim = false;
	std::array< AnimationLayer, 13 > m_real_layers;
	std::array< float, 20 > m_real_poses;
	CCSGOPlayerAnimState* m_real_state = nullptr;
	bool m_got_real_matrix = false;
	matrix3x4_t m_real_matrix[128];
	float m_server_abs_rotation = 0.f;

	enum invalidate_physics_bits
	{
		position_changed = 0x1,
		angles_changed = 0x2,
		velocity_changed = 0x4,
		animation_changed = 0x8,
		bounds_changed = 0x10,
		sequence_changed = 0x20
	};
	bool side[2] = { false,false };
	bool UseFreestandAngle[65];
	float FreestandAngle[65];
	Vector absOriginBackup;
	float pitchHit[65];
	matrix3x4_t Bones[65][128]{};
	void update_player(C_BaseEntity* player);
	void UpdateAnimationState(C_AnimState* state, Vector angle);
	void CreateAnimationState(CCSGOPlayerAnimState* state);
	void ResetAnimationState(CCSGOPlayerAnimState* state);
	void ResetAnimationStatereal(C_AnimState* state);
	void local_player_run(C_BaseEntity* e);
	void update_local_ghetto(C_BaseEntity* local);
	void fake_animation();
	void update_simple_local_player(C_BaseEntity* player, CUserCmd* cmd);

	void manage_local_fake_animstate();

	void FakeAnimations();
	
	void resolve_freestand(C_BaseEntity* player);
	void switch_side(C_BaseEntity* e);
	void build_server_bones(C_BaseEntity* player);
	void build_bones(lag_record_t& es, matrix3x4_t* mat, float& time);
	void build_bones(C_BaseEntity* e, matrix3x4_t* mat);
	void pre_update(C_BaseEntity* e);
	void on_update(C_BaseEntity* e);
	void post_update(C_BaseEntity* e);
	void finalFixForAnimations(C_BaseEntity* e);
	void on_create_move(const bool send_packet);
	void real_animation(const bool send_packet);
	void FrameStage(ClientFrameStage_t stage);
private:
	std::array< AnimationLayer, 13 > o_animlayers{ };
	std::array< PoseParam_t, 24 > o_posparams{ };
	std::array< Vector, 64 > m_mins{ Vector() };
	std::array< QAngle, 64 > m_angle{ QAngle() };
	std::array< Vector, 64 > m_maxs{ Vector() };
	std::array< Vector, 64 > m_absvel{ Vector() };
	std::array< Vector, 64 > m_absvel1{ Vector() };
	std::array< std::uint8_t, 64 > m_unk_byte{ };
	std::array< int, 64 > m_eflags{ };
	std::array< Vector, 64 > m_origin{ Vector() };
	std::array< Vector, 64 > m_last_origin{ Vector() };
	std::array< Vector, 64 > m_vel{ Vector() };
	std::array< Vector, 64 > m_lastvel{ Vector() };
	std::array< float, 64 > m_fake{ 0.0f };
	std::array< float, 64 > ShotTime{ 0.0f };
	std::array< float, 64 > LastShotTime{ 0.0f };
	std::array< int, 64 >    Lag{ 0 };
	std::array< float, 64 > m_last_simtime{ 0.0f };
	std::array< float, 64 > m_simtime{ 0.0f };
	std::array< float, 64 > m_last_moving_gfy{ 0.0f };
	std::array< int, 64 > m_resolvemodes{ };
	std::array< bool, 64 > m_side{ 0 };
	float o_curtime = 0.0f;
	float o_frametime = 0.0f;

	Vector o_absvel;
	Vector o_absvel1;
	Vector o_origin;
	Vector o_absangle;
	Vector o_absorigin;
	Vector o_vel;
	float o_duckamt;
	float o_lby;
	Vector o_eyeangle;
	std::uint8_t o_unk_byte;
	int o_eflags;
	int o_flags;
	bool updated = false;
	bool l = false;
	bool r = false;
	bool shot = false;
	bool updated_info = false;
	void AnimationFix(C_BaseEntity* pEnt);
	void experimentals(C_BaseEntity * pEnt);
	AnimationInfo arr_infos[65];
};
extern Resolver g_Resolver;

