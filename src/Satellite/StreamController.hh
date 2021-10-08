#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "cgf/cgf.hh"
#include "common/Frames.hh"
#include "ipc/Event.hh"
#include "ipc/PollHandle.hh"
#include "ipc/WsaSocket.hh"
#include "StreamController.hh"

class StreamController
{
private:
    struct SockPackage
    {
        std::thread recvThread;
        WsaSocket socket;
        uint32_t numByte;
        uint32_t offset;
        uint32_t currentByte;
        Event stopPollEvent;
        PollHandle<2> poll;
        bool isRecv;
    };

private:
    std::vector<SockPackage> m_data;
    std::atomic_uint32_t m_numFinish;
    uint32_t m_frameCount;
    Frames m_frames;
    std::mutex m_lock;
    std::unique_ptr<char[]> m_currentFrame;
    Event m_frameCompleteEvent;
    Event m_stopThreadEvent;

public:
    bool Init(const std::string& ip, StreamPort startPort, const std::vector<uint32_t>& bytePerSocket);
    Event& GetSignal() { return m_frameCompleteEvent; }
    const char* GetFrame() { return m_frames.PopFront(); }
    uint32_t NumFrameRemain() { return m_frames.Length(); }
    void Stop();

private:
    void PollThread(int slot);
    void OnClose(WsaSocket*, BufferStream<MAX_BUFFER>*);
    void OnRecv(int slot, WsaSocket*, BufferStream<MAX_BUFFER>*);
};