#include "AntiAim.h"
#include "Autowall.h"
#include "Utils.h"
#include "ISurface.h"
#include "IVEngineClient.h"
#include "Aimbot.h"
#include "PlayerInfo.h"
#include "Math.h"
#include "Menu.h"
#include <DirectXMath.h>
#include <optional>
auto client_state = *reinterpret_cast<uintptr_t*>(uintptr_t(GetModuleHandle("engine.dll")) + 0x57E854);
void AntiAim::doubletap(CUserCmd*cmd)
{
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (GetAsyncKeyState('I'))
	{
		static bool counter = false;
		static int counters = 0;
		//local_player->CL_SendMove();
		if (counters == 2)
		{
			counters = 0;
			counter = !counter;

		}
		counters++;
		if (counter)
		{
			cmd->buttons &= ~IN_ATTACK;
			g::m_nTickbaseShift = 100;
		}
		else
		{
			if (!g_pClientState->chokedcommands)
			{
				cmd->buttons |= IN_ATTACK;
				g::m_nTickbaseShift = 0;
			}
		}

	}
}
float desync_delta()
{
	if (!g::pLocalEntity)
		return 0.f;
	auto state = g::pLocalEntity->AnimState();

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
void AntiAim::FreeStanding_jitter() // cancer v1
{
	static float FinalAngle;
	bool bside1 = false;
	bool bside2 = false;
	bool autowalld = false;
	static bool flip = false;
	flip = !flip;

	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == g::pLocalEntity
			|| pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		float angToLocal = g_Math.CalcAngle(g::pLocalEntity->GetOrigin(), pPlayerEntity->GetOrigin()).y;
		Vector ViewPoint = pPlayerEntity->GetOrigin() + Vector(0, 0, 90);

		Vector2D Side1 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal))),(45 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side2 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(45 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal))),(50 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side4 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(50 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };

		Vector Origin = g::pLocalEntity->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };

		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 80 };
			Vector OriginAutowall2 = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_Autowall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					bside1 = true;
					if (g::bSendPacket)
						FinalAngle = angToLocal + g_Menu.Config.yawright;
					else
						FinalAngle = angToLocal - (flip ? 58 : g_Menu.Config.yawrightdesync);
				}
				else if (side == 1)
				{
					bside2 = true;
					if (g::bSendPacket)
						FinalAngle = angToLocal - g_Menu.Config.yawleft;
					else
						FinalAngle = angToLocal + (flip? 58: g_Menu.Config.yawleftdesync);
				}
				autowalld = true;
			}
			else
			{
				for (int side222 = 0; side222 < 2; side222++)
				{
					Vector OriginAutowall222 = { Origin.x + OriginLeftRight[side222].x,  Origin.y - OriginLeftRight[side222].y , Origin.z + 80 };

					if (g_Autowall.CanHitFloatingPoint(OriginAutowall222, OriginAutowall2))
					{
						if (side222 == 0)
						{
							bside1 = true;
							if (g::bSendPacket)
							FinalAngle = angToLocal + g_Menu.Config.yawright;
							else
							FinalAngle = angToLocal - (flip ? 58 : g_Menu.Config.yawrightdesync);
						}
						else if (side222 == 1)
						{
							bside2 = true;
							if(g::bSendPacket)
							FinalAngle = angToLocal - g_Menu.Config.yawleft;
							else
							FinalAngle = angToLocal + (flip ? 58 : g_Menu.Config.yawleftdesync);

						}
						autowalld = true;
					}
				}
			}
		}
	}

	if (!autowalld || (bside1 && bside2))
	{
		if (!g::bSendPacket)
		{
			g::pCmd->viewangles.y += (flip ? 90.0f : -90.0f);
			//local_player->GetAnimState()->m_flGoalFeetYaw -= 40;
		}
		else
		{
			g::pCmd->viewangles.y -= (flip ? 180 : 180);
			g::pCmd->viewangles.Normalize();
		}
	}
	else
		g::pCmd->viewangles.y = FinalAngle;
}
void AntiAim::FreeStanding() // cancer v1
{
	static float FinalAngle;
	bool bside1 = false;
	bool bside2 = false;
	bool autowalld = false;
	static bool flip = false;
	flip = !flip;

	for (int i = 1; i <= g_pEngine->GetMaxClients(); ++i)
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| pPlayerEntity->IsDormant()
			|| pPlayerEntity == g::pLocalEntity
			|| pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		float angToLocal = g_Math.CalcAngle(g::pLocalEntity->GetOrigin(), pPlayerEntity->GetOrigin()).y;
		Vector ViewPoint = pPlayerEntity->GetOrigin() + Vector(0, 0, 90);

		Vector2D Side1 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal))),(45 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side2 = { (45 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(45 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };

		Vector2D Side3 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal))),(50 * cos(g_Math.GRD_TO_BOG(angToLocal))) };
		Vector2D Side4 = { (50 * sin(g_Math.GRD_TO_BOG(angToLocal + 180))) ,(50 * cos(g_Math.GRD_TO_BOG(angToLocal + 180))) };

		Vector Origin = g::pLocalEntity->GetOrigin();

		Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

		Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };

		for (int side = 0; side < 2; side++)
		{
			Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 80 };
			Vector OriginAutowall2 = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

			if (g_Autowall.CanHitFloatingPoint(OriginAutowall, ViewPoint))
			{
				if (side == 0)
				{
					bside1 = true;
					if (g::bSendPacket)
						FinalAngle = angToLocal + 90;
					else
						FinalAngle = angToLocal - 90;
				}
				else if (side == 1)
				{
					bside2 = true;
					if (g::bSendPacket)
						FinalAngle = angToLocal - 90;
					else
						FinalAngle = angToLocal + 90;
				}
				autowalld = true;
			}
			else
			{
				for (int side222 = 0; side222 < 2; side222++)
				{
					Vector OriginAutowall222 = { Origin.x + OriginLeftRight[side222].x,  Origin.y - OriginLeftRight[side222].y , Origin.z + 80 };

					if (g_Autowall.CanHitFloatingPoint(OriginAutowall222, OriginAutowall2))
					{
						if (side222 == 0)
						{
							bside1 = true;
							if (g::bSendPacket)
								FinalAngle = angToLocal + 90;
							else
								FinalAngle = angToLocal - 90;
						}
						else if (side222 == 1)
						{
							bside2 = true;
							if (g::bSendPacket)
								FinalAngle = angToLocal - 90;
							else
								FinalAngle = angToLocal + 90;

						}
						autowalld = true;
					}
				}
			}
		}
	}

	if (!autowalld || (bside1 && bside2))
	{
			g::pCmd->viewangles.y -= 180;
			g::pCmd->viewangles.Normalize();
			g::pCmd->viewangles.NormalizeInPlace();
			g::pCmd->viewangles.Clamp();
	}
	else
		g::pCmd->viewangles.y = FinalAngle;
}
void AntiAim::OnCreateMove()
{
	if (!g_pEngine->IsInGame())
		return;

	if (g::pLocalEntity->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
		return;

	if (!g::pLocalEntity->IsAlive())
		return;

	if (!g::pLocalEntity->GetActiveWeapon() || g::pLocalEntity->IsNade())
		return;

	if ((g::pCmd->buttons & IN_ATTACK))
	{
		if (g::aimangle == Vector(0, 0, 0))
		{
			return;
		}
		else
		{
			if (g::bSendPacket)
				g::pCmd->viewangles = g::aimangle;
			else
				g::pCmd->viewangles.y = g::pLocalEntity->get_max_desync_delta_aimware() + 20.0f;
		}
	}
		
	if (g::pCmd->buttons & IN_USE)
	{
		g::bSendPacket = true;
		return;
	}
	static bool left = false;
	static bool right = false;
	static int choked = 0;
	static bool desync_flip1 = false;
	static bool desync_flip2 = false;
	static bool flip = false;
	flip = !flip;
	float desynctype = 0.0f;
	float desync = g::pLocalEntity->get_max_desync_delta_aimware();
	float inverse_desync = 190.0f - desync;
	switch (g_Menu.Config.jitternewcombo)
	{
	case 0:
		desynctype = g::pLocalEntity->get_max_desync_delta_aimware();
		break;
	case 1:
		desynctype = 190.0f - desync;
		break;
	case 2:
		desynctype = 180.0f - inverse_desync * 0.5f;
		break;
	}

	if (g::bSendPacket)
		desync_flip2 = !desync_flip2;
	g::pCmd->viewangles.y += g_Menu.Config.yaw_additive;
	g::pCmd->viewangles.y -= g_Menu.Config.yaw_subtractive;
	
	switch (g_Menu.Config.pitchss)
	{
	case 0:
		break;
	case 1:
		g::pCmd->viewangles.x += 89.0f;
		break;
	case 2:
		g::pCmd->viewangles.x -= 89.0f;
		break;
	case 3:
		g::pCmd->viewangles.x += flip ? 89.0f : -89.0f;
		break;
	}
	switch (g_Menu.Config.Antiaimtype)
	{
	case 0:
		break;
	case 1:
		g::pCmd->viewangles.y += 180.0f;
		break;
	case 2:
		g::pCmd->viewangles.y += g_Math.RandomFloat(179.0f, 180.0f);
		break;
	case 3:
		g::pCmd->viewangles.y = freestand();
		break;
	}
	if (g_Menu.Config.lbystabler)
	{
		if (g::pCmd->forwardmove == 0.0f)
		{
			g::pCmd->forwardmove += flip ? -1.01f : 1.01f;
		}
	}
	switch (g_Menu.Config.Antiaimtypedsy)
	{
	case 0:

		break;
	case 1:
		desync_flip();
		break;
	case 2:
		jitter();
		break;
	case 3:
		if (!g::bSendPacket)
		{
			g::pCmd->viewangles.y += (desync_flip1 ? desync_delta() : -desync_delta());

		}
		else
		{
			desync_flip1 = !desync_flip1;
			g::pCmd->viewangles.y += 180;
			g::pCmd->viewangles.y -= (desync_flip1 ? g_Menu.Config.jitterrange : -g_Menu.Config.jitterrange);
			g::pCmd->viewangles.Normalize();
		}
		break;
	case 4:
		FreeStanding_jitter();
		break;
	case 5:
		if (GetKeyState(g_Menu.Config.desyncswitchkey))
		{
			if (!g::bSendPacket)
			{
				g::pCmd->viewangles.y -= (desync_flip1 ? 0 : g_Menu.Config.desyncjitterreal);
			}
			else
			{
				desync_flip1 = !desync_flip1;
				g::pCmd->viewangles.y += 180;
				g::pCmd->viewangles.y -= (desync_flip1 ? 0 : -g_Menu.Config.desyncjitterfake);
				g::pCmd->viewangles.Normalize();
			}
		}
		else
		{
			if (!g::bSendPacket)
			{
				g::pCmd->viewangles.y += (desync_flip1 ? 0 : g_Menu.Config.desyncjitterreal);
			}
			else
			{
				desync_flip1 = !desync_flip1;
				g::pCmd->viewangles.y += 180;
				g::pCmd->viewangles.y += (desync_flip1 ? 0 : -g_Menu.Config.desyncjitterfake);
				g::pCmd->viewangles.Normalize(); 
			}
		}
		break;
	case 6:
		if (!g::bSendPacket)
		{
			g::pCmd->viewangles.y += 0 + (desync_flip2 ? g::pLocalEntity->get_max_desync_delta_aimware() : -g::pLocalEntity->get_max_desync_delta_aimware());
		}
		else
		{

			//cmd->viewangles.y -= 180;
			g::pCmd->viewangles.Normalize();
		}
		break;
	case 7:
		
		if (g::bSendPacket)
		{
			g::pCmd->viewangles.y -= /*180*/g_Menu.Config.jitternew + (desync_flip2 ? desynctype : -desynctype);
		}
		//FreeStanding();
		break;
	}

}
 void inline AntiAim::sincos(float radians, float* sine, float* cosine) {
	 __asm {
		 fld dword ptr[radians]
		 fsincos
		 mov edx, dword ptr[cosine]
		 mov eax, dword ptr[sine]
		 fstp dword ptr[edx]
		 fstp dword ptr[eax]
	 }
 }

 void AntiAim::anglevectors(Vector& angles, Vector* forward, Vector* right, Vector* up) {
	 float sp, sy, sr, cp, cy, cr;

	 sincos(angles.x * (PI / 180.0f), &sp, &cp);
	 sincos(angles.y * (PI / 180.0f), &sy, &cy);
	 sincos(angles.z * (PI / 180.0f), &sr, &cr);

	 if (forward) {
		 forward->x = cp * cy;
		 forward->y = cp * sy;
		 forward->z = -sp;
	 }

	 if (right) {
		 right->x = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
		 right->y = -1.0f * sr * sp * sy + -1.0f * cr * cy;
		 right->z = -1.0f * sr * cp;
	 }

	 if (up) {
		 up->x = cr * sp * cy + -sr * -sy;
		 up->y = cr * sp * sy + -sr * cy;
		 up->z = cr * cp;
	 }
 }
float AntiAim::freestand() {
	auto local = g::pLocalEntity;

	float b, r, l;

	if (!local)
		return 0.0f;

	Vector src, dst, forward, right, up;
	C_Trace_Antario tr;
	C_TraceFilter filter(g::pLocalEntity);

	Vector va;
	g_pEngine->GetViewAngles(va);

	va.x = 0.0f;

	anglevectors(va, &forward, &right, &up);


	src = local->GetEyePosition();
	dst = src + forward * 384.0f;

	g_pTrace->TraceRay(C_Ray_Antario(src, dst), 0x46004003, &filter, &tr);

	b = (tr.end - tr.start).Length();

	g_pTrace->TraceRay(C_Ray_Antario(src + right * 35.0f, dst + right * 35.0f), 0x46004003, &filter, &tr);

	r = (tr.end - tr.start).Length();

	g_pTrace->TraceRay(C_Ray_Antario(src - right * 35.0f, dst - right * 35.0f), 0x46004003, &filter, &tr);

	l = (tr.end - tr.start).Length();

	if (l > r)
		return va.y - 90.0f;
	else if (r > l)
		return va.y + 90.0f;
	else if (b > r || b > l)
		return va.y - 180.0f;

	return 0.0f;
}
float AntiAim::freestand_jitter() {
	auto local = g::pLocalEntity;

	float b, r, l;

	if (!local)
		return 0.0f;

	Vector src, dst, forward, right, up;
	C_Trace_Antario tr;
	C_TraceFilter filter(g::pLocalEntity);

	Vector va;
	g_pEngine->GetViewAngles(va);

	va.x = 0.0f;

	anglevectors(va, &forward, &right, &up);


	src = local->GetEyePosition();
	dst = src + forward * 384.0f;

	g_pTrace->TraceRay(C_Ray_Antario(src, dst), 0x46004003, &filter, &tr);

	b = (tr.end - tr.start).Length();

	g_pTrace->TraceRay(C_Ray_Antario(src + right * 35.0f, dst + right * 35.0f), 0x46004003, &filter, &tr);

	r = (tr.end - tr.start).Length();

	g_pTrace->TraceRay(C_Ray_Antario(src - right * 35.0f, dst - right * 35.0f), 0x46004003, &filter, &tr);

	l = (tr.end - tr.start).Length();
	static bool flip = false;
	flip = !flip;
	if (!g::bSendPacket)
	{
		if (l > r)
			return va.y + (flip ? 90.0f : 170.0f);
		else if (r > l)
			return va.y - (flip ? 90.0f : 170.0f);
		else if (b > r || b > l)
			return va.y + (flip ? 90.0f : 170.0f);
	}
	else
	{
		if (l > r)
			return va.y - (135.0f);
		else if (r > l)
			return va.y + (135.0f);
		//else if (b > r || b > l)
			//return va.y - (90);
	}

	return 0.0f;
}
float AntiAim::corrected_tickbase()
{
	CUserCmd* last_ucmd = nullptr;
	int corrected_tickbase = 0;

	corrected_tickbase = (!last_ucmd || last_ucmd->hasbeenpredicted) ? (float)g::pLocalEntity->GetTickBase() : corrected_tickbase++;
	last_ucmd = g::pCmd;
	float corrected_curtime = corrected_tickbase * g_pGlobalVars->intervalPerTick;
	return corrected_curtime;

};

void AntiAim::predict_lby_update(float sampletime, CUserCmd* ucmd, bool& sendpacket)
{
	g::plby_update = false;
	static float next_lby_update_time = 0;
	auto local = g::pLocalEntity;

	if (!(local->GetFlags() & 1))
		return;

	if (local->GetVelocity().Length2D() > 0.1f)
		next_lby_update_time = corrected_tickbase() + 0.22f;
	else if (next_lby_update_time - corrected_tickbase() <= 0.0f) {
		next_lby_update_time = corrected_tickbase() + 1.1f;
		g::plby_update = true;
		sendpacket = false;
	}
	else if (next_lby_update_time - corrected_tickbase() <= 1 * g_pGlobalVars->intervalPerTick)
		sendpacket = true;
}
void AntiAim::desync_flip()
{
	
	if (GetKeyState(g_Menu.Config.desyncswitchkey))
	{
		g::pCmd->viewangles.y += g_Menu.Config.LeftDesyncValue;
		if (g::bSendPacket)
		{
			g::pCmd->viewangles.y += desync_delta();
		}
		else
		{
			if (g::plby_update)
				g::pCmd->viewangles.y -= 122.0f;
			else
				g::pCmd->viewangles.y += 120.0f /*desync_delta() * 2*/;
		}
	}
	else
	{
		g::pCmd->viewangles.y -= g_Menu.Config.RightDesyncValue;
		if (g::bSendPacket)
		{
			g::pCmd->viewangles.y -= desync_delta();
		}
		else
		{
			if (g::plby_update)
				g::pCmd->viewangles.y += 122.0f;
			else
				g::pCmd->viewangles.y -= 120.0f /*desync_delta() * 2*/;

		}
	}

}
mstudiobbox_t* get_hitboxv2(C_BaseEntity* entity, int hitbox_index)
{
	if (entity->IsDormant() || entity->GetHealth() <= 0)
		return NULL;

	const auto pModel = entity->GetModel();
	if (!pModel)
		return NULL;

	auto pStudioHdr = g_pModelInfo->GetStudiomodel(pModel);
	if (!pStudioHdr)
		return NULL;

	auto pSet = pStudioHdr->GetHitboxSet(0);
	if (!pSet)
		return NULL;

	if (hitbox_index >= pSet->numhitboxes || hitbox_index < 0)
		return NULL;

	return pSet->GetHitbox(hitbox_index);
}
Vector GetHitboxPositionv2(C_BaseEntity* entity, int hitbox_id)
{
	auto hitbox = get_hitboxv2(entity, hitbox_id);
	if (!hitbox)
		return Vector(0, 0, 0);

	auto bone_matrix = entity->GetBoneMatrix(hitbox->bone);

	Vector bbmin, bbmax;
	g_Math.VectorTransform(hitbox->min, bone_matrix, bbmin);
	g_Math.VectorTransform(hitbox->max, bone_matrix, bbmax);

	return (bbmin + bbmax) * 0.5f;
}
void NormalizeNum(Vector &vIn, Vector &vOut)
{
	float flLen = vIn.Length();
	if (flLen == 0) {
		vOut.Init(0, 0, 1);
		return;
	}
	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}
void VectorSubtractv2(const Vector& a, const Vector& b, Vector& c)
{
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}
float fov_player(Vector ViewOffSet, Vector View, C_BaseEntity* entity, int hitbox)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	Vector Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	g_Math.AngleVectors(Angles, &Forward);

	Vector AimPos = GetHitboxPositionv2(entity, hitbox); //pvs fix disabled

	VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	NormalizeNum(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / M_PI));
}


