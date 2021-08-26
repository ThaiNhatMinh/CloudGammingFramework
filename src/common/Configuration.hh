#pragma once

#include <cstdlib>
#include <map>
#include <string>
#include <sstream>

class Configuration
{
private:
    std::string m_filePath;
    std::map<std::string, std::string> m_data;

public:
    Configuration() = default;
    Configuration(const std::string &filePath);
    ~Configuration();

    bool IsKeyExist(const std::string &key) { return m_data.find(key) != m_data.end(); };
    template <class T>
    T GetValue(const std::string &key, const T& defaultValue = {}) const;
    template <class T>
    void SetValue(const std::string &key, const T& value)
    {
        std::stringstream ss;
        ss << value;
        m_data[key] = ss.str();
    }
    

private:
    void LoadFile(const std::string &filePath);
    void Save();
};

template<>
inline int Configuration::GetValue(const std::string &key, const int& defaultValue) const
{
    auto iter = m_data.find(key);
    if (iter == m_data.end())
    {
        return defaultValue;
    }
    return std::atoi(iter->second.c_str());
}

template<>
inline float Configuration::GetValue(const std::string &key, const float& defaultValue) const
{
    auto iter = m_data.find(key);
    if (iter == m_data.end())
    {
        return defaultValue;
    }
    return static_cast<float>(std::atof(iter->second.c_str()));
}

template<>
inline std::string Configuration::GetValue(const std::string &key, const std::string& defaultValue) const
{
    auto iter = m_data.find(key);
    if (iter == m_data.end())
    {
        return defaultValue;
    }
    return iter->second;
}
