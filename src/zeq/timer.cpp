
#include "timer.hpp"
#include "timer_pool.hpp"

extern TimerPool gTimerPool;

Timer::Timer(uint32_t periodMilliseconds, Callback callback, void* data)
    : m_poolIndex(INVALID_INDEX),
      m_periodMilliseconds(periodMilliseconds),
      m_callback(callback),
      m_userData(data)
{
    gTimerPool.startTimer(this);
}

Timer::~Timer()
{
    stop();
}

void Timer::stop()
{
    uint32_t index = m_poolIndex;
    
    if (index == INVALID_INDEX)
        return;
    
    m_poolIndex = INVALID_INDEX;
    gTimerPool.markTimerAsDead(index);
}

void Timer::restart()
{
    if (m_poolIndex == INVALID_INDEX)
    {
        gTimerPool.startTimer(this);
        return;
    }
    
    gTimerPool.restartTimer(this);
}
