#include <strsafe.h>
#include "pch.h"
#include "DirectX9.hh"
#include "Logger.hh"
#include "Module.hh"

typedef HRESULT(WINAPI* TDXGISwapChainPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
typedef HRESULT(WINAPI* TCreateDXGIFactory)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory2)(UINT Flags, REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TDXGICreateSwapChain)(IDXGIFactory* This, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);

PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pD3D11CreateDeviceAndSwapChain = NULL;
IDirect3DSwapChain9 *ppSwapChain = NULL;
TDXGISwapChainPresent pDXGISwapChainPresent = NULL;
TCreateDXGIFactory pCreateDXGIFactory = NULL;
TCreateDXGIFactory1 pCreateDXGIFactory1 = NULL;
TCreateDXGIFactory2 pCreateDXGIFactory2 = NULL;
TDXGICreateSwapChain pDXGICreateSwapChain = NULL;

void DumpValues();

DllExport HRESULT __stdcall hook_DXGISwapChainPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) {
    HRESULT hr = pDXGISwapChainPresent(This, SyncInterval, Flags);
    return hr;
}

void proc_hook_IDXGISwapChain_Present(IDXGISwapChain* ppSwapChain) {
    TRACE;
    uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)ppSwapChain;
    pDXGISwapChainPresent = (TDXGISwapChainPresent)pInterfaceVTable[8];

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID&)pDXGISwapChainPresent, hook_DXGISwapChainPresent);
    DetourTransactionCommit();
    DumpValues();
}

DllExport HRESULT WINAPI hook_D3D11CreateDeviceAndSwapChain(
    IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext) {
    TRACE;
    HRESULT hr = pD3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
    if (pDXGISwapChainPresent == NULL && pAdapter != NULL && ppSwapChain != NULL && ppDevice != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
    }
    return hr;
}

DllExport HRESULT __stdcall hook_DXGICreateSwapChain(IDXGIFactory* This, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) {
    TRACE;
    HRESULT hr = pDXGICreateSwapChain(This, pDevice, pDesc, ppSwapChain);
    if (pDXGISwapChainPresent == NULL && pDevice != NULL && ppSwapChain != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
    }
    return hr;
}

DllExport HRESULT __stdcall hook_CreateDXGIFactory(REFIID riid, void** ppFactory) {
    TRACE;
    HRESULT hr = pCreateDXGIFactory(riid, ppFactory);
    if (pDXGICreateSwapChain == NULL && riid == IID_IDXGIFactory && ppFactory != NULL) {

        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)*ppFactory;
        pDXGICreateSwapChain = (TDXGICreateSwapChain)pInterfaceVTable[10];
        // 10: IDXGIFactory::CreateSwapChain
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pDXGICreateSwapChain, hook_DXGICreateSwapChain);
        DetourTransactionCommit();
    }
    return hr;
}
DllExport HRESULT __stdcall hook_CreateDXGIFactory1(REFIID riid, void** ppFactory) {
    TRACE;
    HRESULT hr = pCreateDXGIFactory1(riid, ppFactory);
    if (pDXGICreateSwapChain == NULL && riid == IID_IDXGIFactory && ppFactory != NULL) {

        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)*ppFactory;
        pDXGICreateSwapChain = (TDXGICreateSwapChain)pInterfaceVTable[10];
        // 10: IDXGIFactory::CreateSwapChain
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pDXGICreateSwapChain, hook_DXGICreateSwapChain);
        DetourTransactionCommit();
    }
    return hr;
}

DllExport HRESULT WINAPI hook_CreateDXGIFactory2(UINT Flags,
    REFIID riid,
    void** ppFactory)
{
    TRACE;
    HRESULT hr = pCreateDXGIFactory2(Flags, riid, ppFactory);
    if (pDXGICreateSwapChain == NULL && riid == IID_IDXGIFactory && ppFactory != NULL) {

        uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)*ppFactory;
        pDXGICreateSwapChain = (TDXGICreateSwapChain)pInterfaceVTable[10];
        // 10: IDXGIFactory::CreateSwapChain
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(LPVOID&)pDXGICreateSwapChain, hook_DXGICreateSwapChain);
        DetourTransactionCommit();
    }
    return hr;
}

bool HookD3D11()
{
    HMODULE hMod = CheckModule("d3d11.dll");
    if (hMod == NULL) return false;
    pD3D11CreateDeviceAndSwapChain = GetProcAddress<PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN>(hMod, "D3D11CreateDeviceAndSwapChain");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pD3D11CreateDeviceAndSwapChain, hook_D3D11CreateDeviceAndSwapChain) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook DX11 done..." << std::endl;
    return true;
}

bool HookDXGI()
{
    HMODULE hMod = CheckModule("dxgi.dll");
    if (hMod == NULL) return false;
    pCreateDXGIFactory = GetProcAddress<TCreateDXGIFactory>(hMod, "CreateDXGIFactory");
    pCreateDXGIFactory1 = GetProcAddress<TCreateDXGIFactory1>(hMod, "CreateDXGIFactory1");
    pCreateDXGIFactory2 = GetProcAddress<TCreateDXGIFactory2>(hMod, "CreateDXGIFactory2");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pCreateDXGIFactory, hook_CreateDXGIFactory) << std::endl;
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pCreateDXGIFactory1, hook_CreateDXGIFactory1) << std::endl;
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pCreateDXGIFactory2, hook_CreateDXGIFactory2) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook DXGI done..." << std::endl;
    return true;
}

void DumpValues()
{
    LOG_API << "pD3D11CreateDeviceAndSwapChain: " << pD3D11CreateDeviceAndSwapChain << std::endl;
    LOG_API << "pCreateDXGIFactory: " << pCreateDXGIFactory << std::endl;
    LOG_API << "pCreateDXGIFactory1: " << pCreateDXGIFactory1 << std::endl;
    LOG_API << "pCreateDXGIFactory2: " << pCreateDXGIFactory2 << std::endl;
    LOG_API << "pDXGISwapChainPresent: " << pDXGISwapChainPresent << std::endl;
    LOG_API << "pDXGICreateSwapChain: " << pDXGICreateSwapChain << std::endl;
}

void HookDirectX()
{
    TRACE;
    while (true)
    {
        if (HookD3D9()) break;
        Sleep(100);
    }
    while (true)
    {
        if (HookD3D11()) break;
        Sleep(100);
    }
    while (true)
    {
        if (HookDXGI()) break;
        Sleep(100);
    }
}