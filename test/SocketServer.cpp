#include "common/Message.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/Event.hh"
#include "cgf/cgf.hh"
#include <iostream>
#include "Satellite/Frames.hh"
struct Frame
    {
        std::unique_ptr<char[]> data;
        std::size_t length;
    };

const std::size_t size = 1920*1080*3;
class Server : public WsaSocketPollEvent
{
    Event event;
    WsaSocket client;
    WsaSocket sock;
    std::string buffer;
    bool m_bIsReceivingFrame = false;
    Frame m_currentFrame;
    Frames m_frames;

public:
    Server()
    {
        if (!sock.Open(4568) || !event.Create("Local\\AAA"))
        {
            throw std::exception();
        }
        AddEvent(event, static_cast<EventCallback>(&Server::OnExit));
        AddSocket(sock, nullptr, static_cast<AcceptCallback>(&Server::OnAccept));
        m_currentFrame.data.reset(new char[size]);
        m_currentFrame.length = 0;
        m_frames.Init(20, size);
    }

    void OnAccept(WsaSocket&& newSocket)
    {
        client = std::move(newSocket);
        this->AddSocket(client, static_cast<SocketCallback>(&Server::OnRecv));
    }

    bool OnExit(const Event* event)
    {
        return false;
    }

    void OnRecv(WsaSocketInformation* sock)
    {
        if (sock->recvBuffer.Length() < MSG_HEADER_LENGTH) return;
        sock->recvBuffer.SetCurrentPosition(0);
        if (m_bIsReceivingFrame)
        {
            std::size_t byteRemain = size - m_currentFrame.length;
            std::size_t byteToCopy = sock->recvBuffer.Length();
            if (byteToCopy > byteRemain) byteToCopy = byteRemain;
            sock->recvBuffer.Extract(m_currentFrame.data.get() + m_currentFrame.length, byteToCopy);
            LOG_DEBUG << sock->recvBuffer.GetCurrentPosition() << ":" << sock->recvBuffer.Length() << std::endl;
            m_currentFrame.length += byteToCopy;
            if (m_currentFrame.length == size)
            {
                static int num = 0;
                num++;
                LOG_DEBUG << "Num " << num << std::endl;
                m_frames.PushBack(m_currentFrame.data.get());
                m_currentFrame.length = 0;
                m_bIsReceivingFrame = false;
            }
            sock->recvBuffer.SetCurrentPosition(sock->recvBuffer.Length());
            return;
        }

        MessageHeader header;
        sock->recvBuffer >> header;
        if (header.code == Message::MSG_FRAME)
        {
            m_bIsReceivingFrame = true;
        } else
        {
            LOG_DEBUG << "Unknow code " << header.code << std::endl;
            exit(-1);
        }
        sock->recvBuffer.SetCurrentPosition(sock->recvBuffer.Length());
    }

    void OnSend(WsaSocketInformation* sock)
    {
        
        // sock->sendBuffer << msg;
        // sock->bytesSend += msg.length();
        // if (sock->socket->SendAll(stream.Get(), stream.Get) != msg.length())
        // {
        //     std::cout << "ERROR\n";
        // }
    }

};

int main()
{
    WsaSocket::Init();
    
    Server server;
    std::cout << "Start\n";
    server.PollEvent();
    return 0;
}