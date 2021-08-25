#pragma once
#include <string>
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"

typedef unsigned int StreamPort;

/**
 * Place that control everything
 */
class Sun {
public:
    typedef unsigned int GameId;
    typedef struct
    {
        StreamPort port;
    }GameRegister;

private:
    Event m_launchGame;
    FileMapping m_launchData;

public:
    Sun();
    bool LaunchGame(GameId id);
};