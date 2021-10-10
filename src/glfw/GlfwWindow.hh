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
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(int, int, int)> m_mouseCallback;
    std::function<void(float, float)> m_mouseMoveCallback;
    std::function<void(unsigned int)> m_charCallback;
    std::function<void(double, double)> m_scrollCallback;
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
    void GetFramebufferSize(uint32_t* width, uint32_t* height);
    void MakeContext();
    GLFWwindow* GetGlfw();
    void* GetNativeHandle();
    void SetWinMoveCallback(std::function<void(int, int)> callback);
    void SetKeyCallback(std::function<void(int, int, int, int)> callback);
    void SetMouseCallback(std::function<void(int, int, int)> callback);
    void SetMouseMoveCallback(std::function<void(float, float)> callback);
    void SetInputTextCallback(std::function<void(unsigned int)> callback);
    void SetScrollCallback(std::function<void(double, double)> callback);

protected:
    void OnResize(int width, int height);
    void OnMove(int x, int y);
    void OnKey(int key, int scancode, int action, int mods);
    void OnMouse(int button, int action, int mods);
    void OnMouseMove(double x, double y);
    void OnChar(unsigned int c);
    void OnScroll(double xoffset, double yoffset);

private:
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void WindowMoveCallback(GLFWwindow* window, int x, int y);
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseCallback(GLFWwindow*,int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow*, double xpos, double ypos);
    static void CharCallback(GLFWwindow*, unsigned int c);
    static void ScrollCallback(GLFWwindow*, double xoffset, double yoffset);
};