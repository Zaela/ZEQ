
#include "log.hpp"

Log gLog;
extern Config gConfig;

Log::Log()
    : m_logFile(nullptr)
{
    
}

void Log::init()
{
    Thread::init();
}

void Log::signalClose()
{
    Thread::signalClose();
    m_conditionVar.notify_all();
}

void Log::threadProc()
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    
    bool stdoutEmpty    = true;
    bool logEmpty       = true;
    
    while (run())
    {
        if (stdoutEmpty && logEmpty)
            m_conditionVar.wait(lock);
        
        stdoutEmpty = false;
        logEmpty    = false;
        
        if (m_stdoutMutex.try_lock())
        {
            if (m_stdoutQueue.empty())
            {
                stdoutEmpty = true;
                m_stdoutMutex.unlock();
            }
            else
            {
                for (const char* str : m_stdoutQueue)
                {
                    m_stdoutInnerQueue.push_back(str);
                }
                
                m_stdoutQueue.clear();
                m_stdoutMutex.unlock();
                // We don't set stdoutEmpty = true here because m_stdoutQueue may be refilled by the time we finish our loop
                
                for (const char* str : m_stdoutInnerQueue)
                {
                    ::printf("%s", str);
                    delete[] str;
                }
                
                m_stdoutInnerQueue.clear();
            }
        }
        
        if (m_logMutex.try_lock())
        {
            if (m_logQueue.empty())
            {
                logEmpty = true;
                m_logMutex.unlock();
            }
            else
            {
                for (const char* str : m_logQueue)
                {
                    m_logInnerQueue.push_back(str);
                }
                
                m_logQueue.clear();
                m_logMutex.unlock();
                // We don't set logEmpty = true here because m_logQueue may be refilled by the time we finish our loop
                
                for (const char* str : m_logInnerQueue)
                {
                    fprintf(m_logFile, "%s", str);
                    delete[] str;
                }
                
                m_logInnerQueue.clear();
            }
        }
    }
}

void Log::operator()(const char* fmt, ...)
{
    va_list check;
    va_start(check, fmt);
    
    queue(fmt, check, m_logQueue, m_logMutex);
    
    va_end(check);
}

void Log::printf(const char* fmt, ...)
{
    va_list check;
    va_start(check, fmt);
    
    queue(fmt, check, m_stdoutQueue, m_stdoutMutex);
    
    va_end(check);
}

void Log::print(const char* str, int len)
{
    char* copy = new char[len];
    memcpy(copy, str, len);
    
    m_stdoutMutex.lock();
    m_stdoutQueue.push_back(copy);
    m_stdoutMutex.unlock();
    
    m_conditionVar.notify_all();
}

void Log::queue(const char* fmt, va_list check, std::vector<const char*>& queue, AtomicMutex& mutex)
{
    va_list args;
    va_copy(args, check);
    
    int len = vsnprintf(nullptr, 0, fmt, check) + 1;
    
    char* str = new char[len];
    vsnprintf(str, len, fmt, args);
    
    va_end(args);
    
    mutex.lock();
    queue.push_back(str);
    mutex.unlock();
    
    m_conditionVar.notify_all();
}

// Exports for LuaJIT
void Log_print(const char* str, int len)
{
    gLog.print(str, len);
}
