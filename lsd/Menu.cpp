#include "Menu.h"
#include "TGFCfg.h"
#include "Vector.h"
#include "ISurface.h"
#include "Color.h"
#include "GlobalVars.h"
#include "XorStr.h"

Menu g_Menu;

void Menu::Render()
{  
	static bool Pressed = false;

	if (!Pressed && GetAsyncKeyState(VK_INSERT))
		Pressed = true;
	else if (Pressed && !GetAsyncKeyState(VK_INSERT))
	{
		Pressed = false;
		menuOpened = !menuOpened;
	}

	static Vector2D oldPos;
	static Vector2D mousePos;

	static int dragX = 0;
	static int dragY = 0;
	static int Width = 400;
	static int Height = 500;
	static int headerx = 21;
	static int headery = 400;

	static int iScreenWidth, iScreenHeight;

	static std::string Title = XorStr("L S D");

	static bool Dragging = false;
	bool click = false;

	if (menuOpened)
	{
		if (GetAsyncKeyState(VK_LBUTTON))
			click = true;

		g_pEngine->GetScreenSize(iScreenWidth, iScreenHeight);
		Vector2D MousePos = g_pSurface->GetMousePosition();

		if (Dragging && !click)
		{
			Dragging = false;
		}

		if (Dragging && click)
		{
			Pos.x = MousePos.x - dragX;
			Pos.y = MousePos.y - dragY;
		}
		
		if (g_pSurface->MouseInRegion(Pos.x, Pos.y, Width, 20))
		{
			Dragging = true;
			dragX = MousePos.x - Pos.x;
			dragY = MousePos.y - Pos.y;
		}

		if (Pos.x < 0)
			Pos.x = 0;
		if (Pos.y < 0)
			Pos.y = 0;
		if ((Pos.x + Width) > iScreenWidth)
			Pos.x = iScreenWidth - Width;
		if ((Pos.y + Height) > iScreenHeight)
			Pos.y = iScreenHeight - Height;

		//g_pSurface->RoundedFilledRect(Pos.x, Pos.y, Width, Height, 10, Color(25, 25, 25, 225));
		g_pSurface->OutlinedRect(Pos.x, Pos.y, Width, Height, Color(0, 0, 0, 225));

	/*	static float rainbow = 0;


		if (rainbow < 255)
			rainbow += 0.25f;
		else
			rainbow = 0.f;



		static float sub;
		if (rainbow >= 255)
			sub += 0.25f;
		else
			sub = 0.f;



		if (rainbow < 255)
			g_pSurface->RoundedFilledRect(Pos.x - 10, Pos.y - 10, Width + 20, Height + 20, 10, Color(0, 125, 125, rainbow));
		else
			g_pSurface->RoundedFilledRect(Pos.x - 10, Pos.y - 10, Width + 20, Height + 20, 10, Color(0, 125, 125, 255 - sub));

		g_pSurface->RoundedFilledRect(Pos.x, Pos.y, Width, Height, 10, Color(25, 25, 25, 255));

		g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 20, Width, Height - 40, 5, Color(33, 33, 33, 255));*/     //halo

		//g_pSurface->OutlinedRect(Pos.x-10, Pos.y+ 10, Width+ 10, Height+ 10, Color(0, 0, 0, 225));
		
		g_pSurface->RoundedFilledRect(Pos.x, Pos.y, Width, Height,0.0f, Color(33, 33, 33, 200));
		//g_pSurface->GradientSideways(Pos.x, Pos.y, Width / 2 + 66, 1, Color(20, 20, 20, 200), Color(20, 20, 20, 200), 1);
		//g_pSurface->FilledRect(Pos.x, Pos.y, Width, Height, Color(15, 28, 20, 225));
		
		//g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 20, Width, Height - 40, 5, Color(33, 33, 33, 255));

		GroupTabPos[0] = Pos.x + 85;
		GroupTabPos[1] = Pos.y + 25;
		GroupTabPos[2] = Width - 91;
		GroupTabPos[3] = Height - 50;

		ControlsX = GroupTabPos[0];
		GroupTabBottom = GroupTabPos[1] + GroupTabPos[3];

		g_pSurface->RoundedFilledRect(Pos.x, Pos.y, headery, headerx,0.0f, Color(33, 33, 33, 255));
		g_pSurface->DrawT(Pos.x + 170, Pos.y + 2, Color(255, 255, 255, 255), g::Logs, false, Title.c_str()); // title
		//g_pSurface->DrawT(Pos.x + 6, Pos.y + (Height - 18), Color(255, 255, 255, 255), g::CourierNew, false, Title.c_str()); // bottom text

		OffsetY = GroupTabPos[1] + 7;

		static bool CfgInitLoad = true;
		static bool CfgInitSave = true;

		static int SaveTab = 0;

		if (g_pSurface->MouseInRegion(Pos.x, Pos.y + 445 + 2, 80, 26))
		{
			if (CfgInitLoad && click)
			{
				SaveTab = 0;

				g_Config->Load();
				CfgInitLoad = false;
			}
		}
		else
			CfgInitLoad = true;

		if (g_pSurface->MouseInRegion(Pos.x, Pos.y + 400 + 2, 80, 26))
		{
			if (CfgInitSave && click)
			{
				SaveTab = 1;

				g_Config->Save();
				CfgInitSave = false;
			}
		}
		else
			CfgInitSave = true;

		g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 400 + 2, 70, 50, 5, Color(119, 119, 119, 255));

	if (SaveTab == 0)
		{
		//g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 295, 80, 25, 5, Color(120, 209, 109, 255));
		g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 445 + 2, 80, 26, 5, Color(53, 53, 53, 255));
		}
		else
		{
		//	g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 270, 80, 25, 5, Color(120, 209, 109, 255));
			g_pSurface->RoundedFilledRect(Pos.x, Pos.y + 400 + 2, 80, 26, 5, Color(53, 53, 53, 255));
		}

		g_pSurface->DrawT(Pos.x , Pos.y + 400 + 2, Color(255, 255, 255, 255), g::CourierNew, false, "save");
		g_pSurface->DrawT(Pos.x, Pos.y + 445 + 2, Color(255, 255, 255, 255), g::CourierNew, false, "load");

		TabOffset = 0;
		SubTabOffset = 0;
		PreviousControl = -1;
		OldOffsetY = 0;
		 
		Tab("A");//rage
		{
			//SubTab("main");
			//{
			
				CheckBox("enable", &Config.Aimbot);
				CheckBox("auto shoot", &Config.Autoshoot);
				CheckBox("autostop", &Config.Autostop);
				CheckBox("autoscope", &Config.auto_scope);
				Slider(100, "hitchance", &Config.HitchanceValue);
				Slider(100, "wall damage", &Config.Mindmg);
				ComboBox("Hitscan", { "Head","Body","Mixed","backtrack" }, &Config.hitscan);
				Slider(100, "head scale", &Config.HeadScale);
				Slider(100, "body scale", &Config.BodyScale);
				CheckBox("rapid fire", &Config.doubletap);
				Tab("C");//antiaim
				{
					ComboBox("fakelag", { "factor", "adaptive" }, & Config.fakelags); //keep
					Slider(17, "value", &Config.Fakelag); // keep
					//Slider(17, "value2", &Config.sendfakelag); // keep
					ComboBox("Pitch", { "disable", "down","up","flip" }, &Config.pitchss); //keep
					ComboBox("Yaw", { "disable", "back" ,"custom back"," autodirecton" }, &Config.Antiaimtype); //keep			
					ComboBox("Dsync", { "disable", "flip", "jitter","switch desync","custom freestand","real jitter","offset jitter","freestand desync" }, &Config.Antiaimtypedsy); //kep
					CheckBox("stableize lby", &Config.lbystabler);
					Slider(120, "slowwalk speed ", &Config.fakeduckspeed);
					Slider(360, "add yaw ", &Config.yaw_additive);
					Slider(360, "add yaw ", &Config.yaw_subtractive);
					if (Config.Antiaimtypedsy == 1)
					{
						Slider(360, "left", &Config.RightDesyncValue);
						Slider(360, "right", &Config.LeftDesyncValue);
					}
					if(Config.Antiaimtypedsy == 2||Config.Antiaimtypedsy == 3)
					Slider(180, "jitter range", &Config.jitterrange);
					if (Config.Antiaimtypedsy == 4)
					{
						Slider(360, "left", &Config.yawleft);
						Slider(360, "desyncleft", &Config.yawleftdesync);
						Slider(360, "right", &Config.yawright);
						Slider(360, "desyncright", &Config.yawrightdesync);
						Slider(360, "jitter val", &Config.desyncjitterreal);
					}
					if (Config.Antiaimtypedsy == 5)
					{
						Slider(360, "real val", &Config.desyncjitterreal);
						Slider(360, "fake val", &Config.desyncjitterfake);
					}
					if (Config.Antiaimtypedsy == 7)
					{
						Slider(180, "jitter val", &Config.jitternew);
						ComboBox("Desynctypes", { "normal", "inverse","inverse delta","" }, & Config.jitternewcombo);
						
					}
						
				//	Slider(100, "right eye", &Config.resolverslider);
					//Slider(60, "left eye ", &Config.resolverslider1);
				}
			//	CheckBox("Spread Seeds", &Config.spreadcircle);
		//		Slider(10, "fakeduckspeed", &Config.fakeduckspeed);
				//CheckBox("resolver", &Config.Resolver);
			//	CheckBox("Resolve Help", &Config.canceresolver);

		//	}

		//	SubTab("position");
			//{
				//CheckBox("multipoint", &Config.MultiPoint);
				//if (Config.MultiPoint)
			//	{
				//	Slider(100, "head scale", &Config.HeadScale);
					//Slider(100, "body scale", &Config.BodyScale);
			//	}
				//CheckBox("delay shot", &Config.DelayShot);
				//CheckBox("ignore limbs on-move", &Config.IgnoreLimbs);
		//	}

		//	SubTab("target");
		//	{
				//CheckBox("shot backtrack", &Config.ShotBacktrack);
				//CheckBox("position backtrack", &Config.PosBacktrack);
				//CheckBox("baim if lethal", &Config.BaimLethal);
				//CheckBox("baim bad pitch", &Config.BaimPitch);
				//CheckBox("baim in air", &Config.BaimInAir);
		//	}
		}

		Tab("D");//visuals
		{
			CheckBox("2d box", &Config.Box);
			CheckBox("no zoom", &Config.NoZoom);
			Slider(150, "debug fov", &Config.Fov);
			Slider(150, "viewmodel fov", &Config.vFov);
			//CheckBox("nightmode", &Config.Nightmode);
			
			CheckBox("Chams", &Config.fakeduck);
			CheckBox("hand chams", &Config.hand_chams);
			CheckBox("weapon chams", &Config.weapon_chams);
			CheckBox("desync chams", &Config.local_chams);
			//Slider(4000000, "bone mask", &Config.nightmodeval);
		}
		Tab("G");//misc
		{
			std::string knife_options[15] = {
		"Default",
		"M9 Bayonet",
		"Bayonet",
		"Flip",
		"Gut",
		"Karambit",
		"Huntsman",
		"Falchion",
		"Bowie",
		"Butterfly",
		"Shadow Daggers",
		"Navaja",
		"Stiletto",
		"Ursus",
		"Talon"
			};
			CheckBox("event logs", &Config.eventlogs);
			CheckBox("damage esp", &Config.damageesp);
			CheckBox("capsule overlay", &Config.capsuleoverlayh);
			CheckBox("hit sound", &Config.hitsound);
			ComboBox("Knifes", { "Default",
		"M9 Bayonet",
		"Bayonet",
		"Flip",
		"Gut",
		"Karambit",
		"Huntsman",
		"Falchion",
		"Bowie",
		"Butterfly",
		"Shadow Daggers",
		"Navaja",
		"Stiletto",
		"Ursus",
		"Talon",
		"Classic","Cord","Cannis","Outdoor","Skeleton"}, &Config.knife_model); //keep
			static int CurrentKeybind = 0;

			ComboBox("Keybind", { "Thirdperson", "Doubletap", "Force Baim","Desync Switch" }, &CurrentKeybind);
			//keybind(&Config.Test, "a");
			switch (CurrentKeybind)
			{
			case 0: keybind(&Config.thirdpersonkey, "a");
				break;
			case 1: keybind(&Config.doubletapkey, "b");
				break;
			case 2: keybind(&Config.baimkey, "c");
				break;
			case 3: keybind(&Config.desyncswitchkey, "d");
				break;
			}
			//CheckBox("event logs", &Config.eventlogs);
		//	Button("load", &Config.load);

		//	Button("save", &Config.save);

			
			//CheckBox("desync chams", &Config.local_chams);
		}
		//	SubTab("esp");
			//{
			//	CheckBox("active", &Config.Esp);
			//	ComboBox("font", { "courier", "tahoma" }, &Config.Font);
			//	ColorPicker("font color", Config.FontColor);
			//	ComboBox("draw name", { "off", "top", "right" }, &Config.Name);
			//	ComboBox("draw health", { "off", "right" }, &Config.HealthVal);
			//	ComboBox("draw weapon", { "off", "bottom", "right" }, &Config.Weapon);
			//	ColorPicker("box color", Config.BoxColor);
			//	CheckBox("health bar", &Config.HealthBar);
				//MultiComboBox("draw skeleton", { "normal", "backtrack" }, Config.Skeleton);
				//ColorPicker("skele color", Config.SkeletonColor);
			//}

		//	SubTab("render");
			//{
			//	CheckBox("hitbox points", &Config.HitboxPoints);
				//CheckBox("chams", &Config.Chams); //keep
			//	CheckBox("no zoom", &Config.NoZoom);
			//	CheckBox("no scope", &Config.NoScope);
			//	CheckBox("no recoil", &Config.NoRecoil);
			//	CheckBox("no smoke", &Config.NoSmoke);
			//	Slider(150, "fov", &Config.Fov);
			//	CheckBox("Brightness Adjustment", &Config.AlphaModulatePropscheckbox);
				
			//	CheckBox("Bullet Tracers", &Config.bullettracers);
				//CheckBox("crosshair", &Config.Crosshair);
		//	}
	//	}

	//	Tab("misc");
	//	{
		//	CheckBox("bhop", &Config.Bhop);
			//CheckBox("autostrafe", &Config.AutoStrafe);
			//CheckBox("Event Logs", &Config.eventlogs);
			//CheckBox("KillFeed Preserve", &Config.killfeedpreserve);
			//CheckBox("Clantag Spam", &Config.ClantagSpammer);
			//CheckBox("legit backtrack", &Config.LegitBacktrack);
			//CheckBox("make every gun the old ak47", &Config.Ak47meme);
			//Slider(360, "test", &Config.Test);
	//	}

		TabSize = TabOffset;
		SubTabSize = SubTabOffset;
	}
}

