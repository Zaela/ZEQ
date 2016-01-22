
#include "lua.hpp"

Lua gLua;

void Lua::init()
{
    L = luaL_newstate();
    
    if (!L)
    {
        //throw
    }
    
    luaL_openlibs(L);
    
    runScript(LUA_INIT_PATH);
}

Lua::~Lua()
{
    if (L)
        lua_close(L);
}

bool Lua::runScript(const std::string& path, int numReturns)
{
    if (luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, numReturns, 0))
    {
        printf("Lua::runScript error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

bool Lua::runFunc(int numArgs, int numReturns)
{
    if (lua_pcall(L, numArgs, numReturns, 0))
    {
        printf("Lua::runFunc error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

void Lua::readConfigFile(const Config::NameMapping* nameMapping, uint32_t count)
{
    lua_createtable(L, 0, count);
    
    for (uint32_t i = 0; i < count; i++)
    {
        const char* key = nameMapping[i].key;
        int index       = nameMapping[i].index;
        
        lua_pushstring(L, key);
        lua_pushinteger(L, index);
        lua_settable(L, -3);
    }
    
    lua_setglobal(L, "ConfigKeyToIndexMapping");
    
    if (!runScript(LUA_CONFIG_RUNNER_PATH, 1))
    {
        // Put an empty table where we expected the config table to end up
        lua_newtable(L);
    }
}

void Lua::getConfigOption(int index)
{
    lua_pushinteger(L, index);
    lua_gettable(L, CONFIG_INDEX);
}

int Lua::getConfigInt(int index, int _default)
{
    getConfigOption(index);
    
    int ret = _default;
    
    if (!lua_isnil(L, -1))
        ret = lua_tointeger(L, -1);
    
    lua_pop(L, 1);
    return ret;
}

double Lua::getConfigDouble(int index, double _default)
{
    getConfigOption(index);
    
    double ret = _default;
    
    if (!lua_isnil(L, -1))
        ret = lua_tonumber(L, -1);
    
    lua_pop(L, 1);
    return ret;
}

bool Lua::getConfigBool(int index, bool _default)
{
    getConfigOption(index);
    
    bool ret = _default;
    
    if (!lua_isnil(L, -1))
        ret = lua_toboolean(L, -1);
    
    lua_pop(L, 1);
    return ret;
}

void Lua::initDatabase()
{
    if (!runScript(LUA_INIT_DB_PATH))
    {
        //throw
    }
}

bool Lua::convertZone(const std::string& shortname)
{
    if (!runScript(LUA_GET_ZONE_CONVERTER_PATH, 1))
    {
        //throw
        printf("failed to get zone converter func\n");
        return false;
    }
    
    // Converter func is on top of stack, takes shortname as argument
    lua_pushstring(L, shortname.c_str());
    return runFunc(1);
}
