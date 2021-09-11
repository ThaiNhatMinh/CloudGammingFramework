#pragma once
#include <string>
#include <vector>

#include "common/AutoClose.hh"
#include "common/BufferStream.hh"
#include "common/NonCopyable.hh"
#include "Event.hh"
#include "WaitableTimer.hh"
#include "Win32.hh"

class WsaSocket: public NonCopyable
{
public:
    static void Init();
    typedef AutoClose<SOCKET, closesocket, INVALID_SOCKET> AutoCloseSocket;
    typedef AutoClose<WSAEVENT, WSACloseEvent, INVALID_HANDLE_VALUE> AutoCloseEvent;

private:
    AutoCloseSocket m_handle;
    AutoCloseEvent m_event;

public:
    WsaSocket() { TRACE;};
    ~WsaSocket() { Release();}
    WsaSocket(SOCKET handle);
    WsaSocket(WsaSocket&& other);
    WsaSocket& operator=(WsaSocket&& other);
    bool operator==(const WsaSocket& other);
    bool Open(unsigned short port);
    bool Connect(std::string ip, unsigned short port);
    bool ConnectToHost(std::string host, unsigned short port);

    int Send(const void* buffer, int length) const;
    std::size_t SendAll(const void* buffer, int length) const;
    int Recv(std::string& buffer) const;

    WSAEVENT GetEvent() const { return m_event.get(); };
    SOCKET GetHandle() const { return m_handle.get(); };

    void Release();
};

class WsaSocketPollEvent
{
protected:
    const static std::size_t MAX_BUFFER = 10240;
    struct WsaSocketInformation;
    typedef void (WsaSocketPollEvent::*SocketCallback)(WsaSocketInformation* info);
    typedef void (WsaSocketPollEvent::*AcceptCallback)(WsaSocket&& newSocket);
    typedef bool (WsaSocketPollEvent::*EventCallback)(const Event* info);
    typedef void (WsaSocketPollEvent::*TimerCallback)(const WaitableTimer* timer);
    struct WsaSocketInformation
    {
        WsaSocket* socket;
        BufferStream<MAX_BUFFER> recvBuffer;
        /** Call after recvice data */
        SocketCallback recvCallback;
        AcceptCallback acceptCallback;
    };

    struct EventInformation
    {
        const Event* event;
        EventCallback callback;
    };

    struct TimerInformation
    {
        const WaitableTimer* timer;
        TimerCallback callback;
    };
    
private:
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    std::vector<WsaSocketInformation> m_sockets;
    std::vector<EventInformation> m_events;
    std::vector<TimerInformation> m_timers;

public:
    virtual void OnClose(WsaSocketInformation* sock) {};
    bool AddSocket(WsaSocket& newSocket, SocketCallback recvCallback = nullptr, AcceptCallback accept = nullptr);
    bool AddEvent(const Event& event, EventCallback callback);
    bool AddTimer(const WaitableTimer& timer, TimerCallback callback);
    void PollEvent();
    void UpdateArray();
};
