#pragma once
#include "Define.hh"
#include "Win32.hh"

struct GameInstance
{
    GameId Id;
    HANDLE ProcessHandle;
};