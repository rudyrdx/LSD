#include "Events.h"
enum ConsoleColor
{
	BLACK = 0,
	DARKBLUE = FOREGROUND_BLUE,
	DARKGREEN = FOREGROUND_GREEN,
	DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
	DARKRED = FOREGROUND_RED,
	DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
	DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
	DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	GRAY = FOREGROUND_INTENSITY,
	BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
	GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
	CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
	RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
	MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
	YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};
eventManager eventmanager;
c_damageesp damageesp;
std::vector<BulletImpact_t> Impacts;
float GetCurtimes()
{

	if (!g::pLocalEntity)
		return -1;

	return static_cast<float>(g::pLocalEntity->GetTickBase()) * g_pGlobalVars->intervalPerTick;
}


void DrawFF(int X, int Y, unsigned int Font, bool center_width, bool center_height, Color Color, std::string Input)
{
	/* char -> wchar */
	size_t size = Input.size() + 1;
	auto wide_buffer = std::make_unique<wchar_t[]>(size);
	mbstowcs_s(0, wide_buffer.get(), size, Input.c_str(), size - 1);

	/* check center */
	int width = 0, height = 0;
	g_pSurface->LogGetTextSize(Font, wide_buffer.get(), width, height);
	if (!center_width)
		width = 0;
	if (!center_height)
		height = 0;

	/* call and draw*/
	g_pSurface->LogDrawSetTextColor(Color);
	g_pSurface->LogDrawSetTextFont(Font);
	g_pSurface->DrawSetTextPos(X - (width * .5), Y - (height * .5));
	g_pSurface->LogDrawPrintText(wide_buffer.get(), wcslen(wide_buffer.get()));
}
Vector2D GetTextSize(unsigned int Font, std::string Input)
{
	/* char -> wchar */
	size_t size = Input.size() + 1;
	auto wide_buffer = std::make_unique<wchar_t[]>(size);
	mbstowcs_s(0, wide_buffer.get(), size, Input.c_str(), size - 1);

	int width, height;
	g_pSurface->LogGetTextSize(Font, wide_buffer.get(), width, height);

	return Vector2D(width, height);
}


namespace FEATURES
{
	namespace MISC
	{
		InGameLogger in_game_logger;
		void ColorLine::Draw(int x, int y, unsigned int font)
		{
			for (int i = 0; i < texts.size(); i++)
			{
				//DrawFF(x, y, font, false, false, colors[i], texts[i]);
				//fixed that shit with nodraw at all
				g_pSurface->DrawT(x, y, colors[i], g::Logs, false, texts[i].c_str());
				x += GetTextSize(font, texts[i]).x;
			}
		}

		void InGameLogger::Do()
		{
			if (log_queue.size() > max_lines_at_once)
				log_queue.erase(log_queue.begin() + max_lines_at_once, log_queue.end());

			for (int i = 0; i < log_queue.size(); i++)
			{
				auto log = log_queue[i];
				float time_delta = fabs(GetCurtimes() - log.time);

				int height = ideal_height + (16 * i);

				/// erase dead logs
				if (time_delta > text_time)
				{
					log_queue.erase(log_queue.begin() + i);
					break;
				}
				if (time_delta > text_time - slide_out_speed)
					height = height + (((slide_out_speed - (text_time - time_delta)) / slide_out_speed) * slide_out_distance);

				/// fade out
				if (time_delta > text_time - text_fade_out_time)
					log.color_line.ChangeAlpha(255 - (((time_delta - (text_time - text_fade_out_time)) / text_fade_out_time) * 255.f));
				/// fade in
				if (time_delta < text_fade_in_time)
					log.color_line.ChangeAlpha((time_delta / text_fade_in_time) * 255.f);

				int width = ideal_width;

				/// slide from left
				if (time_delta < text_fade_in_time)
					width = (time_delta / text_fade_in_time) * static_cast<float>(slide_in_distance) + (ideal_width - slide_in_distance);
				/// slider from right
				if (time_delta > text_time - text_fade_out_time)
					width = ideal_width + (((time_delta - (text_time - text_fade_out_time)) / text_fade_out_time) * static_cast<float>(slide_out_distance));


				log.color_line.Draw(width, height, g::Logs);


			}
		}
	}
}


