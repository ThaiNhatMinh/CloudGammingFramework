#include "pch.h"
#include "DllMain.h"


void ConsoleSetup()
{
    // With this trick we'll be able to print content to the console, and if we have luck we could get information printed by the game.
    AllocConsole();
    SetConsoleTitle("[+] Hooking DirectX 11");
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);
}


void MainThread(void* pHandle)
{
    ConsoleSetup();
    HookDirectX();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        LOG << "Detach process" << std::endl;
        break;
    }
    return TRUE;
}
