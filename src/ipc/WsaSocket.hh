#include <list>
#include <string>
#include <vector>

#include "common/AutoClose.hh"
#include "common/NonCopyable.hh"
#include "Win32.hh"
#include "Event.hh"

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
    WsaSocket() {};
    WsaSocket(SOCKET handle);
    WsaSocket(WsaSocket&& other);
    WsaSocket& operator=(WsaSocket&& other);
    bool Open(unsigned short port);
    bool Connect(std::string ip, unsigned short port);
    bool ConnectToHost(std::string host, unsigned short port);

    int Send(const void* buffer, int length) const;
    int SendAll(const void* buffer, int length) const;
    int Recv(std::string& buffer) const;

    WSAEVENT GetEvent() const { return m_event.get(); };
    SOCKET GetHandle() const { return m_handle.get(); };
};

class WsaEvent
{
private:
    WsaSocket::AutoCloseEvent m_event;

public:
    
};

class WsaSocketPollEvent
{
protected:
    const static std::size_t MAX_BUFFER = 4096;
    struct WsaSocketInformation;
    typedef void (WsaSocketPollEvent::*callback)(WsaSocketInformation* info);
    struct WsaSocketInformation
    {
        const WsaSocket* socket;
        std::size_t bytesSend;
        std::size_t bytesRecv;
        char sendBuffer[MAX_BUFFER];
        char recvBuffer[MAX_BUFFER];
        /** Call before send */
        callback sendCallback;
        /** Call after recvice data */
        callback recvCallback;
    };
    
private:
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    std::list<WsaSocketInformation> m_sockets;
    Event m_exit;

public:
    virtual void OnAccept(WsaSocket&& newSocket) {};
    bool AddSocket(const WsaSocket& newSocket, callback sendCallback = nullptr, callback recvCallback = nullptr);
    void SetExitEvent(Event&& exitEvent) { m_exit = std::move(exitEvent); UpdateArray();};
    void PollEvent();
    void UpdateArray();
};
