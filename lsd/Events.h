#pragma once
#include "GlobalVars.h"
#include "CGlobalVarsBase.h"
#include "Singleton.h"
#include "Aimbot.h"
#include "Autowall.h"
#include "LagComp.h"
#include "AntiAim.h"
#include "Utils.h"
#include "IVEngineClient.h"
#include "PlayerInfo.h"
#include "ICvar.h"
#include "Math.h"
#include "Hitboxes.h"
#include "Menu.h"
#include "IVModelInfo.h"
#include "ClientClass.h"
#include <array>
#include <deque>
#include <algorithm>
#include <DirectXMath.h>
#include "Interfaces.h"
#include "IBaseClientDll.h"
#include "IClientMode.h"
#include "ISurface.h"
#include "IGameEvent.h"
#include "CInput.h"
#include "CModelRender.h"
#include "IVModelInfo.h"
#include "IMaterial.h"
#include "IVRenderView.h"
#include <assert.h>
#include "IEngineTrace.h"
class eventManager;
extern eventManager eventmanager;

class eventManager
{
	class eventListener
		: public IGameEventListener
	{
	public:
		void start() {
			g_pEventManager->AddListener(this, "player_hurt", false);
			g_pEventManager->AddListener(this, "start_vote", true);
			g_pEventManager->AddListener(this, "bullet_impact", false);
			g_pEventManager->AddListener(this, "weapon_fire", false);
			g_pEventManager->AddListener(this, "player_death", false);
			g_pEventManager->AddListener(this, "player_connect", false);
			g_pEventManager->AddListener(this, "item_purchase", false);
			g_pEventManager->AddListener(this, "bomb_pickup", false);
			g_pEventManager->AddListener(this, "bomb_beginplant", false);
			g_pEventManager->AddListener(this, "bomb_begindefuse", false);
			g_pEventManager->AddListener(this, "enter_bombzone", false);
			g_pEventManager->AddListener(this, "round_prestart", false);
			g_pEventManager->AddListener(this, "bullet_impact", false);
			g_pEventManager->AddListener(this, "player_footstep", false);
			g_pEventManager->AddListener(this, "player_jump", false);
			g_pEventManager->AddListener(this, "player_falldamage", false);
			g_pEventManager->AddListener(this, "round_start", false);
			g_pEventManager->AddListener(this, "client_disconnect", false);
			g_pEventManager->AddListener(this, "cs_game_disconnected", false);
			//cs_game_disconnected
		}
		void stop() {
			g_pEventManager->RemoveListener(this);
		}
		void FireGameEvent(IGameEvent* event) override {
			eventmanager.onEvent(event);
		}
		int GetEventDebugID(void) override {
			return EVENT_DEBUG_ID_INIT;
		}
	};
	eventListener listener;
public:
	void Initialize();
	void Remove();
	void onEvent(IGameEvent* event);
};
struct BulletImpact_t
{
	float			flImpactTime;
	Vector			vecImpactPos;
	Color			color;
	C_BaseEntity* pPlayer;
	__forceinline BulletImpact_t()
	{
		vecImpactPos = Vector(0.0f, 0.0f, 0.0f);
		flImpactTime = 0.0f;
		color = Color(255, 255, 255);
		pPlayer = nullptr;
	}
	__forceinline BulletImpact_t(C_BaseEntity* player, Vector pos, float time, Color col = Color(255, 255, 255))
	{
		pPlayer = player;
		flImpactTime = time;
		vecImpactPos = pos;
		color = col;
	}
};
extern float GetCurtimes();
extern std::vector<BulletImpact_t> Impacts;
namespace FEATURES
{

	namespace MISC
	{
		class ColorLine
		{
		public:
			ColorLine() {}

			ColorLine(std::string text, Color color = Color(255, 255, 255, 255))
			{
				texts.push_back(text);
				colors.push_back(color);
			}

			void PushBack(std::string text, Color color = Color(255, 255, 255, 255))
			{
				texts.push_back(text);
				colors.push_back(color);
			}

			void PushFront(std::string text, Color color = Color(255, 255, 255, 255))
			{
				texts.insert(texts.begin(), text);
				colors.insert(colors.begin(), color);
			}

			void ChangeAlpha(int alpha)
			{
				for (auto& color : colors)
					color.alpha = alpha;
			}

			void Draw(int x, int y, unsigned int font);

			std::string Text()
			{
				std::string text;
				for (const auto string : texts)
					text += string;

				return text;
			}

		private:
			std::vector<std::string> texts;
			std::vector<Color> colors;
		};

		class InGameLogger
		{
		public:
			struct Log
			{
				Log()
				{
					time = GetCurtimes();
				}

				Log(std::string text, Color color = Color(255, 255, 255, 255))
				{
					color_line = ColorLine(text, color);
					time = GetCurtimes();
				}

				ColorLine color_line;
				float time;
			};

		public:
			void Do();

			void AddLog(Log log)
			{
				log_queue.insert(log_queue.begin(), log);
			}

		private:
			std::vector<Log> log_queue;

		private:
			const float text_time = 7.f;
			const float text_fade_in_time = 0.3f;
			const float text_fade_out_time = 0.2f;

			const int max_lines_at_once = 13;

			const int ideal_height = 1;
			const int ideal_width = 1;

			const int slide_in_distance = 20;
			const int slide_out_distance = 20;
			const float slide_out_speed = 0.3f;
		};

		extern InGameLogger in_game_logger;
	}
}
#define MAX_FLOATING_TEXTS 50

class FloatingText
{
public:
	bool valid = false;
	float startTime = 1.f;
	int damage = 0;
	int hitgroup = 0;
	Vector hitPosition = Vector(0, 0, 0);
	int randomIdx = 0;
};

class c_damageesp
{
public:
	void draw_damage();
	void on_player_hurt(IGameEvent* event);
	void on_bullet_impact(IGameEvent* event);
private:
	std::array<FloatingText, MAX_FLOATING_TEXTS> floatingTexts;
	Vector c_impactpos = Vector(0, 0, 0);
	int floatingTextsIdx = 0;
};

extern c_damageesp damageesp;