#pragma once
#include <string>

template<std::size_t length = 1024>
class BufferStream
{
private:
    char m_pBuffer[length];
    std::size_t m_currentPosition;

public:
    BufferStream(): m_currentPosition(0) {}

    template<class type>
    BufferStream& operator>>(type& out)
    {
        std::memcpy(&out, &m_pBuffer[m_currentPosition], sizeof(type));
        m_currentPosition += sizeof(type);
        return *this;
    }

    template<class type>
    BufferStream& operator<<(type& out)
    {
        std::memcpy(&m_pBuffer[m_currentPosition], &out, sizeof(type));
        m_currentPosition += sizeof(type);
        return *this;
    }

    void SetCurrentPosition(std::size_t pos)
    {
        m_currentPosition = pos;
    }

    const char* Get() const
    {
        return m_pBuffer;
    }
};