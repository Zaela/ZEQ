
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
    
    while (run())
    {
        m_conditionVar.wait(lock);
        
        if (m_stdoutMutex.try_lock())
        {
            for (std::string& str : m_stdoutQueue)
            {
                ::printf(str.c_str());
            }
            
            fflush(stdout);
            m_stdoutQueue.clear();
            m_stdoutMutex.unlock();
        }
        
        if (m_logFile && m_logMutex.try_lock())
        {
            for (std::string& str : m_logQueue)
            {
                fprintf(m_logFile, "%s", str.c_str());
            }
            
            fflush(m_logFile);
            m_logQueue.clear();
            m_logMutex.unlock();
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

void Log::queue(const char* fmt, va_list check, std::vector<std::string>& queue, AtomicMutex& mutex)
{
    va_list args;
    va_copy(args, check);
    
    int len = vsnprintf(nullptr, 0, fmt, check) + 1;
    
    std::string str;
    str.resize(len);
    vsnprintf((char*)str.c_str(), len, fmt, args);
    
    va_end(args);
    
    mutex.lock();
    queue.push_back(str);
    mutex.unlock();
    
    m_conditionVar.notify_all();
}
