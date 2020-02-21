#include <thread>
#include "Hooks.h"
#include "Utils.h"
#include "GlobalVars.h"

#pragma once
#define Example 275118122
#define jarves -923648772
#define ingame 902659832
#define v1g1l 1210409540
#define faithb -763983631
#define beri 72041803
#define sayan 753618087
#define case 107790432
#define gunner -862385588
#define desikiller -1972464766
UCHAR szFileSys[255], szVolNameBuff[255];
DWORD dwMFL, dwSysFlags;
DWORD dwSerial;
LPCTSTR szHD = "C:\\";
HINSTANCE HThisModule;

int OnDllAttach()
{
	interfaces::Init();
    Hooks::Init();
	Sleep(500);
	while (!GetAsyncKeyState(VK_END))
		Sleep(500);

	Hooks::Restore();

	FreeLibraryAndExitThread(HThisModule, 1);
	return 0;
	
}


auto __stdcall DllMain(HMODULE module, DWORD reason_for_call, LPVOID reserved) -> bool
{
	if (reason_for_call == DLL_PROCESS_ATTACH && GetModuleHandleA("csgo.exe")) {
		//DisableThreadLibraryCalls(module);
		CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(OnDllAttach), module, NULL, NULL);
	} return TRUE;
}

