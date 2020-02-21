#include "ESP.h"
#include "Aimbot.h"
#include "LagComp.h"
#include "ClientClass.h"
#include "Utils.h"
#include "Definitions.h"
#include "Imaterial.h"
#include "IVEngineClient.h"
#include "IVModelInfo.h"
#include "PlayerInfo.h"
#include "ISurface.h"
#include "Hitboxes.h"

#include "ICvar.h"
#include "Math.h"
#include "Menu.h"

ESP g_ESP;

void ESP::RenderBox()
{
	g_pSurface->OutlinedRect(Box.left, Box.top, Box.right, Box.bottom, color);
	g_pSurface->OutlinedRect(Box.left + 1, Box.top + 1, Box.right - 2, Box.bottom - 2, Color(0, 0, 0, 240));
	g_pSurface->OutlinedRect(Box.left - 1, Box.top - 1, Box.right + 2, Box.bottom + 2, Color(0, 0, 0, 240));
}

void ESP::RenderName(C_BaseEntity* pEnt, int iterator)
{
    PlayerInfo_t pInfo;
    g_pEngine->GetPlayerInfo(iterator, &pInfo);

	if (g_Menu.Config.Aimbot)
		g_pSurface->DrawT(Box.left + (Box.right / 2), Box.top - 16, textcolor, font, true, pInfo.szName);
	else if (g_Menu.Config.Name == 2)
	{
		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top, textcolor, font, false, pInfo.szName);
		offsetY += 1;
	}
}

void ESP::RenderInfo()
{
	if (!g_pEngine->IsConnected())
		return;
	auto nci = g_pEngine->GetNetChannelInfo();
	std::string ping = std::to_string((int)(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING) * 1000)) + " ms";
	std::string reght = "Right";
	std::string laft = "left";
	g_pSurface->DrawT(8, 315, textcolor, font, false,  ("PING: " + ping).c_str());

	std::string fps = std::to_string(static_cast<int>(1.f / g_pGlobalVars->frametime));
	g_pSurface->DrawT(8, 290, textcolor, font, false, ("FPS: " + fps).c_str());

	if (GetKeyState(VK_RIGHT)|| GetKeyState(VK_LEFT))
	{
		g_pSurface->DrawT(8, 265, textcolor, font, false, ("REAL: " + reght).c_str());
	}
	else
	{
		g_pSurface->DrawT(8, 265, textcolor, font, false, ("REAL: " + laft).c_str());
	}
	auto chocked_commands = *(int*)(uintptr_t(g_pClientState) + 0x4D28);

	int screenSizeX, screenCenterX;
	int screenSizeY, screenCenterY;
	g_pEngine->GetScreenSize(screenSizeX, screenSizeY);
	RECT FakeSize = g_pSurface->GetTextSizeRect(g::CourierNew, "Fake");
	RECT CHOKESIZE = g_pSurface->GetTextSizeRect(g::CourierNew, "Choke");
	float ChokedWidth;
	ChokedWidth = (((chocked_commands * (CHOKESIZE.right / 14.f))));
	float DesyncWidth;
	screenCenterX = screenSizeX - 1350;
	screenCenterY = 350;

	int screenW, screenH;
	g_pEngine->GetScreenSize(screenW, screenH);
	float centreX = screenW / 2, centreY = screenH / 2;

	

	int CHOKE_G = chocked_commands / 14.f * 255.f;
	int CHOKE_R = 255.f - CHOKE_G;
	int CHOKE_B = 0.f;

	static bool Show_Choke = true;

	int IndicatorPos = 0;


	if (Show_Choke && g::pLocalEntity->IsAlive()) {
		
		g_pSurface->DrawT(screenCenterX - 5 - IndicatorPos + (CHOKESIZE.right / 2), screenCenterY - 14, Color(255, 255, 255, 255), g::CourierNew, true, "Fakelag");

		g_pSurface->FilledRect(screenCenterX - 5 - IndicatorPos, screenCenterY, CHOKESIZE.right, 4, Color(0, 0, 0, 185));
		g_pSurface->FilledRect(screenCenterX - 5 - IndicatorPos, screenCenterY, ChokedWidth, 4, Color(CHOKE_R, CHOKE_G, CHOKE_B, 255));
		g_pSurface->OutlinedRect(screenCenterX - 5 - IndicatorPos, screenCenterY, CHOKESIZE.right, 4, Color(0, 0, 0, 255));;

		IndicatorPos += CHOKESIZE.right;
	}
}

