#include <functional>
#include "StreamProtocol.hh"
#include "Logger.hh"
#include "RenderStream.hh"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

RenderStream* instance;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

RenderStream::RenderStream():m_pBuffer(nullptr), m_ShowMenu(false)
{
    Socket::InitSocket();
    if (!m_server.Listen(PORT))
    {
        LOG << "Open port " << PORT << " failed" << std::endl;
    }
    // m_server.EnableNonblock();
    m_serverRunning = true;
    m_thread = std::thread(&RenderStream::ServerThread, this);
    instance = this;
}

RenderStream::~RenderStream()
{
    m_serverRunning = false;
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return instance->hWndProc(uMsg, wParam, lParam);
}

LRESULT RenderStream::hWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT mPos;
    GetCursorPos(&mPos);
    ScreenToClient(m_hwnd, &mPos);
    ImGui::GetIO().MousePos.x = (float)mPos.x;
    ImGui::GetIO().MousePos.y = (float)mPos.y;

    if (uMsg == WM_KEYUP && wParam == VK_INSERT)
    {
        m_ShowMenu = !m_ShowMenu;
    }
    if (uMsg == WM_KEYUP && wParam == VK_DELETE)
    {
        SendFrame();
    }

    if (m_ShowMenu)
    {
        ImGui_ImplWin32_WndProcHandler(m_hwnd, uMsg, wParam, lParam);
    }
    return CallWindowProc(m_OriginalWndProcHandler, m_hwnd, uMsg, wParam, lParam);
}

void RenderStream::TickFrame()
{
    m_timer.Tick();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    //Menu is displayed when m_ShowMenu is TRUE
    if (m_ShowMenu)
    {
        ImGui::Begin("Hello, world!"); 
        ImGui::Text("FPS: %d, frame time: %f", m_timer.GetFPS(), m_timer.GetDeltaTime());
        ImGui::End();
    }
    ImGui::EndFrame();

    ImGui::Render();
}

void RenderStream::InitImGui(HWND hwnd)
{
    TRACE;
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui::GetIO().ImeWindowHandle = hwnd;
    m_OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)::hWndProc);
    m_hwnd = hwnd;
}

void RenderStream::SetFrameSize(std::size_t w, std::size_t h)
{
    TRACE;
    LOG << "Width: " << w << " Height: " << h << std::endl;
    if (m_pBuffer) delete[] m_pBuffer;
    m_pBuffer = new char[w * h * BYTE_PER_PIXEL];
    m_Width = w;
    m_Height = h;
}

char* RenderStream::GetBuffer()
{
    return m_pBuffer;
}

void RenderStream::SendFrame()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_condVar.notify_one();
}

void RenderStream::ServerThread()
{
    TRACE;
    struct sockaddr_in remoteaddr;
    while (true)
    {
        LOG << "Waiting for client...\n";
        Socket cl = m_server.Accept(&remoteaddr);
        if (cl.IsValid())
        {
            LOG << "Accecpt client from: " << remoteaddr.sin_port << std::endl;
            m_client = std::move(cl);
            std::string cmd = BuildSetupCommand(m_Width, m_Height, "ASDSD");
            LOG << "Sent SETUP: " << m_client.SendAll(cmd) << std::endl;
        }
        while (m_serverRunning)
        {
            std::unique_lock<std::mutex> mlock(m_mutex);
            LOG << "Start waiting...\n";
            m_condVar.wait(mlock);
            LOG << "Start sending...\n";
            std::string buffer = BuildFrameCommand(m_pBuffer, m_Width * m_Height * BYTE_PER_PIXEL + 1);
            if (m_client.IsValid())
            {
                int sent = m_client.SendAll(buffer);
                if (sent == -1)
                    break;
            }
        }
    }
}
