#pragma once
#include <string>
#include "Handle.hh"
#include "common/NonCopyable.hh"

class WaitableTimer: public NonCopyable
{
private:
    std::string m_name;
    AutoCloseHandle m_handle;

public:
    bool Create(const std::string &name, bool manualReset = false);
    bool Open(const std::string &name);
    bool SetTime(std::size_t timeInMs, std::size_t period = 0) const;
    bool Cancel() const;
    HANDLE GetHandle() const { return m_handle.get(); }

    WaitableTimer() = default;
    WaitableTimer& operator=(WaitableTimer&& other)
    {
        m_handle = std::move(other.m_handle);
        return *this;
    }

    WaitableTimer(WaitableTimer&& other)
    {
        m_handle = std::move(other.m_handle);
    }
};