float XM_PI = 3.141592654f;

void ESP::draw_spread()
{
	if (!g_Menu.Config.Aimbot)
		return;

	if (!g::pLocalEntity)
		return;

	if (!g::pLocalEntity->IsAlive())
		return;

	if (!g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		return;

	auto weapon = g::pLocalEntity->GetActiveWeapon();

	if (!weapon)
		return;

	if (weapon) {
		int screen_w, screen_h;
		g_pEngine->GetScreenSize(screen_w, screen_h);
		int cross_x = screen_w / 2, cross_y = screen_h / 2;

		float recoil_step = screen_h / 180;

		cross_x -= (int)(g::pLocalEntity->GetAimPunchAngle().y * recoil_step);
		cross_y += (int)(g::pLocalEntity->GetAimPunchAngle().x * recoil_step);

		weapon->GetAccuracyPenalty();
		float inaccuracy = weapon->GetInaccuracy();
		float spread = weapon->GetSpread();

		float cone = inaccuracy * spread;
		cone *= screen_h * 0.7f;
		cone *= 90.f / 180;

		for (int seed{ }; seed < 512; ++seed) {
			g_Math.RandomSeed(g_Math.RandomFloat(0, 512));

			float	rand_a = g_Math.RandomFloat(0.f, 1.f);
			float	pi_rand_a = g_Math.RandomFloat(0.f, 2.f * XM_PI);
			float	rand_b = g_Math.RandomFloat(0.0f, 1.f);
			float	pi_rand_b = g_Math.RandomFloat(0.f, 2.f *  XM_PI);

			float spread_x = cos(pi_rand_a) * (rand_a * inaccuracy) + cos(pi_rand_b) * (rand_b * spread);
			float spread_y = sin(pi_rand_a) * (rand_a * inaccuracy) + sin(pi_rand_b) * (rand_b * spread);

			float max_x = cos(pi_rand_a) * cone + cos(pi_rand_b) * cone;
			float max_y = sin(pi_rand_a) * cone + sin(pi_rand_b) * cone;

			float step = screen_h / 180 * 90.f;
			int screen_spread_x = (int)(spread_x * step * 0.7f);
			int screen_spread_y = (int)(spread_y * step * 0.7f);

			float percentage = (rand_a * inaccuracy + rand_b * spread) / (inaccuracy + spread);

			g_pSurface->OutlinedRect(cross_x + screen_spread_x, cross_y + screen_spread_y, 1, 1,Color(255,255,255,255));
		}
	}

}

void GrenadePrediction(C_BaseEntity* localPlayer, C_BaseCombatWeapon* weapon)
{
	auto IsGrenade = [](ItemDefinitionIndex item)
	{
		if (item == ItemDefinitionIndex::WEAPON_FLASHBANG
			|| item == ItemDefinitionIndex::WEAPON_HEGRENADE
			|| item == ItemDefinitionIndex::WEAPON_SMOKEGRENADE
			|| item == ItemDefinitionIndex::WEAPON_MOLOTOV
			|| item == ItemDefinitionIndex::WEAPON_DECOY
			|| item == ItemDefinitionIndex::WEAPON_INCGRENADE)
			//|| item == ItemDefinitionIndex::weapon_tagrenade
			return true;
		else
			return false;
	};
	auto molotov_throw_detonate_time = g_pCvar->FindVar("molotov_throw_detonate_time");
	auto GetGrenadeDetonateTime = [molotov_throw_detonate_time](ItemDefinitionIndex item)
	{
		switch (item)
		{
		case ItemDefinitionIndex::WEAPON_FLASHBANG:
		case ItemDefinitionIndex::WEAPON_HEGRENADE:
			return 1.5f;
			break;
		case ItemDefinitionIndex::WEAPON_INCGRENADE:
		case ItemDefinitionIndex::WEAPON_MOLOTOV:
			return molotov_throw_detonate_time->GetFloat();
			break;
			//case ItemDefinitionIndex::weapon_tagrenade:
			//	return 5.f;
			//	break;
		}

		return 3.f;
	};

	auto DrawLine = [](Vector start, Vector end, int r, int g, int b, int a = 255)
	{
		Vector startw2s, endw2s;

		if (!Utils::WorldToScreen2(start, startw2s)
			|| !Utils::WorldToScreen2(end, endw2s))
			return;

		g_pSurface->DrawSetColor(r, g, b, a);
		g_pSurface->DrawLine(startw2s.x, startw2s.y, endw2s.x, endw2s.y);
	};

	auto PhysicsClipVelocity = [](const Vector& in, const Vector& normal, Vector& out, float overbounce)
	{
		int blocked = 0;
		float angle = normal[2];

		if (angle > 0)
			blocked |= 1; // floor

		if (!angle)
			blocked |= 2; // step

		float backoff = in.Dot(normal) * overbounce;

		for (int i = 0; i < 3; i++)
		{
			out[i] = in[i] - (normal[i] * backoff);

			if (out[i] > -0.1f && out[i] < 0.1f)
				out[i] = 0;
		}

		return blocked;
	};

	PlayerInfo_t playerInfo;
	auto itemIndex = weapon->GetItemDefinitionIndex();
	auto wpnData = weapon->GetCSWpnData();

	if (!wpnData
		|| !IsGrenade(itemIndex))
		return;

	Vector angThrow, forward;

	g_pEngine->GetViewAngles(angThrow);

	angThrow.x = g_Math.NormalizeYaw(angThrow.x);
	angThrow.x -= (90.f - abs(angThrow.x)) * 0.11111111f;

	g_Math.AngleVectors(angThrow, &forward);

	// WIP
}


void ESP::RenderWeaponName(C_BaseEntity* pEnt)
{
    auto weapon = pEnt->GetActiveWeapon();

    if (!weapon)
        return;

    auto strWeaponName = weapon->GetName();

    strWeaponName.erase(0, 7);

	if (g_Menu.Config.Aimbot)
		g_pSurface->DrawT(Box.left + (Box.right / 2), Box.top + Box.bottom, textcolor, font, true, strWeaponName.c_str());
	else if (g_Menu.Config.Weapon == 2)
	{
		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (offsetY * 11), textcolor, font, false, strWeaponName.c_str());
		offsetY += 1;
	}
}

