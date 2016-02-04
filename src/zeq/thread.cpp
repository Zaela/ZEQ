
#include "thread.hpp"

Thread::Thread()
    : m_runThread(false)
{

}

void Thread::init()
{
    m_runThread = true;
    
    std::thread thread(startThread, this);
    thread.detach();
}

void Thread::startThread(Thread* thread)
{
    std::lock_guard<std::mutex> lock(thread->m_threadLifetimeMutex);
    thread->threadProc();
}

void Thread::signalClose()
{
    m_runThread = false;
}

void Thread::waitUntilClosed()
{
    std::lock_guard<std::mutex> lock(m_threadLifetimeMutex);
}
