#include "Socket.h"
#include <iostream>
#include <sstream>
#if __linux__
#include <fcntl.h> //fcntl
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)
#elif defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#endif
#include "Logger.hh"


void SocketErrror()
{
#ifdef WIN32
    int error = WSAGetLastError();
    LOG << "[Socket] Error:" << error << std::endl;
#else
    LOG << "[Socket] Error:" << errnostr(errno) << std::endl;
#endif
}

Socket::Socket() : m_Handle(INVALID_SOCKET)
{
}

Socket::~Socket()
{
#ifdef WIN32
    closesocket(m_Handle);
#elif __linux__
    close(m_Handle);
#endif
}

Socket::Socket(Socket &&other)
{
    this->m_Handle = other.m_Handle;
    other.m_Handle = INVALID_SOCKET;
}

Socket &Socket::operator=(Socket &&other)
{
    if (this == &other)
        return *this;
    this->m_Handle = other.m_Handle;
    other.m_Handle = INVALID_SOCKET;

    return *this;
}

bool Socket::Connect(std::string ip, USHORT port)
{
    m_Handle = socket(AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS);
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &(remote.sin_addr)) <= 0)
    {
        std::cerr << "[Socket] Can't set remote sin_addr";
        SocketErrror();
        return 0;
    }

    remote.sin_port = htons(port);
    if (connect(m_Handle, (struct sockaddr *)&remote, sizeof(struct sockaddr)) < 0)
    {
        LOG << "[Socket] Connect error" << std::endl;
        SocketErrror();
        return 0;
    }

    return 1;
}

bool Socket::ConnectToHost(std::string host, USHORT port)
{
    int iResult;
    std::stringstream ss;
    ss << port;

    //struct addrinfo *result = NULL,	*ptr = NULL;

    // // Resolve the server address and port
    // auto iResult = getaddrinfo(host.c_str(), ss.str().c_str(), &hints, &result);
    // if (iResult != 0) {

    // 	std::cout << "[Socket] Can't get address info: " << host << ":" << port << std::endl;
    // 	return 0;
    // }

    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host.c_str(), "http", &hints, &servinfo)) != 0)
    {
        std::cout << "[Socket] Can't get address info: " << host << ":" << port << std::endl;
        SocketErrror();
        return false;
    }

    bool r = false;
    for (auto ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        m_Handle = socket(ptr->ai_family, ptr->ai_socktype,
                          ptr->ai_protocol);
        if (m_Handle != INVALID_SOCKET)
        {
            iResult = connect(m_Handle, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR)
            {
#ifdef WIN32
                closesocket(m_Handle);
#elif __linux__
                close(m_Handle);
#endif
                r = false;
            }
            else
            {
                r = true;
                break;
            }
        }
    }

    freeaddrinfo(servinfo);

    return r;
}

bool Socket::Listen(std::size_t port)
{
    TRACE;
    std::stringstream ss;
    ss << port;
    struct addrinfo *result = NULL, *ptr = NULL, hints;

#ifdef WIN32
    ZeroMemory(&hints, sizeof(hints));
#endif
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    // Resolve the local address and port to be used by the server
    auto iResult = getaddrinfo(NULL, ss.str().c_str(), &hints, &result);
    if (iResult != 0)
    {
        LOG << "[Socket] getaddrinfo failed: " << iResult << std::endl;
        SocketErrror();
        return 0;
    }
    LOG << "Socket: " << result->ai_family << " ai_socktype: " << result->ai_socktype << " ai_protocol:" << result->ai_protocol << std::endl;
    m_Handle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_Handle == INVALID_SOCKET)
    {
        LOG << "[Socket] Error at socket()" << std::endl;
        SocketErrror();
        return 0;
    }

    iResult = bind(m_Handle, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        LOG << "[Socket] bind failed\n";
        SocketErrror();
        freeaddrinfo(result);
        return 0;
    }

    if (listen(m_Handle, SOMAXCONN) == SOCKET_ERROR)
    {
        LOG << "[Socket] Listen failed with error" << std::endl;
        SocketErrror();
        return 0;
    }

    return 1;
}

int Socket::Send(const std::string &buffer)
{
    // 	int querylen = buffer.length();
    // 	const char* p = buffer.c_str();
    // 	int sent = 0;
    // 	while (sent < querylen)
    // 	{
    // 		auto tmpres = send(m_Handle, p + sent, querylen - sent, 0);
    // 		if (tmpres == -1)
    // 		{
    // #if __linux__
    // 			LOG <<"[Socket] Send Error: %s\n", strerror(errno));
    // #elif defined(WIN32)
    // 			int error = WSAGetLastError();
    // 			LOG <<"[Socket] Send Error: %d", error);
    // #endif

    // 			return -1;
    // 		}
    // 		sent += tmpres;
    // 	}

    int nsend = 0;
    nsend = send(m_Handle, buffer.data(), buffer.length(), 0);

    if (nsend == -1)
    {
        LOG <<"[Socket] Send error\n";
        SocketErrror();
    }
    return nsend;
}