void Menu::Tab(std::string name)
{
	int TabArea[4] = { Pos.x, Pos.y+9 + 20 + (TabOffset * 60), 80, 60 };

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(TabArea[0], TabArea[1], TabArea[2], TabArea[3]))
		TabNum = TabOffset;

	if (TabOffset == 0)
      g_pSurface->RoundedFilledRect(TabArea[0], TabArea[1], 70, (TabSize * 60), 2, Color(0,0,0,0));

	if (TabOffset == TabNum)
	{
	//	g_pSurface->RoundedFilledRect(TabArea[0], TabArea[1], 80, TabArea[3], 5, Color(81, 115, 244, 255));
		g_pSurface->RoundedFilledRect(TabArea[0], TabArea[1], 70, TabArea[3], 2, Color(48, 170, 240, 255));
	}

	g_pSurface->DrawT(TabArea[0] + 22, TabArea[1] + 10, Color(255, 255, 255, 255), g::logo, false, name.c_str());

	TabOffset += 1;
	PreviousControl = -1;
}

void Menu::SubTab(std::string name)
{
	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	RECT TextSize = g_pSurface->GetTextSizeRect(g::CourierNew, name.c_str());

	static int TabSkip = 0;

	if (SubTabOffset == 0)
		g_pSurface->RoundedFilledRect(GroupTabPos[0], GroupTabPos[1], GroupTabPos[2], 21, 6, Color(119, 119, 119, 255));

	if (SubTabSize != 0 && TabSkip == TabNum)
	{
		if (TabNum >= SubTabSize)
			TabNum = 0;

		int TabLength = (GroupTabPos[2] / SubTabSize);

		int GroupTabArea[4] = { (GroupTabPos[0]) + (TabLength * SubTabOffset), GroupTabPos[1], TabLength, 21 };

		if ((GroupTabArea[0] + GroupTabArea[3]) <= (GroupTabPos[0] + GroupTabPos[2]))
		{
			int TextPosition[2] = { GroupTabArea[0] + (TabLength / 2) - (TextSize.right / 2), (GroupTabArea[1] + 10) - (TextSize.bottom / 2) };

			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(GroupTabArea[0], GroupTabArea[1], GroupTabArea[2], GroupTabArea[3]))
				SubTabNum = SubTabOffset;

			int Offset = ((SubTabSize - 1) == SubTabOffset) ? 0 : 1;

			if (((SubTabSize - 1) == SubTabOffset) && (((TabLength * SubTabSize) > GroupTabPos[2]) || ((TabLength * SubTabSize) < GroupTabPos[2])))
				Offset = (GroupTabPos[2] - (TabLength * SubTabSize));

			if (SubTabNum == SubTabOffset)
				g_pSurface->RoundedFilledRect(GroupTabArea[0], GroupTabArea[1], GroupTabArea[2] + Offset, GroupTabArea[3], 5, Color(53, 53, 53, 255));

			g_pSurface->DrawT(TextPosition[0], TextPosition[1], Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());
		}
	}

	if (TabSkip != TabNum) // frame skip for drawing
		TabSkip = TabNum;

	if (SubTabOffset == SubTabNum)
		OffsetY += 20;

	SubTabOffset += 1;
	PreviousControl = -1;
}

