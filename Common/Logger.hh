#pragma once
#include <sstream>
#include <iostream>

class Log
{
    std::stringstream log;
public:
    Log() = default;
    ~Log();
    std::stringstream& Message();
};

#ifndef LOG_TO_FILE
#define LOG std::cout
#define LOG_API std::cout << "API:"
#define TRACE std::cout << __FUNCTION__ "\n"
#else
#define LOG Log().Message()
#define LOG_API Log().Message() << "API:"
#define TRACE Log().Message() << __FUNCTION__  << std::endl
#endif