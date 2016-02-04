
#include "database.hpp"

Database gDatabase;
extern Lua gLua;
extern Log gLog;

Database::Database()
    : m_Sqlite(nullptr)
{
    
}

void Database::init()
{
    FILE* fp = fopen(ZEQ_DATABASE_FILE, "r");
    
    int rc = sqlite3_open_v2(
        ZEQ_DATABASE_FILE,
        &m_Sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );
    
    if (rc != SQLITE_OK)
    {
        //throw
    }
    
    if (!fp)
        gLua.initDatabase();
    else
        fclose(fp);
}

Database::~Database()
{
    sqlite3_close(m_Sqlite);
}

void Database::prepare(const std::string& sql, Query& query)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_Sqlite, sql.c_str(), sql.length(), &stmt, nullptr);
    
    if (rc != SQLITE_OK)
    {
        //throw
    }
    
    query.setStmt(stmt);
}

void Database::beginTransaction()
{
    Query query;
    prepare("BEGIN", query);
    query.commit();
}

void Database::commitTransaction()
{
    Query query;
    prepare("COMMIT", query);
    query.commit();
}

int64_t Database::lastInsertId()
{
    return sqlite3_last_insert_rowid(m_Sqlite);
}

Query::Query()
: m_Stmt(nullptr)
{
    
}

Query::~Query()
{
    sqlite3_finalize(m_Stmt);
    m_Stmt = nullptr;
}

bool Query::select()
{
    int rc;
    
    do
    {
        rc = sqlite3_step(m_Stmt);
    }
    while (rc == SQLITE_BUSY);
    
    switch (rc)
    {
    case SQLITE_ROW:
        return true;
    case SQLITE_DONE:
        sqlite3_reset(m_Stmt);
        return false;
    default:
        //throw
        gLog.printf("QUERY ERR\n");
        break;
    }
    
    return false;
}

void Query::commit()
{
    select();
}

void Query::reset()
{
    sqlite3_reset(m_Stmt);
}

void Query::bindInt(int col, int val)
{
    sqlite3_bind_int(m_Stmt, col, val);
}

void Query::bindInt64(int col, int64_t val)
{
    sqlite3_bind_int64(m_Stmt, col, val);
}

void Query::bindDouble(int col, double val)
{
    sqlite3_bind_double(m_Stmt, col, val);
}

void Query::bindString(int col, const std::string& val)
{
    sqlite3_bind_text(m_Stmt, col, val.c_str(), val.length(), SQLITE_STATIC);
}

void Query::bindBlob(int col, const void* data, uint32_t len)
{
    sqlite3_bind_blob(m_Stmt, col, data, len, SQLITE_STATIC);
}

int Query::getInt(int col)
{
    return sqlite3_column_int(m_Stmt, col - 1);
}

int64_t Query::getInt64(int col)
{
    return sqlite3_column_int64(m_Stmt, col - 1);
}

double Query::getDouble(int col)
{
    return sqlite3_column_double(m_Stmt, col - 1);
}

const char* Query::getString(int col)
{
    return (const char*)sqlite3_column_text(m_Stmt, col - 1);
}

void Query::getString(int col, std::string& str)
{
    str = (const char*)sqlite3_column_text(m_Stmt, col - 1);
}

const byte* Query::getBlob(int col, uint32_t& length)
{
    col--;
    length = sqlite3_column_bytes(m_Stmt, col);
    return (const byte*)sqlite3_column_blob(m_Stmt, col);
}
