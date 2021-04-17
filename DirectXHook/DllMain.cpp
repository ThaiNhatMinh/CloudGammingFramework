#include "pch.h"
#include "DllMain.h"
#include "DirectX9.hh"
#include "DirectX10.hh"
#include "DirectX11.hh"
#include "DXGI.hh"
#include "OpenGL.hh"
#include "RenderStream.hh"

RenderStream* streamer = nullptr;

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
    TRACE;
    ConsoleSetup();
    streamer = new RenderStream();
    HookD3D11();
    HookDXGI();
    HookD3D9();
    HookD3D10();
    HookOpenGL();
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
