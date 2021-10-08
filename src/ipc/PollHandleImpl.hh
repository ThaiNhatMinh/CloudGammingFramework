#pragma once
#include <algorithm>
#include "common/Logger.hh"
#include "PollHandle.hh"
#include "common/Module.hh"

template <uint8_t size>
PollHandle<size>::PollHandle() : m_size(0)
{
    std::memset(m_handles, 0, MAXIMUM_WAIT_OBJECTS * sizeof(HANDLE));
}

template <uint8_t size>
void PollHandle<size>::Poll(std::size_t timeout)
{
    if (!m_sockets.empty())
    {
        throw std::exception("Use PollHandle::PollSocket to poll");
    }

    while (true)
    {
        DWORD index = WaitForMultipleObjects(static_cast<DWORD>(m_size), m_handles, false, static_cast<DWORD>(timeout));
        if (index == WAIT_FAILED)
        {
            LASTERROR;
            throw std::exception("Poll failed");
        }
        else if (index == WAIT_TIMEOUT)
        {
            continue;
        }
        index -= WAIT_OBJECT_0;
        // Wait for network events on all sockets
        std::size_t handleSize = m_handless.size();
        std::size_t eventSize = m_events.size();
        std::size_t timerSize = m_timers.size();
        PollAction action;
        // TODO: Improve this code
        if (index < handleSize)
        {
            action = m_handless[index].second(m_handless[index].first);
        }
        else if (index < handleSize + eventSize)
        {
            action = m_events[index - handleSize].second(m_events[index - handleSize].first);
        }
        else
        {
            action = m_timers[index - handleSize - eventSize].second(m_timers[index - handleSize - eventSize].first);
        }
        if (action == PollAction::STOP_POLL)
        {
            break;
        }
        else if (action == PollAction::REMOVE)
        {
            Remove(m_handless, m_handless[index].first);
        }
    }
}

template <uint8_t size>
void PollHandle<size>::PollOnce(std::size_t timeout)
{
    if (!m_sockets.empty())
    {
        throw std::exception("Use PollHandle::PollSocket to poll");
    }
    DWORD index = WaitForMultipleObjects(static_cast<DWORD>(m_size), m_handles, false, static_cast<DWORD>(timeout));
    if (index == WAIT_FAILED || index == WAIT_TIMEOUT)
    {
        return;
    }
    index -= WAIT_OBJECT_0;
    // Wait for network events on all sockets
    std::size_t handleSize = m_handless.size();
    std::size_t eventSize = m_events.size();
    std::size_t timerSize = m_timers.size();
    PollAction action;
    // TODO: Improve this code
    if (index < handleSize)
    {
        action = m_handless[index].second(m_handless[index].first);
    }
    else if (index < handleSize + eventSize)
    {
        action = m_events[index - handleSize].second(m_events[index - handleSize].first);
    }
    else
    {
        action = m_timers[index - handleSize - eventSize].second(m_timers[index - handleSize - eventSize].first);
    }
    if (action == PollAction::REMOVE)
    {
        Remove(m_handless, m_handless[index].first);
    }
}


template <uint8_t size>
void PollHandle<size>::Dump()
{
    if (!m_handless.empty())
    {
        for (auto iter = m_handless.begin(); iter != m_handless.end(); iter++)
        {
            LOG_DEBUG << "HANDLE: " << iter->first << ' ';
        }
        LOG << '\n';
    }
    if (!m_events.empty())
    {
        for (auto iter = m_events.begin(); iter != m_events.end(); iter++)
        {
            LOG_DEBUG << "m_events: " << iter->first->GetHandle() << ' ';
        }
        LOG << '\n';
    }
    if (!m_timers.empty())
    {
        for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++)
        {
            LOG_DEBUG << "m_timers: " << iter->first->GetHandle() << ' ';
        }
        LOG << '\n';
    }
    if (!m_sockets.empty())
    {
        for (auto iter = m_sockets.begin(); iter != m_sockets.end(); iter++)
        {
            LOG_DEBUG << "m_sockets: " << iter->socket->GetHandle() << ' ';
        }
        LOG << '\n';
    }
}