void Menu::CheckBox(std::string name, bool* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool pressed = false;

	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 277, OffsetY, 16, 16))
	{
		if (pressed)
			*item = !*item;
		pressed = false;
	}

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 277, OffsetY, 16, 16) && !pressed)
		pressed = true;

	if (*item == true)
		g_pSurface->RoundedFilledRect(ControlsX + 277, OffsetY, 14, 14, 3.0f, Color(48, 170, 240, 255));
	else
		g_pSurface->RoundedFilledRect(ControlsX + 277, OffsetY, 14, 14, 3.0f, Color(117, 125, 130, 255));

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::trajen, false, name.c_str());

	OldOffsetY = OffsetY;
	OffsetY += 30;
	PreviousControl = check_box;
}

void Menu::Slider(int max, std::string name, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	float pixelValue = max / 114.f;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY + 5, 120, 8))
		*item = (g_pSurface->GetMousePosition().x - (ControlsX + 155)) * pixelValue;

	if (*item > max)
		*item = max;
	if (*item < 0)
		*item = 0;

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::trajen, false, name.c_str());
	g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY + 5, 120, 8, 1.8f, Color(62, 62, 62, 255));
	if ((*item / pixelValue) > 0)
		g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY + 5, (*item / pixelValue) + 2, 7, 1.8f, Color(48, 170, 240, 230));

	g_pSurface->RoundedFilledRect(ControlsX + 153 + (*item / pixelValue), OffsetY + 3, 12, 12, 6, Color(117, 125, 130, 255));

	g_pSurface->DrawT(ControlsX + 292, OffsetY + 1, Color(255, 255, 255, 255), g::trajen, true, std::to_string(*item).c_str());


	OldOffsetY = OffsetY;
	OffsetY += 30;
	PreviousControl = slider;
}

