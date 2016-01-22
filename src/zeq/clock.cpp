
#include "clock.hpp"

#ifdef ZEQ_WINDOWS

#else
uint64_t Clock::milliseconds()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

uint64_t Clock::microseconds()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000 + t.tv_nsec / 1000;
}

uint64_t Clock::unixSeconds()
{
    return time(nullptr);
}

void Clock::sleepMilliseconds(uint32_t ms)
{
    poll(nullptr, 0, ms);
}
#endif
