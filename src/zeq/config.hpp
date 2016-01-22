
#ifndef _ZEQ_CONFIG_HPP_
#define _ZEQ_CONFIG_HPP_

#include "define.hpp"

class Config
{
public:
    enum
    {
        // Screen Options
        ScreenWidth,
        ScreenHeight,
        Fullscreen,
        UseNativeAspectRatio,
        HideTitleBar,
        Vsync,
        AntiAliasLevel,
        // Zone Loading Options
        CacheOctreesIfLongerThan,
        AlwaysCacheOctrees,
        OPTIONS_COUNT
    };
    
    struct NameMapping
    {
        const char* key;
        int index;
    };

private:
    
    
public:
    void init();

    static int      getInt(int option, int _default = 0);
    static double   getDouble(int option, double _default = 0.0);
    static bool     getBool(int option, bool _default = false);
};

#endif//_ZEQ_CONFIG_HPP_
