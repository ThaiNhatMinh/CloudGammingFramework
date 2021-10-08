#include "common/Message.hh"
#include "ipc/Event.hh"
#include "ipc/PollHandle.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/cgf.hh"
#include <iostream>
#include "common/Frames.hh"
struct Frame
    {
        std::unique_ptr<char[]> data;
        uint32_t length;
    };

const std::size_t size = 1920*1080*3;
class Server
{
    Event event;
    WsaSocket client;
    WsaSocket sock;
    std::string buffer;
    bool m_bIsReceivingFrame = false;
    Frame m_currentFrame;
    Frames m_frames;
    PollHandle64 m_socketPoll;

public:
    Server()
    {
        if (!sock.Open(4568) || !event.Create("Local\\AAA"))
        {
            throw std::exception();
        }
        m_socketPoll.AddEvent(event, std::bind(&Server::OnExit, this, std::placeholders::_1));
        POLL_ADD_SOCKET_LISTEN(m_socketPoll, sock, &Server::OnClose, &Server::OnAccept);
        m_currentFrame.data.reset(new char[size]);
        m_currentFrame.length = 0;
        m_frames.Init(20, size);
    }

    void OnAccept(WsaSocket* newSocket, BufferStream10KB* buffer)
    {
        client = std::move(*newSocket);
        POLL_ADD_SOCKET_RECV(m_socketPoll, client, &Server::OnClose, &Server::OnRecv);
    }

    PollAction OnExit(const Event* event)
    {
        return PollAction::STOP_POLL;
    }

    void OnClose(WsaSocket* sock, BufferStream10KB* buffer)
    {
        sock->Release();
    }

    void OnRecv(WsaSocket* sock, BufferStream10KB* buffer)
    {
        if (buffer->Length() < MSG_HEADER_LENGTH) return;
        buffer->SetCurrentPosition(0);
        if (m_bIsReceivingFrame)
        {
            std::size_t byteRemain = size - m_currentFrame.length;
            std::size_t byteToCopy = buffer->Length();
            if (byteToCopy > byteRemain) byteToCopy = byteRemain;
            buffer->Extract(m_currentFrame.data.get() + m_currentFrame.length, byteToCopy);
            LOG_DEBUG << buffer->GetCurrentPosition() << ":" << buffer->Length() << std::endl;
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
            buffer->SetCurrentPosition(buffer->Length());
            return;
        }

        MessageHeader header;
        *buffer >> header;
        if (header.code == Message::MSG_FRAME)
        {
            m_bIsReceivingFrame = true;
        } else
        {
            LOG_DEBUG << "Unknow code " << header.code << std::endl;
            exit(-1);
        }
        buffer->SetCurrentPosition(buffer->Length());
    }

    void OnSend(WsaSocket* sock, BufferStream10KB* buffer)
    {
        
        // sock->sendBuffer << msg;
        // sock->bytesSend += msg.length();
        // if (sock->socket->SendAll(stream.Get(), stream.Get) != msg.length())
        // {
        //     std::cout << "ERROR\n";
        // }
    }

    void Poll()
    {
        m_socketPoll.PollSocket(INFINITE);
    }

};

int main()
{
    WsaSocket::Init();
    
    Server server;
    std::cout << "Start\n";
    server.Poll();
    return 0;
}