int Socket::SendAll(const std::string &buffer)
{
    int sent = 0;
    int bufferLength = buffer.size();
    const char* pBuffer = buffer.c_str();
    while (sent < bufferLength)
    {
        int nsend = send(m_Handle, pBuffer, bufferLength - sent, 0);
        if (nsend < 0)
        {
            LOG <<"[Socket] Send error\n";
            SocketErrror();
            return -1;
        }
        sent += nsend;
    }

    return sent;
}

int Socket::RecvAll(std::string &data, uint32_t timeout)
{
    static char data2[1924 * 1024 * 10];
    memset(data2, 0, sizeof(data2));

    char *p = data2;
    int size_recv, total_size = 0;

    char chunk[BUFSIZ];

#if __linux__
    struct timeval begin, now;
    double timediff;
    //make socket non blocking
    fcntl(m_Handle, F_SETFL, O_NONBLOCK);
    //beginning time
    gettimeofday(&begin, NULL);
#elif defined(WIN32)
    DWORD begin, now;
    DWORD timediff;
    u_long iMode = 1;
    ioctlsocket(m_Handle, FIONBIO, &iMode);
    begin = GetTickCount();
#endif
    //loop

    while (1)
    {
#if __linux__
        gettimeofday(&now, NULL);
        //time elapsed in seconds
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
#elif defined(WIN32)
        now = GetTickCount();
        //time elapsed in seconds
        timediff = (DWORD)((now - begin) * 0.001f);
#endif

        //if you got some data, then break after timeout
        if (total_size > 0 && timediff > timeout)
        {
            break;
        }

        //if you got no data at all, wait a little longer, twice the timeout
        else if (timediff > timeout * 2)
        {
            break;
        }

        memset(chunk, 0, BUFSIZ); //clear the variable
        if ((size_recv = recv(m_Handle, chunk, BUFSIZ, 0)) < 0)
        {
#if __linux__
            usleep(100000);
#elif defined(WIN32)
            Sleep(100);
#endif
        }
        else
        {
            total_size += size_recv;
            //LOG <<"%s", chunk);
            //data2.append(chunk);
            memcpy(p, chunk, size_recv);
            p += size_recv;
            //reset beginning time
#if __linux__
            gettimeofday(&now, NULL);
#elif defined(WIN32)
            now = GetTickCount();
#endif
        }
    }

    data.resize(total_size);
    memcpy(&data[0], data2, total_size);
    return total_size;
}

int Socket::Recv(std::string &data)
{
    char chunk[BUFSIZ];
    int size_recv = recv(m_Handle, chunk, BUFSIZ, 0);
    if (size_recv > 0)
    {
        data.assign(chunk, size_recv);
    }
    else if (size_recv < 0)
    {
        SocketErrror();
    }
    return size_recv;
}

Socket Socket::Accept(sockaddr_in *remoteaddr)
{

    auto ClientSocket = INVALID_SOCKET;

    // Accept a client socket
#if defined(WIN32)
    int addrlen = sizeof(sockaddr_in);
    ClientSocket = accept(m_Handle, (struct sockaddr *)remoteaddr, &addrlen);
#elif defined(__linux__)
    int addrlen = sizeof(sockaddr);
    ClientSocket = accept(m_Handle, (struct sockaddr *)remoteaddr, (socklen_t *)&addrlen);
#endif
    if (ClientSocket == INVALID_SOCKET)
    {
        return Socket();
    }

    return std::move(Socket(ClientSocket));
}

bool Socket::IsValid()
{
    return m_Handle != INVALID_SOCKET;
}

int Socket::SetOpt(int level, int optname, const char *optval, int optlen)
{
    return setsockopt(m_Handle, level, optname, optval, optlen);
}

void Socket::EnableNonblock()
{
#if __linux__
    int flags = fcntl(m_Handle, F_GETFL, 0);
    fcntl(m_Handle, F_SETFL, flags | O_NONBLOCK);
#elif defined(WIN32)
    u_long iMode = 1;
    ioctlsocket(m_Handle, FIONBIO, &iMode);
#endif
}

Socket::Socket(int handle) : m_Handle(handle)
{
}

int Socket::GetHandle()
{
    return m_Handle;
}

void Socket::InitSocket()
{
#ifdef WIN32
    static WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        LOG << "WSAStartup failed: " << iResult;
        return;
    }
#endif
}

void Socket::DestroySocket()
{
#ifdef WIN32
    WSACleanup();
#endif
}