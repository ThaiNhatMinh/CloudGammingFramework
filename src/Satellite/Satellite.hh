#pragma once
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "cgf/cgf.hh"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "ipc/Event.hh"
#include "ipc/WsaSocket.hh"
#include "Frames.hh"

class Satellite : public WsaSocketPollEvent
{
public:
    enum Status
    {
        INITED,
        CONNECTED,
        REQUESTED_GAME,
        RECEIVING_STREAM,
        CLOSING_GAME
    };

private:
    struct Frame
    {
        std::unique_ptr<char[]> data;
        std::size_t length;
    };

private:
    WsaSocket m_serverSocket;
    WsaSocket m_gameSocket;
    StreamPort m_gamePort;
    std::string m_serverIp;
    std::thread m_thread;
    Event m_signal;
    ClientId m_id;
    std::size_t m_gameWidth;
    std::size_t m_gameHeight;
    char m_bytePerPixel = 3;
    bool m_bIsReceivingFrame;
    Frame m_currentFrame;
    Frames m_frames;
    cgfResolutionfun m_resFunc;
    cgfFramefun m_frameFunc;
    std::vector<Event> m_events;
    HANDLE m_handle[MAXIMUM_WAIT_OBJECTS];

public:
    Satellite(): m_bIsReceivingFrame(false) { WsaSocket::Init(); }
    bool Initialize(cgfResolutionfun resFunc, cgfFramefun frameFunc);
    bool Connect(ClientId id, const std::string& ip, unsigned short port);
    bool RequestGame(GameId id);
    bool SendInput(InputEvent event);
    void Finalize();
    bool PollEvent(std::size_t timeout);
    bool CloseGame();

private:
    void OnRecvServer(WsaSocketInformation* sock);
    void OnRecvGame(WsaSocketInformation* sock);
    bool OnFinalize(const Event* event);
    void InternalThread();
};