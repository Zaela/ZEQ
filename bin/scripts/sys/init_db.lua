
local File      = require "File"
local Database  = require "Database"

local db = Database("zeq.db")

local sql = File.readAll("scripts/database/db_schema.sql")
db:execTransaction(sql)
