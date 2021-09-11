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
#include "Sun/Sun.hh"
#include "FpsLocker.hh"

/**
 * A class run within a game to communicate with Sun
 */
class Planet : public WsaSocketPollEvent
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
    WsaSocket m_socket;
    WsaSocket m_client;
    char m_KeyStatus[512];
    std::size_t m_Width;
    std::size_t m_Height;
    unsigned char m_BytePerPixel = 3;
    std::unique_ptr<char> m_pFramePackage; 
    WaitableTimer m_disconnectTimer;
    Sun::GameRegister* m_pInfo;
    FpsLocker m_fpsLocker;

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

    void SetResolution(std::size_t w, std::size_t h);

    void SetFrame(const void* pData);

    bool ShouldExit();
private:
    /**
     * Receive event from client and put it into queue
     */
    void InternalThread();

    bool QueryInformation();
    void OnRecv(WsaSocketInformation* sock);
    void OnAccept(WsaSocket&& newConnect);
    void OnClose(WsaSocketInformation* sock) override;
    void OnDisconnectTimeout(const WaitableTimer* timer);
    bool OnFinalize(const Event* sock);
    void InitKeyStatus();
    void SendFrame();
};