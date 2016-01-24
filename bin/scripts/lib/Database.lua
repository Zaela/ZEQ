
local Class     = require "Class"
local Struct    = require "Struct"
local ffi       = require "ffi"

ffi.cdef[[
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

int sqlite3_open_v2(const char*, sqlite3**, uint32_t, void*);
int sqlite3_close(sqlite3*);
int sqlite3_prepare_v2(sqlite3*, const char* sql, int, sqlite3_stmt**, void*);
int sqlite3_exec(sqlite3*, const char* sql, void*, void*, char**);
int sqlite3_finalize(sqlite3_stmt*);
int sqlite3_step(sqlite3_stmt*);
int sqlite3_reset(sqlite3_stmt*);
int sqlite3_free(void*);

const char* sqlite3_errmsg(sqlite3*);

int64_t sqlite3_last_insert_rowid(sqlite3*);

int sqlite3_bind_int(sqlite3_stmt*, int col, int);
int sqlite3_bind_int64(sqlite3_stmt*, int col, int64_t);
int sqlite3_bind_double(sqlite3_stmt*, int col, double);
int sqlite3_bind_null(sqlite3_stmt*, int col);
int sqlite3_bind_text(sqlite3_stmt*, int col, const char*, size_t, void*);
int sqlite3_bind_blob(sqlite3_stmt*, int col, void*, size_t, void*);

int sqlite3_column_int(sqlite3_stmt*, int col);
int64_t sqlite3_column_int64(sqlite3_stmt*, int col);
int sqlite3_column_bytes(sqlite3_stmt*, int col);
const char* sqlite3_column_text(sqlite3_stmt*, int col);
void* sqlite3_column_blob(sqlite3_stmt*, int col);
]]

local C = ffi.C

local OPEN_READWRITE    = 0x00000002
local OPEN_CREATE       = 0x00000004
local OPEN_FULLMUTEX    = 0x00010000

local OK    = 0
local ERROR = 1
local BUSY  = 5
local ROW   = 100
local DONE  = 101

local Database = Class("Database")

function Database.new(path)
    local ptr = ffi.new("sqlite3*[1]")
    local rc = C.sqlite3_open_v2(path, ptr, OPEN_READWRITE + OPEN_CREATE + OPEN_FULLMUTEX, nil)
    
    if rc ~= OK then
        local errmsg = C.sqlite3_errmsg(ptr[0])
        local err = string.format("Could not open database: %s", ffi.string(errmsg))
        error(err)
    end
    
    return ptr[0]
end

function Database:__gc()
    C.sqlite3_close(self)
end

function Database:errmsg()
    return ffi.string(C.sqlite3_errmsg(self))
end

local errmsg = ffi.new("char*[1]")

function Database:exec(sql)
    local rc = C.sqlite3_exec(self, sql, nil, nil, errmsg)
    
    if rc ~= OK then
        local err = ffi.string(errmsg[0])
        C.sqlite3_free(errmsg[0])
        error(string.format("Bad SQLite exec: %s", err))
    end
end

function Database:execTransaction(sql)
    self:beginTransaction()
    self:exec(sql)
    self:commitTransaction()
end

function Database:transaction(func)
    self:beginTransaction()
    func()
    self:commitTransaction()
end

function Database:prepare(sql)
    local stmt = ffi.new("sqlite3_stmt*[1]")
    local rc = C.sqlite3_prepare_v2(self, sql, -1, stmt, nil)
    
    if rc ~= OK then
        error(string.format("Bad SQLite prepare: %s", self:errmsg()))
    end
    
    return stmt[0]
end

function Database:getLastInsertId()
    return C.sqlite3_last_insert_rowid(self)
end

function Database:beginTransaction()
    self:exec("BEGIN")
end

function Database:commitTransaction()
    self:exec("COMMIT")
end

function Database:analyze()
    self:exec("ANALYZE")
end

local Stmt = Class("DatabaseStmt")

function Stmt:finalize()
    C.sqlite3_finalize(self)
end

function Stmt:select()
    local rc
    
    repeat
        rc = C.sqlite3_step(self)
    until rc ~= BUSY
    
    if rc == DONE then
        C.sqlite3_reset(self)
        return false
    end
    
    if rc == ROW then return true end
    
    error("Bad SQLite query")
end

Stmt.commit = Stmt.select

function Stmt:reset()
    C.sqlite3_reset(self)
end

function Stmt:bindInt(col, val)
    C.sqlite3_bind_int(self, col, val)
end

function Stmt:bindInt64(col, val)
    C.sqlite3_bind_int64(self, col, val)
end

function Stmt:bindDouble(col, val)
    C.sqlite3_bind_double(self, col, val)
end

function Stmt:bindString(col, str)
    C.sqlite3_bind_text(self, col, str, #str, nil)
end

function Stmt:bindBlob(col, data, len)
    C.sqlite3_bind_blob(self, col, data, len, nil)
end

function Stmt:bindNull(col)
    C.sqlite3_bind_null(self, col)
end

function Stmt:getInt(col)
    return C.sqlite3_column_int(self, col - 1)
end

function Stmt:getInt64(col)
    return C.sqlite3_column_int64(self, col - 1)
end

function Stmt:getString(col)
    col = col - 1
    local len = C.sqlite3_column_bytes(self, col)
    local str = C.sqlite3_column_text(self, col)
    return ffi.string(str, len)
end

function Stmt:getBlob(col)
    col = col - 1
    local len   = C.sqlite3_column_bytes(self, col)
    local data  = C.sqlite3_column_blob(self, col)
    return ffi.copy(data, len), len
end

ffi.metatype("sqlite3", Database)
ffi.metatype("sqlite3_stmt", Stmt)

return Database
