
#ifndef _ZEQ_LUA_HPP_
#define _ZEQ_LUA_HPP_

#include "define.hpp"
#include "config.hpp"
#include <sqlite3.h>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <luaconf.h>
#include <lauxlib.h>
#include <luajit.h>
}

#define LUA_INIT_PATH "scripts/sys/init.lua"
#define LUA_INIT_DB_PATH "scripts/sys/init_db.lua"
#define LUA_CONFIG_RUNNER_PATH "scripts/sys/read_config.lua"
#define LUA_GET_ZONE_CONVERTER_PATH "scripts/sys/get_zone_convert.lua"

class Lua
{
private:
    lua_State* L;

    static const int TRACEBACK_INDEX    = 1;
    static const int CONFIG_INDEX       = 2;

private:
    void getConfigOption(int index);

public:
    void init();

    ~Lua();

    bool runScript(const std::string& path, int numReturns = 0);
    bool runFunc(int numArgs = 0, int numReturns = 0);
    void readConfigFile(const Config::NameMapping* nameMapping, uint32_t count);

    int     getConfigInt(int index, int _default);
    double  getConfigDouble(int index, double _default);
    bool    getConfigBool(int index, bool _default);

    void initDatabase();

    bool convertZone(const std::string& shortname);
};

#endif//_ZEQ_LUA_HPP_
