#include "CloudGammingFramework.hh"
#include "Planet/Planet.hh"


static Planet myPlanet;

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool cgfRegisterGame(const char* gameName, GraphicApi type, InputCallback handler)
{
	// WNDPROC originalWndProcHandler = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)hWndProc);

    return myPlanet.Init(gameName, type, handler);
}

void cgfPollEvent(DispatchType type)
{
    myPlanet.PollEvent(type);
}

int cgfGetKeyStatus(Key key)
{
    return myPlanet.GetKeyStatus(key);
}

void cgfFinalize()
{
    myPlanet.Finalize();
}

void cgfSetResolution(unsigned int width, unsigned int height, unsigned char bpp)
{
    myPlanet.SetResolution(width, height, bpp);
}

void cgfSetFrame(const void* pData)
{
    myPlanet.SetFrame(pData);
}

bool cgfShouldExit()
{
    return myPlanet.ShouldExit();
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return S_OK;
}