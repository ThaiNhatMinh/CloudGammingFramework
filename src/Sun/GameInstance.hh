#pragma once
#include "cgf/CloudGammingFramework.hh"
#include "Win32.hh"

struct GameInstance
{
    GameId Id;
    HANDLE ProcessHandle;
};