#include "WaitableTimer.hh"
#include "common/Module.hh"

bool WaitableTimer::Create(const std::string &name, bool manualReset)
{
    HANDLE handle = CreateWaitableTimer(nullptr, manualReset, name.c_str());
    if (handle == INVALID_HANDLE_VALUE)
    {
        LASTERROR;
        return false;
    }
    m_name = name;
    m_handle = handle;
    return true;
}

bool WaitableTimer::Open(const std::string &name)
{
    HANDLE handle = OpenWaitableTimer(TIMER_ALL_ACCESS , FALSE, name.c_str());
    if (handle == INVALID_HANDLE_VALUE)
    {
        LASTERROR;
        return false;
    }
    m_name = name;
    m_handle = handle;
    return true;
}

bool WaitableTimer::SetTime(std::size_t timeInMs, std::size_t period) const
{
    LARGE_INTEGER large;
    large.QuadPart = timeInMs * 10000;
    large.QuadPart *= -1;

    if (SetWaitableTimer(m_handle.get(), &large, static_cast<LONG>(period), nullptr, nullptr, false) == FALSE)
    {
        LASTERROR;
        return false;
    }
    return true;
}

bool WaitableTimer::Cancel() const
{
    if (CancelWaitableTimer(m_handle.get()) == FALSE)
    {
        LASTERROR;
        return false;
    }
    return true;
}
