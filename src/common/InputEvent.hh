#pragma once
#include <Windows.h>

struct InputEvent
{
    struct Win32
    {
        UINT msg;
        WPARAM wParam;
        LPARAM lParam;
    };
    union
    {
        Win32 win32;
        // Glfw glfw;
    };
};