void ESP::RenderHealth(C_BaseEntity* pEnt)
{
	if (g_Menu.Config.Aimbot)
	{
		g_pSurface->FilledRect(Box.left - 6, Box.top - 1, 4, Box.bottom + 2, Color(0, 0, 0, 240));
		int pixelValue = pEnt->GetHealth() * Box.bottom / 100;
		g_pSurface->FilledRect(Box.left - 5, Box.top + Box.bottom - pixelValue, 2, pixelValue, Color(0, 255, 0, 250));
	}

	if (g_Menu.Config.HealthVal == 1)
	{
		std::string Health = "HP " + std::to_string(pEnt->GetHealth());
		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (offsetY * 11), textcolor, font, false, Health.c_str());
		offsetY += 1;
	}
	//const auto overlays = pEnt->AnimOverlays();

	//if (pEnt->AnimOverlays()) {
	//	
	//	auto print_layer = [&](int x, int y, int idx,int times_called) {

	//		char buf[64];
	//		char buf1[64];
	//		char buf2[64];
	//		char buf3[64];

	//		sprintf_s(buf, "weight: %f",pEnt->AnimOverlays()[idx].m_flWeight);
	//		sprintf_s(buf1, "order: %d",pEnt->AnimOverlays()[idx].m_nOrder);
	//		sprintf_s(buf2, "cycle: %f",pEnt->AnimOverlays()[idx].m_flCycle);
	//		sprintf_s(buf3, "layer no. %d", idx);
	//		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (times_called * 69), textcolor, font, false, buf3);
	//		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (times_called * 69) + 15, textcolor, font, false, buf);
	//		//g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (times_called * 69) + 30, textcolor, font, false, buf1);
	//		g_pSurface->DrawT(Box.left + Box.right + 5, Box.top + (times_called * 69) + 30, textcolor, font, false, buf2);
	//	};

	//	auto cur_x = Box.right;
	//	auto cur_y = Box.top;

	//	for (auto j = 0; j < 13; j++) {
	//		print_layer(cur_x, cur_y, j, j);
	//		cur_y += 76;
	//	}
	//}
}