void Menu::Slider_float(float max, std::string name, float* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	float pixelValue = max / 114.f;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY + 5, 120, 8))
		* item = (g_pSurface->GetMousePosition().x - (ControlsX + 155)) * pixelValue;

	if (*item > max)
		* item = max;
	if (*item < 0.f)
		* item = 0.f;

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::trajen, false, name.c_str());
	g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY + 5, 120, 8, 5, Color(62, 62, 62, 255));
	g_pSurface->RoundedFilledRect(ControlsX + 153 + (*item / pixelValue), OffsetY + 5, 6, 9, 5, Color(119, 119, 119, 255));

	g_pSurface->DrawT(ControlsX + 292, OffsetY + 1, Color(255, 255, 255, 255), g::trajen, true, std::to_string(*item).c_str());

	OldOffsetY = OffsetY;
	OffsetY += 30;
	PreviousControl = slider;
}

void Menu::ComboBox(std::string name, std::vector< std::string > itemname, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::trajen, false, name.c_str());
	g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 16, 5, Color(117, 125, 130, 255));

	if (open)
	{
		g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), 5, Color(117, 125, 130, 255));

		for (int i = 0; i < itemname.size(); i++)
		{
			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY + 16 + i * 16, 140, 16))
				*item = i;

			if (*item == i)
				g_pSurface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 5, Color(48, 170, 240, 255));

			g_pSurface->DrawT(ControlsX + 159, OffsetY + 16 + (i * 16), Color(255, 255, 255, 255), g::trajen, false, itemname.at(i).c_str());
		}
	}

	g_pSurface->DrawT(ControlsX + 159, OffsetY, Color(255, 255, 255, 255), g::trajen, false, itemname.at(*item).c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 30 + (itemname.size() * 16);
	else
		OffsetY += 30;

	PreviousControl = combo_box;
}

