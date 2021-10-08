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
    WsaSocket() = default;
    ~WsaSocket() { Release();}
    WsaSocket(SOCKET handle);
    WsaSocket(WsaSocket&& other);
    WsaSocket& operator=(WsaSocket&& other);
    bool operator==(const WsaSocket& other);
    bool operator!=(const WsaSocket& other);
    bool Open(unsigned short port);
    bool Connect(std::string ip, unsigned short port);
    bool ConnectToHost(std::string host, unsigned short port);

    int Send(const void* buffer, uint32_t length) const;
    uint32_t SendAll(const void* buffer, uint32_t length) const;
    int Recv(std::string& buffer) const;

    WSAEVENT GetEvent() const { return m_event.get(); };
    SOCKET GetHandle() const { return m_handle.get(); };
    bool IsValid() { return m_handle != INVALID_SOCKET;}

    void Release();
};
