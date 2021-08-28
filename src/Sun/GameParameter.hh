#pragma once
#include <string>
#include "Define.hh"

struct GameParameter
{
    GameId Id;
    std::string ExecuteLocation;
    std::size_t RegisterTimeOut = 30 * 1000;
    std::size_t ClientDisconnectTimeOut = 30 * 1000;
};