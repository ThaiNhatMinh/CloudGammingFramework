#pragma once
#include "Win32.hh"
#include "cgf/CloudGammingFramework.hh"

struct InputEvent
{
    enum EventType
    {
        KEY,
        MOUSE_MOVE,
        MOUSE_ACTION
    };
    struct KeyEvent
    {
       Action action;
       Key key;
    };

    struct MousePosEvent
    {
        float x;
        float y;
    };

    struct MouseActionEvent
    {
        Action action;
        int key;
    };

    EventType type;
    union
    {
        KeyEvent key;
        MousePosEvent mousePos;
        MouseActionEvent mouseAction;
    };
};