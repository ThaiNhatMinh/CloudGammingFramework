#include "framework.h"

typedef HRESULT(WINAPI* TCreateDXGIFactory)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory2)(UINT Flags, REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TDXGICreateSwapChain)(IDXGIFactory* This, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
typedef HRESULT(WINAPI* TDXGISwapChainPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
DllExport void proc_hook_IDXGISwapChain_Present(IDXGISwapChain* ppSwapChain);

TDXGISwapChainPresent pDXGISwapChainPresent = NULL;
TDXGICreateSwapChain pDXGICreateSwapChain = NULL;
TCreateDXGIFactory pCreateDXGIFactory = NULL;
TCreateDXGIFactory1 pCreateDXGIFactory1 = NULL;
TCreateDXGIFactory2 pCreateDXGIFactory2 = NULL;
static void (*ImGui_NewFrameFunc)() = NULL;

DllExport HRESULT WINAPI hook_DXGICreateSwapChain(IDXGIFactory* This, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain) {
    TRACE;
    HRESULT hr = pDXGICreateSwapChain(This, pDevice, pDesc, ppSwapChain);
    if (pDXGISwapChainPresent == NULL && pDevice != NULL && ppSwapChain != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
    }
    return hr;
}

DllExport HRESULT WINAPI hook_CreateDXGIFactory(REFIID riid, void** ppFactory) {
    TRACE;
    HRESULT hr = pCreateDXGIFactory(riid, ppFactory);
    if (pDXGICreateSwapChain == NULL && riid == IID_IDXGIFactory && ppFactory != NULL)
    {
        LOG << "Hook hook_DXGICreateSwapChain\n";
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

DllExport HRESULT WINAPI hook_CreateDXGIFactory1(REFIID riid, void** ppFactory) {
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

DllExport HRESULT __stdcall hook_DXGISwapChainPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) {
    ImGui_NewFrameFunc();
    streamer->TickFrame();
    HRESULT hr = pDXGISwapChainPresent(This, SyncInterval, Flags);
    return hr;
}

DllExport void proc_hook_IDXGISwapChain_Present(IDXGISwapChain* ppSwapChain) {
    TRACE;
    uintptr_t* pInterfaceVTable = (uintptr_t*)*(uintptr_t*)ppSwapChain;
    pDXGISwapChainPresent = (TDXGISwapChainPresent)pInterfaceVTable[8];
    DXGI_SWAP_CHAIN_DESC desc;
    ppSwapChain->GetDesc(&desc);
    streamer->InitImGui(desc.OutputWindow);
    LOG << "W: " << desc.BufferDesc.Width << " H: " << desc.BufferDesc.Height << std::endl;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID&)pDXGISwapChainPresent, hook_DXGISwapChainPresent);
    DetourTransactionCommit();
}

bool IsDXGIInit()
{
    return pDXGISwapChainPresent != NULL;
}

void SetImGui(void (*func)())
{
    ImGui_NewFrameFunc = func;
}