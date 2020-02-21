#include "GlobalVars.h"
#include "Menu.h"
#include <array>
//#define TIME_TO_TICKSd( dt )		( (int)( 0.5 + (float)(dt) / g_pGlobalVars->intervalPerTick ) )

namespace g
{
	CUserCmd*      pCmd = nullptr;
	C_BaseEntity*  pLocalEntity = nullptr;
	std::uintptr_t uRandomSeed = NULL;
	Vector         OriginalView;
	bool           bSendPacket;
	bool           should_setup_local_bones=false;
	bool           m_got_fake_matrix=false;
	bool		   LagPeek = false;
	bool		   enable_bones = false;
	bool		   plby_update = false;
	int            TargetIndex = -1;
	int            Damage = 0;
	float curtime = 0.0f;
	int            HurtTime = 0;
	bool           freezetime = false;
	Vector         EnemyEyeAngs[65];
	float          w2s_matrix[4][4];
	float          simtime;
	Vector         AimbotHitbox[65][28];
	Vector         aimangle;
	Vector         hitpos;
	Vector         RealAngle;
	Vector         FakeAngle;
	Vector         AngleAA;
	matrix3x4_t         fakematrix[128];
	matrix3x4_t         realmatrix[128];
	float cheat_start_time;
	bool ushootbro = false;
	bool Shot[65];
	int  shotfiredb = 0;
	std::array< int, 64 > missed_shots{ 0 };
	int  shot_id = 0;
	int  shotshagdiab = 0;
	int  shothitb = 0;
	int  m_nTickbaseShift =0;
	int  attacktime = 0;
	bool           Hit[65];
	bool          missed;
	bool          isattacking;
	bool          intp;
	int           MissedShots[65];
	DWORD CourierNew;
	DWORD logo;
	DWORD trajen;
	DWORD Tahoma;
	DWORD Logs;
}

