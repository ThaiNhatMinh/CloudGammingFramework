#pragma once
#include "cgf/CloudGammingFramework.hh"
#include "ipc/WsaSocket.hh"
#include "Win32.hh"

struct GameInstance
{
    GameId Id = INVALID_GAMEID;
    HANDLE ProcessHandle = INVALID_HANDLE_VALUE;
    GameStatus status;
    ClientId clientId = INVALID_CLIENTID;
};