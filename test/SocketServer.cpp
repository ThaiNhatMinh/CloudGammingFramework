#include "common/Message.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/Event.hh"
#include "cgf/cgf.hh"
#include <iostream>

class Server : public WsaSocketPollEvent
{
    Event event;
    WsaSocket client;
public:
    Server()
    {
        if (!event.Open("Local\\AAA"))
        {
            throw std::exception();
        }
    }
    void OnAccept(WsaSocket&& newSocket) override
    {
        client = std::move(newSocket);
        this->AddSocket(client, static_cast<callback>(&Server::OnSend), static_cast<callback>(&Server::OnRecv));
    }

    void OnRecv(WsaSocketInformation* sock)
    {
        std::string msg(sock->recvBuffer.Get(), sock->recvBuffer.Length());
        if (msg == "EXIT")
            event.Signal();
        // std::cout << "Recv: " << msg << std::endl;
        constexpr std::size_t size = sizeof(InputEvent);
        while (sock->recvBuffer.Length() >= MSG_INPUT_PACKAGE_SIZE)
        {
            MessageHeader header;
            sock->recvBuffer >> header;
            if (header.code == Message::MSG_INPUT)
            {
                InputEvent event;
                sock->recvBuffer >> event;
            } else
            {
                LOG_ERROR << "Unknow message code: " << header.code << std::endl;
            }
        }
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
    WsaSocket sock;
    Event event;
    if (!sock.Open(4568) || !event.Create("Local\\AAA"))
    {
        std::cout << "Failed to open\n";
    }
    Server server;
    server.SetExitEvent(std::move(event));
    server.AddSocket(sock);
    std::cout << "Start\n";
    server.PollEvent();
    return 0;
}