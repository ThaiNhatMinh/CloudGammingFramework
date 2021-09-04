#pragma once
#include "Win32.hh"
#include <string>
#include <functional>

class PollHandle
{
public:
    typedef std::function<bool(HANDLE)> EventCallback;

private:
    EventCallback m_callbacks[MAXIMUM_WAIT_OBJECTS];
    HANDLE m_handles[MAXIMUM_WAIT_OBJECTS];
    std::size_t m_size;

public:
    PollHandle();
    void Poll(std::size_t timeout);
    bool Add(HANDLE handle, EventCallback callback);
    void Remove(HANDLE handle);
};