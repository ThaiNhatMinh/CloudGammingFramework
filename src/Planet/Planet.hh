#pragma once
#include <queue>
#include <string>
#include <thread>

#include "cgf/CloudGammingFramework.hh"
#include "cgf/InputEvent.hh"
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "ipc/WsaSocket.hh"
#include "Sun/Sun.hh"

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
    StreamPort m_port;
    WsaSocket m_socket;
    WsaSocket m_client;
    char m_KeyStatus[512];

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
private:
    /**
     * Receive event from client and put it into queue
     */
    void InternalThread();

    void QueryPort();
    void OnRecv(WsaSocketInformation* sock);
    void OnAccept(WsaSocket&& newConnect) override;
    void InitKeyStatus();
};