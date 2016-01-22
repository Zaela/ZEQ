
local Class			= require "Class"
local Struct		= require "Struct"
local BinUtil		= require "BinUtil"
local ffi			= require "ffi"
local bit			= require "bit"
local FragHeader	= require "wld/FragHeader"

local FragTypes = {
	[0x03] = require "wld/Frag03",
	[0x04] = require "wld/Frag04",
	[0x05] = require "wld/Frag05",
	[0x10] = require "wld/Frag10",
	[0x11] = require "wld/Frag11",
	[0x12] = require "wld/Frag12",
	[0x13] = require "wld/Frag13",
	[0x14] = require "wld/Frag14",
	[0x15] = require "wld/Frag15",
	[0x2d] = require "wld/Frag2D",
	[0x30] = require "wld/Frag30",
	[0x31] = require "wld/Frag31",
	[0x36] = require "wld/Frag36",
}

local Header = Struct[[
	uint32_t signature;
	uint32_t version;
	uint32_t fragCount;
	uint32_t unknownA[2];
	uint32_t stringBlockLen;
	uint32_t unknownB;
    
    static const uint32_t VERSION1 = 0x00015500;
	static const uint32_t VERSION2 = 0x1000C800;
]]

local Signature = BinUtil.toFileSignature(string.char(0x02, 0x3d, 0x50, 0x54))
local Version1  = 0x00015500
local Version2  = 0x1000c800

local WLD = Class("WLD")

function WLD.new(data, len)
	if data == nil then
		error("Invalid input")
	end

	local p = Header:sizeof()

	local function tooShort()
		if p > len then
			error("WLD is too short for the length of data indicated")
		end
	end

	tooShort()
	local header = Header:cast(data)
	if header.signature ~= Signature then
		error("File is not a valid WLD")
	end
    
    local version = bit.band(header.version, 0xFFFFFFFE)
	if version ~= Version1 and version ~= Version2 then
		error("Invalid WLD version")
	end
	version = (version == header.VERSION1) and 1 or 2

	local stringBlock = BinUtil.Char:cast(data + p)
	p = p + header.stringBlockLen
	tooShort()

	local wld = {
		_stringBlock		= stringBlock,
		_stringBlockLen		= header.stringBlockLen,
		_rawData			= data,
		_rawLen				= len,
		_header				= header,
        _version            = version,
	}

	WLD:instance(wld)
	wld:encodeStrings()

	-- Gather fragments
	local fragsByIndex		= {}
	local fragsByNameIndex	= {}
	local fragsByName		= {}
	local fragsByType		= {}

	local function byType(frag)
		local t = frag:type()
		local tbl = fragsByType[t]
		if not tbl then
			tbl = {}
			fragsByType[t] = tbl
		end

		tbl[#tbl + 1] = frag
	end

	local minRef = -header.stringBlockLen
	for i = 1, header.fragCount do
		local frag = FragHeader:cast(data + p)
		local cast = FragTypes[frag:type()]

		if cast then
			frag = cast:cast(frag)
		end

		fragsByIndex[i] = frag
		byType(frag)
		local nameref = frag:getNameRef()
		if nameref < 0 and nameref > minRef then
			fragsByNameIndex[nameref] = frag
			fragsByName[ffi.string(stringBlock - nameref)] = frag
		end

		p = p + frag:length()
		tooShort()
	end

	wld._fragsByIndex		= fragsByIndex
	wld._fragsByNameIndex	= fragsByNameIndex
	wld._fragsByName		= fragsByName
	wld._fragsByType		= fragsByType

	return wld
end

local xor	= bit.bxor
local hash	= BinUtil.Byte.Array(8, 0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A)
function WLD.encodeString(str, len)
	for i = 0, len - 1 do
		str[i] = xor(str[i], hash[i % 8])
	end
end

function WLD:encodeStrings()
	WLD.encodeString(self._stringBlock, self._stringBlockLen)
end

function WLD:getStringBlock()
	return self._stringBlock, self._stringBlockLen
end

function WLD:getData()
	return self._rawData, self._rawLen
end

function WLD:getVersion()
    return self._version
end

-------------------------------------------------------------------

function WLD:getFragByRef(ref)
	if ref > 0 then
		return self._fragsByIndex[ref]
	end
	if ref == 0 then ret = -1 end
	return self._fragsByNameIndex[ref]
end

function WLD:getFragByRefVar(frag)
	return self:getFragByRef(frag.ref)
end

function WLD:getFragsByType(t)
	return ipairs(self._fragsByType[t])
end

function WLD:getFragCountByType(t)
    if not self._fragsByType[t] then return 0 end
    return #self._fragsByType[t]
end

function WLD:getFragNameByRef(ref)
	if ref < 0 and ref > -self._stringBlockLen then
		return ffi.string(self._stringBlock - ref)
	end
end

function WLD:getFragName(frag)
	return self:getFragNameByRef(frag:getNameRef())
end

return WLD
