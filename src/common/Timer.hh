
#pragma once
#include <stdio.h>

class Timer
{
private:
    __int64 m_StartTime;
    __int64 m_PrevTime;
    __int64 m_CurrentTime;
    __int64 m_TickPerSecond;
    double m_DeltaTime;
    int m_FPS;
public:
    Timer();
    double GetGameTime() const; // in microsecond
    double GetDeltaTime() const; // in microsecond

    /**
     * Call before message loop
     */
    void Reset();

    /**
     * Call every frame
     */
    void Tick();

    int GetFPS();

};
