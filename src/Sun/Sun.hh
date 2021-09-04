#pragma once
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "ipc/WsaSocket.hh"
#include "common/Configuration.hh"
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
    std::mutex m_lockLaunchGame;
    Event m_launchGame;
    FileMapping m_launchData;
    std::vector<GameParameter> m_gameDb;
    Configuration* m_pConfig;
    std::map<StreamPort, GameInstance> m_gameInstances;
    WsaSocket m_socket;
    std::list<std::pair<WsaSocket, ClientId>> m_clients;
public:
    Sun(Configuration* config, const std::vector<GameParameter>& gamedb);
    ~Sun();

private:
    const GameParameter* FindGame(GameId id);
    void OnAccept(WsaSocket&& newConnect) override;
    void OnClose(WsaSocketInformation* sock) override;
    void OnRecvFromClient(WsaSocketInformation* sock);
    void LaunchGame(const WsaSocket* client, GameId id);
    StreamPort LaunchGame(GameId id);
    StreamPort FindFreePort();
    StreamPort FindExistRunningGame(ClientId id);
    ClientId FindClient(const WsaSocket* client);
    void StartWaitingForClient();
};