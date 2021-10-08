#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <list>
#include "cgf/cgf.hh"
#include "ipc/WsaSocket.hh"
#include "ipc/PollHandle.hh"
#include "ipc/Event.hh"
#include "StreamInformation.hh"
#include "common/Timer.hh"

class StreamController
{
private:
    enum Status
    {
        ILDE,
        SENDING
    };
    struct Package
    {
        Event sendEvent;
        Event stopEvent;
        std::thread thread;
        Status status;
        WsaSocket socket;
        uint32_t offset;
        uint32_t length;
        PollHandle<2> poll;
    };
    
private:
    StreamInformation m_info;
    std::list<Package> m_packages;
    std::list<WsaSocket> m_listenSocket;
    // std::list<WsaSocket> m_clientSocket;
    // std::vector<std::unique_ptr<char[]>> m_buffers;
    // std::vector<std::thread> m_sendThread;
    // std::vector<uint32_t> m_byteSend;
    StreamPort m_startPort;
    uint32_t m_totalByte;
    uint32_t m_frameCounter;
    std::thread m_pollThread;
    Event m_stopPollEvent;
    Event m_sendFrameEvent;
    std::unique_ptr<char[]> m_buffer;
    Timer m_timer;
    PollHandle64 m_socketPoll;
    bool m_bFullSocket;

public:
    void SetInformation(const StreamInformation& info, StreamPort startPort);
    void SetFrame(const void* pData);
    void StopPoll();
    std::vector<uint32_t> GetBytePerSocket();
private:
    void OnEmptyRecv(WsaSocket* newConnect, BufferStream10KB* buffer);
    void OnAccept(WsaSocket* newConnect, BufferStream10KB* buffer);
    void OnClose(WsaSocket* sock, BufferStream10KB* buffer);
    void InternalThread();
    void SendThread(Package* package);
    PollAction SendData(Package* package, const Event* e);
};
