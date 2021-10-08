#pragma once
#include <string>
#include <vector>

template<uint32_t length = 1024>
class BufferStream
{
private:
    char m_pBuffer[length];
    uint32_t m_currentPosition;
    uint32_t m_length;

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
        uint32_t size = sizeof(type);
        std::memcpy(&m_pBuffer[m_currentPosition], &out, sizeof(type));
        m_currentPosition += size;
        m_length += size;
        return *this;
    }

    template<class type>
    BufferStream& operator<<(const std::vector<type>& out)
    {
        uint32_t size = sizeof(type) * static_cast<uint32_t>(out.size());
        std::memcpy(&m_pBuffer[m_currentPosition], out.data(), size);
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
        m_currentPosition += static_cast<uint32_t>(in.length());
        m_length += static_cast<uint32_t>(in.length());
        return *this;
    }

    template<>
    inline BufferStream& operator>>(std::string& in)
    {
        if (m_length == m_currentPosition) return *this;
        in.resize(m_length - m_currentPosition);
        Extract(const_cast<char*>(in.data()), m_length - m_currentPosition);
        return *this;
    }

    const char* operator[](uint32_t index) const
    {
        return &m_pBuffer[index];
    }

    char* operator[](uint32_t index)
    {
        return &m_pBuffer[index];
    }

    void SetCurrentPosition(uint32_t pos)
    {
        m_currentPosition = pos;
    }

    void Reset()
    {
        m_currentPosition = 0;
        m_length = 0;
    }

    uint32_t GetCurrentPosition()
    {
        return m_currentPosition;
    }

    uint32_t Length()
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

    void Extract(void* pBuffer, uint32_t extractLength)
    {
        std::memcpy(pBuffer, &m_pBuffer[m_currentPosition], extractLength);
        std::memmove(&m_pBuffer[m_currentPosition], &m_pBuffer[m_currentPosition] + extractLength, m_length - m_currentPosition - extractLength);
        m_length -= extractLength;
    }

    constexpr uint32_t Capacity()
    {
        return length;
    }
};

typedef BufferStream<1024> BufferStream1KB;
typedef BufferStream<10240> BufferStream10KB;