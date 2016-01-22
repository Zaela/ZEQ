
local Struct    = require "Struct"
local Class        = require "Class"
local ffi        = require "ffi"

local Header = Struct.named("WLDFragHeader", [[
    uint32_t    _len;
    uint32_t    _type;
    int            _nameref;
]])

local Wrapper

local Frag = Class("WldFrag")

function Frag:length()
    -- nameref is not considered part of the header for size, but it's *always* there
    return 8 + self.header._len
end

function Frag:type()
    return self.header._type
end

function Frag:getNameRef()
    return self.header._nameref
end

function Frag:setNameRef(ref)
    self.header._nameref = ref
end

function Frag:cast(ptr)
    return Wrapper:cast(ptr)
end

function Frag:addChildren()
    -- deliberately empty
end

Wrapper = Struct([[
    WLDFragHeader header;
]], Frag)

return Frag
