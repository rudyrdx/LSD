#pragma once

#include "Autowall.h"
#include "Aimbot.h"
#include "LagComp.h"
#include "GlobalVars.h"
#include "Vector.h"
#include "Math.h"
#include "ICvar.h"
#include "CPrediction.h"
#include "Menu.h"
#include <iostream>
#include "Vector.h"
#include <algorithm>

// kinda just throw shit in here

#define _SOLVEY(a, b, c, d, e, f) ((c * b - d * a) / (c * f - d * e))
#define SOLVEY(...) _SOLVEY(?, ?, ?, ?, ?, ?)
#define SOLVEX(y, world, forward, right) ((world.x - right.x * y) / forward.x)
#define Square(x) ((x)*(x))


class Misc
{
public:
	void OnCreateMove()
	{
		this->pCmd = g::pCmd;
		this->pLocal = g::pLocalEntity;

		this->RemoveFlash();
		this->DoAutostrafe();
		this->DoBhop();
	//	this->FakeDuck();
		this->AutoRevolver();
		this->SpoofSvCheats();
		//this->DoFakeLag();
	};
	void SpoofSvCheats()
	{

		ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");

		if (sv_cheats->GetInt() != 1)
		{
			*reinterpret_cast<int*>((DWORD)& sv_cheats->fnChangeCallback + 0xC) = 0;
			sv_cheats->SetValue(1);
		}
	}

	 static void angle_vectorsss(const Vector& angles, Vector& forward, Vector& right, Vector& up)
	{
		const auto sp = sin(DEG2RAD(angles.x)), cp = cos(DEG2RAD(angles.x)),
			sy = sin(DEG2RAD(angles.y)), cy = cos(DEG2RAD(angles.y)),
			sr = sin(DEG2RAD(angles.z)), cr = cos(DEG2RAD(angles.z));

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;

		right.x = -1 * sr * sp * cy + -1 * cr * -sy;
		right.y = -1 * sr * sp * sy + -1 * cr * cy;
		right.z = -1 * sr * cp;

		up.x = cr * sp * cy + -sr * -sy;
		up.y = cr * sp * sy + -sr * cy;
		up.z = cr * cp;
	}
	void fix_movement(CUserCmd* cmd, Vector& wishangle)
	{
		Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
		angle_vectorsss(wishangle, view_fwd, view_right, view_up);
		angle_vectorsss(cmd->viewangles, cmd_fwd, cmd_right, cmd_up);

		const auto v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
		const auto v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
		const auto v12 = sqrtf(view_up.z * view_up.z);

		const Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
		const Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
		const Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

		const auto v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
		const auto v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
		const auto v18 = sqrtf(cmd_up.z * cmd_up.z);

		const Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
		const Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
		const Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

		const auto v22 = norm_view_fwd.x * cmd->forwardmove;
		const auto v26 = norm_view_fwd.y * cmd->forwardmove;
		const auto v28 = norm_view_fwd.z * cmd->forwardmove;
		const auto v24 = norm_view_right.x * cmd->sidemove;
		const auto v23 = norm_view_right.y * cmd->sidemove;
		const auto v25 = norm_view_right.z * cmd->sidemove;
		const auto v30 = norm_view_up.x * cmd->upmove;
		const auto v27 = norm_view_up.z * cmd->upmove;
		const auto v29 = norm_view_up.y * cmd->upmove;

		cmd->forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
			+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
			+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
		cmd->sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
			+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
			+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
		cmd->upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
			+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
			+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

		const auto ratio = 2.f - fmaxf(fabsf(cmd->sidemove), fabsf(cmd->forwardmove)) / 450.f;
		cmd->forwardmove *= ratio;
		cmd->sidemove *= ratio;

		wishangle = cmd->viewangles;
	}
	void MovementFix(Vector& oldang) // i think osmium
	{
		Vector vMovements(g::pCmd->forwardmove, g::pCmd->sidemove, 0.f);

		if (vMovements.Length2D() == 0)
			return;

		Vector vRealF, vRealR;
		Vector aRealDir = g::pCmd->viewangles;
		aRealDir.Clamp();

		g_Math.AngleVectors(aRealDir, &vRealF, &vRealR, nullptr);
		vRealF[2] = 0;
		vRealR[2] = 0;

		VectorNormalize(vRealF);
		VectorNormalize(vRealR);

		Vector aWishDir = oldang;
		aWishDir.Clamp();

		Vector vWishF, vWishR;
		g_Math.AngleVectors(aWishDir, &vWishF, &vWishR, nullptr);

		vWishF[2] = 0;
		vWishR[2] = 0;

		VectorNormalize(vWishF);
		VectorNormalize(vWishR);

		Vector vWishVel;
		vWishVel[0] = vWishF[0] * g::pCmd->forwardmove + vWishR[0] * g::pCmd->sidemove;
		vWishVel[1] = vWishF[1] * g::pCmd->forwardmove + vWishR[1] * g::pCmd->sidemove;
		vWishVel[2] = 0;

		float a = vRealF[0], b = vRealR[0], c = vRealF[1], d = vRealR[1];
		float v = vWishVel[0], w = vWishVel[1];

		float flDivide = (a * d - b * c);
		float x = (d * v - b * w) / flDivide;
		float y = (a * w - c * v) / flDivide;

		g::pCmd->forwardmove = x;
		g::pCmd->sidemove = y;
	}

