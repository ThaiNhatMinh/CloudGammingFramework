#include "Satellite.hh"
#include "common/BufferStream.hh"
#include "common/Message.hh"

Satellite::~Satellite()
{
    Finalize();
    LOG_DEBUG << "End Satellite\n";
}

bool Satellite::Initialize(cgfResolutionfun resFunc, cgfFramefun frameFunc)
{
    std::stringstream ss;
    ss << this;
    m_resFunc = resFunc;
    m_frameFunc = frameFunc;
    Event resEvent, frameEvent;
    if (!resEvent.Create("Local\\ResEvent" + ss.str()) || !frameEvent.Create("Local\\FraneEvent" + ss.str()))
    {
        return false;
    }
    m_events.push_back(std::move(resEvent));
    m_events.push_back(std::move(frameEvent));
    m_handle[0] = m_events[0].GetHandle();
    m_handle[1] = m_events[1].GetHandle();
    m_status = Status::INITED;
    return true;
}


bool Satellite::Connect(ClientId id, const std::string& ip, unsigned short port)
{
    std::stringstream ss;
    ss << this;
    if (!m_serverSocket.Connect(ip, port) || !m_signal.Create("Local\\Satellite" + ss.str()))
    {
        return false;
    }

    BufferStream1KB stream;
    MessageHeader header;
    header.code = Message::MSG_INIT;
    stream << header;
    stream << id;
    if (m_serverSocket.SendAll(stream.Get(), stream.Length()) == SOCKET_ERROR)
    {
        LOG_ERROR << "Send error\n";
        return false;
    }

    m_serverIp = ip;
    AddSocket(m_serverSocket, static_cast<SocketCallback>(&Satellite::OnRecvServer));
    AddEvent(m_signal, static_cast<EventCallback>(&Satellite::OnFinalize));
    m_thread = std::thread(&Satellite::InternalThread, this);
    m_signal.Wait(100);
    m_bIsReceivingFrame = false;
    m_currentFrame.length = 0;
    m_status = Status::CONNECTED;
    return true;
}

bool Satellite::RequestGame(GameId id)
{
    BufferStream1KB stream;
    stream << Message::MSG_START_GAME;
    stream << id;
    if (m_serverSocket.SendAll(stream.Get(), stream.Length()) == SOCKET_ERROR)
    {
        LOG_ERROR << "Send error\n";
        return false;
    }
    return true;
}

bool Satellite::SendInput(InputEvent event)
{
    if (m_status != Status::RECEIVING_STREAM) return false;
    BufferStream1KB stream;
    stream << Message::MSG_INPUT;
    stream << event;
    if (m_gameSocket.SendAll(stream.Get(), stream.Length()) == SOCKET_ERROR)
    {
        LOG_ERROR << "Send error\n";
        return false;
    }

    return true;
}

void Satellite::OnRecvServer(WsaSocketInformation* sock)
{
    if (sock->recvBuffer.Length() < MSG_HEADER_LENGTH) return;
    sock->recvBuffer.SetCurrentPosition(0);
    MessageHeader header;
    sock->recvBuffer >> header;
    if (header.code == Message::MSG_START_GAME_RESP && sock->recvBuffer.Length() >= sizeof(StreamPort))
    {
        int status;
        sock->recvBuffer >> status;
        if (status == INVALID_PORT)
        {
            LOG_ERROR << "Start game failed\n";
            return;
        }
        m_gamePort = status;
        LOG_INFO << "Start game success, port from server " << m_gamePort << std::endl;
        if (!m_gameSocket.Connect(m_serverIp, m_gamePort))
        {
            LOG_ERROR << "Failed to connect to game\n";
            return;
        }
        AddSocket(m_gameSocket, static_cast<SocketCallback>(&Satellite::OnRecvGame));
    } else
    {
        LOG_ERROR << "Unknow code: " << header.code << std::endl;
    }
}

void Satellite::OnRecvGame(WsaSocketInformation* sock)
{
    if (sock->recvBuffer.Length() < MSG_HEADER_LENGTH) return;
    sock->recvBuffer.SetCurrentPosition(0);
    if (m_bIsReceivingFrame)
    {
        std::size_t size = m_gameWidth * m_gameHeight * m_bytePerPixel;
        std::size_t byteRemain = size - m_currentFrame.length;
        std::size_t byteToCopy = sock->recvBuffer.Length();
        if (byteToCopy > byteRemain) byteToCopy = byteRemain;
        sock->recvBuffer.Extract(m_currentFrame.data.get() + m_currentFrame.length, byteToCopy);
        m_currentFrame.length += byteToCopy;
        if (m_currentFrame.length == size)
        {
            m_frames.PushBack(m_currentFrame.data.get());
            m_currentFrame.length = 0;
            m_bIsReceivingFrame = false;
            m_events[1].Signal();
        }
        sock->recvBuffer.SetCurrentPosition(sock->recvBuffer.Length());
        return;
    }

    MessageHeader header;
    sock->recvBuffer >> header;
    if (header.code == Message::MSG_RESOLUTION && sock->recvBuffer.Length() >= 2 * sizeof(int))
    {
        int w, h;
        char bpp;
        sock->recvBuffer >> w >> h >> bpp;
        m_gameWidth = w;
        m_gameHeight = h;
        m_bytePerPixel = bpp;
        m_frames.Init(20, m_gameWidth * m_gameHeight * m_bytePerPixel);
        m_currentFrame.data.reset(new char[m_gameWidth * m_gameHeight * m_bytePerPixel]);
        m_currentFrame.length = 0;
        m_events[0].Signal();
        m_status = Status::RECEIVING_STREAM;
    } else if (header.code == Message::MSG_FRAME)
    {
        m_bIsReceivingFrame = true;
    } else
    {
        LOG_DEBUG << "Unknow code " << header.code << std::endl;
        throw std::exception("Invalid message code");
    }
    sock->recvBuffer.SetCurrentPosition(sock->recvBuffer.Length());
}

void Satellite::InternalThread()
{
    m_signal.Signal();
    WsaSocketPollEvent::PollEvent();
    LOG_DEBUG << "Close InternalThread\n";
}

void Satellite::Finalize()
{
    if (m_status == Status::FINALIZE) return;
    m_signal.Signal();
    if (m_thread.joinable()) m_thread.join();
    m_status = Status::FINALIZE;
}

bool Satellite::OnFinalize(const Event* event)
{
    return false;
}

bool Satellite::PollEvent(std::size_t timeout)
{
    DWORD index = WaitForMultipleObjects(m_events.size(), m_handle, false, timeout);
    if (index == WAIT_TIMEOUT || index == WAIT_FAILED) return false;
    index -= WAIT_OBJECT_0;
    if (index == 0) m_resFunc(m_gameWidth, m_gameHeight, m_bytePerPixel);
    else if (index == 1)
    {
        auto frame = m_frames.PopFront();
        m_frameFunc(frame);
    } else
    {
        LOG_ERROR << "Invalid index:" << index << std::endl;
        return false;
    }
    return true;
}

bool Satellite::CloseGame()
{
    BufferStream1KB stream;
    MessageHeader header;
    header.code = Message::MSG_STOP_GAME;
    stream << header;
    m_gameSocket.SendAll(stream.Get(), stream.Length());
    return true;
}
