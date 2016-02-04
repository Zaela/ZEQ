
#ifndef _ZEQ_LOG_HPP_
#define _ZEQ_LOG_HPP_

#include "define.hpp"
#include "thread.hpp"
#include "config.hpp"
#include "lock.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>

// This class encapsulates a centralized logging & terminal output thread
class Log : public Thread
{
private:
    FILE* m_logFile;

    AtomicMutex m_stdoutMutex;
    AtomicMutex m_logMutex;

    std::vector<std::string> m_stdoutQueue;
    std::vector<std::string> m_logQueue;

    std::condition_variable m_conditionVar;

private:
    virtual void threadProc();

    void queue(const char* fmt, va_list check, std::vector<std::string>& queue, AtomicMutex& mutex);

public:
    Log();

    virtual void init();
    virtual void signalClose();

    void operator()(const char* fmt, ...);
    void printf(const char* fmt, ...);
};

#endif//_ZEQ_LOG_HPP_
