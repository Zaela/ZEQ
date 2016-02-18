
#ifndef _ZEQ_THREAD_HPP_
#define _ZEQ_THREAD_HPP_

#include "define.hpp"
#include "lock.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

class Thread
{
private:
    bool        m_runThread;
    AtomicMutex m_threadLifetimeMutex;

public:
    static  void startThread(Thread* thread);
    virtual void threadProc() = 0;

protected:
    bool run() const { return m_runThread; }

public:
    Thread();

    // Starts the thread
    virtual void init();
    virtual void signalClose();
    void waitUntilClosed();
};

#endif//_ZEQ_THREAD_HPP_
