#pragma once

#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include <deque>


struct LegitPlayerRecords
{
	Vector HitBoxes[19];
	float SimTime;
};

class Legitbot
{
public:
	std::deque<LegitPlayerRecords> PlayerRecord[65];
	void OnCreateMove();
private:
};
extern Legitbot g_Legitbot;