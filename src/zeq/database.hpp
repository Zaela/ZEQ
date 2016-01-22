
#ifndef _ZEQ_DATABASE_HPP_
#define _ZEQ_DATABASE_HPP_

#include <string>
#include <utility>
#include <sqlite3.h>
#include "define.hpp"
#include "lua.hpp"

#define ZEQ_DATABASE_FILE "zeq.db"

class Database;

class Query
{
private:
    sqlite3_stmt* m_Stmt;

private:
    friend class Database;
    void setStmt(sqlite3_stmt* stmt) { m_Stmt = stmt; }

public:
    Query();
    ~Query();

    bool select();
    void commit();
    void reset();

    void bindInt(int col, int val);
    void bindInt64(int col, int64_t val);
    void bindDouble(int col, double val);
    void bindString(int col, const std::string& val);
    void bindBlob(int col, const void* data, uint32_t len);

    int         getInt(int col);
    int64_t     getInt64(int col);
    double      getDouble(int col);
    const char* getString(int col);
    void        getString(int col, std::string& str);
    const byte* getBlob(int col, uint32_t& length);
};

class Database
{
private:
    sqlite3* m_Sqlite;

public:
    Database();
    ~Database();

    void init();

    void prepare(const std::string& sql, Query& query);
    void beginTransaction();
    void commitTransaction();

    int64_t lastInsertId();
};

#endif//_ZEQ_DATABASE_HPP_
