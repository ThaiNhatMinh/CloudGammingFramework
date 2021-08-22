#include "Planet.hh"

bool Planet::Init(const char* game, GraphicApi type, WndProcHandler handler)
{
    m_gameName = game;
    m_graphicApi = type;
    m_inputHandler = handler;
    m_pollEvent = std::thread(&Planet::InternalThread, this);
    // TODO: Register with Sun, get port and listen on that port
    return true; 
}

void Planet::PollEvent()
{
    if (m_inputEvents.empty()) return;

    InputEvent event = m_inputEvents.front();
    m_inputEvents.pop();

    m_inputHandler(event.win32.msg, event.win32.wParam, event.win32.lParam);
}

void Planet::InternalThread()
{

}