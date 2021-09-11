#include "Win32.hh"
#include "Module.hh"
#include "Timer.hh"

Timer::Timer():m_CurrentTime(0),m_DeltaTime(0),m_PrevTime(0), m_StartTime(0)
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&m_TickPerSecond))
    {
        LastError();
        throw std::exception("Can not query performance frequency");
    }

    Reset();
}

double Timer::GetGameTime() const
{
    double duration = (m_CurrentTime - m_StartTime) * 1000000.0;
    return (duration / m_TickPerSecond);

}

double Timer::GetDeltaTime() const
{
    return m_DeltaTime;
}

void Timer::Reset()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTime);
    m_PrevTime = m_StartTime;
}

void Timer::Tick()
{
    // Get the time this frame.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrentTime);

    // Time difference between this frame and the previous.
    m_DeltaTime = (m_CurrentTime - m_PrevTime) * 1000000.0 / m_TickPerSecond;

    // Prepare for next frame
    m_PrevTime = m_CurrentTime;

    static double timepass = 0;
    static int fps = 0;
    fps++;
    timepass += m_DeltaTime;
    if (timepass >= 1.0)
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
