#include <exception>

#include "common/Logger.hh"
#include "common/Message.hh"
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
    WsaSocket::Init();
    if (!m_socket.Open(m_port))
    {
        LOG_ERROR << "Open port failed\n";
        return false;
    }
    AddEvent(m_finalize, static_cast<EventCallback>(&Planet::OnFinalize));
    AddSocket(m_socket);
    m_pollEvent = std::thread(&Planet::InternalThread, this);
    m_pollEvent.detach();
    InitKeyStatus();
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
        m_KeyStatus[event.key.key] = event.key.action;
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
    WsaSocketPollEvent::PollEvent();
    // Signal poll stopped;
    m_finalize.Signal();
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
    constexpr std::size_t size = sizeof(InputEvent);
    while (sock->recvBuffer.Length() >= MSG_INPUT_PACKAGE_SIZE)
    {
        sock->recvBuffer.SetCurrentPosition(0);
        MessageHeader header;
        sock->recvBuffer >> header;
        if (header.code == Message::MSG_INPUT)
        {
            InputEvent event;
            sock->recvBuffer >> event;
            m_inputEvents.push(event);
        } else
        {
            LOG_ERROR << "Unknow message code: " << header.code << std::endl;
        }
    }
}

void Planet::OnAccept(WsaSocket &&newConnect)
{
    if (m_client.GetHandle() != INVALID_SOCKET)
    {
        LOG_ERROR << "A client already connected\n";
        return;
    }
    m_client = std::move(newConnect);
    AddSocket(m_client, nullptr, static_cast<SocketCallback>(&Planet::OnRecv));
}

void Planet::OnClose(WsaSocketInformation* sock)
{
    if (sock->socket->GetHandle() != m_client.GetHandle())
    {
        LOG_ERROR << "Close on unknow socket\n";
        return;
    }

    m_client.Release();
    LOG_DEBUG << "Client disconnected\n";
}

int Planet::GetKeyStatus(Key key)
{
    return m_KeyStatus[key];
}

void Planet::Finalize()
{
    m_finalize.Signal();
    if (!m_finalize.Wait(500))
    {
        LOG_WARN << "Failed to waiting for thread stop\n";
        return;
    }
    LOG_DEBUG << "Finalize planet\n";
}

void Planet::SetResolution(std::size_t w, std::size_t h)
{
    m_Width = w;
    m_Height = h;
    m_pFramePackage.reset(new char[w*h*m_BytePerPixel + MSG_HEADER_LENGTH]);
    
}

void Planet::SetFrame(const void* pData)
{
    std::size_t size = m_Width * m_Height * m_BytePerPixel;
    CreateFrameMsg(m_pFramePackage.get(), size + MSG_HEADER_LENGTH, pData, size);
    SendFrame();
}

void Planet::SendFrame()
{
    if (m_client.GetHandle() == INVALID_SOCKET) return;
    std::size_t size = m_Width * m_Height * m_BytePerPixel + MSG_HEADER_LENGTH;
    if (m_client.SendAll(m_pFramePackage.get(), size) < size)
    {
        LOG_ERROR << "Send failed: " << size << std::endl;
    }
}

bool Planet::ShouldExit()
{    
    /**
     * TODO: Check if client disconnect timeout, or if client request close
     */
    return false;
}

bool Planet::OnFinalize(const Event* sock)
{
    return false;
}