void Console_SetColor(ConsoleColor color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}
void Console_Log(const char* text, ...)
{
	if (!text)
		return;

	va_list va_args;
	char buffer[2048] = { 0 };
	va_start(va_args, text);
	_vsnprintf(buffer, sizeof(buffer), text, va_args);
	va_end(va_args);
	Console_SetColor(ConsoleColor::MAGENTA);
	std::cout << ("LSD");
	Console_SetColor(ConsoleColor::DARKGRAY);
	std::cout << (": ") << buffer << std::endl;
}
float dot_product(const Vector& a, const Vector& b) {
	return (a.x * b.x
		+ a.y * b.y
		+ a.z * b.z);
}
static void CapsuleOverlay(C_BaseEntity* pPlayer, Color col, float duration)
{
	if (!pPlayer)
		return;

	studiohdr_t* pStudioModel = g_pModelInfo->GetStudiomodel((model_t*)pPlayer->GetModel());
	if (!pStudioModel)
		return;

	static matrix3x4_t pBoneToWorldOut[128];
	if (!pPlayer->SetupBones(pBoneToWorldOut, 128, 0x00000100, pPlayer->GetSimulationTime()))
		return;

	mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);
	if (!pHitboxSet)
		return;

	auto VectorTransform2 = [](const Vector in1, matrix3x4_t in2, Vector& out)
	{

		out[0] = dot_product(in1, Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
		out[1] = dot_product(in1, Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
		out[2] = dot_product(in1, Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
	};

	for (int i = 0; i < pHitboxSet->numhitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pHitboxSet->GetHitbox(i);
		if (!pHitbox)
			continue;

		Vector vMin, vMax;
		VectorTransform2(pHitbox->min, pBoneToWorldOut[pHitbox->bone], vMin); //nullptr???
		VectorTransform2(pHitbox->max, pBoneToWorldOut[pHitbox->bone], vMax);

		if (pHitbox->radius > -1)
		{
			g_pIVDebugOverlay->AddCapsuleOverlay(vMin, vMax, pHitbox->radius, col.red, col.green, col.blue, col.alpha, duration);
		}
	}
}

void eventManager::onEvent(IGameEvent* event)
{
	static const char* team[]{ "",""," (Terrorist)"," (Counter-Terrorist)" };
	static const char* hasdefusekit[]{ "without defuse kit.","with defuse kit." };
	static const char* hasbomb[]{ "without the bomb.","with the bomb." };


	//if (strstr(event->GetName(), "round_start") || strstr(event->GetName(), "round_start") || strstr(event->GetName(), "round_prestart") || strstr(event->GetName(), "client_disconnect") || strstr(event->GetName(), "cs_game_disconnected"))
	//	fix_crash();


	if (g_Menu.Config.damageesp)
	{
		if (!strcmp(event->GetName(), "bullet_impact"))
			damageesp.on_bullet_impact(event);
		if (!strcmp(event->GetName(), "player_hurt"))
			damageesp.on_player_hurt(event);
	}

	if (!strcmp(event->GetName(), "bullet_impact"))
	{
		if (!event)
			return;

		if (!(g_pEngine->IsConnected() || g_pEngine->IsInGame()))
			return;


		auto local = g::pLocalEntity;

		if (!local || local->GetHealth() <= 0)
			return;

		float x = event->GetFloat(("x"));
		float y = event->GetFloat(("y"));
		float z = event->GetFloat(("z"));

		const auto target = reinterpret_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(("userid")))));

		if (!target || target != local)
			return;

		BulletImpact_t impact(target, Vector(x, y, z), g_pGlobalVars->curtime, Color(255, 0, 0, 255));

		Impacts.push_back(impact);
	}
	if (!strcmp(event->GetName(), "player_hurt"))
	{
		if (!g::pLocalEntity || !g::pLocalEntity->IsAlive())
			return;

		auto hurt = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));
		if (!hurt || g_pEngine->GetPlayerForUserID(event->GetInt("attacker")) != g_pEngine->GetLocalPlayer() || hurt->GetTeam() == g::pLocalEntity->GetTeam())
			return;
		g::missed_shots.at(hurt->EntIndex());
	}
	if (!strcmp(event->GetName(), "player_hurt"))
	{
		auto _attacker = event->GetInt("attacker");
		if (!_attacker) return;
		if (_attacker == g::pLocalEntity->EntIndex())
		{
			int index = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));
			g::Hit[index] = true;
		}
		if (g_Menu.Config.capsuleoverlayh)
		{
			C_BaseEntity* hittedplayer = (C_BaseEntity*)g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

			if (hittedplayer && hittedplayer->EntIndex() > 0 && hittedplayer->EntIndex() < 64)
			{
				if (g::pLocalEntity && hittedplayer != g::pLocalEntity)
					CapsuleOverlay(hittedplayer, Color(1 * 255, 1 * 255, 1 * 255, 1 * 255), 1.0f);
			}
		}
		
		if (g_pEngine->GetPlayerForUserID(_attacker) == g_pEngine->GetLocalPlayer())
		{

			g::shothitb++;
			if(g_Menu.Config.hitsound)
			g_pSurface->play_sound("buttons\\arena_switch_press_02.wav");

		}
	}
	// reset past shot threshold
	if (g::shotfiredb >= 4 || g::shotfiredb == 0 || g::shotshagdiab < 0) {
		g::shotfiredb = 0;
		g::shothitb = 0;
		g::shotshagdiab = 0;
	}

	// calculate shots missed
	if (g::shotfiredb > 0)
		g::shotshagdiab = g::shotfiredb - g::shothitb;
	/*if (g_Menu.Config.logscreen)
	{
		if (!strcmp(event->GetName(), "player_hurt"))
		{
			auto userid = event->GetInt("userid");
			auto _attacker = event->GetInt("attacker");

			if (!userid || !_attacker)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);
			auto attacker_id = g_pEngine->GetPlayerForUserID(_attacker);

			PlayerInfo_t userid_info, attacker_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			if (!g_pEngine->GetPlayerInfo(attacker_id, &attacker_info))
				return;

			std::stringstream string;
			string << "[" << attacker_info.szName << "]" << " did " << event->GetInt("dmg_health") << " damage to " << userid_info.szName << ".";
			Game_Msg(string.str().c_str());
		}
	}*/

	/*if (g_Menu.Config.logscreen)
	{
		if (!strcmp(event->GetName(), "player_death"))
		{
			auto userid = event->GetInt("userid");
			auto attacker = event->GetInt("attacker");

			if (!userid || !attacker)
				return;


			auto userid_id = g_pEngine->GetPlayerForUserID(userid);
			auto attacker_id = g_pEngine->GetPlayerForUserID(attacker);

			PlayerInfo_t userid_info, attacker_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			if (!g_pEngine->GetPlayerInfo(attacker_id, &attacker_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << team[event->GetInt("team")] << " killed by " << "[" << attacker_info.szName << "]" << ".";
			Game_Msg(string.str().c_str());
		}
	}*/

	/*if (Config.visuals.eventlogs[2])
	{
		if (!strcmp(event->GetName(), "player_connect"))
		{
			int PlayerID = g_pEngine->GetPlayerForUserID(event->GetInt(("userid")));
			PlayerInfo_t PlayerInfo;
			if (g_pEngine->GetPlayerInfo(PlayerID, &PlayerInfo)) {
				std::stringstream string;
				string << "[" << PlayerInfo.szName << "]" << " just connected" << ".";
				Game_Msg(string.str().c_str());
			}
		}
	}*/

	/*if (g_Menu.Config.logscreen)
	{
		if (!strcmp(event->GetName(), "item_purchase"))
		{

			auto userid = event->GetInt("userid");

			if (!userid)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);

			PlayerInfo_t userid_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << team[event->GetInt("team")] << " purchased a(n) " << event->GetString("weapon") << ".";
			Game_Msg(string.str().c_str());

		}
	}*/

	/*if (Config.visuals.eventlogs[4])
	{
		if (!strcmp(event->GetName(), "bomb_pickup"))
		{
			auto userid = event->GetInt("userid");

			if (!userid)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);

			PlayerInfo_t userid_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << " has pickup bomb" << ".";
			Game_Msg(string.str().c_str());

		}
	}*/

	/*if (Config.visuals.eventlogs[5])
	{
		if (!strcmp(event->GetName(), "bomb_beginplant"))
		{

			auto userid = event->GetInt("userid");

			if (!userid)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);

			PlayerInfo_t userid_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << " has began planting the bomb at site " << event->GetInt("site") << ".";
			Game_Msg(string.str().c_str());

		}
	}*/

	/*if (Config.visuals.eventlogs[6])
	{
		if (!strcmp(event->GetName(), "bomb_begindefuse"))
		{
			auto userid = event->GetInt("userid");

			if (!userid)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);

			PlayerInfo_t userid_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << " has began defusing the bomb " << hasdefusekit[event->GetBool("haskit")];
			Game_Msg(string.str().c_str());

		}
	}*/

	/*if (g_Menu.Config.logscreen)
	{
		if (!strcmp(event->GetName(), "enter_bombzone"))
		{
			auto userid = event->GetInt("userid");

			if (!userid)
				return;

			auto userid_id = g_pEngine->GetPlayerForUserID(userid);

			PlayerInfo_t userid_info;
			if (!g_pEngine->GetPlayerInfo(userid_id, &userid_info))
				return;

			std::stringstream string;
			string << "[" << userid_info.szName << "]" << " entered the bomb zone " << hasbomb[event->GetBool("hasbomb")];
			Game_Msg(string.str().c_str());

		}
	}*/
}
bool TransformScreen(const Vector& in, Vector& out)
{
	static ptrdiff_t ptrViewMatrix;
	if (!ptrViewMatrix)
	{//                                                          
		ptrViewMatrix = static_cast<ptrdiff_t>(Utils::FindSignature("client_panorama.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));
		ptrViewMatrix += 0x3;
		ptrViewMatrix = *reinterpret_cast<uintptr_t*>(ptrViewMatrix);
		ptrViewMatrix += 176;
	}
	const VMatrix& worldToScreen = g_pEngine->WorldToScreenMatrix(); // matrix



	int ScrW, ScrH;

	g_pEngine->GetScreenSize(ScrW, ScrH);

	float w = worldToScreen[3][0] * in[0] + worldToScreen[3][1] * in[1] + worldToScreen[3][2] * in[2] + worldToScreen[3][3];
	out.z = 0; // 0 poniewaz z nie jest nam potrzebne | uzywamy tylko wysokosci i szerokosci (x,y)
	if (w > 0.01)
	{
		float inverseWidth = 1 / w; // inverse na 1 pozycje ekranu
		out.x = (ScrW / 2) + (0.5 * ((worldToScreen[0][0] * in[0] + worldToScreen[0][1] * in[1] + worldToScreen[0][2] * in[2] + worldToScreen[0][3]) * inverseWidth) * ScrW + 0.5);
		out.y = (ScrH / 2) - (0.5 * ((worldToScreen[1][0] * in[0] + worldToScreen[1][1] * in[1] + worldToScreen[1][2] * in[2] + worldToScreen[1][3]) * inverseWidth) * ScrH + 0.5);
		return true;
	}
	return false;
}
void c_damageesp::draw_damage()
{
	if (!g_Menu.Config.damageesp)
		return;
	auto local = g::pLocalEntity;

	if (!local || local->GetHealth() <= 0)
		return;

	float curTime = g_pGlobalVars->curtime;

	for (int i = 0; i < MAX_FLOATING_TEXTS; i++)
	{
		FloatingText* txt = &floatingTexts[i % MAX_FLOATING_TEXTS];

		if (!txt->valid)
			continue;

		float endTime = txt->startTime + 1.1f;

		if (endTime < curTime)
		{
			txt->valid = false;
			continue;
		}

		Vector origin_screen;

		if (!TransformScreen(txt->hitPosition, origin_screen))
			return;

		float t = 1.0f - (endTime - curTime) / (endTime - txt->startTime);

		origin_screen.y -= t * (35.0f);
		origin_screen.x -= (float)txt->randomIdx * t * 3.0f;

		char msg[12];
		sprintf_s(msg, 12, ("-%dHP"), txt->damage);



		Color damage_color;
		if (txt->damage >= 90 && txt->damage <= 800)
			damage_color = Color(255, 0, 0, 255);
		else if (txt->damage >= 50 && txt->damage <= 89)
			damage_color = Color(220, 0, 220, 255);
		else if (txt->damage >= 0 && txt->damage <= 49)
			damage_color = Color(0, 255, 0, 255);


		//Text(std::roundf(origin_screen.x), std::roundf(origin_screen.y - 18), msg, g::CourierNew, damage_color);

	}
}

void c_damageesp::on_bullet_impact(IGameEvent* event)
{
	auto local = g::pLocalEntity;

	if (!local || local->GetHealth() <= 0)
		return;

	float x = event->GetFloat(("x"));
	float y = event->GetFloat(("y"));
	float z = event->GetFloat(("z"));

	const auto target = reinterpret_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(("userid")))));

	if (!target || target != local)
		return;

	c_impactpos = Vector(x, y, z);

}

void c_damageesp::on_player_hurt(IGameEvent* event)
{
	auto local = g::pLocalEntity;

	if (!local || local->GetHealth() <= 0)
		return;

	float curTime = g_pGlobalVars->curtime;

	int dmg_health = event->GetInt(("dmg_health"));
	int hitgroup = event->GetInt(("hitgroup"));

	const auto entity = reinterpret_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(("userid")))));
	const auto attacker = reinterpret_cast<C_BaseEntity*>(g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt(("attacker")))));

	if (!entity || attacker != local)
		return;

	FloatingText txt;
	txt.startTime = curTime;
	txt.hitgroup = hitgroup;
	txt.hitPosition = c_impactpos;
	txt.damage = dmg_health;
	txt.randomIdx = g_Math.RandomFloat(-5.f, 5.f);
	txt.valid = true;

	floatingTexts[floatingTextsIdx++ % MAX_FLOATING_TEXTS] = txt;
}