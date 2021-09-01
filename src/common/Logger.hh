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
#define STR_HELPER(x) #x
#ifndef LOG_TO_FILE
#define LOG std::cout
#define LOG_DEBUG std::cout << "[DEBUG]:"
#define LOG_WARN std::cout << "[WARN]:"
#define LOG_ERROR std::cout << __FUNCTION__ "[ERROR]:"
#define LOG_INFO std::cout << "[INFO]:"
#define LOG_API std::cout << "[API]:"
#define TRACE std::cout << __FUNCTION__ ":"  << __LINE__ << "\n"
#else
#define LOG Log().Message()
#define LOG_WARN Log().Message() << "[WARN]: "
#define LOG_ERROR Log().Message() << "[ERR]: "
#define LOG_INFO Log().Message() << "[INFO]: "
#define LOG_DEBUG Log().Message() << "[DEBUG]: "
#define LOG_API Log().Message() << "[API]:"
#define TRACE Log().Message() << __FUNCTION__  << std::endl
#endif