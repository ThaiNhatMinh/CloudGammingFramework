#pragma once
#include <string>
#if __linux__
#include <sys/socket.h>
#elif defined(WIN32) || defined(_WIN32)
#include <ws2tcpip.h>
#endif

class Socket
{
public:
    Socket();
    ~Socket();
    Socket(int handle);
    Socket(Socket&&);
    Socket& operator=(Socket&&);

    bool Connect(std::string ip, USHORT port);
    bool ConnectToHost(std::string host, USHORT port);
    bool Listen(std::size_t port);
    int Send(const std::string& buffer);
    int RecvAll(std::string& data, int timeout);
    int Recv(std::string& data);
    Socket Accept(sockaddr_in*);
    bool IsValid();
    int SetOpt(int level, int optname, const char* optval, int optlen);
    void EnableNonblock();

    int GetHandle();
private:

    int m_Handle;
};