	void ThirdPerson(ClientFrameStage_t curStage)
	{
		if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected() || !g::pLocalEntity)
			return;

		static bool init = false;
 
		if (GetKeyState(g_Menu.Config.thirdpersonkey) && g::pLocalEntity->IsAlive())
		{
			
			if (init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);
				g_pEngine->ExecuteClientCmd("thirdperson");
				g::intp = true;

			}
			init = false;
		}
		else
		{
			if (!init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);
				g_pEngine->ExecuteClientCmd("firstperson");
				g::intp = false;
			}
			init = true;
		}

		if (curStage == FRAME_RENDER_START && GetKeyState(g_Menu.Config.thirdpersonkey) && g::pLocalEntity->IsAlive())
		{
			g_pPrediction->SetLocalViewAngles(Vector(g::RealAngle.x, g::RealAngle.y, 0)); // lol
		}

		//if (g_pEngine->IsConnected() && g_pEngine->IsInGame() && curStage == FRAME_RENDER_START && GetKeyState(VK_MBUTTON) && !g::pLocalEntity->IsAlive() )
		//{
		//	g::pLocalEntity->SetObserverMode() = 5;
		//}


	}

	void RemoveFlash()
	{
		//if (g::pLocalEntity && g::pLocalEntity->m_flFlashDuration() > 0.0f)
		//	g::pLocalEntity->m_flFlashDuration() = 0.0f;
	}

	void Crosshair()
	{
		if (!g::pLocalEntity)
			return;

		if (!g::pLocalEntity->IsAlive())
			return;
		if (g_Menu.Config.Aimbot)
		{
			if (g::pLocalEntity->IsScoped())
			{
				int Height, Width;
				g_pEngine->GetScreenSize(Width, Height);

				Vector punchAngle = g::pLocalEntity->GetAimPunchAngle();

				float x = Width / 2;
				float y = Height / 2;
				int dy = Height / 90;
				int dx = Width / 90;
				x -= (dx*(punchAngle.y));
				y += (dy*(punchAngle.x));

				Vector2D screenPunch = { x, y };

				//	g_pSurface->Line(0, screenPunch.y, Width, screenPunch.y, Color(0, 0, 0, 255));
		//g_pSurface->Line(screenPunch.x, 0, screenPunch.x, Height, Color(0, 0, 0, 255));
				int screenW, screenH;
				g_pEngine->GetScreenSize(screenW, screenH);
				float centreX = screenW / 2, centreY = screenH / 2;

				g_pSurface->Line(centreX - 10, centreY, centreX + 10, centreY, Color(255, 255, 255, 255));
				g_pSurface->Line(centreX, centreY - 10, centreX, centreY + 10, Color(255, 255, 255, 255));
			}
			else
			{
				int screenW, screenH;
				g_pEngine->GetScreenSize(screenW, screenH);
				float centreX = screenW / 2, centreY = screenH / 2;

				g_pSurface->Line(centreX - 10, centreY, centreX + 10, centreY, Color(0, 0, 0, 255));
				g_pSurface->Line(centreX, centreY - 10, centreX, centreY + 10, Color(0, 0, 0, 255));
			}
		}
		static bool init = false;
		static bool init2 = false;

		if (g_Menu.Config.Aimbot)
		{
			if (g::pLocalEntity->IsScoped())
			{
				if (init2)
				{
					ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
					*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
					sv_cheats->SetValue(1);

					g_pEngine->ExecuteClientCmd("weapon_debug_spread_show 0");
					g_pEngine->ExecuteClientCmd("crosshair 0");
					g_pEngine->ExecuteClientCmd("mat_postprocess_enable 0");
				}
				init2 = false;
			}
			else
			{
				if (!init2)
				{
					ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
					*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
					sv_cheats->SetValue(1);

					g_pEngine->ExecuteClientCmd("weapon_debug_spread_show 3");
				//	g_pEngine->ExecuteClientCmd("crosshair 1");
				}
				init2 = true;
			}

			init = false;
		}
		else
		{
			if (!init)
			{
				ConVar* sv_cheats = g_pCvar->FindVar("sv_cheats");
				*(int*)((DWORD)&sv_cheats->fnChangeCallback + 0xC) = 0; // ew
				sv_cheats->SetValue(1);

				g_pEngine->ExecuteClientCmd("weapon_debug_spread_show 0");
				//g_pEngine->ExecuteClientCmd("crosshair 1");
			}
			init = true;
		}
	}

	void NormalWalk() // heh
	{
		g::pCmd->buttons &= ~IN_MOVERIGHT;
		g::pCmd->buttons &= ~IN_MOVELEFT;
		g::pCmd->buttons &= ~IN_FORWARD;
		g::pCmd->buttons &= ~IN_BACK;

		if (g::pCmd->forwardmove > 0.f)
			g::pCmd->buttons |= IN_FORWARD;
		else if (g::pCmd->forwardmove < 0.f)
			g::pCmd->buttons |= IN_BACK;
		if (g::pCmd->sidemove > 0.f)
		{
			g::pCmd->buttons |= IN_MOVERIGHT;
		}
		else if (g::pCmd->sidemove < 0.f)
		{
			g::pCmd->buttons |= IN_MOVELEFT;
		}
	}

	inline float FastSqrts(float x) {

		unsigned int i = *(unsigned int*)& x;
		i += 127 << 23;
		i >>= 1;

		return *(float*)& i;
	}
