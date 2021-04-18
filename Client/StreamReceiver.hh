#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD2
#include "imgui/imgui.h"
#include "GlfwWindow.hh"
#include "StreamProtocol.hh"
#include "Socket.h"
#include "Vector2.hh"

class StreamReceiver
{
private:
    class State
    {
    protected:
        StreamReceiver* m_owner;
    public:
        State(StreamReceiver* owner);
        virtual ~State() = default;
        virtual void Draw() = 0;
    };

    class HomeState : public State
    {
    private:
        char m_ip[20];
        int m_port;
    public:
        HomeState(StreamReceiver* owner);
        void Draw();
    };

    class Streamming : public State
    {
    private:
        V2uInt32 m_size;
        GLuint m_texture;
        std::mutex m_frameLock;
        std::queue<FrameCommand> m_frames;
        std::atomic_bool m_update;
    public:
        Streamming(StreamReceiver* owner, V2uInt32 size);
        void Draw();
        void Update(const FrameCommand& frame);
    private:
        void Update();
        GLuint CreateTexture();
    };

private:
    V2uInt32 m_size;
    Socket m_socket;
    Window* m_pWindow;
    std::unique_ptr<State> m_pCurrentState;
    std::thread m_thread;
public:
    StreamReceiver(Window* pWindow);
    ~StreamReceiver();
    void Draw();
    bool ConnectTo(const std::string& ip, int port);

private:
    void Receive();
};