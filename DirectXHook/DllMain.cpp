#include "pch.h"
#include <vector>

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

typedef HRESULT (STDMETHODCALLTYPE *TGetRawInputData) (
	HRAWINPUT hRawInput,
	UINT uiCommand,
	LPVOID pData,
	PUINT pcbSize,
	UINT cbSizeHeader
);
TGetRawInputData pGetRawInputData = NULL;
// Hook function that replaces the GetRawInputData API
DllExport HRESULT __stdcall hook_GetRawInputData(
		HRAWINPUT hRawInput,
		UINT uiCommand,
		LPVOID pData,
		PUINT pcbSize,
		UINT cbSizeHeader
	)
{
    TRACE;
    HRESULT hr;
    hr = pGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
    return hr;
}

void GetRawInputDevice(bool is_mame_game);

void HookInput()
{
    HMODULE hMod = CheckModule("user32.dll");
    if (hMod == NULL)
        return;
    pGetRawInputData = (TGetRawInputData)
        GetProcAddress(hMod, "GetRawInputData");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID &)pGetRawInputData, hook_GetRawInputData);
    DetourTransactionCommit();
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
    HookInput();
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
