#include "BulletTracer.h"

/* declarations */
bullettracer::player_hurt_event player_hurt_listener;
bullettracer::bullet_impact_event bullet_impact_listener;

/* bulletracer */
bullettracer g_bullettracer;

/* trace logs */
std::vector<trace_info> trace_logs;

/* color used for the tracers */
Color color;

/* bullet tracer draw */
void bullettracer::draw_beam(Vector src, Vector end, Color color)
{
	BeamInfo_t info;
	info.m_nType = TE_BEAMPOINTS;
	info.m_pszModelName = "sprites/purplelaser1.vmt";
	//info.m_pszHaloName = -1;
	info.m_nHaloIndex = -1;
	info.m_flHaloScale = 0.f;
	info.m_flLife = 5;
	info.m_flWidth = 5;
	info.m_flEndWidth = 7;
	info.m_flFadeLength = 1;
	info.m_flAmplitude = 0.f;
	info.m_flBrightness = color.alpha;
	info.m_flSpeed = 0.1f;
	info.m_nStartFrame = 0;
	info.m_flFrameRate = 0;
	info.m_flRed = color.red;
	info.m_flGreen = color.green;
	info.m_flBlue = color.blue;
	info.m_nSegments = -1;
	info.m_bRenderable = true;
	info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE;
	info.m_vecStart = src; // Vector(0, 0, 5);
	info.m_vecEnd = end;

	Beam_t* beam = g_pViewRenderBeams->CreateBeamPoints(info);

	if (beam)
		g_pViewRenderBeams->DrawBeam(beam);
}

/* listeners */
void bullettracer::listener()
{
	g_pEventManager->AddListener(&bullet_impact_listener, "bullet_impact", false);

	g_pEventManager->AddListener(&player_hurt_listener, "player_hurt", false);
}

/* bullet impact */
void bullettracer::bullet_impact_event::FireGameEvent(IGameEvent * p_event)
{
	/* check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* return if not event */
	if (!p_event)
		return;

	/* return if localplayer is null */
	if (g::pLocalEntity == nullptr)
		return;

	/* check if we have the bullet tracers enabled */
	if (!g_Menu.Config.Aimbot)
		return;

	/* bullet impact */
	if (!strcmp(p_event->GetName(), "bullet_impact"))
	{
		Vector position(p_event->GetFloat("x"), p_event->GetFloat("y"), p_event->GetFloat("z"));
		if (position == Vector(0, 0, 0))
			return;
		static float r;r += 0.001f; if (r > 1) r = 0;

		/* get the shooter */
		auto shooter = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("userid")));

		if (shooter == nullptr)
			return;

		if (shooter)
			trace_logs.push_back(trace_info(shooter->GetEyePosition(), position, g_pGlobalVars->realtime, p_event->GetInt("userid")));

		if (shooter == g::pLocalEntity)
			color = color.FromHSB(r, 1, 1);
		else if (shooter->GetTeam() != g::pLocalEntity->GetTeam())
			color = color.FromHSB(r, 1, 1);
		else
			color = Color(0, 0, 0, 0);
	}
}

/* player hurt */
void bullettracer::player_hurt_event::FireGameEvent(IGameEvent * p_event)
{
	/* return if not event */
	if (!p_event)
		return;

	/* local player */
	auto localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	/* get the attacker */
	auto attacker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("attacker")));

	/* check if we are attacker */
	if (attacker == localplayer)
	{
		static float r; r += 0.9f; if (r > 1) r = 0;
		color = color.FromHSB(r, 1, 1);
	}
}

/* draw */
void bullettracer::draw()
{
	/* check if we have the bullet tracers enabled */
	if (!g_Menu.Config.Aimbot)
		return;

	/* if we are connected */
	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;

	for (unsigned int i = 0; i < trace_logs.size(); i++)
	{
		/* draw the beam */
	  //g_bullettracer.draw_beam(trace_logs[i].start, trace_logs[i].position, color);
		g_bullettracer.draw_beam(trace_logs[i].start, trace_logs[i].position, color);;

		trace_logs.erase(trace_logs.begin() + i);
	}
	
}
