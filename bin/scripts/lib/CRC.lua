
local bit        = require "bit"
local crc_table    = require "crc_table"

local lshift        = bit.lshift
local rshift        = bit.rshift
local xor            = bit.bxor
local bitwise_and    = bit.band

local CRC = {}

function CRC.calcString(name)
    local val = 0
    for i = 1, #name + 1 do
        local index = bitwise_and(xor(rshift(val, 24), name:byte(i)), 0xFF)
        val = xor(lshift(val, 8), crc_table[index])
    end
    return val
end

return CRC