void ESP::RenderHitboxPoints(C_BaseEntity* pEnt)
{
	if (g::pLocalEntity->IsAlive())
	{
		for (int hitbox = 0; hitbox < 28; hitbox++)
		{
			Vector2D w2sHitbox;
			Utils::WorldToScreen(g::AimbotHitbox[pEnt->EntIndex()][hitbox], w2sHitbox);
			g_pSurface->OutlinedRect(w2sHitbox.x - 2, w2sHitbox.y - 2, 4, 4, color);
		}
	}
}

//void ESP::RenderSkeleton(C_BaseEntity* pEnt) // the best
//{
//	if (g_LagComp.PlayerRecord[pEnt->EntIndex()].size() == 0)
//		return;
//	int size = 0;
//
//	if (g_Menu.Config.Aimbot)
//		size++;
//	if (g_Menu.Config.Skeleton[1])
//		size++;
//
//	for (int mode = 0; mode < size; mode++)
//	{
//		Vector Hitbox[19];
//		Vector2D Hitboxw2s[19];
//
//		int Record = 0;
//
//		if (mode == 0 && g_Menu.Config.Aimbot)
//		{
//			Record = g_LagComp.PlayerRecord[pEnt->EntIndex()].size() - 1;
//
//			if (g_LagComp.ShotTick[pEnt->EntIndex()] != -1)
//				Record = g_LagComp.ShotTick[pEnt->EntIndex()];
//		}
//
//		for (int hitbox = 0; hitbox < 19; hitbox++)
//		{
//			Hitbox[hitbox] = pEnt->GetHitboxPosition(hitbox, g_LagComp.PlayerRecord[pEnt->EntIndex()].at(Record).Matrix);
//			Utils::WorldToScreen(Hitbox[hitbox], Hitboxw2s[hitbox]);
//		}
//
//		//spine
//		g_pSurface->Line(Hitboxw2s[HITBOX_HEAD].x, Hitboxw2s[HITBOX_HEAD].y, Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_UPPER_CHEST].x, Hitboxw2s[HITBOX_UPPER_CHEST].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_UPPER_CHEST].x, Hitboxw2s[HITBOX_UPPER_CHEST].y, Hitboxw2s[HITBOX_LOWER_CHEST].x, Hitboxw2s[HITBOX_LOWER_CHEST].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_LOWER_CHEST].x, Hitboxw2s[HITBOX_LOWER_CHEST].y, Hitboxw2s[HITBOX_THORAX].x, Hitboxw2s[HITBOX_THORAX].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_THORAX].x, Hitboxw2s[HITBOX_THORAX].y, Hitboxw2s[HITBOX_BELLY].x, Hitboxw2s[HITBOX_BELLY].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_BELLY].x, Hitboxw2s[HITBOX_BELLY].y, Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, skelecolor);
//
//		//right leg
//		g_pSurface->Line(Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, Hitboxw2s[HITBOX_RIGHT_THIGH].x, Hitboxw2s[HITBOX_RIGHT_THIGH].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_THIGH].x, Hitboxw2s[HITBOX_RIGHT_THIGH].y, Hitboxw2s[HITBOX_RIGHT_CALF].x, Hitboxw2s[HITBOX_RIGHT_CALF].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_CALF].x, Hitboxw2s[HITBOX_RIGHT_CALF].y, Hitboxw2s[HITBOX_RIGHT_FOOT].x, Hitboxw2s[HITBOX_RIGHT_FOOT].y, skelecolor);
//
//		//right arm
//		g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].x, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].x, Hitboxw2s[HITBOX_RIGHT_UPPER_ARM].y, Hitboxw2s[HITBOX_RIGHT_FOREARM].x, Hitboxw2s[HITBOX_RIGHT_FOREARM].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_RIGHT_FOREARM].x, Hitboxw2s[HITBOX_RIGHT_FOREARM].y, Hitboxw2s[HITBOX_RIGHT_HAND].x, Hitboxw2s[HITBOX_RIGHT_HAND].y, skelecolor);
//
//		//left leg
//		g_pSurface->Line(Hitboxw2s[HITBOX_PELVIS].x, Hitboxw2s[HITBOX_PELVIS].y, Hitboxw2s[HITBOX_LEFT_THIGH].x, Hitboxw2s[HITBOX_LEFT_THIGH].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_THIGH].x, Hitboxw2s[HITBOX_LEFT_THIGH].y, Hitboxw2s[HITBOX_LEFT_CALF].x, Hitboxw2s[HITBOX_LEFT_CALF].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_CALF].x, Hitboxw2s[HITBOX_LEFT_CALF].y, Hitboxw2s[HITBOX_LEFT_FOOT].x, Hitboxw2s[HITBOX_LEFT_FOOT].y, skelecolor);
//
//		//left arm
//		g_pSurface->Line(Hitboxw2s[HITBOX_NECK].x, Hitboxw2s[HITBOX_NECK].y, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].x, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_UPPER_ARM].x, Hitboxw2s[HITBOX_LEFT_UPPER_ARM].y, Hitboxw2s[HITBOX_LEFT_FOREARM].x, Hitboxw2s[HITBOX_LEFT_FOREARM].y, skelecolor);
//		g_pSurface->Line(Hitboxw2s[HITBOX_LEFT_FOREARM].x, Hitboxw2s[HITBOX_LEFT_FOREARM].y, Hitboxw2s[HITBOX_LEFT_HAND].x, Hitboxw2s[HITBOX_LEFT_HAND].y, skelecolor);
//
//		
//	}
//}

