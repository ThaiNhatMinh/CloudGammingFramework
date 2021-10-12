#include "common/Message.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/cgf.hh"
#include <iostream>
#include <string>

int main()
{
    return -1;
    WsaSocket::Init();
    WsaSocket sock;
    if (!sock.Connect("127.0.0.1", 4568))
    {
        std::cout << "Failed to connect\n";
        return 0;
    }

    std::unique_ptr<char[]> buffer;
    int w = 1920, h=1080, bpp = 3;
    buffer.reset(new char[w*h*bpp + MSG_HEADER_LENGTH]);
    MessageHeader header;
    header.code = Message::MSG_FRAME;
    std::memcpy(buffer.get(), &header, MSG_HEADER_LENGTH);
    for (int i = 0; i< w * h * bpp; i++)
    {
        buffer[i + MSG_HEADER_LENGTH] = 'A' + i % 26;
    }
    buffer[MSG_HEADER_LENGTH + 0] = 1;
    buffer[MSG_HEADER_LENGTH + 1] = 1;
    buffer[MSG_HEADER_LENGTH + 2] = 1;
    buffer[ w*h*bpp - 1] = 1;
    buffer[ w*h*bpp - 2] = 1;
    buffer[ w*h*bpp - 3] = 1;
    int i = 0;
    while (i++ < 200000)
    {
        std::size_t numSend = 0;
        while (numSend < w*h*bpp + MSG_HEADER_LENGTH)
        {
            numSend += sock.SendAll(buffer.get(), w*h*bpp + MSG_HEADER_LENGTH);
            if (numSend == 0) return 0;
            std::cout << "Error " << i << " " << numSend <<std::endl;
        }

        Sleep(1000/60);
    }
    
    return 0;
}