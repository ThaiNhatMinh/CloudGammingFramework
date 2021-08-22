#pragma once
#include <queue>
#include <string>
#include <thread>
#include "api/CloudGammingFramework.hh"
#include "common/InputEvent.hh"

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

public:
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
};