void Menu::MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool multiPressed = false;
	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;
	std::string itemsSelected = "";
	int lastItem = 0;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	g_pSurface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::trajen, false, name.c_str());
	g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 16, 5, Color(117, 125, 130, 255));

	if (open)
	{
		g_pSurface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), 5, Color(117, 125, 130, 255));

		for (int i = 0; i < itemname.size(); i++)
		{
			if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY + 16 + (i * 16), 140, 16))
			{
				if (multiPressed)
					item[i] = !item[i];
				multiPressed = false;
			}

			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 153, OffsetY + 16 + (i * 16), 140, 16) && !multiPressed)
				multiPressed = true;

			if (item[i])
				g_pSurface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 5, Color(120, 209, 109, 255));
			else
				g_pSurface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 5, Color(192, 97, 108, 255));

			g_pSurface->DrawT(ControlsX + 159, OffsetY + 16 + (i * 16), Color(255, 255, 255, 255), g::trajen, false, itemname.at(i).c_str());
		}

	}

	bool items = false;

	// man look at all these for loops i sure am retarded

	for (int i = 0; i < itemname.size(); i++)
	{
		if (item[i])
		{
			if (lastItem < i)
				lastItem = i;
		}
	}

	for (int i = 0; i < itemname.size(); i++)
	{
		if (item[i])
		{
			items = true;
			RECT TextSize = g_pSurface->GetTextSizeRect(g::trajen, itemsSelected.c_str());
			RECT TextSizeGonaAdd = g_pSurface->GetTextSizeRect(g::trajen, itemname.at(i).c_str());
			if (TextSize.right + TextSizeGonaAdd.right < 130)
				itemsSelected += itemname.at(i) + ((lastItem == i) ? "" : ", ");
		}
	}

	if (!items)
		itemsSelected = "off";

	g_pSurface->DrawT(ControlsX + 159, OffsetY, Color(255, 255, 255, 255), g::trajen, false, itemsSelected.c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 30 + (itemname.size() * 16);
	else
		OffsetY += 30;

	PreviousControl = multi_box;
}

