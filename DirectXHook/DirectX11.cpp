#include <strsafe.h>
#include "pch.h"
#include "DXGI.hh"
#include "Logger.hh"
#include "Module.hh"
#include "RenderStream.hh"
#include "imgui/imgui_impl_dx11.h"


PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pD3D11CreateDeviceAndSwapChain = NULL;
PFN_D3D11_CREATE_DEVICE pD3D11CreateDevice = NULL;

extern void proc_hook_IDXGISwapChain_Present(IDXGISwapChain* ppSwapChain);

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
    LOG << "pAdapter: " << pAdapter << "|ppSwapChain: " << "|ppDevice: " << ppDevice << std::endl;
    LOG << "DirectX11...." << std::endl;
    SetImGui(ImGui_ImplDX11_NewFrame);
    if (!IsDXGIInit() && pAdapter != NULL && ppSwapChain != NULL && ppDevice != NULL) {
        proc_hook_IDXGISwapChain_Present(*ppSwapChain);
        ImGui_ImplDX11_Init(*ppDevice, *ppImmediateContext);
    }
    return hr;
}
DllExport HRESULT WINAPI hook_D3D11CreateDevice(
    _In_opt_ IDXGIAdapter *pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    CONST D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    ID3D11Device **ppDevice,
    D3D_FEATURE_LEVEL *pFeatureLevel,
    ID3D11DeviceContext **ppImmediateContext)
{
    TRACE;
    return pD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}

bool HookD3D11()
{
    HMODULE hMod = CheckModule("d3d11.dll");
    if (hMod == NULL) return false;
    pD3D11CreateDeviceAndSwapChain = GetProcAddress<PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN>(hMod, "D3D11CreateDeviceAndSwapChain");
    pD3D11CreateDevice = GetProcAddress<PFN_D3D11_CREATE_DEVICE>(hMod, "D3D11CreateDevice");
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pD3D11CreateDeviceAndSwapChain, hook_D3D11CreateDeviceAndSwapChain) << std::endl;
    LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pD3D11CreateDevice, hook_D3D11CreateDevice) << std::endl;
    DetourTransactionCommit();
    LOG << "Hook DX11 done..." << std::endl;
    return true;
}

// typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// typedef void(__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
// typedef void(__stdcall *D3D11CreateQueryHook) (ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery);
// typedef void(__stdcall *D3D11PSSetShaderResourcesHook) (ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews);
// typedef void(__stdcall *D3D11ClearRenderTargetViewHook) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4]);



// D3D11PresentHook                phookD3D11Present = nullptr;
// D3D11DrawIndexedHook            phookD3D11DrawIndexed = nullptr;
// D3D11CreateQueryHook			phookD3D11CreateQuery = nullptr;
// D3D11PSSetShaderResourcesHook	phookD3D11PSSetShaderResources = nullptr;
// D3D11ClearRenderTargetViewHook  phookD3D11ClearRenderTargetViewHook = nullptr;

// DWORD_PTR*                         pSwapChainVTable = nullptr;
// DWORD_PTR*						   pDeviceVTable = nullptr;
// DWORD_PTR*                         pDeviceContextVTable = nullptr;
// static ID3D11Device*            g_pd3dDevice = nullptr;
// static ID3D11DeviceContext*     g_pd3dContext = nullptr;
// static IDXGISwapChain*          g_pSwapChain = nullptr;

// HRESULT __stdcall PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
// {
// 	TRACE;
// 	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
// }

// DWORD __stdcall HookDX11_Init()
// {
//     WNDCLASSEX windowClass;
//     windowClass.cbSize = sizeof(WNDCLASSEX);
//     windowClass.style = CS_HREDRAW | CS_VREDRAW;
//     windowClass.lpfnWndProc = DefWindowProc;
//     windowClass.cbClsExtra = 0;
//     windowClass.cbWndExtra = 0;
//     windowClass.hInstance = GetModuleHandle(NULL);
//     windowClass.hIcon = NULL;
//     windowClass.hCursor = NULL;
//     windowClass.hbrBackground = NULL;
//     windowClass.lpszMenuName = NULL;
//     windowClass.lpszClassName = "Kiero";
//     windowClass.hIconSm = NULL;

