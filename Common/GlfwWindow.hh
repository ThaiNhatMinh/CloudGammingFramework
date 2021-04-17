#pragma once
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
    friend class Vulkan;

public:
    Window();
    Window(uint32_t width, uint32_t height, const std::string& name);
    ~Window();

    void HandleEvent();
    int ShouldClose();
    void SwapBuffer();
    void Resize(uint32_t width, uint32_t height);
    void SetName(const std::string& name);
    void OnResize(int width, int height);
    GLFWwindow* GetGlfw();

private:
static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

};