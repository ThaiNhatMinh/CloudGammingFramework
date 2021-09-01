#pragma once
#include <list>
#include <map>
#include <string>
#include <vector>
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "ipc/WsaSocket.hh"
#include "common/Configuration.hh"
#include "Define.hh"
#include "GameInstance.hh"

struct GameParameter;

/**
 * Place that control everything
 */
class Sun : public WsaSocketPollEvent {
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
    WsaSocket m_socket;
    std::list<WsaSocket> m_clients;
public:
    Sun(Configuration* config, const std::vector<GameParameter>& gamedb);

private:
    const GameParameter* FindGame(GameId id);
    void OnAccept(WsaSocket&& newConnect) override;
    void OnRecvFromClient(WsaSocketInformation* sock);
    void LaunchGame(const WsaSocket* client, GameId id);
    StreamPort LaunchGame(GameId id);
    StreamPort FindFreePort();
    StreamPort FindExistRunningGame();
};