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
        uint32_t frameCounter;
        long long sendTime;
        std::chrono::duration<float, std::milli> delta;
    };

    struct DownloadingFrame
    {
        std::atomic_uint32_t numFinish;
        std::unique_ptr<char[]> frame;
        DownloadingFrame() = default;
        DownloadingFrame(uint32_t size): numFinish(0), frame(new char[size]) {}
    };

private:
    std::vector<SockPackage> m_data;
    uint32_t m_frameCount;
    Frames m_frames;
    std::mutex m_lock;
    std::vector<std::unique_ptr<DownloadingFrame>> m_downloading;
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