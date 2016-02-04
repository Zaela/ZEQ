
#include "perf_timer.hpp"
#include "log.hpp"

extern Log gLog;

PerfTimer::PerfTimer()
    : m_us(Clock::microseconds())
{
    
}
    
uint64_t PerfTimer::microseconds()
{
    return Clock::microseconds() - m_us;
}
    
uint64_t PerfTimer::milliseconds()
{
    return microseconds() / 1000;
}
    
double PerfTimer::seconds()
{
    double us = (double)microseconds();
    return us * 0.000001;
}
    
void PerfTimer::print(const char* name)
{
    double sec = seconds();
    gLog.printf("%s in %g seconds\n", name, sec);
}
    
void PerfTimer::printMilli(const char* name)
{
    uint64_t ms = milliseconds();
    gLog.printf("%s in %llu milliseconds\n", name, ms);
}
    
void PerfTimer::printMicro(const char* name)
{
    uint64_t us = microseconds();
    gLog.printf("%s in %llu microseconds\n", name, us);
}
    
void PerfTimer::printf(const char* fmt, ...)
{
    double sec = seconds();
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    
    gLog.printf(" in %g seconds\n", sec);
}
