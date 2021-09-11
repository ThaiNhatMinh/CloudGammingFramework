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
        LastSocketError();
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

bool WsaSocket::operator==(const WsaSocket& other)
{
    return m_handle == other.m_handle;
}

bool WsaSocket::Open(unsigned short port)
{
    AutoCloseSocket Listen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Listen == INVALID_SOCKET)
    {
        LastSocketError();
        return false;
    }
    SOCKADDR_IN InternetAddr;
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);

    if (bind(Listen.get(), (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) != 0)
    {
        LastSocketError();
        return false;
    }

    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(Listen.get(), event.get(), FD_ACCEPT | FD_CLOSE) != 0)
    {
        LastSocketError();
        return false;
    }
    if (listen(Listen.get(), SOMAXCONN) != 0)
    {
        LastSocketError();
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
        LastSocketError();
        return false;
    }
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &(remote.sin_addr)) == -1)
    {
        LastSocketError();
        return false;
    }

    remote.sin_port = htons(port);
    if (connect(handle.get(), (struct sockaddr *)&remote, sizeof(struct sockaddr)) != 0)
    {
        LastSocketError();
        return false;
    }
    AutoCloseEvent event = WSACreateEvent();
    if (WSAEventSelect(handle.get(), event.get(), FD_READ | FD_WRITE | FD_CLOSE) != 0)
    {
        LastSocketError();
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
        LastSocketError();
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
        LastSocketError();
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

std::size_t WsaSocket::SendAll(const void* buffer, int length) const
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
                LastSocketError();
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
        LastSocketError();
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
        LastSocketError();
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
    while (true)
    {
        // Wait for network events on all sockets
        std::size_t eventSize = m_events.size();
        std::size_t timerSize = m_timers.size();
        std::size_t eventTotal = m_sockets.size() + eventSize + timerSize;
        std::size_t Index = WSAWaitForMultipleEvents(eventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
        if (Index == WSA_WAIT_FAILED)
        {
            LastError();
            continue;
        }
        Index = Index - WSA_WAIT_EVENT_0;
        if (Index < eventSize)
        {
            if (!(this->*m_events[Index].callback)(m_events[Index].event))
                break;
            continue;
        } else if (Index < timerSize + eventSize)
        {
            int IndexTimer = Index - eventSize;
            (this->*m_timers[IndexTimer].callback)(m_timers[IndexTimer].timer);
            continue;
        }

        int IndexSocket = Index - eventSize - timerSize;
        WSANETWORKEVENTS NetworkEvents;
        WSAEnumNetworkEvents(m_sockets[IndexSocket].socket->GetHandle(), EventArray[Index], &NetworkEvents);
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
            (this->*m_sockets[IndexSocket].acceptCallback)(accept(m_sockets[IndexSocket].socket->GetHandle(), nullptr, nullptr));
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
            int numRecv = m_sockets[IndexSocket].socket->Recv(buffer);
            if (numRecv != SOCKET_ERROR)
            {
                while (m_sockets[IndexSocket].recvBuffer.Length() + buffer.length() > m_sockets[IndexSocket].recvBuffer.Capacity())
                {
                    // LOG_DEBUG << "Full capacity: " << buffer.length() << " " << m_sockets[IndexSocket].recvBuffer.Length() << std::endl;
                    (this->*m_sockets[IndexSocket].recvCallback)(&m_sockets[IndexSocket]);
                }
                m_sockets[IndexSocket].recvBuffer << buffer;
                (this->*m_sockets[IndexSocket].recvCallback)(&m_sockets[IndexSocket]);
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
            LOG_DEBUG << "Socket " << m_sockets[IndexSocket].socket->GetHandle() << " is writeable\n";
        }

        if (NetworkEvents.lNetworkEvents & FD_CLOSE)
        {
            if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            {
                LOG_ERROR << "FD_CLOSE failed with error:";
                LastErrorWithCode(NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
            }
            OnClose(&m_sockets[IndexSocket]);
            m_sockets.erase(m_sockets.begin() + IndexSocket);
            UpdateArray();
        }
    }
}

void WsaSocketPollEvent::UpdateArray()
{
    std::size_t eventTotal = m_sockets.size() + m_events.size() + m_timers.size();
    int index = 0;
    for (auto iter = m_events.begin(); iter != m_events.end(); iter++, index++)
    {
        EventArray[index] = iter->event->GetHandle();
    }
    for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++, index++)
    {
        EventArray[index] = iter->timer->GetHandle();
    }
    for (auto iter = m_sockets.begin(); iter != m_sockets.end(); iter++, index++)
    {
        EventArray[index] = iter->socket->GetEvent();
    }
}

bool WsaSocketPollEvent::AddSocket(WsaSocket& newSocket, SocketCallback recvCallback, AcceptCallback acceptCallback)
{
    if (m_sockets.size() + m_events.size() + m_timers.size() > WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }
    WsaSocketInformation info;
    info.socket = &newSocket;
    info.recvCallback = recvCallback;
    info.acceptCallback = acceptCallback;
    m_sockets.push_back(info);
    UpdateArray();
    return true;
}

bool WsaSocketPollEvent::AddEvent(const Event& event, EventCallback callback)
{
    if (m_sockets.size() + m_events.size() + m_timers.size() > WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }

    EventInformation info;
    info.event = &event;
    info.callback = callback;
    m_events.push_back(info);
    UpdateArray();
    return true;
}

bool WsaSocketPollEvent::AddTimer(const WaitableTimer& timer, TimerCallback callback)
{
    if (m_sockets.size() + m_events.size() + m_timers.size() > WSA_MAXIMUM_WAIT_EVENTS)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }

    TimerInformation info;
    info.timer = &timer;
    info.callback = callback;
    m_timers.push_back(info);
    UpdateArray();
    return true;
}
