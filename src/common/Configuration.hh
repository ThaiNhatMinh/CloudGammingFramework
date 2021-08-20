#pragma once

#include <cstdlib>
#include <map>
#include <string>

class Configuration
{
private:
    std::string m_filePath;
    std::map<std::string, std::string> m_data;

public:
    Configuration(const std::string &filePath);
    ~Configuration();

    bool IsKeyExist(const std::string &key) { return m_data.find(key) != m_data.end(); };
    template <class T>
    T GetValue(const std::string &key, T defaultValue = {});

private:
    void LoadFile(const std::string &filePath);
    void Save();
};

template<>
int Configuration::GetValue(const std::string &key, int defaultValue)
{
    if (m_data.find(key) == m_data.end())
    {
        return defaultValue;
    }
    return std::atoi(m_data[key].c_str());
}

template<>
float Configuration::GetValue(const std::string &key, float defaultValue)
{
    if (m_data.find(key) == m_data.end())
    {
        return defaultValue;
    }
    return static_cast<float>(std::atof(m_data[key].c_str()));
}

template<>
std::string Configuration::GetValue(const std::string &key, std::string defaultValue)
{
    if (m_data.find(key) == m_data.end())
    {
        return defaultValue;
    }
    return m_data[key];
}
