#include <chrono>
#include <thread>
#include "Win32.hh"
#include "common/Module.hh"
#include "FpsLocker.hh"

double MS = 1000.0;
FpsLocker::FpsLocker()
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&m_TickPerSecond))
    {
        LASTERROR;
        throw std::exception("Can not query performance frequency");
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
}

void FpsLocker::SetFps(int fps)
{
    m_frameTime = (1.0 / fps) * m_TickPerSecond;
    LOG_DEBUG << "Frame time: " << m_frameTime << " m_TickPerSecond: " << m_TickPerSecond << std::endl;
}

void FpsLocker::FrameStart()
{
    // QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
}

void FpsLocker::FrameEnd()
{
    __int64 currentTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
    __int64 duration = currentTime - m_StartTime;
    if (duration < m_frameTime)
    {
        double sleepTime = (m_frameTime - duration) / m_TickPerSecond * 1000;
        // LOG_DEBUG << "duration time: " << duration << " sleepTime: " << sleepTime << std::endl;
        std::this_thread::sleep_for(std::chrono::duration<double, std::micro>(sleepTime));
    } else
    {
        LOG_ERROR << "Large frame" << duration << std::endl;
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
    // double sleep = ((m_StartTime - currentTime) *1000)/ m_TickPerSecond;
    // LOG_DEBUG << "Time: " << sleep + duration*1000/ m_TickPerSecond << " Sleep time: " << sleep << std::endl;
}