
local ffi		= require "ffi"
local BinUtil	= require "BinUtil"

local assert = assert

ffi.cdef[[
/*
unsigned long zlib_compressBound(unsigned long sourceLen);
int zlib_compress(uint8_t* dest, unsigned long* destLen, const uint8_t* source, unsigned long sourceLen);
int zlib_uncompress(uint8_t* dest, unsigned long* destLen, const uint8_t* source, unsigned long sourceLen);
*/
unsigned long compressBound(unsigned long sourceLen);
int compress2(uint8_t* dest, unsigned long* destLen, const uint8_t* source, unsigned long sourceLen, int level);
int uncompress(uint8_t* dest, unsigned long* destLen, const uint8_t* source, unsigned long sourceLen);
]]

local C = ffi.load("z")--ffi.C

local Zlib = {}

local lenArg = BinUtil.ULong.Arg()

function Zlib.decompressToBuffer(data, len, outbuf, outbufLen)
	lenArg[0] = outbufLen
	local res = C.uncompress(outbuf, lenArg, data, len)
	assert(res == 0)
end

function Zlib.compressToBuffer(data, len, outbuf, outbufLen)
	lenArg[0] = outbufLen
	local res = C.compress2(outbuf, lenArg, data, len, 9)
	assert(res == 0)
	return lenArg[0]
end

return Zlib
