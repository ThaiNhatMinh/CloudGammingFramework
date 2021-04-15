#include "pch.h"

#include "DirectX9.hh"
#include "Logger.hh"
#include "Module.hh"
#include "Timer.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui.h"

typedef IDirect3D9 * (WINAPI* TDirect3DCreate9)(UINT SDKVersion);
typedef HRESULT (STDMETHODCALLTYPE *TD3D9CreateDevice)(
    IDirect3DDevice9 * This,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice9 **ppReturnedDeviceInterface
);

typedef HRESULT (STDMETHODCALLTYPE *TD3D9DevicePresent)(
    IDirect3DDevice9 * This,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion
);

typedef HRESULT (STDMETHODCALLTYPE *TD3D9SwapChainPresent)(
    IDirect3DSwapChain9 * This,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion,
    DWORD dwFlags
);

typedef HRESULT (STDMETHODCALLTYPE *TD3D9GetSwapChain)(
    IDirect3DDevice9 *This,
    UINT iSwapChain,
    IDirect3DSwapChain9 **ppSwapChain
);

TDirect3DCreate9 pDirect3DCreate9 = NULL;
TD3D9CreateDevice pD3D9CreateDevice = NULL;
TD3D9DevicePresent pD3D9DevicePresent = NULL;
TD3D9SwapChainPresent pD3D9SwapChainPresent = NULL;
TD3D9GetSwapChain pD3D9GetSwapChain = NULL;
Timer timer;
WNDPROC OriginalWndProcHandler = nullptr;
bool g_ShowMenu = false;

DllExport IDirect3D9 * WINAPI hook_Direct3DCreate9(UINT SDKVersion);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool HookD3D9()
{
    HMODULE hMod = CheckModule("d3d9.dll");
    if (hMod == NULL) return false;
    pDirect3DCreate9 = GetProcAddress<TDirect3DCreate9>(hMod, "Direct3DCreate9");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pDirect3DCreate9, hook_Direct3DCreate9) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook D3D9 done..." << std::endl;
    return true;
}


LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT mPos;
    GetCursorPos(&mPos);
    ScreenToClient(hWnd, &mPos);
    ImGui::GetIO().MousePos.x = mPos.x;
    ImGui::GetIO().MousePos.y = mPos.y;

    if (uMsg == WM_KEYUP)
    {
        if (wParam == VK_INSERT)
        {
            g_ShowMenu = !g_ShowMenu;
        }

    }

    if (g_ShowMenu)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        return true;
    }
    return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

HRESULT STDMETHODCALLTYPE hook_D3D9SwapChainPresent(
    IDirect3DSwapChain9 * This,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion,
    DWORD dwFlags
)
{
    timer.Tick();
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    //Menu is displayed when g_ShowMenu is TRUE
    if (g_ShowMenu)
    {
        ImGui::Begin("Hello, world!"); 
        ImGui::Text("FPS: %d, frame time: %f", timer.GetFPS(), timer.GetDeltaTime());
        ImGui::End();
    }
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    HRESULT hr = pD3D9SwapChainPresent(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    // If (In frame to capture frame)
    //     CaptureFrame()
    return hr;
}

HRESULT STDMETHODCALLTYPE hook_D3D9DevicePresent(
    IDirect3DDevice9 * This,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion
)
{
    HRESULT hr = pD3D9DevicePresent(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    // If (In frame to capture frame)
    //     CaptureFrame()
    return hr;
}

HRESULT STDMETHODCALLTYPE hook_D3D9GetSwapChain(
    IDirect3DDevice9 *This,
    UINT iSwapChain,
    IDirect3DSwapChain9 **ppSwapChain
)
{
    TRACE;
    HRESULT hr = pD3D9GetSwapChain(This, iSwapChain, ppSwapChain);
    ImGui_ImplDX9_Init(This);
    if (ppSwapChain != NULL && pD3D9SwapChainPresent == NULL)
    {
        IDirect3DSwapChain9 *pIDirect3DSwapChain9 = *ppSwapChain;
        // IDirect3dSwapChain9
        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)pIDirect3DSwapChain9;
        // IDirect3DSwapChain9::Present
        uintptr_t* ppSwapChainPresent = (uintptr_t*)pInterfaceVTable[3];
        pD3D9SwapChainPresent = (TD3D9SwapChainPresent) ppSwapChainPresent;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pD3D9SwapChainPresent, hook_D3D9SwapChainPresent);
        DetourTransactionCommit();
    }
    return hr;
}

DllExport HRESULT __stdcall hook_D3D9CreateDevice(
    IDirect3DDevice9 *This,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice9 **ppReturnedDeviceInterface)
{
    TRACE;
    HRESULT hr = pD3D9CreateDevice(This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    if(FAILED(hr))
        return hr;
    OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(hFocusWindow, GWLP_WNDPROC, (LONG_PTR)hWndProc);
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hFocusWindow);
    // auto f = io.Fonts->AddFontFromFileTTF("D:\\Code\\imgui\\misc\\fonts\\ProggyClean.ttf", 16);
    // LOG << "Font: " << f << std::endl;
    ImGui::GetIO().ImeWindowHandle = hFocusWindow;
    if (pD3D9DevicePresent == NULL) {
        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)*ppReturnedDeviceInterface;
        // 14: IDirect3DDevice9::GetSwapChain,  17: IDirect3DDevice9::Present
        // 41: IDirect3DDevice9::BeginScene,    42: IDirect3DDevice9::EndScene
        pD3D9GetSwapChain = (TD3D9GetSwapChain)pInterfaceVTable[14];
        pD3D9DevicePresent = (TD3D9DevicePresent)pInterfaceVTable[17];

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pD3D9GetSwapChain, hook_D3D9GetSwapChain);
        DetourAttach(&(LPVOID&)pD3D9DevicePresent, hook_D3D9DevicePresent);
        DetourTransactionCommit();
    }
    return hr;
}

DllExport IDirect3D9 * WINAPI hook_Direct3DCreate9(UINT SDKVersion)
{
    TRACE;
    IDirect3D9 * pDirect3D9 = pDirect3DCreate9(SDKVersion);
    if (pD3D9CreateDevice == NULL) {
        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)pDirect3D9;
        // IDirect3D9::CreateDevice()
        pD3D9CreateDevice = (TD3D9CreateDevice)pInterfaceVTable[16];
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pD3D9CreateDevice, hook_D3D9CreateDevice);
        DetourTransactionCommit();
    }
    return pDirect3D9;
}