#include <exception>

#include "common/Logger.hh"
#include "Planet.hh"
#include "Sun/Sun.hh"
#include "ipc/Named.hh"

bool Planet::Init(const char* game, GraphicApi type, WndProcHandler handler)
{
    m_gameName = game;
    m_graphicApi = type;
    m_inputHandler = handler;
    if (!m_launchGame.Open(LAUNCH_EVENT) || m_launchData.Open(LAUNCH_EVENT_MEMORY, sizeof(Sun::GameRegister)))
    {
        return false;
    }
    QueryPort();
    m_pollEvent = std::thread(&Planet::InternalThread, this);
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
    /**
     * TODO:
     * - Listen on port
     * - Waiting for client
     * - Waiting for input and put to queue
     */
}

void Planet::QueryPort()
{
    m_launchGame.Signal();
    Sun::GameRegister info = {};
    if (!m_launchData.Read(&info))
    {
        throw std::exception("Get port failed");
    }
    m_port = info.port;
    LOG_DEBUG << "Port from sun: " << m_port << std::endl;
}