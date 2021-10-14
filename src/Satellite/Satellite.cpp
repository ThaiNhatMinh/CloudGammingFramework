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
    if (!m_resolutionChangeEvent.Create("Local\\ResEvent" + ss.str()))
    {
        return false;
    }

    m_callbackPoll.AddEvent(m_resolutionChangeEvent, [this](const Event* e)
    {
        this->m_resFunc(this->m_gameWidth, this->m_gameHeight, this->m_bytePerPixel);
        return PollAction::NONE;
    });

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
    MessageHeader header = CreateHeaderMsg(Message::MSG_INIT);
    stream << header;
    stream << id;
    if (m_serverSocket.SendAll(stream.Get(), stream.Length()) == SOCKET_ERROR)
    {
        LOG_ERROR << "Send error\n";
        return false;
    }

    m_serverIp = ip;
    POLL_ADD_SOCKET_RECV(m_socketPoll, m_serverSocket, &Satellite::OnClose, &Satellite::OnRecvServer);
    m_socketPoll.AddEvent(m_signal, [](const Event* e){ return PollAction::STOP_POLL;});
    m_thread = std::thread(&Satellite::InternalThread, this);
    m_signal.Wait(100);
    m_bIsReceivingFrame = false;
    m_status = Status::CONNECTED;
    return true;
}

bool Satellite::RequestGame(GameId id)
{
    BufferStream1KB stream;
    MessageHeader header = CreateHeaderMsg(Message::MSG_START_GAME);
    stream << header << id;
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
    MessageHeader header = CreateHeaderMsg(Message::MSG_INPUT);
    stream << header << event;
    if (m_gameSocketInput.SendAll(stream.Get(), stream.Length()) == SOCKET_ERROR)
    {
        LOG_ERROR << "Send error\n";
        return false;
    }

    return true;
}

void Satellite::OnRecvServer(WsaSocket* sock, BufferStream10KB* buffer)
{
    if (buffer->Length() < MSG_HEADER_LENGTH) return;
    buffer->SetCurrentPosition(0);
    MessageHeader header;
    *buffer >> header;
    if (header.code == Message::MSG_START_GAME_RESP && buffer->Length() >= sizeof(StreamPort))
    {
        StreamPort status;
        *buffer >> status;
        if (status == INVALID_PORT)
        {
            LOG_ERROR << "Start game failed\n";
            return;
        }
        m_gamePort = status;
        LOG_INFO << "Start game success, port from server " << m_gamePort << std::endl;
        if (!m_gameSocketInput.Connect(m_serverIp, m_gamePort))
        {
            LOG_ERROR << "Failed to connect to game\n";
            return;
        }
        POLL_ADD_SOCKET_RECV(m_socketPoll, m_gameSocketInput, &Satellite::OnClose, &Satellite::OnRecvControl);
    } else
    {
        LOG_ERROR << "Unknow code: " << header.code << std::endl;
    }
}

void Satellite::OnClose(WsaSocket* sock, BufferStream10KB* buffer)
{
    if (*sock == m_gameSocketInput)
    {
        m_status = Status::DISCONNECTED;
    }
    sock->Release();
}

void Satellite::OnRecvControl(WsaSocket* sock, BufferStream10KB* buffer)
{
    if (buffer->Length() < MSG_HEADER_LENGTH) return;
    buffer->SetCurrentPosition(0);

    MessageHeader header;
    *buffer >> header;
    if (header.code == Message::MSG_RESOLUTION && buffer->Length() >= 2 * sizeof(int))
    {
        int w, h;
        char bpp;
        *buffer >> w >> h >> bpp;
        m_gameWidth = w;
        m_gameHeight = h;
        m_bytePerPixel = bpp;
        m_resolutionChangeEvent.Signal();
        m_status = Status::RECEIVING_STREAM;
    } else
    {
        LOG_DEBUG << "Unknow code " << header.code << std::endl;
        throw std::exception("Invalid message code");
    }
    *buffer >> header;
    if (header.code == Message::MSG_INIT && buffer->Length() >= sizeof(uint32_t))
    {
        uint32_t numSocket;
        std::vector<uint32_t> bytePerSocket;
        *buffer >> numSocket;
        for (std::size_t i = 0; i < numSocket; i++)
        {
            uint32_t byte;
            *buffer >> byte;
            bytePerSocket.push_back(byte);
        }
        if (!m_streamController.Init(m_serverIp, m_gamePort + 1, bytePerSocket)) throw std::exception("Failed to init stream controller");
        m_callbackPoll.AddEvent(m_streamController.GetSignal(), [this](const Event* e)
        {
            this->m_frameFunc(this->m_streamController.GetFrame());
            if (this->m_streamController.NumFrameRemain() == 0) e->Reset();
            else
                LOG_DEBUG << "Nuim: " << this->m_streamController.NumFrameRemain() << '\n';
            return PollAction::NONE;
        });
    }
    buffer->SetCurrentPosition(buffer->Length());
}

void Satellite::InternalThread()
{
    m_signal.Signal();
    m_socketPoll.PollSocket(INFINITE);
    LOG_DEBUG << "Close InternalThread\n";
}

void Satellite::Finalize()
{
    if (m_status == Status::FINALIZE) return;
    m_signal.Signal();
    m_streamController.Stop();
    if (m_thread.joinable()) m_thread.join();
    m_status = Status::FINALIZE;
}

bool Satellite::OnFinalize(const Event* event)
{
    return false;
}

bool Satellite::PollEvent(std::size_t timeout)
{
    m_callbackPoll.PollOnce(timeout);
    return true;
}

bool Satellite::CloseGame()
{
    BufferStream1KB stream;
    MessageHeader header = CreateHeaderMsg(Message::MSG_STOP_GAME);
    stream << header;
    m_gameSocketInput.SendAll(stream.Get(), stream.Length());
    return true;
}
