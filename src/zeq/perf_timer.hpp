
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
    PerfTimer();
    
    uint64_t    microseconds();
    uint64_t    milliseconds();
    double      seconds();
    
    void print(const char* name);
    void printMilli(const char* name);
    void printMicro(const char* name);
    void printf(const char* fmt, ...);
};

#endif//_ZEQ_PERF_TIMER_HPP_
