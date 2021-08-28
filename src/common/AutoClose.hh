#pragma once
#include "Logger.hh"

template<class HandleType, int (__stdcall *CloseMethod)(HandleType), HandleType invalidValue>
class AutoClose
{
private:
    HandleType m_handle;
    int (__stdcall *m_method)(HandleType) = CloseMethod;

public:
    AutoClose(): m_handle(invalidValue), m_method(nullptr) {}
    AutoClose(HandleType handle) : m_handle(handle) {}
    AutoClose(const AutoClose &) = delete;
    AutoClose &operator=(const AutoClose &) = delete;
    AutoClose(AutoClose &&other)
    {
        m_handle = other.m_handle;
        m_method = other.m_method;
        other.m_handle = invalidValue;
        other.m_method = nullptr;
    }
    AutoClose &operator=(AutoClose &&other)
    {
        m_handle = other.m_handle;
        m_method = other.m_method;
        other.m_handle = invalidValue;
        other.m_method = nullptr;
        return *this;
    }
    ~AutoClose() { Release(); }
    HandleType get() const { return m_handle; }
    bool operator==(HandleType other)
    {
        return m_handle == other;
    }

    void Release()
    {
        if (m_method == nullptr) return;
        LOG_DEBUG << "Release handle: " << m_handle << std::endl;
        m_method(m_handle);
    }
};
