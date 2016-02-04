
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
#include "log.hpp"

extern Database gDatabase;
extern TimerPool gTimerPool;
extern Temp gTemp;
extern Lua gLua;
extern Config gConfig;
extern Log gLog;

int main(int argc, char** argv)
{
    FreeImage_Initialise();

    gLua.init();
    gConfig.init();
    gDatabase.init();
    gLog.init();

    Window win;
    
    win.loadZoneModel(argc > 1 ? argv[1] : "gfaydark");
    
    for (;;)
    {
        gTimerPool.check();
        
        if (!win.mainLoop())
            break;
        
        gTemp.reset();
        Clock::sleepMilliseconds(25);
    }
    
    gLog.signalClose();
    gLog.waitUntilClosed();
    
    FreeImage_DeInitialise();
    return 0;
}
