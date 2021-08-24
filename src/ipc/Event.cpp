#include "Event.hh"


bool Event::Create(const std::string& name, bool initialState, bool manualReset)
{
    AutoCloseHandle handle = CreateEvent(nullptr, manualReset, initialState, name.c_str());
    if (handle == NULL)
    {
        LastError();
        return false;
    }
    m_handle = std::move(handle);
    return true;
}

bool Event::Open(const std::string& name)
{
    AutoCloseHandle handle = OpenEvent(0, false, name.c_str());
    if (handle == NULL)
    {
        LastError();
        return false;
    }
    m_handle = std::move(handle);
    return true;
}

bool Event::Reset()
{
    if (!ResetEvent(m_handle.get()))
    {
        LastError();
        return false;
    }
    return true;
}

bool Event::Signal()
{
    if (!SetEvent(m_handle.get()))
    {
        LastError();
        return false;
    }
    return true;
}