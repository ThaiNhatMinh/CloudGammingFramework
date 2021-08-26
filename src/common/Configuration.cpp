#include <fstream>
#include "Configuration.hh"
#include "Logger.hh"

Configuration::Configuration(const std::string& filePath)
{
    LoadFile(filePath);
    m_filePath = filePath;
}

Configuration::~Configuration()
{
    Save();
}

void Configuration::LoadFile(const std::string& filePath)
{
    std::fstream file(filePath);
    if (!file.is_open())
    {
        LOG << "Cannot open file:" << filePath << std::endl;
        return;
    }

    char cline[255];
    while (!file.eof())
    {
        file.getline(cline, 255);
        std::string line = cline;
        if (line.empty() || line[0] == '#') continue;
        std::size_t pos = line.find_first_of('=');
        if (pos == std::string::npos)
        {
            LOG_WARN << "Unknow line: " << line << std::endl;
        }
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        m_data.insert({key, value});
        LOG_DEBUG << "Config: " << key << "-" << value << std::endl;
    }
}

void Configuration::Save()
{
    if (m_filePath.empty()) return;
    std::fstream file(m_filePath);
    for(auto& pair : m_data)
    {
        file << pair.first << "=" << pair.second << std::endl;
    }

    file.close();
}
