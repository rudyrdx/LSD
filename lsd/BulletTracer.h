
#include <vector>
#include "IClientEntityList.h"
#include "IClientEntity.h"
#include "CGlobalVarsBase.h"
#include "IViewRenderBeams.h"
#include "Singleton.h"
#include "Menu.h"
#include "GlobalVars.h"
#include "CEntity.h"
#include "IGameEvent.h"


/* definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\

/* trace info */
class trace_info
{
public:
	trace_info(Vector starts, Vector positions, float times, int userids)
	{
		this->start = starts;
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	Vector start;
	float time;
	int userid;
};

/* functions */
class bullettracer
{
public:
	void draw();
	void listener();
	create_event_listener(player_hurt_event);
	create_event_listener(bullet_impact_event);
	void draw_beam(Vector src, Vector end, Color color);
};

/* extern bullet tracer */
extern bullettracer g_bullettracer;

class bullettracers : public singleton< bullettracers > {
private:
	class trace_info {
	public:
		trace_info(Vector starts, Vector positions, float times) {
			this->start = starts;
			this->position = positions;
			this->time = times;
		}

		Vector position;
		Vector start;
		float time;
	};

	std::vector<trace_info> logs;

	void draw_beam(Vector src, Vector end, Color color);
public:
	void bullet_events(IGameEvent* event);
	void DrawBulletTracers();
};

class otheresp : public singleton< otheresp >
{
public:
	void hitmarker_event(IGameEvent * event);
	void hitmarkerdynamic_paint();

};

struct impact_info
{
	float x, y, z;
	long long time;
};

struct hitmarker_info
{
	impact_info impact;
	int alpha;
};

extern std::vector<impact_info> impacts;
extern std::vector<hitmarker_info> hitmarkers;