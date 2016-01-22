
local ffi		= require "ffi"
local io		= require "io"
local Class		= require "Class"
local BinUtil	= require "BinUtil"

local File = Class("File")

function File.new(path, mode)
	local f = {
		_file = io.open(path, mode),
	}
	return File:instance(f)
end

function File.exists(path)
	local f = io.open(path)
	if f then
		f:close()
		return true
	end
	return false
end

function File.openBinary(path)
	local file = io.open(path, "rb")
	if not file then return end

	local str = file:read("*a")
	file:close()
	local data = BinUtil.Byte.Array(#str)
	ffi.copy(data, str, #str)
	return data, #str
end

function File:printf(fmt, ...)
	self._file:write(fmt:format(...))
end

function File:isValid()
	return self._file ~= nil
end

function File.readAll(path)
    local file = io.open(path, "r")
    local str = file:read("*a")
    file:close()
    return str
end

--------------------------------------------------------------------------------
-- We can't inherit from the Lua file type directly because the files are
-- considered userdata and we can't change their metatable from the Lua side.
-- So, generate proxies for all the standard file functions instead.
--------------------------------------------------------------------------------
local fileMT = getmetatable(io.stdin)
for key, func in pairs(fileMT) do
	if key:find("^__") then goto skip end -- Skip metamethods
	File[key] = function(self, ...)
		return func(self._file, ...)
	end
	::skip::
end

return File
