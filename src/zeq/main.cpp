
#include "define.hpp"
#include "clock.hpp"
#include "database.hpp"
#include "define.hpp"
#include "window.hpp"
#include "timer_pool.hpp"
#include "timer.hpp"
#include "camera.hpp"
#include "model_resources.hpp"
#include "temp.hpp"
#include "lua.hpp"
#include "config.hpp"

extern Database gDatabase;
extern ModelResources gModelResources;
extern TimerPool gTimerPool;
extern Temp gTemp;
extern Lua gLua;
extern Config gConfig;

int main(int argc, char** argv)
{
    gLua.init();
    gConfig.init();
    gDatabase.init();
    gModelResources.init();
    
    //Timer t(1000, [](Timer*) { printf("hi\n"); });
    
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
    
    return 0;
}
