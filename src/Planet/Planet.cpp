#include <exception>

#include "common/Logger.hh"
#include "common/Socket.hh"
#include "Planet.hh"
#include "Sun/Sun.hh"
#include "ipc/Named.hh"

bool Planet::Init(const char* game, GraphicApi type, InputCallback handler)
{
    m_gameName = game;
    m_graphicApi = type;
    m_inputHandler = handler;
    std::stringstream ss;
    ss << this;
    if (!m_launchGame.Open(LAUNCH_EVENT) || !m_finalize.Create("Local/GameFinalize" + ss.str()) || !m_launchData.Open(LAUNCH_EVENT_MEMORY, sizeof(Sun::GameRegister)))
    {
        LOG_ERROR << "Init IPC failed" << std::endl;
        return false;
    }
    QueryPort();
    Socket::InitSocket();
    if (!m_socket.Open(m_port))
    {
        LOG_ERROR << "Open port failed\n";
        return false;
    }
    SetExitEvent(m_finalize);
    AddSocket(m_socket);
    m_pollEvent = std::thread(&Planet::InternalThread, this);
    m_pollEvent.detach();
    return true; 
}

void Planet::PollEvent()
{
    if (m_inputEvents.empty()) return;

    InputEvent event = m_inputEvents.front();
    m_inputEvents.pop();
    switch (event.type)
    {
    case InputEvent::EventType::KEY:
        m_inputHandler.KeyPressCallback(event.key.action, event.key.key);
        break;
    case InputEvent::EventType::MOUSE_MOVE:
        m_inputHandler.CursorPositionCallback(event.mousePos.x, event.mousePos.y);
        break;
    case InputEvent::EventType::MOUSE_ACTION:
        m_inputHandler.MouseButtonCallback(event.mouseAction.action, event.mouseAction.key);
        break;
    default:
        break;
    }
}

void Planet::InternalThread()
{
    /**
     * TODO:
     * - Listen on port
     * - Waiting for client
     * - Waiting for input and put to queue
     */
    // PollEvent({}, m_finalize);
    WsaSocketPollEvent::PollEvent();
}

void Planet::QueryPort()
{
    m_launchGame.Signal();
    if (!m_launchGame.Wait(1000))
    {
        throw std::exception("Wait respone from sun failed");
    }
    Sun::GameRegister info = {};
    if (!m_launchData.Read(&info))
    {
        throw std::exception("Get port failed");
    }
    m_port = info.port;
    LOG_DEBUG << "Port from sun: " << m_port << std::endl;
}

void Planet::OnRecv(WsaSocketInformation *sock)
{
    std::size_t size = sizeof(InputEvent);
    while (sock->bytesRecv >= size)
    {
        InputEvent event;
        std::memcpy(&event, sock->recvBuffer, size);
        sock->bytesRecv -= size;
        std::memmove(sock->recvBuffer, sock->recvBuffer + size, sock->bytesRecv);
        m_inputEvents.push(event);
    }
}

void Planet::OnAccept(WsaSocket &&newConnect)
{
    m_client = std::move(newConnect);
    AddSocket(m_client, nullptr, static_cast<callback>(&Planet::OnRecv));
}