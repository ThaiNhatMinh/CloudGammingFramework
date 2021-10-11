#include <exception>

#include "common/Logger.hh"
#include "common/Module.hh"
#include "common/Message.hh"
#include "Planet.hh"
#include "Sun/Sun.hh"
#include "ipc/Named.hh"
#include "StreamInformation.hh"

bool Planet::Init(const char* game, GraphicApi type, InputCallback handler)
{
    m_gameName = game;
    m_graphicApi = type;
    m_inputHandler = handler;
    if (!QueryInformation())
    {
        return false;
    }
    WsaSocket::Init();
    if (!m_socketControl.Open(m_pInfo->port))
    {
        LOG_ERROR << "Open port failed\n";
        return false;
    }
    m_socketPoll.AddEvent(m_finalize, std::bind(&Planet::OnFinalize, this, std::placeholders::_1));
    POLL_ADD_SOCKET_LISTEN(m_socketPoll, m_socketControl, &Planet::OnClose, &Planet::OnAcceptControl);
    m_pollEvent = std::thread(&Planet::InternalThread, this);
    InitKeyStatus();
    m_fpsLocker.SetFps(60);
    m_numsocket = 2;
    return true; 
}

void Planet::PollEvent(DispatchType type)
{
    if (m_inputEvents.empty()) return;

    do
    {
        InputEvent event = m_inputEvents.front();
        m_inputEvents.pop();
        // TODO: Convert to map<EventType, callback>
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
        case InputEvent::EventType::TEXT_INPUT:
            m_inputHandler.TextInputCallback(event.text.type, event.text.character);
            break;
        case InputEvent::EventType::SCROLL:
            m_inputHandler.ScrollCallback(event.scroll.xoffset, event.scroll.yoffset);
            break;
        default:
            break;
        }

        if (type == DispatchType::ONE) break;
    } while (!m_inputEvents.empty());
}

void Planet::InternalThread()
{
    /**
     * TODO:
     * - Listen on port
     * - Waiting for client
     * - Waiting for input and put to queue
     */
    m_socketPoll.PollSocket(INFINITE);
}

bool Planet::QueryInformation()
{
    if (!m_launchGame.Open(LAUNCH_EVENT) || !m_launchData.Open(LAUNCH_EVENT_MEMORY, sizeof(Sun::GameRegister)))
    {
        LOG_ERROR << "Init IPC failed" << std::endl;
        return false;
    }
    m_launchGame.Signal();
    if (!m_launchGame.Wait(1000))
    {
        LOG_ERROR << "Wait respone from sun failed";
        return false;
    }
    m_pInfo = m_launchData.Get<Sun::GameRegister>();
    ClientId client = m_pInfo->clientId;
    GameId id = m_pInfo->Id;
    LOG_DEBUG << "Game infor: port=" << m_pInfo->port << " clientid=" << m_pInfo->clientId << std::endl;
    if (!m_disconnectTimer.Create(CreateDisconnectString(m_pInfo->clientId, m_pInfo->Id)))
    {
        return false;
    }

    if (!m_finalize.Create(CreateShutdownString(m_pInfo->clientId, m_pInfo->Id)))
    {
        return false;
    }
    m_launchData.Release();
    if (!m_launchData.Open(CreateGameInfoString(client, id), sizeof(Sun::GameRegister)))
    {
        return false;
    }
    m_pInfo = m_launchData.Get<Sun::GameRegister>();
    m_launchGame.Signal();
    m_socketPoll.AddTimer(m_disconnectTimer, std::bind(&Planet::OnDisconnectTimeout, this, std::placeholders::_1));
    return true;
}

void Planet::OnRecvControl(WsaSocket* sock, BufferStream10KB* buffer)
{
    if (buffer->Length() < MSG_HEADER_LENGTH) return;
    buffer->SetCurrentPosition(0);
    
    MessageHeader header;
    *buffer >> header;
    constexpr std::size_t size = sizeof(InputEvent);
    if (header.code == Message::MSG_INPUT && buffer->Length() >= size)
    {
        InputEvent event;
        *buffer >> event;
        m_inputEvents.push(event);
    } else if (header.code == Message::MSG_STOP_GAME)
    {
        m_pInfo->Status = GameStatus::SHUTTING_DOWN;
    } else
    {
        LOG_ERROR << "Unknow message code: " << header.code << std::endl;
        throw std::exception("Unknow message code");
    }
    buffer->SetCurrentPosition(buffer->Length());
}

void Planet::OnAcceptControl(WsaSocket* newConnect, BufferStream10KB* buffer)
{
    if (m_clientControl.GetHandle() != INVALID_SOCKET)
    {
        LOG_ERROR << "A client already connected\n";
        return;
    }

    m_clientControl = std::move(*newConnect);
    POLL_ADD_SOCKET_RECV(m_socketPoll, m_clientControl, &Planet::OnClose, &Planet::OnRecvControl);
    m_pInfo->Status = GameStatus::STREAMING;

    if (m_Width == 0)
    {
        LOG_ERROR << "Width is not set\n";
        throw std::exception("Width is not set");
    }
    BufferStream1KB stream;
    MessageHeader header;
    header.code = Message::MSG_RESOLUTION;
    stream << header << m_Width << m_Height << m_BytePerPixel;
    header.code = Message::MSG_INIT;
    stream << header << m_numsocket << m_streamController.GetBytePerSocket();
    if (m_clientControl.SendAll(stream.Get(), stream.Length()) < stream.Length())
    {
        LOG_ERROR << "Send error\n";
    }
    LOG_INFO << "Client connected\n";
}

void Planet::OnClose(WsaSocket* sock, BufferStream10KB* buffer)
{
    sock->Release();
    if (*sock != m_clientControl) return;
    LOG_DEBUG << "Client disconnected\n";

    if (m_pInfo->Status != GameStatus::SHUTTING_DOWN || m_pInfo->Status != GameStatus::SHUTDOWN)
    {
        LOG_DEBUG << "Client disconnect while game running, start timer\n";
        m_disconnectTimer.SetTime(m_pInfo->DisconnectTimeout);
    }
}

int Planet::GetKeyStatus(Key key)
{
    return m_KeyStatus[key];
}

void Planet::Finalize()
{
    m_pInfo->Status = GameStatus::SHUTDOWN;
    m_finalize.Signal();
    m_pollEvent.join();
    m_streamController.StopPoll();
    LOG_DEBUG << "Finalize planet\n";
}

void Planet::SetResolution(uint32_t w, uint32_t h, uint8_t bpp)
{
    m_Width = w;
    m_Height = h;
    m_BytePerPixel = bpp;
    StreamInformation info;
    info.Width = w;
    info.Heigh = h;
    info.BytePerPixel = bpp;
    info.NumSocket = m_numsocket;
    m_streamController.SetInformation(info, m_pInfo->port + 1);
}

void Planet::SetFrame(const void* pData)
{
    m_streamController.SetFrame(pData);
    m_fpsLocker.FrameEnd();
}

bool Planet::ShouldExit()
{
    if (m_pInfo->Status == GameStatus::SHUTTING_DOWN)
    {
        return true;
    }
    /**
     * TODO: Check if client disconnect timeout, or if client request close
     */
    return false;
}

PollAction Planet::OnFinalize(const Event* sock)
{
    return PollAction::STOP_POLL;
}

PollAction Planet::OnDisconnectTimeout(const WaitableTimer* timer)
{
    LOG_WARN << "Client disconnect timeout, closing game\n";
    m_pInfo->Status = GameStatus::SHUTTING_DOWN;
    return PollAction::NONE;
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