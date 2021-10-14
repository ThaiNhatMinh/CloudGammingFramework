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

class LogToNull
{
    template<class type>
    LogToNull& operator<<(const type& out)
    {
        return *this;
    }

    template<class type>
    LogToNull& operator>>(const type& out)
    {
        return *this;
    }
};

extern LogToNull lognull;

#define STR_HELPER(x) #x
#ifdef LOG_STD
#define LOG_THREAD std::cout << '[' << std::this_thread::get_id() << ']'
#define LOG std::cout
#define LOG_DEBUG std::cout << "[DEBUG][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_WARN std::cout << "[WARN][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_ERROR std::cout << "[ERROR][" <<  __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_INFO std::cout << "[INFO][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_API std::cout << "[API][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define TRACE std::cout << __FUNCTION__ ":"  << __LINE__ << '\n'
#elif defined(LOG_NULL)
#define LOG
#define LOG_THREAD lognull << '[' << std::this_thread::get_id() << ']'
#define LOG_DEBUG lognull << "[DEBUG][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_WARN lognull << "[WARN][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_ERROR lognull << "[ERROR][" <<  __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_INFO lognull << "[INFO][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_API lognull << "[API][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define TRACE lognull << __FUNCTION__ ":"  << __LINE__ << '\n'
#else
#define LOG Log().Message()
#define LOG_THREAD Log().Message() << '[' << std::this_thread::get_id() << ']'
#define LOG_DEBUG Log().Message() << "[DEBUG][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_WARN Log().Message() << "[WARN][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_ERROR Log().Message() << "[ERROR][" <<  __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_INFO Log().Message() << "[INFO][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define LOG_API Log().Message() << "[API][" << __FUNCTION__ ":"  << __LINE__ << ']'
#define TRACE Log().Message() << __FUNCTION__ ":"  << __LINE__ << '\n'
#endif