#include "Interfaces.h"
#include "Utils.h"
#include "IViewRenderBeams.h"
#include "IClientMode.h"
#include "IBaseClientDll.h"
#include "IClientEntityList.h"
#include "IVEngineClient.h"
#include "CPrediction.h"
#include "IGameEvent.h"
#include "ISurface.h"
#include "IEngineTrace.h"
#include "ISurfaceData.h"
#include "ICvar.h"
#include "IVModelInfo.h"
#include "CModelRender.h"
#include "IMaterial.h"
#include "IVRenderView.h"

#include <fstream>


#define enc_str(s) std::string(s)
#define enc_char(s) enc_str(s).c_str()
#define enc_wstr(s) std::wstring(enc_str(s).begin(), enc_str(s).end())
#define enc_wchar(s) enc_wstr(s).c_str()

// Initializing global interfaces
IBaseClientDLL*     g_pClientDll    = nullptr;
IClientMode*        g_pClientMode   = nullptr;
IClientEntityList*  g_pEntityList   = nullptr;
IVEngineClient*     g_pEngine       = nullptr;
CPrediction*        g_pPrediction   = nullptr;
IGameMovement*      g_pMovement     = nullptr;
IMoveHelper*        g_pMoveHelper   = nullptr;
CGlobalVarsBase*    g_pGlobalVars   = nullptr;
IMemAlloc* g_pMemalloc = nullptr;
IGameEventManager*  g_pEventManager = nullptr;
ISurface*           g_pSurface      = nullptr;
IClientState*		g_pClientState = nullptr;
IEngineTrace*       g_pTrace        = nullptr;
CInput*             g_pInput        = nullptr;
IPhysicsSurfaceProps* g_pSurfaceData = nullptr;
ICVar*              g_pCvar			= nullptr;
IPanel*				g_pPanel		= nullptr;
IVDebugOverlay* g_pIVDebugOverlay = nullptr;
IVModelInfo*		g_pModelInfo	= nullptr;
CModelRender*       g_pModelRender  = nullptr;
IMaterialSystem*    g_pMaterialSys  = nullptr;
IVRenderView*       g_pRenderView   = nullptr;
IViewRenderBeams*   g_pViewRenderBeams = nullptr;
CGlowObjectManager* g_pGlowManager = nullptr;
IRender* g_pRender;

namespace interfaces
{
	template< class T >
	T* FindClass(std::string szModuleName, std::string szInterfaceName, bool bSkip = false)
	{
		if (szModuleName.empty() || szInterfaceName.empty())
			return nullptr;
		typedef PVOID(*CreateInterfaceFn)(const char* pszName, int* piReturnCode);
		CreateInterfaceFn hInterface = nullptr;
		while (!hInterface)
		{
			hInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(szModuleName.c_str()), "CreateInterface");
			Sleep(5);
		}

		char pszBuffer[256];
		for (int i = 0; i < 100; i++)
		{
			sprintf_s(pszBuffer, "%s%0.3d", szInterfaceName.c_str(), i);
			PVOID pInterface = hInterface(pszBuffer, nullptr);

			if (pInterface && pInterface != NULL)
			{
				if (bSkip)
					sprintf_s(pszBuffer, "%s%0.3d", szInterfaceName.c_str(), i + 1);

				Sleep(5);
				break;
			}
		}

