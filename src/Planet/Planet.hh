#pragma once
#include <string>
#include "CloudGammingFramework.hh"
/**
 * A class run within a game to communicate with Sun
 */
class Planet
{
private:
    GraphicApi m_graphicApi;
    std::string m_gameName;
    WndProcHandler m_inputHandler;

public:
    bool Init(const char* gameName, GraphicApi type, WndProcHandler handler);
    void PollEvent();
};