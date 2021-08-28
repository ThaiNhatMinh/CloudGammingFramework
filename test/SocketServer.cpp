#include "ipc/WsaSocket.hh"
#include "ipc/Event.hh"
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
        std::string msg(sock->recvBuffer, sock->bytesRecv);
        if (msg == "EXIT")
            event.Signal();
        std::cout << "Recv: " << msg << std::endl;
        sock->bytesRecv = 0;
    }

    void OnSend(WsaSocketInformation* sock)
    {
        std::string msg = "Hello from server!!";
        std::memcpy(sock->sendBuffer + sock->bytesSend, msg.data(), msg.length());
        sock->bytesSend += msg.length();
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
    server.AddSocket(sock);
    server.SetExitEvent(std::move(event));
    std::cout << "Start\n";
    server.PollEvent();
    return 0;
}