		return (T*)hInterface(pszBuffer, nullptr);
	}

	template<typename T>
	T* CaptureInterface(const char* szModuleName, const char* szInterfaceVersion)
	{
		HMODULE moduleHandle = GetModuleHandleA(szModuleName);
		if (moduleHandle)   /* In case of not finding module handle, throw an error. */
		{
			CreateInterfaceFn pfnFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(moduleHandle, "CreateInterface"));
			return reinterpret_cast<T*>(pfnFactory(szInterfaceVersion, nullptr));
		}
		Utils::Log("Error getting interface %", szInterfaceVersion);
		return nullptr;
	}


	void* FindInterfaceEx(const char* Module, const char* InterfaceName)
	{
		void* Interface = nullptr;
		auto CreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(
			GetModuleHandleA(Module), "CreateInterface"));

		Interface = CreateInterface(InterfaceName, 0);

		//		if (!Interface)
		//			std::cout << (enc_str("Failed to find interface - ") + InterfaceName) << std::endl;
		//		else
		//			std::cout << (enc_str("Successfully found interface - ") + InterfaceName) << std::endl;

		return Interface;
	}


    void Init()
    {
        g_pClientDll    = FindClass<IBaseClientDLL>("client_panorama.dll", "VClient");
        g_pClientMode   = **reinterpret_cast<IClientMode***>    ((*reinterpret_cast<uintptr_t**>(g_pClientDll))[10] + 0x5u);  
        g_pGlobalVars   = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(g_pClientDll))[0]  + 0x1Bu); 
        g_pEntityList   = FindClass<IClientEntityList>("client_panorama.dll", "VClientEntityList");
        g_pEngine       = FindClass<IVEngineClient>("engine.dll", "VEngineClient");
        g_pPrediction   = FindClass<CPrediction>("client_panorama.dll", "VClientPrediction");
        g_pMovement     = FindClass<IGameMovement>("client_panorama.dll", "GameMovement");
        g_pMoveHelper   = **reinterpret_cast<IMoveHelper***>((Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? 8B 46 08 68") + 0x2));  
		g_pEventManager = reinterpret_cast<IGameEventManager*>(FindInterfaceEx("engine.dll", "GAMEEVENTSMANAGER002"));
        g_pSurface      = FindClass<ISurface>("vguimatsurface.dll", "VGUI_Surface");
		g_pGlowManager = (CGlowObjectManager*)(*(DWORD*)(Utils::FindSignature("client_panorama.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 3));  // Get GlowManager
		g_pTrace        = FindClass<IEngineTrace>("engine.dll", "EngineTraceClient");
		g_pSurfaceData  = FindClass<IPhysicsSurfaceProps>("vphysics.dll", "VPhysicsSurfaceProps");
		g_pCvar         = FindClass<ICVar>("vstdlib.dll", "VEngineCvar");
		g_pPanel		= FindClass<IPanel>("vgui2.dll", "VGUI_Panel");
		g_pIVDebugOverlay = FindClass<IVDebugOverlay>("engine.dll", "VDebugOverlay");
		g_pModelInfo    = FindClass<IVModelInfo>("engine.dll", "VModelInfoClient");
		g_pModelRender  = FindClass<CModelRender>("engine.dll", "VEngineModel");
		g_pMaterialSys  = FindClass<IMaterialSystem>("materialsystem.dll", "VMaterialSystem");
		g_pInput = *(CInput * *)(Utils::FindSignature("client_panorama.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 0x1);
		g_pRenderView   = FindClass<IVRenderView>("engine.dll", "VEngineRenderView");
		g_pClientState = **(IClientState * **)(Utils::FindSignature("engine.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);
		g_pMemalloc = *reinterpret_cast<IMemAlloc * *>(GetProcAddress(GetModuleHandle("tier0.dll"), "g_pMemAlloc"));
		g_pViewRenderBeams = *reinterpret_cast<IViewRenderBeams**>((Utils::FindSignature("client_panorama.dll", "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 1));     // Get IViewRenderBeams
		//thanks monarch (from yeti)
		std::ofstream("csgo/materials/glowOverlay.vmt") << R"#("VertexLitGeneric" {
 
	"$additive" "1"
	"$envmap" "models/effects/cube_white"
	"$envmaptint" "[1 1 1]"
	"$envmapfresnel" "1"
	"$envmapfresnelminmaxexp" "[0 1 2]"
	"$alpha" "1"
})#";
		std::ofstream{ "csgo/materials/chamsAnimated.vmt" } <<
			"VertexLitGeneric { $envmap editor/cube_vertigo $envmapcontrast 1 $envmaptint \"[.7 .7 .7]\" $basetexture dev/zone_warning proxies { texturescroll { texturescrollvar $basetexturetransform texturescrollrate 0.6 texturescrollangle 0 } } }";
		std::ofstream("csgo\\materials\\FlatChams.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$no_fullbright" "0"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
		std::ofstream("csgo/materials/ShadedChams.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
      "$ignorez" "0"
      "$envmap" ""
      "$normalmapalphaenvmapmask" "1"
      "$envmapcontrast"  "1"
      "$nofog" "1"
      "$model" "1"
      "$nocull" "0"
      "$selfillum" "1"
      "$halflambert" "1"
      "$znearer" "0"
      "$flat" "1"
}
)#";

    }
}