int closest_to_crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;

	auto local_player = g::pLocalEntity;

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetEyePosition();

	Vector angles;
	g_pEngine->GetViewAngles(angles);

	for (int i = 1; i <= g_pGlobalVars->maxClients; i++)
	{
		auto entity = g_pEntityList->GetClientEntity(i);

		if (!entity || !entity->IsAlive() || entity->GetTeam() == local_player->GetTeam() || entity->IsDormant() || entity == local_player)
			continue;

		float fov = fov_player(local_position, angles, entity, 0);

		if (fov < lowest_fov)
		{
			lowest_fov = fov;
			index = i;
		}
	}

	return index;
}

float AntiAim::halogendirection(float yaw)
{
	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	C_Trace_Antario tr;
	C_TraceFilter filter(g::pLocalEntity);

	Vector viewangles;
	g_pEngine->GetViewAngles(viewangles);

	viewangles.x = 0;



	anglevectors(viewangles, &forward, &right, &up);
	int index = closest_to_crosshair();
	auto entity = g_pEntityList->GetClientEntity(index);
	auto local = g::pLocalEntity;
	if (!local)
		return -1;
	src3D = local->GetEyePosition();
	dst3D = src3D + (forward * 384);

	g_pTrace->TraceRay(C_Ray_Antario(src3D, dst3D), mask_shot, &filter, &tr);
	Back = (tr.end - tr.start).Length();
	g_pTrace->TraceRay(C_Ray_Antario(src3D + right * 35, dst3D + right * 35), mask_shot, &filter, &tr);
	Right = (tr.end - tr.start).Length();
	g_pTrace->TraceRay(C_Ray_Antario(src3D - right * 35, dst3D - right * 35), mask_shot, &filter, &tr);
	Left = (tr.end - tr.start).Length();

	if (Back > Right && Back > Left && Left == Right && !(entity == nullptr))
	{
		return (yaw + 180); //if left and right are equal and better than back
	}

	static bool flip = false;




	if (g::bSendPacket)
	{

		flip = !flip;
		if (Left > Right && !(entity == nullptr))
			return yaw + (flip ? 80 : -80);
		else if (Right > Left && !(entity == nullptr))
			return yaw - (flip ? 80 : 80);
		else
			return  yaw - 90 - ((flip ? 180 : -180) + (flip ? 89 : +87));

	}

	if (!g::bSendPacket)
	{
		if (Left > Right && !(entity == nullptr))
			return yaw - 90.0f;
		else if (Right > Left && !(entity == nullptr))
			return yaw + 90.0f;
		else
			return yaw + (flip ? 150 : 1);

	}
}
void AntiAim::jitter()
{
	static bool flip = false;
	 
	if(g::bSendPacket)
		flip = !flip;

		if (g::bSendPacket)
		{
			g::pCmd->viewangles.y -= 180.0f + (flip ?g_Menu.Config.jitterrange : -g_Menu.Config.jitterrange);
		}
		else
		{
			g::pCmd->viewangles.y -= 180.0F;
		}
}

AntiAim g_AntiAim;


