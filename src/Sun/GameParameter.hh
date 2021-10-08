#pragma once
#include <string>
#include "cgf/CloudGammingFramework.hh"

struct GameParameter
{
    GameId Id;
    std::string ExecuteLocation;
    uint32_t RegisterTimeOut = 30 * 1000;
    uint32_t ClientDisconnectTimeOut = 30 * 1000;
};