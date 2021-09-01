#include "common/Message.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/cgf.hh"
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

    BufferStream1KB stream;
    InputEvent event;
    event.key.key = Key::KEY_A;
    event.key.action = Action::PRESSING;
    stream << event;
    static char buffer[64];
    CreateInputMsg(buffer, 64, event);
    while (true)
    {
        std::string msg;
        std::cin >> msg;
        std::size_t numSend = 0;
        while (numSend < MSG_INPUT_PACKAGE_SIZE)
        {
            numSend += sock.SendAll(buffer, MSG_INPUT_PACKAGE_SIZE);
        }
        if (msg == "EXIT") break;
        std::cout << "SendALL" << std::endl;

        sock.Recv(msg);
        std::cout << "Server respose:" << msg << std::endl;
    }
    
    return 0;
}