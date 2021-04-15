#include <Windows.h>

#include "Timer.h"

Timer::Timer():m_bStoped(0),m_CurrentTime(0),m_DeltaTime(0),m_PauseTime(0),m_PrevTime(0), m_SecondPerCount(0),m_StartTime(0),
m_StopTime(0)
{
    __int64 tickperSecond;
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&tickperSecond))
    {
        // system doesn't support hi-res timer
        return;
    }

    m_SecondPerCount = 1.0 / (double)tickperSecond;
    QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
}

Timer::~Timer()
{
}

float Timer::GetGameTime() const
{
    // If we are stopped, do not count the time that has passed since
    // we stopped.
    //
    // ----*---------------*------------------------------*------> time
    //  startTime       stopTime                      currentTime

    if (m_bStoped)
        return (float)((m_StopTime - m_StartTime) * m_SecondPerCount);

    // The distance currentTime - startTime includes paused time,
    // which we do not want to count. To correct this, we can subtract
    // the paused time from currentTime:
    //
    //  (mCurrTime - mPausedTime) - mBaseTime
    //
    //                     |<-------d------->|
    // ----*---------------*-----------------*------------*------> time
    // mBaseTime        mStopTime        startTime     mCurrTime
    else return (float)((m_CurrentTime - m_PauseTime - m_StartTime) * m_SecondPerCount);

}

float Timer::GetDeltaTime() const
{
    return (float)m_DeltaTime;
}

void Timer::Reset()
{
    __int64 currentTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
    m_StartTime = currentTime;
    m_PrevTime = currentTime;
    m_StopTime = 0;
    m_bStoped = false;
}

void Timer::Start()
{
    __int64 startTime;
    QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
    // Accumulate the time elapsed between stop and start pairs.
    //
    //                |<-------d------->|
    // ---------------*-----------------*------------> time
    //             m_StopTime        startTime

    if (m_bStoped)
    {
        m_PauseTime += (startTime - m_StopTime);
        m_PrevTime = startTime;
        m_StopTime = 0;
        m_bStoped = false;
    }
}

void Timer::Stop()
{
    // if we already stopped, then don't do anything
    if (!m_bStoped)
    {
        __int64 currentTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        m_StopTime = currentTime;
        m_bStoped = true;
    }
}

void Timer::Tick()
{
    if (m_bStoped)
    {
        m_DeltaTime = 0;
        return;
    }

    // Get the time this frame.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);

    // Time difference between this frame and the previous.
    m_DeltaTime = (m_CurrentTime - m_PrevTime) * m_SecondPerCount;

    // Prepare for next frame
    m_PrevTime = m_CurrentTime;

    static double timepass = 0;
    static int fps = 0;
    fps++;
    timepass += m_DeltaTime;
    if (timepass > 1.0)
    {
        m_FPS = fps;
        fps = 0;
        timepass = 0.0;
    }
}

int Timer::GetFPS()
{
    return m_FPS;
}
