#include "common/Logger.hh"
#include "PollHandle.hh"

PollHandle::PollHandle(): m_size(0)
{
    std::memset(m_handles, 0, MAXIMUM_WAIT_OBJECTS * sizeof(HANDLE));
    std::memset(m_callbacks, 0, MAXIMUM_WAIT_OBJECTS * sizeof(EventCallback));
}

void PollHandle::Poll(std::size_t timeout)
{
    while (true)
    {
        DWORD index = WaitForMultipleObjects(m_size, m_handles, false, timeout);
        if (index == WAIT_FAILED || index == WAIT_TIMEOUT) continue;
        index -= WAIT_OBJECT_0;
        if (!m_callbacks[index](m_handles[index])) break;
    }
}

bool PollHandle::Add(HANDLE handle, EventCallback callback)
{
    if (m_size >= MAXIMUM_WAIT_OBJECTS)
    {
        LOG_ERROR << "Reach maximum object\n";
        return false;
    }
    if (callback == nullptr)
    {
        LOG_ERROR << "Callback must non null\n";
        return false;
    }

    m_handles[m_size] = handle;
    m_callbacks[m_size] = callback;
    m_size++;
}

void PollHandle::Remove(HANDLE handle)
{
    for (std::size_t i = 0; i < m_size; i++)
    {
        if (m_handles[i] == handle)
        {
            if (i == m_size - 1)
            {
                m_size--;
                return;
            }
            std::memmove(&m_handles[i], &m_handles[i + 1], (m_size - i - 1) * sizeof(HANDLE));
            std::memmove(&m_callbacks[i], &m_callbacks[i + 1], (m_size - i - 1) * sizeof(EventCallback));
            break;
        }
    }
    LOG_ERROR << "Handle " << handle << " not found\n";
}
