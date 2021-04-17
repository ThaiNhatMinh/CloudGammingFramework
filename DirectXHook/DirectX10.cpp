

#include "framework.h"
#include "imgui/imgui_impl_dx10.h"

typedef HRESULT (WINAPI *TD3D10CreateDeviceAndSwapChain)(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device **ppDevice
);

typedef HRESULT (WINAPI *TD3D10CreateDeviceAndSwapChain1)(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    D3D10_FEATURE_LEVEL1 HardwareLevel,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device1 **ppDevice
);

DllExport HRESULT WINAPI hook_D3D10CreateDeviceAndSwapChain(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device **ppDevice);
DllExport HRESULT WINAPI hook_D3D10CreateDeviceAndSwapChain1(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    D3D10_FEATURE_LEVEL1 HardwareLevel,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device1 **ppDevice
);

extern void proc_hook_IDXGISwapChain_Present(IDXGISwapChain* ppSwapChain);
bool HookD3D10_1();
TD3D10CreateDeviceAndSwapChain pD3D10CreateDeviceAndSwapChain = NULL;
TD3D10CreateDeviceAndSwapChain1 pD3D10CreateDeviceAndSwapChain1 = NULL;

bool HookD3D10()
{
    HMODULE hMod = CheckModule("d3d10.dll");
    if (hMod == NULL) return false;
    pD3D10CreateDeviceAndSwapChain = GetProcAddress<TD3D10CreateDeviceAndSwapChain>(hMod, "D3D10CreateDeviceAndSwapChain");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID &)pD3D10CreateDeviceAndSwapChain, hook_D3D10CreateDeviceAndSwapChain) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook D3D10 done..." << std::endl;
    return HookD3D10_1();
    // return true;
}

bool HookD3D10_1()
{
    HMODULE hMod = CheckModule("d3d10_1.dll");
    if (hMod == NULL) return false;
    pD3D10CreateDeviceAndSwapChain1 = GetProcAddress<TD3D10CreateDeviceAndSwapChain1>(hMod, "D3D10CreateDeviceAndSwapChain1");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID &)pD3D10CreateDeviceAndSwapChain1, hook_D3D10CreateDeviceAndSwapChain1) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook D3D10_1 done..." << std::endl;
    return true;
}


DllExport HRESULT WINAPI hook_D3D10CreateDeviceAndSwapChain(
    IDXGIAdapter *pAdapter,
    D3D10_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    UINT SDKVersion,
    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
    IDXGISwapChain **ppSwapChain,
    ID3D10Device **ppDevice)
{
    TRACE;
    HRESULT hr = pD3D10CreateDeviceAndSwapChain(pAdapter, DriverType, Software, 
            Flags, SDKVersion, pSwapChainDesc, 
            ppSwapChain, ppDevice);
    LOG << "DirectX10....";
    SetImGui(ImGui_ImplDX10_NewFrame);
    if (!IsDXGIInit() && pAdapter != NULL && ppSwapChain != NULL && ppDevice != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
        ImGui_ImplDX10_Init(*ppDevice);
    }
    return hr;
}

// Hook function that replaces the D3D10CreateDeviceAndSwapChain1() API
DllExport HRESULT WINAPI hook_D3D10CreateDeviceAndSwapChain1(
        IDXGIAdapter *pAdapter,
        D3D10_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        D3D10_FEATURE_LEVEL1 HardwareLevel,
        UINT SDKVersion,
        DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
        IDXGISwapChain **ppSwapChain,
        ID3D10Device1 **ppDevice
    )
{
    HRESULT hr = pD3D10CreateDeviceAndSwapChain1(pAdapter, DriverType, Software, 
                Flags, HardwareLevel, SDKVersion, 
                pSwapChainDesc, ppSwapChain, ppDevice);

    LOG << "DirectX10_1....";
    SetImGui(ImGui_ImplDX10_NewFrame);
    if (!IsDXGIInit() && pAdapter != NULL && ppSwapChain != NULL && ppDevice != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
    }
    return hr;
}