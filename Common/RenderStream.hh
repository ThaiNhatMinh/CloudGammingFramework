#pragma once
#include <Windows.h>
#include "Timer.h"

class RenderStream
{
private:
    Timer m_timer;
    HWND m_hwnd;
    WNDPROC m_OriginalWndProcHandler = nullptr;
    bool m_ShowMenu;
public:
    RenderStream();
    ~RenderStream() = default;
    void InitImGui(HWND hwnd);
    void TickFrame();
    LRESULT hWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};