void Menu::ColorPicker(std::string name, ColorV2& item) // best coder in the universe
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	if (PreviousControl == slider || PreviousControl == -1)
		return;

	int CtrXoffset = 0;

	if (PreviousControl != check_box)
		CtrXoffset = 132;
	else
		CtrXoffset = 256;

	int yoffset = OldOffsetY + 10;
	int xoffset = ControlsX + 330;

	Color rainbow;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + CtrXoffset, OldOffsetY, 16, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + CtrXoffset, OldOffsetY, 16, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	if (open)
	{
		g_pSurface->RoundedFilledRect(xoffset, OldOffsetY, 100, 20, 5, Color(0, 0, 0, 255));
		g_pSurface->RoundedFilledRect(xoffset, OldOffsetY + 100, 100, 20, 5, Color(255, 255, 255, 255));

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset, OldOffsetY, 100, 10))
		{
			item.red = 0;
			item.green = 0;
			item.blue = 0;
			item.alpha = 255;
		}

		if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset, OldOffsetY + 110, 100, 10))
		{
			item.red = 255;
			item.green = 255;
			item.blue = 255;
			item.alpha = 255;
		}

		for (int i = 0; i < 100; i++)
		{
			if (xoffset >= ControlsX + 430)
			{
				xoffset -= 100;
				yoffset += 10;
			}

			float hue = (i * .01f);

			rainbow.FromHSV(hue, 1.f, 1.f);

			g_pSurface->FilledRect(xoffset, yoffset, 10, 10, rainbow);

			if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(xoffset, yoffset, 10, 10))
			{
				item.red = rainbow.red;
				item.green = rainbow.green;
				item.blue = rainbow.blue;
				item.alpha = 255.f;
			}

			xoffset += 10;
		}
	}

	rainbow.red = item.red;
	rainbow.green = item.green;
	rainbow.blue = item.blue;
	rainbow.alpha = 255;

	g_pSurface->RoundedFilledRect(ControlsX + CtrXoffset, OldOffsetY, 16, 16, 5, rainbow);
}

