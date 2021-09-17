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