void Planet::InitKeyStatus()
{
    m_KeyStatus[Key::KEY_SPACE] = -1;
    m_KeyStatus[Key::KEY_APOSTROPHE] = -1;
    m_KeyStatus[Key::KEY_COMMA] = -1;
    m_KeyStatus[Key::KEY_MINUS] = -1;
    m_KeyStatus[Key::KEY_PERIOD] = -1;
    m_KeyStatus[Key::KEY_SLASH] = -1;
    m_KeyStatus[Key::KEY_0] = -1;
    m_KeyStatus[Key::KEY_1] = -1;
    m_KeyStatus[Key::KEY_2] = -1;
    m_KeyStatus[Key::KEY_3] = -1;
    m_KeyStatus[Key::KEY_4] = -1;
    m_KeyStatus[Key::KEY_5] = -1;
    m_KeyStatus[Key::KEY_6] = -1;
    m_KeyStatus[Key::KEY_7] = -1;
    m_KeyStatus[Key::KEY_8] = -1;
    m_KeyStatus[Key::KEY_9] = -1;
    m_KeyStatus[Key::KEY_SEMICOLON] = -1;
    m_KeyStatus[Key::KEY_EQUAL] = -1;
    m_KeyStatus[Key::KEY_A] = -1;
    m_KeyStatus[Key::KEY_B] = -1;
    m_KeyStatus[Key::KEY_C] = -1;
    m_KeyStatus[Key::KEY_D] = -1;
    m_KeyStatus[Key::KEY_E] = -1;
    m_KeyStatus[Key::KEY_F] = -1;
    m_KeyStatus[Key::KEY_G] = -1;
    m_KeyStatus[Key::KEY_H] = -1;
    m_KeyStatus[Key::KEY_I] = -1;
    m_KeyStatus[Key::KEY_J] = -1;
    m_KeyStatus[Key::KEY_K] = -1;
    m_KeyStatus[Key::KEY_L] = -1;
    m_KeyStatus[Key::KEY_M] = -1;
    m_KeyStatus[Key::KEY_N] = -1;
    m_KeyStatus[Key::KEY_O] = -1;
    m_KeyStatus[Key::KEY_P] = -1;
    m_KeyStatus[Key::KEY_Q] = -1;
    m_KeyStatus[Key::KEY_R] = -1;
    m_KeyStatus[Key::KEY_S] = -1;
    m_KeyStatus[Key::KEY_T] = -1;
    m_KeyStatus[Key::KEY_U] = -1;
    m_KeyStatus[Key::KEY_V] = -1;
    m_KeyStatus[Key::KEY_W] = -1;
    m_KeyStatus[Key::KEY_X] = -1;
    m_KeyStatus[Key::KEY_Y] = -1;
    m_KeyStatus[Key::KEY_Z] = -1;
    m_KeyStatus[Key::KEY_LEFT_BRACKET] = -1;
    m_KeyStatus[Key::KEY_BACKSLASH] = -1;
    m_KeyStatus[Key::KEY_RIGHT_BRACKET] = -1;
    m_KeyStatus[Key::KEY_GRAVE_ACCENT] = -1;
    m_KeyStatus[Key::KEY_WORLD_1] = -1;
    m_KeyStatus[Key::KEY_WORLD_2] = -1;
    m_KeyStatus[Key::KEY_ESCAPE] = -1;
    m_KeyStatus[Key::KEY_ENTER] = -1;
    m_KeyStatus[Key::KEY_TAB] = -1;
    m_KeyStatus[Key::KEY_BACKSPACE] = -1;
    m_KeyStatus[Key::KEY_INSERT] = -1;
    m_KeyStatus[Key::KEY_DELETE] = -1;
    m_KeyStatus[Key::KEY_RIGHT] = -1;
    m_KeyStatus[Key::KEY_LEFT] = -1;
    m_KeyStatus[Key::KEY_DOWN] = -1;
    m_KeyStatus[Key::KEY_UP] = -1;
    m_KeyStatus[Key::KEY_PAGE_UP] = -1;
    m_KeyStatus[Key::KEY_PAGE_DOWN] = -1;
    m_KeyStatus[Key::KEY_HOME] = -1;
    m_KeyStatus[Key::KEY_END] = -1;
    m_KeyStatus[Key::KEY_CAPS_LOCK] = -1;
    m_KeyStatus[Key::KEY_SCROLL_LOCK] = -1;
    m_KeyStatus[Key::KEY_NUM_LOCK] = -1;
    m_KeyStatus[Key::KEY_PRINT_SCREEN] = -1;
    m_KeyStatus[Key::KEY_PAUSE] = -1;
    m_KeyStatus[Key::KEY_F1] = -1;
    m_KeyStatus[Key::KEY_F2] = -1;
    m_KeyStatus[Key::KEY_F3] = -1;
    m_KeyStatus[Key::KEY_F4] = -1;
    m_KeyStatus[Key::KEY_F5] = -1;
    m_KeyStatus[Key::KEY_F6] = -1;
    m_KeyStatus[Key::KEY_F7] = -1;
    m_KeyStatus[Key::KEY_F8] = -1;
    m_KeyStatus[Key::KEY_F9] = -1;
    m_KeyStatus[Key::KEY_F10] = -1;
    m_KeyStatus[Key::KEY_F11] = -1;
    m_KeyStatus[Key::KEY_F12] = -1;
    m_KeyStatus[Key::KEY_F13] = -1;
    m_KeyStatus[Key::KEY_F14] = -1;
    m_KeyStatus[Key::KEY_F15] = -1;
    m_KeyStatus[Key::KEY_F16] = -1;
    m_KeyStatus[Key::KEY_F17] = -1;
    m_KeyStatus[Key::KEY_F18] = -1;
    m_KeyStatus[Key::KEY_F19] = -1;
    m_KeyStatus[Key::KEY_F20] = -1;
    m_KeyStatus[Key::KEY_F21] = -1;
    m_KeyStatus[Key::KEY_F22] = -1;
    m_KeyStatus[Key::KEY_F23] = -1;
    m_KeyStatus[Key::KEY_F24] = -1;
    m_KeyStatus[Key::KEY_F25] = -1;
    m_KeyStatus[Key::KEY_KP_0] = -1;
    m_KeyStatus[Key::KEY_KP_1] = -1;
    m_KeyStatus[Key::KEY_KP_2] = -1;
    m_KeyStatus[Key::KEY_KP_3] = -1;
    m_KeyStatus[Key::KEY_KP_4] = -1;
    m_KeyStatus[Key::KEY_KP_5] = -1;
    m_KeyStatus[Key::KEY_KP_6] = -1;
    m_KeyStatus[Key::KEY_KP_7] = -1;
    m_KeyStatus[Key::KEY_KP_8] = -1;
    m_KeyStatus[Key::KEY_KP_9] = -1;
    m_KeyStatus[Key::KEY_KP_DECIMAL] = -1;
    m_KeyStatus[Key::KEY_KP_DIVIDE] = -1;
    m_KeyStatus[Key::KEY_KP_MULTIPLY] = -1;
    m_KeyStatus[Key::KEY_KP_SUBTRACT] = -1;
    m_KeyStatus[Key::KEY_KP_ADD] = -1;
    m_KeyStatus[Key::KEY_KP_ENTER] = -1;
    m_KeyStatus[Key::KEY_KP_EQUAL] = -1;
    m_KeyStatus[Key::KEY_LEFT_SHIFT] = -1;
    m_KeyStatus[Key::KEY_LEFT_CONTROL] = -1;
    m_KeyStatus[Key::KEY_LEFT_ALT] = -1;
    m_KeyStatus[Key::KEY_LEFT_SUPER] = -1;
    m_KeyStatus[Key::KEY_RIGHT_SHIFT] = -1;
    m_KeyStatus[Key::KEY_RIGHT_CONTROL] = -1;
    m_KeyStatus[Key::KEY_RIGHT_ALT] = -1;
    m_KeyStatus[Key::KEY_RIGHT_SUPER] = -1;
    m_KeyStatus[Key::KEY_MENU] = -1;
}