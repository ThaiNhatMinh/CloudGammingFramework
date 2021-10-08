#pragma once
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "ipc/PollHandle.hh"
#include "ipc/WsaSocket.hh"
#include "common/Configuration.hh"
#include "GameInstance.hh"

struct GameParameter;

/**
 * Place that control everything
 */
class Sun
{
public:
    typedef struct
    {
        GameId Id;
        ClientId clientId;
        StreamPort port;
        uint32_t DisconnectTimeout;
        GameStatus Status;
    } GameRegister;

private:
    std::mutex m_lockLaunchGame;
    Event m_launchGame;
    FileMapping m_launchData;
    Event m_pollProcessEvent;
    std::vector<GameParameter> m_gameDb;
    Configuration* m_pConfig;
    std::map<StreamPort, GameInstance> m_gameInstances;
    WsaSocket m_socket;
    std::list<std::pair<WsaSocket, ClientId>> m_clients;
    std::thread m_monitorProcess;
    PollHandle64 m_pollProcess;
    PollHandle64 m_pollSocket;

public:
    Sun(Configuration* config, const std::vector<GameParameter>& gamedb);
    ~Sun();
    void Start();

private:
    const GameParameter* FindGame(GameId id);
    void OnAccept(WsaSocket* newConnect, BufferStream<MAX_BUFFER>*);
    void OnClose(WsaSocket* sock, BufferStream<MAX_BUFFER>*);
    void OnRecvFromClient(WsaSocket* sock, BufferStream<MAX_BUFFER>*);
    void LaunchGame(const WsaSocket* client, GameId id);
    StreamPort LaunchGame(ClientId clientId, GameId id);
    StreamPort FindFreePort();
    StreamPort FindExistRunningGame(ClientId id);
    ClientId FindClient(const WsaSocket* client);
    void StartWaitingForClient();
    void MonitorProcess();
    PollAction OnProcessClose(HANDLE handle);
    PollAction OnEventPollProcess(HANDLE handle);
};