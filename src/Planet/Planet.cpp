#include "Planet.hh"

bool Planet::Init(const char* game, GraphicApi type, WndProcHandler handler)
{
    m_gameName = game;
    m_graphicApi = type;
    m_inputHandler = handler;
    return true; 
}