bool manually_set_key = false;

char* KeyStringsStick[254] = {
	"INVLD", "M1", "M2", "BRK", "M3", "M4", "M5",
	"INVLD", "BSPC", "TAB", "INVLD", "INVLD", "INVLD", "ENTER", "INVLD", "INVLD", "SHI",
	"CTRL", "ALT", "PAU", "CAPS", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"ESC", "INVLD", "INVLD", "INVLD", "INVLD", "SPACE", "PGUP", "PGDOWN", "END", "HOME", "LEFT",
	"UP", "RIGHT", "DOWN", "INVLD", "PRNT", "INVLD", "PRTSCR", "INS", "DEL", "INVLD", "0", "1",
	"2", "3", "4", "5", "6", "7", "8", "9", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
	"V", "W", "X", "Y", "Z", "LFTWIN", "RGHTWIN", "INVLD", "INVLD", "INVLD", "NUM0", "NUM1",
	"NUM2", "NUM3", "NUM4", "NUM5", "NUM6", "NUM7", "NUM8", "NUM9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3",
	"F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20",
	"F21",
	"F22", "F23", "F24", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"NUM LOCK", "SCROLL LOCK", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "LSHFT", "RSHFT", "LCTRL",
	"RCTRL", "LMENU", "RMENU", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "NTRK", "PTRK", "STOP", "PLAY", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", ";", "+", ",", "-", ".", "/?", "~", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "{", "\\|", "}", "'\"", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD", "INVLD",
	"INVLD", "INVLD"
};
bool keys[256];
bool oldKeys[256];
bool GetKeyPress(unsigned int key)
{
	if (keys[key] == true && oldKeys[key] == false)
		return true;
	return false;
}

