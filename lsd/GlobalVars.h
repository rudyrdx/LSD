#pragma once
#include "CInput.h"
#include "CEntity.h"

namespace g
{
	extern CUserCmd*      pCmd;
	extern C_BaseEntity*  pLocalEntity;
	extern std::uintptr_t uRandomSeed;
	extern Vector         OriginalView;
	extern bool           bSendPacket;
	extern bool           enable_bones;
	extern bool           should_setup_local_bones;
	extern bool           m_got_fake_matrix;
	extern bool			  LagPeek;
	extern bool			  plby_update;
	extern float cheat_start_time;
	extern bool           freezetime;
	extern int            TargetIndex;
	extern int            Damage;
	extern float curtime;
	extern int            HurtTime;
	extern Vector         EnemyEyeAngs[65];
	extern Vector         AimbotHitbox[65][28];
	extern Vector         RealAngle;
	extern Vector         aimangle;
	extern Vector         hitpos;
	extern Vector         FakeAngle;
	extern Vector         AngleAA;
	extern 	matrix3x4_t         fakematrix[128];
	extern 	matrix3x4_t         realmatrix[128];
	extern bool           Shot[65];
	extern bool           Hit[65];
	extern bool           missed;
	extern bool           isattacking;
	extern bool           intp;
	extern int            MissedShots[65];
	extern float          w2s_matrix[4][4];
	extern float          simtime;
	extern int  shotfiredb; // tried logging through event logger using weapon_fire didnt work 
	extern int  shot_id; 
	extern std::array< int, 64 > missed_shots;
	extern int  shotshagdiab;
	extern int  shothitb;
	extern int  m_nTickbaseShift;
	extern int  attacktime;
	extern DWORD CourierNew;
	extern DWORD logo;
	extern DWORD trajen;
	extern DWORD Tahoma;
	extern DWORD Logs;

	extern bool ushootbro;
}