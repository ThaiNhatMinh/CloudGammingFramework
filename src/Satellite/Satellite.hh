#include <string>
#include <thread>

#include "cgf/CloudGammingFramework.hh"
#include "ipc/Event.hh"
#include "ipc/WsaSocket.hh"

class Satellite : public WsaSocketPollEvent
{
public:
    enum Status
    {
        INITED,
        CONNECTED,
        REQUESTED_GAME,
        RECEIVING_STREAM
    };

private:
    WsaSocket m_serverSocket;
    WsaSocket m_gameSocket;
    StreamPort m_gamePort;
    std::string m_serverIp;
    std::thread m_thread;
    Event m_signal;

public:
    Satellite() { WsaSocket::Init(); }
    bool Connect(const std::string& ip, unsigned short port);
    bool RequestGame(GameId id);
    bool SendInput(InputEvent event);
    void Finalize();
private:
    void OnRecvServer(WsaSocketInformation* sock);
    void OnRecvGame(WsaSocketInformation* sock);
    bool OnFinalize(const Event* event);
    void InternalThread();
};