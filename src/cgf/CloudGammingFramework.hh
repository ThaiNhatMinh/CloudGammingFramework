#pragma once

enum GraphicApi
{
    DIRECTX_9,
    DIRECTX_10,
    DIRECTX_11,
    DIRECTX_12,
    OPENGL,
    VULKAN,
};

typedef unsigned long long MY_WPARAM;
typedef unsigned long long MY_LPARAM;
typedef void* MY_HWND;
typedef void (* cgfCursorposfun)(double xpos,double ypos);
typedef void (* cgfMousebuttonfun)(int button, int action, int mods);
typedef void (* WndProcHandler)(unsigned int msg, MY_WPARAM wParam, MY_LPARAM lParam);

bool cgfRegisterGame(const char* gameName, GraphicApi type, MY_HWND hWnd, WndProcHandler handler);
void cgfPollEvent();
