
#include "define.hpp"
#include "clock.hpp"
#include "database.hpp"
#include "define.hpp"
#include "window.hpp"
#include "timer_pool.hpp"
#include "timer.hpp"
#include "camera.hpp"
#include "temp.hpp"
#include "lua.hpp"
#include "config.hpp"

extern Database gDatabase;
extern TimerPool gTimerPool;
extern Temp gTemp;
extern Lua gLua;
extern Config gConfig;

#include <thread>
#include <condition_variable>
#include <mutex>

std::mutex mutex;
std::condition_variable cv;

static void threadProc()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock);
    
    printf("Loading done!!!\n");
}

int main(int argc, char** argv)
{
    gLua.init();
    gConfig.init();
    gDatabase.init();

    Window win;
    
    std::thread thread(threadProc);
    thread.detach();
    win.loadZoneModel(argc > 1 ? argv[1] : "gfaydark");
    
    cv.notify_all();
    
    for (;;)
    {
        gTimerPool.check();
        
        if (!win.mainLoop())
            break;
        
        gTemp.reset();
        Clock::sleepMilliseconds(25);
    }
    
    return 0;
}
