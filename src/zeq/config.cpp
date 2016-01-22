
#include "config.hpp"
#include "lua.hpp"

Config gConfig;
extern Lua gLua;

void Config::init()
{
    static const NameMapping nameMapping[] = {
        // Screen Options
        {"screenwidth",     ScreenWidth},
        {"screenheight",    ScreenHeight},
        {"fullscreen",      Fullscreen},
        {"usenativeaspectratio", UseNativeAspectRatio},
        {"hidetitlebar",    HideTitleBar},
        {"vsync",           Vsync},
        {"antialiaslevel",  AntiAliasLevel},
        // Zone Loading Options
        {"cacheoctreesiflongerthan",    CacheOctreesIfLongerThan},
        {"alwayscacheoctrees",          AlwaysCacheOctrees}
    };
    
    gLua.readConfigFile(nameMapping, sizeof(nameMapping) / sizeof(NameMapping));
}

int Config::getInt(int configEnum, int _default)
{
    return gLua.getConfigInt(configEnum, _default);
}

double Config::getDouble(int configEnum, double _default)
{
    return gLua.getConfigDouble(configEnum, _default);
}

bool Config::getBool(int configEnum, bool _default)
{
    return gLua.getConfigBool(configEnum, _default);
}
