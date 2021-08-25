#pragma once

#include <windows.h>

enum GraphicApi
{
    DIRECTX_9,
    DIRECTX_10,
    DIRECTX_11,
    DIRECTX_12,
    OPENGL,
    VULKAN,
};

typedef void (* cgfCursorposfun)(double xpos,double ypos);
typedef void (* cgfMousebuttonfun)(int button, int action, int mods);
typedef void (* WndProcHandler)(UINT msg, WPARAM wParam, LPARAM lParam);

bool cgfRegisterGame(const char* gameName, GraphicApi type, HWND hWnd, WndProcHandler handler);
void cgfPollEvent();
