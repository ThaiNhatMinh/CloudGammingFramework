#pragma once
#include <string>

class FpsLocker
{
public:
    const std::size_t ONE_SEC_IN_MICRO = 1000000;
private:
    double m_frameTime;
    __int64 m_StartTime;
    __int64 m_TickPerSecond;
public:
    FpsLocker();

    void SetFps(int fps);
    void FrameStart();
    void FrameEnd();
};