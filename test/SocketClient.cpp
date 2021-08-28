#include "ipc/WsaSocket.hh"
#include <iostream>
#include <string>

int main()
{
    WsaSocket::Init();
    WsaSocket sock;
    if (!sock.Connect("127.0.0.1", 4568))
    {
        std::cout << "Failed to connect\n";
        return 0;
    }

    while (true)
    {
        std::string msg;
        std::cin >> msg;
        std::size_t numSend = 0;
        while (numSend < msg.length())
        {
            numSend += sock.SendAll(msg.data(), msg.length());
        }
        if (msg == "EXIT") break;
        std::cout << "SendALL" << std::endl;

        sock.Recv(msg);
        std::cout << "Server respose:" << msg << std::endl;
    }
    
    return 0;
}