#pragma once
#include "cgf/CloudGammingFramework.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/FileMapping.hh"
#include "Win32.hh"

struct GameInstance
{
    GameId Id = INVALID_GAMEID;
    HANDLE ProcessHandle = INVALID_HANDLE_VALUE;
    ClientId clientId = INVALID_CLIENTID;
    // Share memory between Sun and Planet
    FileMapping info;
};