#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>
#include <execution>
#include <algorithm>

namespace fs = std::filesystem;

struct ColorV2
{
	float red, green, blue, alpha;
};

enum
{
	check_box,
	slider,
	combo_box,
	multi_box
};

class Menu
{
public:
	bool menuOpened;
	void Render();

	struct
	{
		bool Aimbot;
		bool Nightmode;
		bool local_chams;
		bool weapon_chams;
		bool hand_chams;
		bool doubletap;
		bool fakeduck;
		int Hitchance;
		int nightmodeval;
		int tickbaseshift;
		int HitchanceValue;
		int hitscan;
		int Mindmg;
		int Visibledmg;
		int Resolver;
		bool canceresolver;
		bool hitboxess[5];
		int BodyScale;
		int HeadScale;
		int Pelvisscale;
		int neckscale;
		int chestscale;
		bool MultiPoint;
		bool FakeDuck;
		int fakeduckspeed;
		int fakelags;
		bool DelayShot;
		bool IgnoreLimbs;
		bool Autostop;
		bool Autoshoot;
		bool auto_scope;
		bool asdasdasd;
		bool chamsenabledd;
		bool FixShotPitch;
		bool hitpoints;
		bool PosBacktrack;
		bool ShotBacktrack;
		bool BaimLethal;
		bool BaimPitch;
		bool BaimInAir;

		bool Antiaim;
		int Antiaimtype;
		int pitchss;
		int yawleft;
		int yawleftdesync;
		int yawright;
		int yawrightdesync;
		int Antiaimtypedsy;
		bool DesyncAngle;
		bool lbystabler;
		bool RandJitterInRange;
		int	JitterRange;
		int	knife_model;
		int	desyncjitterreal;
		int	jitternew;
		int	jitternewcombo;
		int	desyncjitterfake;
		int	Fakelag;
		int	sendfakelag;
		bool FakeLagOnPeek;
		bool ChokeShotOnPeek;
		int	 resolverslider;
		int	 resolverslider1;

		bool Esp;
		int Font;
		ColorV2 FontColor = { 255.f,255.f,255.f,255.f };
		int Name;
		int HealthVal;
		int Weapon;
		bool Box;
		ColorV2 BoxColor = { 255.f,255.f,255.f,255.f };
		bool HealthBar;
		bool Skeleton[2] = { false,false };
		ColorV2 SkeletonColor = { 255.f,255.f,255.f,255.f };
		bool HitboxPoints;
		bool Chams;
		ColorV2 ChamsColor = { 255.f,255.f,255.f,255.f };
		ColorV2 handcolor = { 255.f,255.f,255.f,255.f };
		bool worldcolorcheck;
		ColorV2 worldcolor = { 255.f,255.f,255.f,255.f };
		bool NoZoom;
		bool NoScope;
		bool animfix;
		bool spreadcircle;
		bool NoRecoil;
		bool NoSmoke;
		bool AlphaModulatePropscheckbox;
		ColorV2 skycolor = { 255.f,255.f,255.f,255.f };
		bool bullettracers;
		int AlphaModulateProps;
		int Fov;
		int vFov;
		bool Crosshair;

		bool Bhop;
		bool AutoStrafe;
		bool eventlogs;
		bool killfeedpreserve;
		bool ClantagSpammer;
		bool LegitBacktrack;
		bool Ak47meme;
		bool logscreen;
		bool damageesp;
		bool capsuleoverlayh;
		bool hitsound;
		bool save;
		bool load;
		int	Test;
		int	thirdpersonkey;
		int	doubletapkey;
		int	baimkey;
		int	desyncswitchkey;
		int RightDesyncValue;
		int yaw_additive;
		int yaw_subtractive;
		int jitterrange;
		int LeftDesyncValue;
		int sliderdesyncval;
	}Config;

private:
	struct
	{
		float x = 0.f, y = 0.f;
	}Pos; // lol

	enum
	{
		check_box,
		slider,
		combo_box,
		multi_box,
		button
	};



	int ControlsX;
	int GroupTabBottom;
	int OffsetY;
	int OldOffsetY;
	int TabOffset;
	int SubTabOffset;
	float SubTabSize; // cpp fuckin sux had to make this a float or the whole thing crashes
	float TabSize;
	int GroupTabPos[4];

	int TabNum = 0;
	int SubTabNum = 0;
	int PreviousControl = -1;

	void Tab(std::string name);
	void SubTab(std::string name);
	void CheckBox(std::string name, bool* item);
	void Slider(int max, std::string name, int* item);
	void Slider_float(float max, std::string name, float* item);
	void ComboBox(std::string name, std::vector< std::string > itemname, int* item);
	void MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item);
	void ColorPicker(std::string name, ColorV2& item);
	void keybind(int* item, std::string name);
	void Button(std::string name, bool* item);
};


extern Menu g_Menu;