#pragma once
#include <memory>
#include <queue>

class Frames
{
private:
    std::size_t m_frameSize;
    std::size_t m_frontPos;
    std::size_t m_backPos;
    std::vector<std::unique_ptr<char[]>> m_frames;

public:
    void Init(std::size_t numFrame, std::size_t frameSize);

    const char* PopFront();
    void PushBack(const char* pData);
    uint32_t Length();
};