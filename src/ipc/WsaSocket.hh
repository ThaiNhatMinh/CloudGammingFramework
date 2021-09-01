#include <list>
#include <string>
#include <vector>

#include "common/AutoClose.hh"
#include "common/BufferStream.hh"
#include "common/NonCopyable.hh"
#include "Event.hh"
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
    WsaSocket() {};
    WsaSocket(SOCKET handle);
    WsaSocket(WsaSocket&& other);
    WsaSocket& operator=(WsaSocket&& other);
    bool operator==(const WsaSocket& other);
    bool Open(unsigned short port);
    bool Connect(std::string ip, unsigned short port);
    bool ConnectToHost(std::string host, unsigned short port);

    int Send(const void* buffer, int length) const;
    int SendAll(const void* buffer, int length) const;
    int Recv(std::string& buffer) const;

    WSAEVENT GetEvent() const { return m_event.get(); };
    SOCKET GetHandle() const { return m_handle.get(); };

    void Release();
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
        BufferStream<MAX_BUFFER> recvBuffer;
        BufferStream<MAX_BUFFER> sendBuffer;
        /** Call before send */
        callback sendCallback;
        /** Call after recvice data */
        callback recvCallback;
    };
    
private:
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    std::list<WsaSocketInformation> m_sockets;
    std::vector<HANDLE> m_event;
    const Event* m_exit;

public:
    virtual void OnAccept(WsaSocket&& newSocket) {};
    virtual void OnClose(WsaSocketInformation* sock) {};
    bool AddSocket(const WsaSocket& newSocket, callback sendCallback = nullptr, callback recvCallback = nullptr);
    void SetExitEvent(const Event& exitEvent) { m_exit = &exitEvent; UpdateArray();};
    void PollEvent();
    void UpdateArray();
};
