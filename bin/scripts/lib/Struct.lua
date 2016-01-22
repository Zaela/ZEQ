
local ffi = require "ffi"

local string		= string
local setmetatable	= setmetatable

local Struct = {}
Struct.__index = Struct
Struct.__call = function(t, ...) return t.Type(...) end

local function create(type, class)
	local s = {
		Type	= type,
		Ptr		= ffi.typeof("$*", type),
		Array	= ffi.typeof("$[?]", type),
		Arg		= ffi.typeof("$[1]", type),
	}

	if class then ffi.metatype(type, class) end

	return setmetatable(s, Struct)
end

function Struct.new(contents, class)
	-- anonymous struct, does not pollute the (immutable) global C namespace;
	-- can be replaced at runtime, but cannot be a member of another struct
	local str = string.format("struct { %s }", contents)
	return create(ffi.typeof(str), class)
end

function Struct.packed(contents, class)
	ffi.cdef("#pragma pack(1)")
	local ret = Struct(contents, class)
	ffi.cdef("#pragma pack()")
	return ret
end

function Struct.named(name, contents, class)
	local str = string.format("typedef struct %s { %s } %s;", name, contents, name)
	ffi.cdef(str)
	return create(ffi.typeof(name), class)
end

function Struct.primitive(typename)
	return create(ffi.typeof(typename))
end

function Struct:hasField(name)
	return ffi.offsetof(self.Type, name) ~= nil
end

function Struct:sizeof()
	return ffi.sizeof(self.Type)
end

function Struct:cast(ptr)
	return ffi.cast(self.Ptr, ptr)
end

setmetatable(Struct, {__call = function(t, ...) return Struct.new(...) end})

return Struct