void Menu::keybind(int* item, std::string name)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	if (PreviousControl == slider || PreviousControl == -1)
		return;

	int ComboSize = 130; // 163
	static std::string nameSelected;


	static bool IsGettingKey = false;
	static int Key = 0;
	static bool GoodKeyName = false;
	Color text_color = Color(120, 120, 120, 255);
	std::copy(keys, keys + 255, oldKeys);
	for (int x = 0; x < 255; x++)
	{
		//oldKeys[x] = oldKeys[x] & keys[x];
		keys[x] = (GetAsyncKeyState(x));
	}

	char NameBuffer[128];
	char* KeyName = "Unbinded";
	RECT text_size = g_pSurface->GetTextSizeRect(g::trajen, KeyName);
	static bool LetsBindy0 = false;
	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 159, OffsetY, 130, 20) && menuOpened)
	{
		if (!IsGettingKey)
		{
			IsGettingKey = true;
		}
	}

	if (IsGettingKey)
	{
		for (int i = 0; i < 255; i++)
		{
			if (GetKeyPress(i))
			{
				if (i == VK_ESCAPE)
				{
					Key = -1;
					*item = Key;
					IsGettingKey = false;
					return;
				}

				Key = i;
				*item = Key;
				IsGettingKey = false;
				return;
			}
		}
	}


	if (IsGettingKey)
	{
		KeyName = "~~~~~";
		text_color = Color(143, 143, 143, 255);
	}
	else
	{
		if (*item >= 0)
		{
			KeyName = KeyStringsStick[*item];
			if (KeyName)
			{
				GoodKeyName = true;
			}
			else
			{
				if (GetKeyNameText(*item << 16, NameBuffer, 127))
				{
					KeyName = NameBuffer;
					GoodKeyName = true;
				}
			}
		}

		if (!GoodKeyName)
		{
			KeyName = "Unbinded";
		}
	}

	if (manually_set_key)
	{
		Key = *item;
		manually_set_key = false;
	}


	//g_pSurface->FilledRect(x_offset + 20, y_offset, ComboSize, 20, Color(45, 46, 53, MenuAlpha_Main));
	g_pSurface->OutlinedRect(ControlsX + 159, OffsetY, ComboSize, 20, Color(0, 0, 0, 200));
	g_pSurface->DrawT(ControlsX + 159 + 5, OffsetY + 3, Color(143, 143, 143, 200), g::trajen, false,
		KeyName);

	OffsetY += 29;
}

void Menu::Button(std::string name, bool* item)
{
	static bool pressed = false;


	if (!GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 15, 120, 15))
	{
		if (pressed)
		{
			if (item)
				* item = !*item;
		}
		pressed = false;
	}

	if (GetAsyncKeyState(VK_LBUTTON) && g_pSurface->MouseInRegion(ControlsX + 6, OffsetY + 15, 120, 15) && !pressed)
		pressed = true;


	if (pressed)
		g_pSurface->FilledRect(ControlsX + 6, OffsetY + 15, 120, 15, Color(53, 53, 53, 255));
	else
		g_pSurface->FilledRect(ControlsX + 6, OffsetY + 15, 120, 15, Color(120, 120, 120, 255));


	if (*item == true)
		item = false;

	g_pSurface->DrawT(ControlsX + 55, OffsetY + 15, Color(255, 255, 255, 255), g::trajen, false, name.c_str());

	OldOffsetY = OffsetY;
	OffsetY += 26;
	PreviousControl = button;

}




