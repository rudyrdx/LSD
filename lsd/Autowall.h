#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include "IEngineTrace.h"

struct FireBulletData
{
	FireBulletData(const Vector &eyePos, C_BaseEntity* entity) : src(eyePos) , filter(entity)
	{
	}

	Vector          src;
	C_Trace_Antario         enter_trace;
	Vector          direction;
	C_TraceFilter   filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
};
template<typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

public:
	static T& Get()
	{
		static T inst{};
		return inst;
	}
};




class Autowall : public Singleton< Autowall >
{
public:
	struct Autowall_Return_Info
	{
		int damage;
		int hitgroup;
		int penetration_count;
		bool did_penetrate_wall;
		float thickness;
		Vector end;
		C_BaseEntity* hit_entity;
	};
	struct Autowall_Info
	{
		Vector start;
		Vector end;
		Vector current_position;
		Vector direction;

		ITraceFilter* filter;
		C_Trace_Antario enter_trace;

		float thickness;
		float current_damage;
		int penetration_count;
	};
	void trace_line(Vector& start, Vector& end, unsigned int mask, C_BaseEntity* ignore, C_Trace_Antario* trace);
	bool trace_to_exit(C_Trace_Antario* enter_trace, Vector start, Vector dir, C_Trace_Antario* exit_trace);
	bool handle_bullet_penetration(WeaponInfo_t* info, Autowall_Info& data);
	
	void ScaleDamage(C_BaseEntity* entity, WeaponInfo_t* weapon_info, int hitgroup, float& current_damage);
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);
	bool VectortoVectorVisibles(Vector src, Vector point, C_BaseEntity* pEnt);
	float Damage(const Vector& point);
	Autowall_Return_Info CalculateDamage(Vector start, Vector end, C_BaseEntity* from_entity = nullptr, C_BaseEntity* to_entity = nullptr, int specific_hitgroup = -1);

	inline bool IsAutowalling() const
	{
		return is_autowalling;
	}
private:
	bool is_autowalling = false;
};
extern Autowall g_Autowall;

extern void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, C_Trace_Antario* tr);


struct returninfo {
	int damage = -1;
	int hitgroup = -1;
	int walls = 4;
	bool did_penetrate_wall = false;
	float thickness = 1.f;

	C_BaseEntity* ent = nullptr;
	Vector end = Vector();
};

struct fbdata {
	Vector start = Vector();
	Vector end = Vector();
	Vector pos = Vector();
	Vector dir = Vector();

	ITraceFilter* filter = nullptr;
	C_Trace_Antario trace;

	float thickness = 1.f;
	float damage = 1.f;
	int walls = 4;
};

class c_autowall {
public:
	bool can_hit_float_point(const Vector& point, const Vector& source, C_BaseEntity* pEnt);
	float get_estimated_point_damage(Vector point);
	bool trace_to_exit_short(Vector& point, Vector& dir, const float step_size, float max_distance);
	float get_thickness(Vector& start, Vector& end);
	float get_point_damage(Vector point, C_BaseEntity* e);
	returninfo autowall(Vector start, Vector end, C_BaseEntity* from_ent = nullptr, C_BaseEntity* to_ent = nullptr, int hitgroup = -1);
private:
	void clip_trace_to_player(Vector& start, Vector& end, C_BaseEntity* ent, unsigned int mask, ITraceFilter* filter, C_Trace_Antario* trace);
	void scale_damage(C_BaseEntity* ent, WeaponInfo_t* inf, int& hitgroup, float& damage);
	bool handle_bullet_penetration(WeaponInfo_t* inf, fbdata& bullet);
	bool trace_to_exit(C_Trace_Antario* enter_trace, Vector& start, Vector& dir, C_Trace_Antario* exit_trace);
	bool is_breakable(C_BaseEntity* e);
};

extern c_autowall* autowall;
