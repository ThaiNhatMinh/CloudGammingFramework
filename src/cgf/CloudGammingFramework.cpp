#include "api/CloudGammingFramework.hh"
#include "Planet/Planet.hh"


static Planet myPlanet;

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool cgfRegisterGame(const char* gameName, GraphicApi type, HWND hWnd, WndProcHandler handler)
{
	WNDPROC originalWndProcHandler = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)hWndProc);

    return myPlanet.Init(gameName, type, handler);
}

void cgfPollEvent()
{
    myPlanet.PollEvent();
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return S_OK;
}