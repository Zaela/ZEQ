
#ifndef _ZEQ_LOCK_HPP_
#define _ZEQ_LOCK_HPP_

#include "define.hpp"
#include <atomic>

class AtomicMutex
{
private:
    std::atomic_flag m_flag;

public:
    AtomicMutex() : m_flag(ATOMIC_FLAG_INIT) { }
    
    void lock()
    {
        for (;;)
        {
            if (try_lock())
                return;
        }
    }
    
    void unlock()
    {
        m_flag.clear();
    }
    
    bool try_lock()
    {
        return m_flag.test_and_set() == false;
    }
};

#endif//_ZEQ_LOCK_HPP_
