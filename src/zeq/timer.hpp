
#ifndef _ZEQ_TIMER_HPP_
#define _ZEQ_TIMER_HPP_

#include "define.hpp"
#include <functional>

class TimerPool;

class Timer
{
public:
    typedef std::function<void(Timer*)> Callback;

private:
    friend class TimerPool;

    uint32_t    m_poolIndex;
    uint32_t    m_periodMilliseconds;
    Callback    m_callback;
    union
    {
        void*   m_userData;
        int     m_luaCallback;
    };
    
    static const uint32_t INVALID_INDEX = 0xFFFFFFFF;

public:
    Timer(uint32_t periodMilliseconds, Callback callback, void* data = nullptr);
    ~Timer();

    uint32_t    getPeriodMilliseconds() const { return m_periodMilliseconds; }
    void*       getUserData() const { return m_userData; }

    void stop();
    void restart();
};

#endif//_ZEQ_TIMER_HPP_