void ESP::BoundBox(C_BaseEntity* pEnt)
{
	Box.bottom = 0;
	Box.top = 0;
	Box.left = 0;
	Box.right = 0;

	Vector2D w2sBottom, w2sTop;

	Utils::WorldToScreen(pEnt->GetOrigin() - Vector(0, 0, 8), w2sBottom); 
	Utils::WorldToScreen(pEnt->GetHitboxPos(0) + Vector(0, 0, 10), w2sTop);
	
	int Middle = w2sBottom.y - w2sTop.y;
	int Width = Middle / 3.f;

	Box.bottom = Middle;
	Box.top = w2sTop.y;
	Box.left = w2sBottom.x - Width;
	Box.right = Width * 2;
}
void ESP::RenderHitpoints(int index)
{
	
}
void ESP::Render()
{
    if (!g_pEngine->IsInGame() || !g_Menu.Config.Aimbot)
        return;
	if (g_Menu.Config.Aimbot && g::pLocalEntity->IsAlive())
	{
		RenderInfo();
		//draw_spread();

	}
	color = Color(g_Menu.Config.ChamsColor.red, g_Menu.Config.ChamsColor.green, g_Menu.Config.ChamsColor.blue, 255);
	textcolor = Color(g_Menu.Config.ChamsColor.red, g_Menu.Config.ChamsColor.green, g_Menu.Config.ChamsColor.blue, 255);
	skelecolor = Color(g_Menu.Config.ChamsColor.red, g_Menu.Config.ChamsColor.green, g_Menu.Config.ChamsColor.blue, 255);

	(g_Menu.Config.Font == 0) ? font = g::CourierNew : font = g::Tahoma;

	for (int i = 1; i < g_pEngine->GetMaxClients(); ++i)
    {
        C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity(i);

        if (!pPlayerEntity
            || !pPlayerEntity->IsAlive()
            || pPlayerEntity->IsDormant()
            || pPlayerEntity == g::pLocalEntity
			|| pPlayerEntity->GetTeam() == g::pLocalEntity->GetTeam())
            continue;

		offsetY = 0;

		BoundBox(pPlayerEntity);

		//if (Box.bottom == 0)
		//	continue;

	//	if (g_Menu.Config.Aimbot)
		//	RenderSkeleton(pPlayerEntity);

       // if (g_Menu.Config.Box)
         //   RenderBox();

	//	if (g_Menu.Config.HitboxPoints)
	//		RenderHitboxPoints(pPlayerEntity);
		RenderHitpoints(i);
        if (g_Menu.Config.Aimbot)
            RenderName(pPlayerEntity, i);

		
        if (g_Menu.Config.Aimbot)
            RenderWeaponName(pPlayerEntity);

		if (g_Menu.Config.Aimbot)
			RenderHealth(pPlayerEntity);


    }
/* i was working on exptrapolation 4ever ago i did in here because i just wanted to see it visualy
	if (g::pLocalEntity->IsAlive())
	{
		float angDelta;
		float angExtrap;
		Vector originExtrap;
		Vector2D angCalc, w2sBottom, w2sTop;

		static float oldSimtime;
		static float storedSimtime;

		static std::deque<TestPos> oldPos;

		if (storedSimtime != g::pLocalEntity->GetSimulationTime())
		{
			oldPos.push_back(TestPos{ g::pLocalEntity->GetOrigin(), g::pLocalEntity->GetSimulationTime() });
			oldSimtime = storedSimtime;
			storedSimtime = g::pLocalEntity->GetSimulationTime();
		}

		float simDelta = storedSimtime - oldSimtime;

		originExtrap = g::pLocalEntity->GetOrigin();

		Utils::WorldToScreen(g::pLocalEntity->GetOrigin() + (g::pLocalEntity->GetVelocity() * simDelta), w2sTop);
		g_pSurface->FilledRect(w2sTop.x - 3, w2sTop.y - 3, 6, 6, Color(255, 0, 0, 255));

		if (oldPos.size() > 3)
			oldPos.erase(oldPos.begin());

		if (oldPos.size() == 3)
		{
			angDelta = g_Math.CalcAngle(oldPos.at(1).Pos, oldPos.at(2).Pos).y - g_Math.CalcAngle(oldPos.at(0).Pos, oldPos.at(1).Pos).y;
			angExtrap = g_Math.CalcAngle(Vector(0,0,0), g::pLocalEntity->GetVelocity()).y + angDelta;
			angExtrap += 90;

			float tempExtrap = (g::pLocalEntity->GetVelocity().Length2D() * simDelta);

			angCalc = { (tempExtrap * sin(g_Math.GRD_TO_BOG(angExtrap))), (tempExtrap * cos(g_Math.GRD_TO_BOG(angExtrap))) };

			originExtrap = { originExtrap.x + angCalc.x, originExtrap.y - angCalc.y, g::pLocalEntity->GetVelocity().z * simDelta };			
		}

		if ((g::pLocalEntity->GetFlags() & FL_ONGROUND))
		{
			originExtrap.z = g::pLocalEntity->GetOrigin().x;
		}
		else
		{

		}

		Utils::WorldToScreen(originExtrap, w2sBottom);
		g_pSurface->FilledRect(w2sBottom.x - 3, w2sBottom.y - 3, 6, 6, Color(0, 255, 0, 255));
	}
	*/
}