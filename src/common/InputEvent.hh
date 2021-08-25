#pragma once
#include "Win32.hh"

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