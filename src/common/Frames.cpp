#include "Frames.hh"

void Frames::Init(std::size_t numFrame, std::size_t frameSize)
{
    m_frameSize = frameSize;
    m_frames.clear();
    for (std::size_t i = 0; i < numFrame; i++)
    {
        m_frames.emplace_back(new char[frameSize]);
    }
    m_frontPos = 0;
    m_backPos = 0;
}

const char* Frames::PopFront()
{
    if (m_frontPos == m_backPos) return m_frames[m_frontPos].get();
    char* pRes = m_frames[m_frontPos].get();
    m_frontPos++;
    if (m_frontPos >= m_frames.size())
    {
        m_frontPos = 0;
    }
    return pRes;
}

void Frames::PushBack(const char* pData)
{
    std::memcpy(m_frames[m_backPos].get(), pData, m_frameSize);
    m_backPos++;
    if (m_backPos >= m_frames.size())
    {
        m_backPos = 0;
    }
}

uint32_t Frames::Length()
{
    if (m_frontPos == m_backPos) return 0;
    if (m_backPos > m_frontPos) return static_cast<uint32_t>(m_backPos - m_frontPos);
    return static_cast<uint32_t>(m_frames.size() - (m_frontPos - m_backPos));
}