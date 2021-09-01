#include "Satellite.hh"
#include "common/BufferStream.hh"
#include "common/Message.hh"

bool Satellite::Connect(const std::string& ip, unsigned short port)
{
    std::stringstream ss;
    ss << this;
    if (!m_serverSocket.Connect(ip, port) || !m_signal.Create("Local\\Satellite" + ss.str()))
    {
        return false;
    }

    m_serverIp = ip;
    AddSocket(m_serverSocket, nullptr, static_cast<SocketCallback>(&Satellite::OnRecvServer));
    AddEvent(m_signal, static_cast<EventCallback>(&Satellite::OnFinalize));
    m_thread = std::thread(&Satellite::InternalThread, this);
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
    // TODO: Check if client already start game
    return true;
}

bool Satellite::SendInput(InputEvent event)
{
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
    MessageHeader header;
    sock->recvBuffer >> header;
    // if (header.code == Message::MSG_RESOLUTION && )
    LOG_DEBUG << header.code << std::endl;
}

void Satellite::InternalThread()
{
    WsaSocketPollEvent::PollEvent();
}

void Satellite::Finalize()
{
    if (!m_signal.Signal())
    {
        m_thread.detach();
        return;
    }
    m_thread.join();
}

bool Satellite::OnFinalize(const Event* event)
{
    return false;
}
