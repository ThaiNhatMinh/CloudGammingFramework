#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Timer.h"
#include "Socket.h"

class RenderStream
{
public:
    const int PORT = 5678;
private:
    Timer m_timer;
    Socket m_server;
    Socket m_serverControl;
    Socket m_client;
    Socket m_clientControl;
    HWND m_hwnd;
    WNDPROC m_OriginalWndProcHandler = nullptr;
    bool m_ShowMenu;
    char* m_pBuffer;
    std::thread m_thread;
    std::thread m_threadControl;
    uint32_t m_Width;
    uint32_t m_Height;
    bool m_serverRunning;
    std::mutex m_mutex;
    std::condition_variable m_condVar;

public:
    RenderStream();
    ~RenderStream();
    void InitImGui(HWND hwnd);
    void TickFrame();
    void SetFrameSize(std::size_t width, std::size_t height);
    char* GetBuffer();
    void SendFrame();
    LRESULT hWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    void ServerThread();
    void ControlThread();
};