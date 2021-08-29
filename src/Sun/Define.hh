#pragma once
typedef unsigned int StreamPort;


const StreamPort INVALID_PORT = 0;

enum Command
{
    START_GAME,
    STOP_GAME,
    RECONNECT_GAME
};