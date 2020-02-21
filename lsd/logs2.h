#pragma once
#include "IGameEvent.h"
#include "Vector.h"
#include "IClientEntity.h"
#include "CGlobalVarsBase.h"
#include "IClientEntityList.h"
#include "IGameEvent.h"
#include "GlobalVars.h"
#include "ISurface.h"
#include "Menu.h"
#include "Color.h"
#include "PlayerInfo.h"
#include "Singleton.h"
#include "ICvar.h"

/* definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\



/* functions */
class Logs
{
public:
	void listener();
	create_event_listener(player_hurt_event);
};

/* extern logs */
extern Logs g_logs;