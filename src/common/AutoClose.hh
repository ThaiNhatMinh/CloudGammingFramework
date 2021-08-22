#pragma once

template<class HandleType, int (__stdcall *CloseMethod)(HandleType)>
class AutoClose
{
private:
    HandleType m_handle;
    int (__stdcall *m_method)(HandleType) = CloseMethod;

public:
    AutoClose(): m_handle(INVALID_HANDLE_VALUE), m_method(nullptr) {}
    AutoClose(HandleType handle) : m_handle(handle) {}
    AutoClose(const AutoClose &) = delete;
    AutoClose &operator=(const AutoClose &) = delete;
    AutoClose(AutoClose &&other)
    {
        m_handle = other.m_handle;
        other.m_handle = INVALID_HANDLE_VALUE;
    }
    AutoClose &operator=(AutoClose &&other)
    {
        m_handle = other.m_handle;
        other.m_handle = INVALID_HANDLE_VALUE;
        return *this;
    }
    ~AutoClose() { Release(); }
    HandleType get() { return m_handle; }
    bool operator==(HandleType other)
    {
        return m_handle == other;
    }

    void Release()
    {
        if (m_method == nullptr) return;
        m_method(m_handle);
    }
};