//     ::RegisterClassEx(&windowClass);
//     HWND window = ::CreateWindow(windowClass.lpszClassName, "Kiero DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

//     D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
// 	D3D_FEATURE_LEVEL obtainedLevel;
// 	DXGI_SWAP_CHAIN_DESC sd;
// 	{
// 		ZeroMemory(&sd, sizeof(sd));
// 		sd.BufferCount = 1;
// 		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
// 		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
// 		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
// 		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
// 		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
// 		sd.OutputWindow = window;
// 		sd.SampleDesc.Count = 1;
// 		sd.Windowed = ((GetWindowLongPtr(window, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
// 		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
// 		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
// 		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

// 		sd.BufferDesc.Width = 1;
// 		sd.BufferDesc.Height = 1;
// 		sd.BufferDesc.RefreshRate.Numerator = 0;
// 		sd.BufferDesc.RefreshRate.Denominator = 1;
// 	}

// 	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &obtainedLevel, &g_pd3dContext);
// 	if (FAILED(hr))
// 	{
// 		MessageBox(window, "Failed to create device and swapchain.", "Fatal Error", MB_ICONERROR);
// 		return E_FAIL;
// 	}

// 	pSwapChainVTable = (DWORD_PTR*)(g_pSwapChain);
// 	pSwapChainVTable = (DWORD_PTR*)(pSwapChainVTable[0]);

// 	pDeviceVTable = (DWORD_PTR*)(g_pd3dDevice);
// 	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

// 	pDeviceContextVTable = (DWORD_PTR*)(g_pd3dContext);
// 	pDeviceContextVTable = (DWORD_PTR*)(pDeviceContextVTable[0]);
//     proc_hook_IDXGISwapChain_Present(g_pSwapChain);
// 	// DetourTransactionBegin();
//     // DetourUpdateThread(GetCurrentThread());
//     // LOG << "DetourAttach:" << DetourAttach(&(LPVOID&)pSwapChainVTable[8], PresentHook) << std::endl;
//     // DetourTransactionCommit();
// 	// if (MH_CreateHook((DWORD_PTR*)pSwapChainVTable[8], PresentHook, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK) { return 1; }
// 	// if (MH_EnableHook((DWORD_PTR*)pSwapChainVTable[8]) != MH_OK) { return 1; }
// 	// if (MH_CreateHook((DWORD_PTR*)pDeviceContextVTable[12], DrawIndexedHook, reinterpret_cast<void**>(&phookD3D11DrawIndexed)) != MH_OK) { return 1; }
// 	// if (MH_EnableHook((DWORD_PTR*)pDeviceContextVTable[12]) != MH_OK) { return 1; }
// 	// if (MH_CreateHook((DWORD_PTR*)pDeviceVTable[24], hookD3D11CreateQuery, reinterpret_cast<void**>(&phookD3D11CreateQuery)) != MH_OK) { return 1; }
// 	// if (MH_EnableHook((DWORD_PTR*)pDeviceVTable[24]) != MH_OK) { return 1; }
// 	// if (MH_CreateHook((DWORD_PTR*)pDeviceContextVTable[8], hookD3D11PSSetShaderResources, reinterpret_cast<void**>(&phookD3D11PSSetShaderResources)) != MH_OK) { return 1; }
// 	// if (MH_EnableHook((DWORD_PTR*)pDeviceContextVTable[8]) != MH_OK) { return 1; }
// 	// if (MH_CreateHook((DWORD_PTR*)pSwapChainVTable[50], ClearRenderTargetViewHook, reinterpret_cast<void**>(&phookD3D11ClearRenderTargetViewHook)) != MH_OK) { return 1; }
// 	// if (MH_EnableHook((DWORD_PTR*)pSwapChainVTable[50]) != MH_OK) { return 1; }

// // 	DWORD old_protect;
// // 	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &old_protect);
// // const int UninjectLibraryKey = VK_DELETE;

// // 	do {
// // 		Sleep(100);
// // 	} while (!(GetAsyncKeyState(UninjectLibraryKey) & 0x1));

// // 	// g_pd3dDevice->Release();
// // 	// g_pd3dContext->Release();
// // 	// g_pSwapChain->Release();

// // 	Beep(220, 100);

// 	return S_OK;
// }