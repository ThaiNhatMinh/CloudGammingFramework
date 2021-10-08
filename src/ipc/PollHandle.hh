#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Win32.hh"
#include "Event.hh"
#include "WaitableTimer.hh"
#include "WsaSocket.hh"

const static std::size_t MAX_BUFFER = 10240;

enum PollAction
{
    NONE,
    STOP_POLL,
    REMOVE
};

template<uint8_t size = MAXIMUM_WAIT_OBJECTS>
class PollHandle
{
public:
    typedef std::function<PollAction(HANDLE)> HandleCallback;
    typedef std::function<PollAction(const Event*)> EventCallback;
    typedef std::function<PollAction(const WaitableTimer*)> TimerCallback;
    typedef std::function<void(WsaSocket*, BufferStream<MAX_BUFFER>*)> SocketCallback;

private:
    struct WsaSocketInformation
    {
        WsaSocket* socket;
        BufferStream<MAX_BUFFER> recvBuffer;
        /** Call after recvice data */
        SocketCallback recvCallback;
        SocketCallback closeCallback;
        SocketCallback acceptCallback;
    };

private:
    std::size_t m_size;
    HANDLE m_handles[MAXIMUM_WAIT_OBJECTS];
    std::vector<WsaSocketInformation> m_sockets;
    std::vector<std::pair<HANDLE, HandleCallback>> m_handless;
    std::vector<std::pair<const Event*, EventCallback>> m_events;
    std::vector<std::pair<const WaitableTimer*, TimerCallback>> m_timers;

public:
    PollHandle();
    void Poll(std::size_t timeout);
    void PollOnce(std::size_t timeout);
    void PollSocket(std::size_t timeout);

    bool Add(HANDLE handle, HandleCallback callback);
    bool AddSocketForRecv(WsaSocket &newSocket, SocketCallback closeCallback, SocketCallback recvCallback);
    bool AddSocketForListen(WsaSocket &newSocket, SocketCallback closeCallback, SocketCallback accept);
    bool AddEvent(const Event& event, EventCallback callback);
    bool AddTimer(const WaitableTimer& timer, TimerCallback callback);

    void UpdateArray();

    template<class T, class Callback>
    void Remove(std::vector<std::pair<T, Callback>>& list, T value)
    {
        for (auto& iter = list.begin(); iter != list.end(); iter++)
        {
            if (iter->first == value)
            {
                list.erase(iter);
                break;
            }
        }

        UpdateArray();
    }
    void Dump();
};

#include "PollHandleImpl.hh"

typedef PollHandle<64> PollHandle64;

#define POLL_ADD_SOCKET_RECV(poll, sock, closeMethod, recvMethod) (poll).AddSocketForRecv((sock), std::bind((closeMethod), this, std::placeholders::_1, std::placeholders::_2), std::bind((recvMethod), this, std::placeholders::_1, std::placeholders::_2))

#define POLL_ADD_SOCKET_LISTEN(poll, sock, closeMethod, acceptMethod) (poll).AddSocketForListen((sock), std::bind((closeMethod), this, std::placeholders::_1, std::placeholders::_2), std::bind((acceptMethod), this, std::placeholders::_1, std::placeholders::_2))