#define squares( x ) ( x * x )
	void do_fakewalk(CUserCmd* get_cmd, float get_speed)
	{

		if (get_speed <= 0.f)
			return;

		float min_speed = (float)(FastSqrts(squares(get_cmd->forwardmove) + squares(get_cmd->sidemove) + squares(get_cmd->upmove)));
		if (min_speed <= 0.f)
			return;

		if (get_cmd->buttons & IN_DUCK)
			get_speed *= 2.94117647f;

		if (min_speed <= get_speed)
			return;

		float kys = get_speed / min_speed;

		get_cmd->forwardmove *= kys;
		get_cmd->sidemove *= kys;
		get_cmd->upmove *= kys;
	}

	CUserCmd*     pCmd;
	C_BaseEntity* pLocal;
	void nightmode()
	{
		if (!g_Menu.Config.Nightmode)
		{
			for (auto i = 1; i <= g_pEntityList->GetHighestEntityIndex(); ++i)
			{
				auto entity = g_pEntityList->GetClientEntity(i);

				if (!entity || entity->IsDormant())
					continue;

				if (entity->GetClientClass()->ClassID == EClassIds::CEnvTonemapController)
				{
					auto hdr = dynamic_cast<C_EnvTonemapController*> (entity);
					hdr->use_custom_auto_exposure_min() = false;
					hdr->use_custom_auto_exposure_max() = false;
					hdr->custom_auto_exposure_min() = 1.f;
					hdr->custom_auto_exposure_max() = 1.f;
				}
			}

			return;
		}

		for (auto i = 1; i <=g_pEntityList->GetHighestEntityIndex(); ++i)
		{
			auto entity = g_pEntityList->GetClientEntity(i);

			if (!entity || entity->IsDormant())
				continue;

			if (entity->GetClientClass()->ClassID == EClassIds::CEnvTonemapController)
			{
				auto hdr = dynamic_cast<C_EnvTonemapController*> (entity);
				hdr->use_custom_auto_exposure_min() = true;
				hdr->use_custom_auto_exposure_max() = true;
				hdr->custom_auto_exposure_min() = 1.f * (1.f - g_Menu.Config.nightmodeval) + 0.01;
				hdr->custom_auto_exposure_max() = 1.f * (1.f - g_Menu.Config.nightmodeval) + 0.01;
			}
		}
	}
	void AutoRevolver()
	{
		auto me = g::pLocalEntity;
		auto cmd = g::pCmd;
		auto weapon = me->GetActiveWeapon();

		if (!g_Menu.Config.Aimbot)
			return;

		if (!me || !me->IsAlive() || !weapon)
			return;

		if (weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
		{
			static int delay = 0; /// pasted delay meme from uc so we'll stop shooting on high ping
			delay++;

			if (delay <= 15)
				g::pCmd->buttons |= IN_ATTACK;
			else
				delay = 0;
		}
	}



	void DoBhop() const
	{
		if (!g_Menu.Config.Aimbot)
			return;

		if (!g::pLocalEntity->IsAlive())
			return;

		static bool bLastJumped = false;
		static bool bShouldFake = false;

		if (!bLastJumped && bShouldFake)
		{
			bShouldFake = false;
			pCmd->buttons |= IN_JUMP;
		}
		else if (pCmd->buttons & IN_JUMP)
		{
			if (pLocal->GetFlags() & FL_ONGROUND)
			{
				bShouldFake = bLastJumped = true;
				g::bSendPacket = true;
			}
			else
			{
				pCmd->buttons &= ~IN_JUMP;
				bLastJumped = false;
			}
		}
		else
			bShouldFake = bLastJumped = false;
	}
	template<class T, class U>
	static T clamp2(T in, U low, U high) {
		if (in <= low)
			return low;

		if (in >= high)
			return high;

		return in;

	}
	void DoAutostrafe() const
	{
		if (!GetAsyncKeyState(VK_SPACE))
			return;

		if (GetAsyncKeyState('S') || GetAsyncKeyState('D') || GetAsyncKeyState('A') || GetAsyncKeyState('W'))
			return;

		if (g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_NOCLIP || g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
			return;

		if (!(g::pLocalEntity->GetFlags() & FL_ONGROUND))
		{
			if (pCmd->mousedx > 1 || pCmd->mousedx < -1)
			{
				pCmd->sidemove = clamp2(pCmd->mousedx < 0.f ? -450.0f : 450.0f, -450.0f, 450.0f);
			}
			else
			{
				pCmd->forwardmove = 5850.f / g::pLocalEntity->GetVelocity().Length();
				pCmd->sidemove = (pCmd->command_number % 2) == 0 ? -450.0f : 450.0f;
				if (pCmd->forwardmove > 450.0f)
					pCmd->forwardmove = 450.0f;
			}
		}
	}

	void DoFakeLags()
	{
		if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
			return;

		if (!g::pLocalEntity->IsAlive())
			return;

		if (g::pLocalEntity->IsNade())
			return;

		auto NetChannel = g_pEngine->GetNetChannel();

		if (!NetChannel)
			return;

		static bool Hitinit = false;
		bool SkipTick = false;
		static bool canHit = false;
		bool HitCheck = false;

		
		static bool lag = false;
		bool visible = false;
		int choke_amount = 0;
		for (int index = 1; index <= 65; index++)
		{
			auto entity = g_pEntityList->GetClientEntity(index);
			if (!entity) continue;
			if (entity->GetTeam() == g::pLocalEntity->GetTeam()) continue;
			if (entity->GetHealth() <= 0) continue;
			if (entity->GetClientClass()->ClassID != EClassIds::CCSPlayer) continue;
			if (entity->GetOrigin() == Vector(0, 0, 0)) continue;
			if (entity->IsImmune()) continue;
			


			C_Trace_Antario tr;
			C_TraceFilter traceFilter(entity);

			if (g::pLocalEntity)
			{
				traceFilter.pSkip1 = entity;
				g_pTrace->TraceRay(C_Ray_Antario(entity->GetEyePosition(), g::pLocalEntity->GetEyePosition()), mask_shot, &traceFilter, &tr);
				if (tr.m_pEnt == g::pLocalEntity || tr.flFraction == 1.f)
					visible = true;
				else
					visible = false;
			}


			Vector EnemyHead = { entity->GetOrigin().x, entity->GetOrigin().y, (entity->GetBonePosition(0).z + 16) };
			Vector Head = { g::pLocalEntity->GetOrigin().x, g::pLocalEntity->GetOrigin().y, (entity->GetBonePosition(0).z + 16) };
			Vector HeadExtr = (Head + (g::pLocalEntity->GetVelocity() * TIME_TO_TICKS(g_Menu.Config.sendfakelag)));
			Vector OriginExtr = ((g::pLocalEntity->GetOrigin() + (g::pLocalEntity->GetVelocity() * TIME_TO_TICKS(g_Menu.Config.sendfakelag))) + Vector(0, 0, 8));

			int origin_Dmg = Autowall::Get().CalculateDamage(Head,EnemyHead,g::pLocalEntity, entity).damage;
			int head_Dmg = Autowall::Get().CalculateDamage(OriginExtr, HeadExtr, g::pLocalEntity, entity).damage;


			if (g::pLocalEntity->GetVelocity().Length2D() > 4.f && visible && (origin_Dmg >= 5 || head_Dmg >= 5))
				lag = true;
			else
				lag = false;

		}

		if (lag)
			g::bSendPacket = NetChannel->m_nChokedPackets >= g_Menu.Config.sendfakelag;

			if (!lag)
			{
				if (abs(g::pLocalEntity->GetVelocity().Length2D()) < .1f && (g::pLocalEntity->GetFlags() & FL_ONGROUND))
					g::bSendPacket = (NetChannel->m_nChokedPackets >= g_Menu.Config.Fakelag);
				else if (abs(g::pLocalEntity->GetVelocity().Length2D()) >= .1f && (g::pLocalEntity->GetFlags() & FL_ONGROUND))
					g::bSendPacket = (NetChannel->m_nChokedPackets >= g_Menu.Config.Fakelag);
				else if (!(g::pLocalEntity->GetFlags() & FL_ONGROUND))
					g::bSendPacket = (NetChannel->m_nChokedPackets >= g_Menu.Config.Fakelag);
			}
	}
	void aimware_adaptive(bool& send_packet, C_BaseEntity* local, int ammount)
	{
		if (!local)
			return;
	
		float ExtrapolatedSpeed;
		int WishTicks;
		float VelocityY;
		float VelocityX;
		int WishTicks_1;
		signed int AdaptTicks;
		WishTicks = (ammount + 0.5) - 1;

		VelocityY = local->GetVelocity().y;
		VelocityX = local->GetVelocity().x;
		WishTicks_1 = 0;
		AdaptTicks = 2;
		ExtrapolatedSpeed = sqrt((VelocityX * VelocityX) + (VelocityY * VelocityY))
			* g_pGlobalVars->intervalPerTick;

		while ((WishTicks_1 * ExtrapolatedSpeed) <= 64.0)
		{
			if (((AdaptTicks - 1) * ExtrapolatedSpeed) > 64.0)
			{
				++WishTicks_1;
				break;
			}
			if ((AdaptTicks * ExtrapolatedSpeed) > 64.0)
			{
				WishTicks_1 += 2;
				break;
			}
			if (((AdaptTicks + 1) * ExtrapolatedSpeed) > 64.0)
			{
				WishTicks_1 += 3;
				break;
			}
			if (((AdaptTicks + 2) * ExtrapolatedSpeed) > 64.0)
			{
				WishTicks_1 += 4;
				break;
			}
			if (((AdaptTicks + 3) * ExtrapolatedSpeed) > 64.0)
			{
				WishTicks_1 += 5;
				break;
			}
			AdaptTicks += 6;
			WishTicks_1 += 6;
			if (AdaptTicks > 16) // originally 16
				break;
		}

		bool should_choke;

		if (*(int*)(uintptr_t(g_pClientState) + 0x4D28) < WishTicks_1 && *(int*)(uintptr_t(g_pClientState) + 0x4D28) < g_Math.RandomFloat(15, 17))
			should_choke = true;
		else
			should_choke = false;

		if (should_choke)
			send_packet = (WishTicks_1 <= *(int*)(uintptr_t(g_pClientState) + 0x4D28));

		else
			send_packet = (g_Math.RandomFloat(1,4) <= *(int*)(uintptr_t(g_pClientState) + 0x4D28)); // originally "4"
	}


	void fakelag_adaptive(int ticks) // pasted from reversed aimware :^) reversed aimware pasted from chance :^))
	{
		if (!g_Menu.Config.Aimbot)
			return;
		auto speed = g::pLocalEntity->GetVelocity().Length();
		bool standing = speed <= 1.0f;
		float UnitsPerTick = 0.0f;
		int WishTicks = 0;
		int AdaptiveTicks = 2;
		UnitsPerTick = g::pLocalEntity->GetVelocity().Length() * g_pGlobalVars->intervalPerTick;
		if (standing) {
			g::bSendPacket = !(g_pClientState->chokedcommands < 17);
		}
		while ((WishTicks * UnitsPerTick) <= 68.0f) {
			if (((AdaptiveTicks - 1) * UnitsPerTick) > 68.0f) {
				++WishTicks;
				break;
			}
			if ((AdaptiveTicks * UnitsPerTick) > 68.0f) {
				WishTicks += 2;
				break;
			}
			if (((AdaptiveTicks + 1) * UnitsPerTick) > 68.0f) {
				WishTicks += 3;
				break;
			}
			if (((AdaptiveTicks + 2) * UnitsPerTick) > 68.0f) {
				WishTicks += 4;
				break;
			}
			AdaptiveTicks += 5;
			WishTicks += 5;
			if (AdaptiveTicks > 16)
				break;
		}

		g::bSendPacket = !(g_pClientState->chokedcommands < WishTicks);
	}
	void DoFakeLag() 
	{
		if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame() || g_Menu.Config.Fakelag == 0 || !g_Menu.Config.Aimbot)
			return;

		if (!g::pLocalEntity->IsAlive())
			return;

		if (g_pEngine->IsVoiceRecording())
			return;


		auto NetChannels = g_pEngine->GetNetChannel();
		
		if (!NetChannels)
			return;
	
		float hitdmg;
		static auto counter = 0;
		auto local = g::pLocalEntity;
		static auto last_simtime = 0.f;
		auto CL_SendMoved = []() {
			using CL_SendMove_t = void(__fastcall*)(void);
			static CL_SendMove_t CL_SendMoveFd = (CL_SendMove_t)Utils::FindSignature("engine.dll", "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");

			CL_SendMoveFd();
		};
		static auto last_origin = Vector();
		auto net_channel = *reinterpret_cast<NetChannel * *>(reinterpret_cast<uintptr_t>(g_pClientState) + 0x9C);
		auto lc = (last_simtime == (g_pGlobalVars->curtime - g_pGlobalVars->intervalPerTick) * g_pGlobalVars->tickcount && (local->GetOrigin() - last_origin).Length2DSqr() > 4096.f);
		switch (g_Menu.Config.fakelags)
		{
		case 0:
			//lagpeek();
			if (!local || !local->IsAlive()) {
				g::bSendPacket = !counter;
				counter++;
			}
			else {
				g::bSendPacket = counter > g_Menu.Config.Fakelag || lc;
				counter = counter > g_Menu.Config.Fakelag ? 0 : counter + 1;
			}
			break;
		case 1:
			aimware_adaptive(g::bSendPacket=false,g::pLocalEntity,g_Menu.Config.Fakelag);
			
			//DoFakeLags();
			break;
		case 2:
			break;
		}
		last_origin = local->GetOrigin();
		last_simtime = g_pGlobalVars->curtime;
	}
	/*void modelchanger()
	{
		const char* models_to_change[] = {
		("models/player/custom_player/legacy/tm_balkan_varianta.mdl"),
		("models/player/custom_player/legacy/tm_balkan_variantb.mdl"),
		("models/player/custom_player/legacy/tm_balkan_variantc.mdl"),
		("models/player/custom_player/legacy/tm_balkan_variantd.mdl"),
		("models/player/custom_player/legacy/ctm_fbi_variantb.mdl"),
		("models/player/custom_player/legacy/ctm_fbi_variantf.mdl"),
		("models/player/custom_player/legacy/ctm_fbi_variantg.mdl"),
		("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"),
		("models/player/custom_player/legacy/ctm_heavy.mdl"),
		("models/player/custom_player/legacy/ctm_st6.mdl"),
		("models/player/custom_player/legacy/ctm_st6_varianta.mdl"),
		("models/player/custom_player/legacy/ctm_st6_variantb.mdl"),
		("models/player/custom_player/legacy/ctm_st6_variantc.mdl"),
		("models/player/custom_player/legacy/ctm_st6_variantd.mdl"),
		("models/player/custom_player/legacy/tm_balkan_variantg.mdl"),
		("models/player/custom_player/legacy/tm_balkan_varianth.mdl"),
		("models/player/custom_player/legacy/tm_balkan_varianti.mdl"),
		("models/player/custom_player/legacy/tm_balkan_variantj.mdl"),
		("models/player/custom_player/legacy/tm_phoenix_variantf.mdl"),
		("models/player/custom_player/legacy/tm_phoenix_variantg.mdl"),
		("models/player/custom_player/legacy/tm_phoenix_varianth.mdl"),
		};

		for (int i = 1; i <= 64; i++)
		{
			auto entity = (C_BaseEntity*)g_pEntityList->GetClientEntity(i);

			if (!entity)
				continue;

			if (entity->IsPlayer())
			{
				auto ModelIndex = g_pModelInfo->GetModelIndex(models_to_change[g_Vars.misc.model_type]);

				if (ModelIndex)
					entity->SetModelIndex(ModelIndex);
			}
		}
	}
	}*/
};




extern Misc g_Misc;