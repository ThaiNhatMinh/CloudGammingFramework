#include <fstream>
#include "Logger.hh"
class LogFile
{
	std::ofstream m_logFile;
public:
	LogFile()
	{
		m_logFile.open("hook.log");
	}

	~LogFile()
	{
		m_logFile.close();
	}

	void Write(const std::stringstream& ss)
	{
		m_logFile << ss.str();
	}
}instance;

Log::~Log()
{
	instance.Write(log);
}

std::stringstream& Log::Message()
{
	return log;
}
