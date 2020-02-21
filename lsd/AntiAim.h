#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"

class AntiAim
{
public:

	void doubletap(CUserCmd* cmd);

	void FreeStanding_jitter();

	void FreeStanding();

	void OnCreateMove();
	void sincos(float radians, float* sine, float* cosine);
	void predict_lby_update(float sampletime, CUserCmd * ucmd, bool & sendpacket);
	float corrected_tickbase();
	bool m_lby_update_pending = false;
	void anglevectors(Vector& angles, Vector* forward, Vector* right, Vector* up);
	float freestand();
	float freestand_jitter();
	float m_next_update;
	int m_corrected_curtime = 0;
	int m_corrected_tickbase = 0;
	void desync_flip();
	float halogendirection(float yaw);
	void jitter();
	Vector mainangle;
	void calculate_ideal_yaw(CUserCmd* cmd);
	CUserCmd* m_last_ucmd = nullptr;
};
extern AntiAim g_AntiAim;

