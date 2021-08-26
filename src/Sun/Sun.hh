#pragma once
#include <map>
#include <string>
#include <vector>
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "common/Configuration.hh"
#include "Define.hh"
#include "GameInstance.hh"

struct GameParameter;

/**
 * Place that control everything
 */
class Sun {
public:
    typedef struct
    {
        StreamPort port;
    }GameRegister;

private:
    Event m_launchGame;
    FileMapping m_launchData;
    std::vector<GameParameter> m_gameDb;
    Configuration* m_pConfig;
    std::map<StreamPort, GameInstance> m_gameInstances;

public:
    Sun(Configuration* config, const std::vector<GameParameter>& gamedb);
    bool LaunchGame(GameId id);

private:
    const GameParameter* FindGame(GameId id);
    StreamPort FindFreePort();
};