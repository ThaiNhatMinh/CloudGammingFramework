#pragma once
#include <string>
#if __linux__
#include <sys/socket.h>
#elif defined(WIN32) || defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#endif

class Socket
{
public:
    static void InitSocket();
    static void DestroySocket();

    Socket();
    ~Socket();
    Socket(int handle);
    Socket(Socket&&);
    Socket& operator=(Socket&&);
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    bool Connect(std::string ip, USHORT port);
    bool ConnectToHost(std::string host, USHORT port);
    bool Listen(std::size_t port);
    int Send(const std::string& buffer);
    int SendAll(const std::string& buffer);
    int RecvAll(std::string& data, uint32_t timeout);
    int Recv(std::string& data);
    Socket Accept(sockaddr_in*);
    bool IsValid();
    int SetOpt(int level, int optname, const char* optval, int optlen);
    void EnableNonblock();

    int GetHandle();
private:

    int m_Handle;
};