#pragma once
#include <queue>
#include <string>
#include <thread>
#include <memory>

#include "cgf/CloudGammingFramework.hh"
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "ipc/WaitableTimer.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/PollHandle.hh"
#include "Sun/Sun.hh"
#include "FpsLocker.hh"
#include "StreamController.hh"

/**
 * A class run within a game to communicate with Sun
 */
class Planet
{
private:
    GraphicApi m_graphicApi;
    std::string m_gameName;
    InputCallback m_inputHandler;
    std::queue<InputEvent> m_inputEvents;
    std::thread m_pollEvent;
    Event m_launchGame;
    Event m_finalize;
    FileMapping m_launchData;
    WsaSocket m_socketControl;
    WsaSocket m_clientControl;
    char m_KeyStatus[512];
    uint32_t m_Width;
    uint32_t m_Height;
    unsigned char m_BytePerPixel = 3;
    WaitableTimer m_disconnectTimer;
    Sun::GameRegister* m_pInfo;
    FpsLocker m_fpsLocker;
    StreamController m_streamController;
    uint32_t m_numsocket;
    PollHandle64 m_socketPoll;

public:
    Planet() {};

    /**
     * Initialize Planet
     * 
     * @param gameName Name of the game
     * @param type Graphic API used in game
     * @param handler Pointer to function that will receive input event
     * 
     * @return True on success
     */ 
    bool Init(const char* gameName, GraphicApi type, InputCallback handler);
    
    /**
     * Trigger all event in queue
     */
    void PollEvent();

    int GetKeyStatus(Key key);

    void Finalize();

    void SetResolution(uint32_t w, uint32_t h, uint8_t bpp);

    void SetFrame(const void* pData);

    bool ShouldExit();
private:
    /**
     * Receive event from client and put it into queue
     */
    void InternalThread();

    bool QueryInformation();
    void OnRecvControl(WsaSocket* sock, BufferStream10KB* buffer);
    void OnRecv(WsaSocket* sock, BufferStream10KB* buffer);
    void OnAcceptControl(WsaSocket* sock, BufferStream10KB* buffer);
    void OnClose(WsaSocket* sock, BufferStream10KB* buffer);
    PollAction OnDisconnectTimeout(const WaitableTimer* timer);
    PollAction OnFinalize(const Event* sock);
    void InitKeyStatus();
    void SendFrame();
};