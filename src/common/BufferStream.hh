#pragma once
#include <string>

template<std::size_t length = 1024>
class BufferStream
{
private:
    char m_pBuffer[length];
    std::size_t m_currentPosition;
    std::size_t m_length;

public:
    BufferStream(): m_currentPosition(0), m_length(0) {}

    template<class type>
    BufferStream& operator>>(type& out)
    {
        Extract(&out, sizeof(type));
        return *this;
    }

    template<class type>
    BufferStream& operator<<(const type& out)
    {
        std::size_t size = sizeof(type);
        std::memcpy(&m_pBuffer[m_currentPosition], &out, sizeof(type));
        m_currentPosition += size;
        m_length += size;
        return *this;
    }

    template<>
    inline BufferStream& operator<<(const std::string& in)
    {
        if (in.length() + m_length > length)
        {
            LOG_ERROR << "Not enough buffer for " << in.length() + m_length << " byte, buffer size is " << length << std::endl;
            throw std::exception("Not enough buffer for");
        }
        std::memcpy(&m_pBuffer[m_currentPosition], in.data(), in.length());
        m_currentPosition += in.length();
        m_length += in.length();
        return *this;
    }

    const char* operator[](std::size_t index) const
    {
        return &m_pBuffer[index];
    }

    char* operator[](std::size_t index)
    {
        return &m_pBuffer[index];
    }

    void SetCurrentPosition(std::size_t pos)
    {
        m_currentPosition = pos;
    }

    std::size_t GetCurrentPosition()
    {
        return m_currentPosition;
    }

    std::size_t Length()
    {
        return m_length;
    }

    const char* Get() const
    {
        return m_pBuffer;
    }

    char* Get()
    {
        return m_pBuffer;
    }

    void Extract(void* pBuffer, std::size_t extractLength)
    {
        std::memcpy(pBuffer, &m_pBuffer[m_currentPosition], extractLength);
        std::memmove(&m_pBuffer[m_currentPosition], &m_pBuffer[m_currentPosition] + extractLength, m_length - m_currentPosition - extractLength);
        m_length -= extractLength;
    }
};

typedef BufferStream<1024> BufferStream1KB;