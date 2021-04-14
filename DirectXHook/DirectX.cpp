#include "pch.h"
#include <strsafe.h>

typedef HRESULT(WINAPI* TDXGISwapChainPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
typedef HRESULT(WINAPI* TCreateDXGIFactory)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory1)(REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TCreateDXGIFactory2)(UINT Flags, REFIID riid, void** ppFactory);
typedef HRESULT(WINAPI* TDXGICreateSwapChain)(IDXGIFactory* This, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
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

PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pD3D11CreateDeviceAndSwapChain = NULL;
TDirect3DCreate9 pDirect3DCreate9 = NULL;
TD3D9CreateDevice pD3D9CreateDevice = NULL;
TD3D9DevicePresent pD3D9DevicePresent = NULL;
TD3D9SwapChainPresent pD3D9SwapChainPresent = NULL;

TD3D9GetSwapChain pD3D9GetSwapChain = NULL;
IDirect3DSwapChain9 *ppSwapChain = NULL;
TDXGISwapChainPresent pDXGISwapChainPresent = NULL;
TCreateDXGIFactory pCreateDXGIFactory = NULL;
TCreateDXGIFactory1 pCreateDXGIFactory1 = NULL;
TCreateDXGIFactory2 pCreateDXGIFactory2 = NULL;
TDXGICreateSwapChain pDXGICreateSwapChain = NULL;

void DumpValues();

void LastError()
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	LocalFree(lpMsgBuf);
	LOG << (char*)lpMsgBuf << std::endl;
}

HMODULE CheckModule(const char* module)
{
	HMODULE hMod = GetModuleHandle(module);
	if (hMod == NULL)
	{
		LOG << module <<  " not yet load" << std::endl;
		hMod = LoadLibrary(module);
		if (hMod == NULL)
		{
			LOG << module << " load failed" << std::endl;
			LastError();
		}
	}
	return hMod;
}
template<class T>
T GetProcAddress(HMODULE module, const char* funcName)
{
	T res = (T)::GetProcAddress(module, funcName);
	if (res == NULL)
	{
		LastError();
	}
	return res;
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

DllExport HRESULT __stdcall
hook_D3D9CreateDevice(
		IDirect3DDevice9 * This,
		UINT Adapter,
		D3DDEVTYPE DeviceType,
		HWND hFocusWindow,
		DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS *pPresentationParameters,
		IDirect3DDevice9 **ppReturnedDeviceInterface
	)
{
	TRACE;
	HRESULT hr = pD3D9CreateDevice(This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	if(FAILED(hr))
		return hr;

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
	MessageBox(NULL, "SDASD", "SADASD", MB_OK);
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