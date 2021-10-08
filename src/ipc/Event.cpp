#include "Event.hh"
#include "common/Module.hh"
#include "common/Logger.hh"

bool Event::Create(const std::string& name, bool initialState, bool manualReset)
{
    AutoCloseHandle handle = CreateEvent(nullptr, manualReset, initialState, name.c_str());
    if (handle == NULL)
    {
        LOG_ERROR << "CreateEvent failed:" << std::endl;
        LASTERROR;
        return false;
    }
    m_handle = std::move(handle);
    m_name = name;
    return true;
}

bool Event::Open(const std::string& name)
{
    AutoCloseHandle handle = OpenEvent(EVENT_ALL_ACCESS, false, name.c_str());
    if (handle == NULL)
    {
        LOG_ERROR << "OpenEvent failed:" << std::endl;
        LASTERROR;
        return false;
    }
    m_handle = std::move(handle);
    m_name = name;
    return true;
}

bool Event::Reset() const
{
    if (!ResetEvent(m_handle.get()))
    {
        LASTERROR;
        return false;
    }
    return true;
}

bool Event::Signal() const
{
    if (!SetEvent(m_handle.get()))
    {
        LASTERROR;
        return false;
    }
    return true;
}

bool Event::Wait(uint32_t timeOut) const
{
    DWORD res = WaitForSingleObject(m_handle.get(), static_cast<DWORD>(timeOut));
    if (res == WAIT_OBJECT_0) return true;
    if (res == WAIT_TIMEOUT) return false;
    if (res == WAIT_FAILED)
    {
        LASTERROR;
        return false;
    }
    return false;
}

Event& Event::operator=(Event&& other)
{
    this->m_handle = std::move(other.m_handle);
    return *this;
}

Event::Event(Event&& other)
{
    this->m_handle = std::move(other.m_handle);
}
