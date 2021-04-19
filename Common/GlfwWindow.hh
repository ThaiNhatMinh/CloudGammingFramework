#pragma once
#include <functional>
#include <string>

#include "GLFW/glfw3.h"

class Window
{
private:
    const uint32_t WIDTH = 400;
    const uint32_t HEIGHT = 300;
    const std::string DEFAULT_NAME = "Client";

private:
    bool m_bIsWindowResize;
    GLFWwindow* m_pWindow;
    uint32_t m_width;
    uint32_t m_height;
    std::string m_name;
    std::function<void(int, int)> m_moveCallback;
    friend class Vulkan;

public:
    Window();
    Window(uint32_t width, uint32_t height, const std::string& name);
    ~Window();

    void HandleEvent();
    int ShouldClose();
    void SwapBuffer();
    void EnableVsync(bool enable);
    void Resize(uint32_t width, uint32_t height);
    void SetName(const std::string& name);
    void GetSize(uint32_t* width, uint32_t* height);
    void MakeContext();
    GLFWwindow* GetGlfw();
    void SetWinMoveCallback(std::function<void(int, int)> callback);

protected:
    void OnResize(int width, int height);
    void OnMove(int x, int y);

private:
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void WindowMoveCallback(GLFWwindow* window, int x, int y);

};