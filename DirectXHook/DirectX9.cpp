#include "pch.h"

#include "DirectX9.hh"
#include "Logger.hh"
#include "Module.hh"
#include "RenderStream.hh"
#include "Timer.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
IDirect3DSwapChain9 *pSwapChain = NULL;
extern RenderStream* streamer;
void CaptureFrame(IDirect3DSwapChain9* swapchain);

DllExport IDirect3D9 * WINAPI hook_Direct3DCreate9(UINT SDKVersion);

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

HRESULT STDMETHODCALLTYPE hook_D3D9SwapChainPresent(
    IDirect3DSwapChain9 * This,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion,
    DWORD dwFlags
)
{
    ImGui_ImplDX9_NewFrame();
    streamer->TickFrame();
    CaptureFrame(This);
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
    IDirect3DSurface9 *renderSurface;
    HRESULT hr1 = This->GetRenderTarget(0, &renderSurface);
    if (hr1 == D3D_OK)
    {
        D3DSURFACE_DESC desc;
        renderSurface->GetDesc(&desc);
        streamer->SetFrameSize(desc.Width, desc.Width);
    }

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

    streamer->InitImGui(hFocusWindow);

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

static IDirect3DSurface9 *resolvedSurface = NULL;
static IDirect3DSurface9 *offscreenSurface = NULL;

void CaptureFrame(IDirect3DSwapChain9* swapchain)
{
    return;
    static bool called = false;
    if (called) return;
    called = true;
    IDirect3DDevice9* pDevice;
    IDirect3DSurface9 *renderSurface, *oldRenderSurface;
    D3DLOCKED_RECT lockedRect;
    D3DSURFACE_DESC desc;

    swapchain->GetDevice(&pDevice);
    HRESULT hr = pDevice->GetRenderTarget(0, &renderSurface);
    if (FAILED(hr))
    {
        LOG << hr << std::endl;
        return;
    }
    renderSurface->GetDesc(&desc);
    if (desc.MultiSampleType != D3DMULTISAMPLE_NONE)
    {
        if (resolvedSurface == NULL)
        {
            hr = pDevice->CreateRenderTarget(desc.Width, desc.Height,
                                             desc.Format,
                                             D3DMULTISAMPLE_NONE,
                                             0,     // non multisampleQuality
                                             FALSE, // lockable
                                             &resolvedSurface, NULL);
            if (FAILED(hr))
            {
                LOG << hr << std::endl;
            }
        }

        hr = pDevice->StretchRect(renderSurface, NULL,
                                  resolvedSurface, NULL, D3DTEXF_NONE);
        if (FAILED(hr))
        {
            LOG << hr << std::endl;
        }

        oldRenderSurface = renderSurface;
        renderSurface = resolvedSurface;
    }
    LOG << "desc.Format: " << desc.Format << std::endl;
    LOG << "W: " << desc.Width << " H: " << desc.Height << std::endl;
    LOG << "Multisample: " << desc.MultiSampleType << std::endl;
    // create offline surface in system memory
    if(offscreenSurface == NULL) {
        hr = pDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
                desc.Format,
                D3DPOOL_SYSTEMMEM,
                &offscreenSurface, NULL);
        if (FAILED(hr)) {
            LOG << "Create offscreen surface failed.\n";
            return;
        }
    }
    // copy the render-target data from device memory to system memory
    hr = pDevice->GetRenderTargetData(renderSurface, offscreenSurface);

    if (FAILED(hr)) {
        LOG << "GetRenderTargetData failed.\n";
        if(oldRenderSurface)
            oldRenderSurface->Release();
        else
            renderSurface->Release();
        return;
    }

    if(oldRenderSurface)
        oldRenderSurface->Release();
    else
        renderSurface->Release();

    // start to lock screen from offline surface
    hr = offscreenSurface->LockRect(&lockedRect, NULL, NULL);
    if (FAILED(hr)) {
        LOG << "LockRect failed.\n";
        return;
    }
    LOG << "lockedRect: " << lockedRect.Pitch << std::endl;
    void Convertxrgb2rgb(char* src, char* dst, int width, int height);
    Convertxrgb2rgb((char*)lockedRect.pBits, streamer->GetBuffer(),  desc.Width, desc.Height);
    streamer->SendFrame();
    hr = offscreenSurface->UnlockRect();
}

void Convertxrgb2rgb(char* src, char* dst, int width, int height)
{
    // src += 1;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++, src += 4, dst += 3)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
    }
}