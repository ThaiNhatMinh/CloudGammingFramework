
#pragma once
#include <stdio.h>

class Timer
{
private:
    __int64 m_StartTime;
    __int64 m_PauseTime;
    __int64 m_StopTime;
    __int64 m_PrevTime;
    __int64 m_CurrentTime;
    bool m_bStoped;
    double m_SecondPerCount;
    double m_DeltaTime;
    int m_FPS;
public:
    Timer();
    ~Timer();
    float GetGameTime() const; // in second
    float GetDeltaTime() const; // in second

    /**
     * Call before message loop
     */
    void Reset();
    /**
     * Call when unpause
     */
    void Start();
    /**
     * Call when pause
     */
    void Stop();
    /**
     * Call every frame
     */
    void Tick();

    int GetFPS();

};