template <uint8_t size>
void PollHandle<size>::PollSocket(std::size_t timeout)
{
    Dump();
    while (true)
    {
        // Wait for network events on all sockets
        std::size_t handleSize = m_handless.size();
        std::size_t eventSize = m_events.size();
        std::size_t timerSize = m_timers.size();
        DWORD index = WSAWaitForMultipleEvents(static_cast<DWORD>(m_size), m_handles, FALSE, static_cast<DWORD>(timeout), FALSE);
        if (index == WSA_WAIT_FAILED)
        {
            LASTERROR;
            throw std::exception("Poll failed");
        }
        index = index - WSA_WAIT_EVENT_0;
        PollAction action = PollAction::NONE;
        if (index < handleSize)
        {
            action = m_handless[index].second(m_handless[index].first);
        }
        else if (index < handleSize + eventSize)
        {
            action = m_events[index - handleSize].second(m_events[index - handleSize].first);
        }
        else if (index < handleSize + eventSize + timerSize)
        {
            action = m_timers[index - handleSize - eventSize].second(m_timers[index - handleSize - eventSize].first);
        }
        else
        {
            // TODO: Split to smaller function
            std::size_t indexsocket = index - handleSize - eventSize - timerSize;
            WSANETWORKEVENTS NetworkEvents;
            WSAEnumNetworkEvents(m_sockets[indexsocket].socket->GetHandle(), m_handles[index], &NetworkEvents);
            // Check for FD_ACCEPT messages
            if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
            {
                if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                {
                    LOG_ERROR << "FD_ACCEPT failed with error:\n";
                    LastErrorWithCode(NetworkEvents.iErrorCode[FD_ACCEPT_BIT], __FUNCTION__, __LINE__);
                    break;
                }
                // Accept a new connection, and add it to the socket and event lists
                WsaSocket newSock = accept(m_sockets[indexsocket].socket->GetHandle(), nullptr, nullptr);
                m_sockets[indexsocket].acceptCallback(&newSock, nullptr);
            }

            // Process FD_READ notification
            if (NetworkEvents.lNetworkEvents & FD_READ)
            {
                if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
                {
                    LOG_ERROR << "FD_READ failed with error:\n";
                    LastErrorWithCode(NetworkEvents.iErrorCode[FD_READ_BIT], __FUNCTION__, __LINE__);
                    break;
                }
                // Read data from the socket
                std::string buffer;
                int numRecv = m_sockets[indexsocket].socket->Recv(buffer);
                if (numRecv != SOCKET_ERROR)
                {
                    while (m_sockets[indexsocket].recvBuffer.Length() + buffer.length() > m_sockets[indexsocket].recvBuffer.Capacity())
                    {
                        m_sockets[indexsocket].recvCallback(m_sockets[indexsocket].socket, &m_sockets[indexsocket].recvBuffer);
                    }
                    m_sockets[indexsocket].recvBuffer << buffer;
                    m_sockets[indexsocket].recvCallback(m_sockets[indexsocket].socket, &m_sockets[indexsocket].recvBuffer);
                }
            }

            // Process FD_WRITE notification
            if (NetworkEvents.lNetworkEvents & FD_WRITE)
            {
                if (NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
                {
                    LOG_ERROR << "FD_WRITE failed with error:\n";
                    LastErrorWithCode(NetworkEvents.iErrorCode[FD_WRITE_BIT], __FUNCTION__, __LINE__);
                    break;
                }
                LOG_DEBUG << "Socket " << m_sockets[indexsocket].socket->GetHandle() << " is writeable\n";
            }

            if (NetworkEvents.lNetworkEvents & FD_CLOSE)
            {
                if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
                {
                    LOG_ERROR << "FD_CLOSE failed with error:";
                    LastErrorWithCode(NetworkEvents.iErrorCode[FD_CLOSE_BIT], __FUNCTION__, __LINE__);
                }
                m_sockets[indexsocket].closeCallback(m_sockets[indexsocket].socket, &m_sockets[indexsocket].recvBuffer);
                m_sockets.erase(m_sockets.begin() + indexsocket);
                UpdateArray();
            }
        }

        if (action == PollAction::STOP_POLL)
        {
            break;
        }
        else if (action == PollAction::REMOVE)
        {
            Remove(m_handless, m_handless[index].first);
            continue;
        }
    }
}

template <uint8_t size>
bool PollHandle<size>::Add(HANDLE handle, HandleCallback callback)
{
    if (m_size >= size)
    {
        LOG_ERROR << "Reach maximum object\n";
        return false;
    }
    if (callback == nullptr)
    {
        LOG_ERROR << "Callback must non null\n";
        return false;
    }

    m_handless.push_back({handle, callback});
    UpdateArray();
    return true;
}

template <uint8_t size>
void PollHandle<size>::UpdateArray()
{
    int index = 0;
    for (auto iter = m_handless.begin(); iter != m_handless.end(); iter++, index++)
    {
        m_handles[index] = iter->first;
    }
    for (auto iter = m_events.begin(); iter != m_events.end(); iter++, index++)
    {
        m_handles[index] = iter->first->GetHandle();
    }
    for (auto iter = m_timers.begin(); iter != m_timers.end(); iter++, index++)
    {
        m_handles[index] = iter->first->GetHandle();
    }
    for (auto iter = m_sockets.begin(); iter != m_sockets.end(); iter++, index++)
    {
        m_handles[index] = iter->socket->GetEvent();
    }
    m_size = index;
}

template <uint8_t size>
bool PollHandle<size>::AddSocketForListen(WsaSocket &newSocket, SocketCallback closeCallback, SocketCallback acceptCallback)
{
    if (m_size > size)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }
    WsaSocketInformation info;
    info.socket = &newSocket;
    info.recvCallback = nullptr;
    info.acceptCallback = acceptCallback;
    info.closeCallback = closeCallback;
    m_sockets.push_back(info);
    UpdateArray();
    return true;
}

template <uint8_t size>
bool PollHandle<size>::AddSocketForRecv(WsaSocket &newSocket, SocketCallback closeCallback, SocketCallback recvCallback)
{
    if (m_size > size)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }
    WsaSocketInformation info;
    info.socket = &newSocket;
    info.recvCallback = recvCallback;
    info.acceptCallback = nullptr;
    info.closeCallback = closeCallback;
    m_sockets.push_back(info);
    UpdateArray();
    return true;
}

template <uint8_t size>
bool PollHandle<size>::AddEvent(const Event &event, EventCallback callback)
{
    if (m_size > size)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }
    m_events.push_back({&event, callback});
    UpdateArray();
    return true;
}

template <uint8_t size>
bool PollHandle<size>::AddTimer(const WaitableTimer &timer, TimerCallback callback)
{
    if (m_size > size)
    {
        LOG_ERROR << "Reach maximmum connections/event\n";
        return false;
    }

    m_timers.push_back({&timer, callback});
    UpdateArray();
    return true;
}