#pragma once
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "cgf/cgf.hh"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "ipc/Event.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/PollHandle.hh"
#include "common/Frames.hh"
#include "StreamController.hh"

class Satellite
{
public:
    // TODO: Improve to check each bit slot
    enum Status
    {
        INITED              = 1 << 1,
        SERVER_CONNECTED    = 1 << 2,
        REQUESTING_GAME     = 1 << 3,
        RECEIVING_STREAM    = 1 << 4,
        CLOSING_GAME        = 1 << 5,
        FINALIZE            = 1 << 6,
        DISCONNECTED        = 1 << 7,
        GAME_CONNECTED      = 1 << 8
    };

private:

private:
    WsaSocket m_serverSocket;
    WsaSocket m_gameSocketInput;
    StreamPort m_gamePort;
    std::string m_serverIp;
    std::thread m_thread;
    Event m_signal;
    Event m_resolutionChangeEvent;
    ClientId m_id;
    uint32_t m_gameWidth;
    uint32_t m_gameHeight;
    uint8_t m_bytePerPixel = 3;
    bool m_bIsReceivingFrame;
    cgfResolutionfun m_resFunc;
    cgfFramefun m_frameFunc;
    uint32_t m_status;
    StreamController m_streamController;
    PollHandle<2> m_callbackPoll;
    PollHandle64 m_socketPoll;

public:
    Satellite(): m_bIsReceivingFrame(false), m_status(0) { WsaSocket::Init(); }
    ~Satellite();
    bool Initialize(cgfResolutionfun resFunc, cgfFramefun frameFunc);
    bool Connect(ClientId id, const std::string& ip, unsigned short port);
    bool RequestGame(GameId id);
    bool SendInput(InputEvent event);
    void Finalize();

    /**
     * Dispatch MSG_RESOULUTION and MSG_FRAME event
     */
    bool PollEvent(std::size_t timeout);
    bool CloseGame();

private:
    void OnClose(WsaSocket* sock, BufferStream10KB* buffer);
    void OnRecvServer(WsaSocket* sock, BufferStream10KB* buffer);
    void OnRecvStream(WsaSocket* sock, BufferStream10KB* buffer);
    void OnRecvControl(WsaSocket* sock, BufferStream10KB* buffer);
    bool OnFinalize(const Event* event);
    void InternalThread();
};