#pragma once
#include "GlobalVars.h"

struct TestPos
{
	Vector Pos;
	float SimulationTime;
};

class ESP
{
public:
    void Render();
private:
	struct
	{
		int left, top, right, bottom;
	}Box;

	Color color;
	Color textcolor;
	Color skelecolor;
	DWORD font;
	int offsetY;

	void BoundBox(C_BaseEntity* pEnt);
	void RenderHitpoints(int index);
	void draw_spread();
	void RenderBox();
	void RenderWeaponName(C_BaseEntity* pEnt);
    void RenderName(C_BaseEntity* pEnt, int iterator);
	void RenderInfo();
	void RenderHealth(C_BaseEntity* pEnt);
	void RenderHitboxPoints(C_BaseEntity* pEnt);
	void RenderSkeleton(C_BaseEntity* pEnt);
	

};
extern ESP g_ESP;