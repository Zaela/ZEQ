
#ifndef _ZEQ_TIMER_POOL_HPP_
#define _ZEQ_TIMER_POOL_HPP_

#include "define.hpp"
#include "timer.hpp"
#include "clock.hpp"
#include "bit.hpp"
#include "temp.hpp"

class TimerPool
{
private:
    uint32_t    m_capacity;
    uint32_t    m_count;
    uint64_t*   m_triggerTimes;
    Timer**     m_timerObjects;

    static const uint32_t DEFAULT_CAPACITY = 32;

private:
    friend class Timer;
    void swapAndPop(uint32_t index);
    void triggerTimer(uint32_t index);
    void realloc();

    void startTimer(Timer* timer);
    void restartTimer(Timer* timer);
    void markTimerAsDead(uint32_t index);

public:
    TimerPool();
    ~TimerPool();

    void check();
};

#endif//_ZEQ_TIMER_POOL_HPP_
