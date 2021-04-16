#include "RenderStream.hh"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

RenderStream* instance;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

RenderStream::RenderStream()
{
    instance = this;
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

    if (uMsg == WM_KEYUP)
    {
        if (wParam == VK_INSERT)
        {
            m_ShowMenu = !m_ShowMenu;
        }

    }

    if (m_ShowMenu)
    {
        ImGui_ImplWin32_WndProcHandler(m_hwnd, uMsg, wParam, lParam);
        return true;
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