#include <chrono>
#include <sstream>
#include <thread>

#include "WsaSocket.hh"
#include "common/Module.hh"
#include "common/Logger.hh"

WsaSocket::WsaSocket(SOCKET handle): m_handle(handle)
{
    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(handle, event.get(), FD_READ | FD_WRITE | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return;
    }
    m_event = std::move(event);
}

WsaSocket::WsaSocket(WsaSocket&& other)
{
    m_handle = std::move(other.m_handle);
    m_event = std::move(other.m_event);
}

WsaSocket& WsaSocket::operator=(WsaSocket&& other)
{
    m_handle = std::move(other.m_handle);
    m_event = std::move(other.m_event);
    return *this;
}

bool WsaSocket::operator==(const WsaSocket& other)
{
    return m_handle == other.m_handle;
}

bool WsaSocket::operator!=(const WsaSocket& other)
{
    return m_handle != other.m_handle;
}

bool WsaSocket::Open(unsigned short port)
{
    AutoCloseSocket Listen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Listen == INVALID_SOCKET)
    {
        LASTSOCKETERROR
        return false;
    }
    SOCKADDR_IN InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (bind(Listen.get(), (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(Listen.get(), event.get(), FD_ACCEPT | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return false;
    }
    if (listen(Listen.get(), SOMAXCONN) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    m_handle = std::move(Listen);
    m_event = std::move(event);
    return true;
}


bool WsaSocket::Connect(std::string ip, USHORT port)
{
    AutoCloseSocket handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (handle == INVALID_SOCKET)
    {
        LASTSOCKETERROR
        return false;
    }
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &(remote.sin_addr)) == -1)
    {
        LASTSOCKETERROR
        return false;
    }

    remote.sin_port = htons(port);
    if (connect(handle.get(), (struct sockaddr *)&remote, sizeof(struct sockaddr)) != 0)
    {
        LASTSOCKETERROR
        return false;
    }
    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(handle.get(), event.get(), FD_READ | FD_WRITE | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    m_handle = std::move(handle);
    m_event = std::move(event);
    return true;
}

bool WsaSocket::ConnectToHost(std::string host, USHORT port)
{
    std::stringstream ss;
    ss << port;

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), ss.str().c_str(), &hints, &servinfo) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    SOCKET handle = INVALID_SOCKET;
    bool r = false;
    for (auto ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (handle == INVALID_SOCKET) continue;
        int iResult = connect(handle, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == 0) break;
    }
    freeaddrinfo(servinfo);
    if (handle ==INVALID_SOCKET) return false;
    m_handle = handle;
    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(handle, event.get(), FD_READ | FD_WRITE | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    m_event = std::move(event);
    return true;
}

int WsaSocket::Send(const void* buffer, uint32_t length) const
{
    return send(m_handle.get(), static_cast<const char*>(buffer), length, 0);
}

uint32_t WsaSocket::SendAll(const void* buffer, uint32_t length) const
{
    int sent = 0;
    int bufferLength = length;
    const char* pBuffer = static_cast<const char*>(buffer);
    while (sent < bufferLength)
    {
        int nsend = send(m_handle.get(), pBuffer + sent, bufferLength - sent, 0);
        if (nsend == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);
                TRACE;
            }else
            {
                LASTSOCKETERROR
                return sent;
            }
        } else sent += nsend;
    }
    return sent;
}

int WsaSocket::Recv(std::string& buffer) const
{
    char data[4096];
    int rc = recv(m_handle.get(), data, sizeof(data), 0);
    if (rc != SOCKET_ERROR)
    {
        buffer.resize(rc, 0);
        std::memcpy(const_cast<char*>(buffer.c_str()), data, rc);
        return rc;
    } else
    {
        LASTSOCKETERROR
        return SOCKET_ERROR;
    }
}

void WsaSocket::Init()
{
    static WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        LASTSOCKETERROR
    }
}

void WsaSocket::Release()
{
    m_event.Release();
    m_handle.Release();
}
