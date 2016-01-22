
#ifndef _ZEQ_PERF_TIMER_HPP_
#define _ZEQ_PERF_TIMER_HPP_

#include "clock.hpp"
#include <cstdio>
#include <cstdarg>

class PerfTimer
{
private:
    uint64_t m_us;

public:
    PerfTimer() : m_us(Clock::microseconds()) { }
    
    uint64_t microseconds()
    {
        return Clock::microseconds() - m_us;
    }
    
    uint64_t milliseconds()
    {
        return microseconds() / 1000;
    }
    
    double seconds()
    {
        double us = (double)microseconds();
        return us * 0.000001;
    }
    
    void print(const char* name)
    {
        double sec = seconds();
        ::printf("%s in %g seconds\n", name, sec);
    }
    
    void printMilli(const char* name)
    {
        uint64_t ms = milliseconds();
        ::printf("%s in %llu milliseconds\n", name, ms);
    }
    
    void printMicro(const char* name)
    {
        uint64_t us = microseconds();
        ::printf("%s in %llu microseconds\n", name, us);
    }
    
    void printf(const char* fmt, ...)
    {
        double sec = seconds();
        
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        
        ::printf(" in %g seconds\n", sec);
    }
};

#endif//_ZEQ_PERF_TIMER_HPP_
