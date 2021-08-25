#pragma once
#include <queue>
#include <string>
#include <thread>
#include "cgf/CloudGammingFramework.hh"
#include "common/InputEvent.hh"
#include "ipc/Event.hh"
#include "ipc/FileMapping.hh"
#include "Sun/Sun.hh"

/**
 * A class run within a game to communicate with Sun
 */
class Planet
{
private:
    GraphicApi m_graphicApi;
    std::string m_gameName;
    WndProcHandler m_inputHandler;
    std::queue<InputEvent> m_inputEvents;
    std::thread m_pollEvent;
    Event m_launchGame;
    FileMapping m_launchData;
    StreamPort m_port;

public:
    /**
     * Initialize Planet
     * 
     * @param gameName Name of the game
     * @param type Graphic API used in game
     * @param handler Pointer to function that will receive input event
     * 
     * @return True on success
     */ 
    bool Init(const char* gameName, GraphicApi type, WndProcHandler handler);
    
    /**
     * Trigger all event in queue
     */
    void PollEvent();

private:
    /**
     * Receive event from client and put it into queue
     */
    void InternalThread();

    void QueryPort();
};