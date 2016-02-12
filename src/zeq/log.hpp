
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
#include <memory>

// This class encapsulates a centralized logging & terminal output thread
class Log : public Thread
{
private:
    FILE* m_logFile;

    AtomicMutex m_stdoutMutex;
    AtomicMutex m_logMutex;

    std::vector<const char*> m_stdoutQueue;
    std::vector<const char*> m_stdoutInnerQueue;
    std::vector<const char*> m_logQueue;
    std::vector<const char*> m_logInnerQueue;

    std::condition_variable m_conditionVar;

private:
    virtual void threadProc() override;

    void queue(const char* fmt, va_list check, std::vector<const char*>& queue, AtomicMutex& mutex);

public:
    Log();

    virtual void init() override;
    virtual void signalClose() override;

    void operator()(const char* fmt, ...);
    void printf(const char* fmt, ...);
    void print(const char* str, int len);
};

// Exports for LuaJIT
ZEQ_EXPORT void Log_print(const char* str, int len); // 'len' includes null terminator

#endif//_ZEQ_LOG_HPP_
