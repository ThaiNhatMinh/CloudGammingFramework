#include <sstream>
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
    LOG_DEBUG << "Socket handle: " << handle << std::endl;
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
    LOG_DEBUG << "Socket handle: " << m_handle.get() << std::endl;
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
    if (WSAEventSelect(handle.get(), event.get(), FD_ACCEPT | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    m_handle = std::move(handle);
    m_event = std::move(event);
    LOG_DEBUG << "Socket handle: " << m_handle.get() << std::endl;
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
    if (WSAEventSelect(handle, event.get(), FD_ACCEPT | FD_CLOSE) != 0)
    {
        LASTSOCKETERROR
        return false;
    }

    m_event = std::move(event);
    LOG_DEBUG << "Socket handle: " << m_handle.get() << std::endl;
    return true;
}

int WsaSocket::Send(const void* buffer, int length) const
{
    return send(m_handle.get(), static_cast<const char*>(buffer), length, 0);
}

int WsaSocket::SendAll(const void* buffer, int length) const
{
    int sent = 0;
    int bufferLength = length;
    const char* pBuffer = static_cast<const char*>(buffer);
    while (sent < bufferLength)
    {
        int nsend = send(m_handle.get(), pBuffer + sent, bufferLength - sent, 0);
        if (nsend == SOCKET_ERROR)
        {
            LASTSOCKETERROR
            return sent;
        }
        sent += nsend;
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

//==========================================================================

void WsaSocketPollEvent::PollEvent()
{
    UpdateArray();
    while (true)
    {
        // Wait for network events on all sockets
        std::size_t eventTotal = m_sockets.size() + 1;
        int Index = WSAWaitForMultipleEvents(eventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
        Index = Index - WSA_WAIT_EVENT_0;
        if (Index == 0) break; // exit event is signal

        auto sock = m_sockets.begin();
        for (std::size_t i = 1; i < Index; i++)
        {
            sock = ++sock;
        }
        WSANETWORKEVENTS NetworkEvents;
        WSAEnumNetworkEvents(sock->socket->GetHandle(), EventArray[Index], &NetworkEvents);
        // Check for FD_ACCEPT messages
        if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                LOG_ERROR << "FD_ACCEPT failed with error:";
                LastErrorWithCode(NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
                break;
            }
            // Accept a new connection, and add it to the socket and event lists
            OnAccept(accept(sock->socket->GetHandle(), NULL, NULL));
        }

        // Process FD_READ notification
        if (NetworkEvents.lNetworkEvents & FD_READ)
        {
            if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
                LOG_ERROR << "FD_READ failed with error:";
                LastErrorWithCode(NetworkEvents.iErrorCode[FD_READ_BIT]);
                break;
            }
            // Read data from the socket
            std::string buffer;
            int numRecv = sock->socket->Recv(buffer);
            if (numRecv != SOCKET_ERROR)
            {
                std::memcpy(sock->recvBuffer + sock->bytesRecv, buffer.data(), numRecv);
                sock->bytesRecv += numRecv;
                if (sock->recvCallback)
                    (this->*sock->recvCallback)(&(*sock));
            }
        }

        // Process FD_WRITE notification
        if (NetworkEvents.lNetworkEvents & FD_WRITE)
        {
            if (NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
            {
                LOG_ERROR << "FD_WRITE failed with error:";
                LastErrorWithCode(NetworkEvents.iErrorCode[FD_WRITE_BIT]);
                break;
            }
            if (sock->sendCallback)
                (this->*sock->sendCallback)(&(*sock));
            if (sock->bytesSend > 0)
            {
                int numSend = sock->socket->Send(sock->sendBuffer, sock->bytesSend);
                if (numSend != SOCKET_ERROR)
                {
                    sock->bytesSend -= numSend;
                    std::memmove(&sock->sendBuffer + numSend, sock->sendBuffer, sock->bytesSend);
                }
            }
        }

        if (NetworkEvents.lNetworkEvents & FD_CLOSE)
        {
            if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            {
                LOG_ERROR << "FD_CLOSE failed with error:";
                LastErrorWithCode(NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
            }
            sock = m_sockets.erase(sock);
            UpdateArray();
        }
    }
}

void WsaSocketPollEvent::UpdateArray()
{
    std::size_t eventTotal = m_sockets.size();
    EventArray[0] = m_exit->GetHandle();
    int index = 1;
    for (auto iter = m_sockets.begin(); iter != m_sockets.end(); iter++, index++)
    {
        EventArray[index] = iter->socket->GetEvent();
    }
}

bool WsaSocketPollEvent::AddSocket(const WsaSocket& newSocket, callback sendCallback, callback recvCallback)
{
    if (m_sockets.size() + 1 >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG_ERROR << "Reach maximmum connections\n";
        return false;
    }
    WsaSocketInformation info;
    info.socket = &newSocket;
    info.bytesRecv = 0;
    info.bytesSend = 0;
    info.sendCallback = sendCallback;
    info.recvCallback = recvCallback;
    m_sockets.push_back(info);
    UpdateArray();
    return true;
}
