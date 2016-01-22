
local BinUtil	= require "BinUtil"
local Class		= require "Class"
local Struct	= require "Struct"
local Buffer	= require "Buffer"
local Zlib		= require "Zlib"
local File		= require "File"
local CRC		= require "CRC"
local ffi		= require "ffi"
local Config    = require "Config"

local table		= table
local ipairs 	= ipairs

local Header = Struct[[
	uint32_t offset;
	uint32_t signature;
	uint32_t unknown;
]]

local BlockHeader = Struct[[
	uint32_t deflatedLen;
	uint32_t inflatedLen;
]]

local DirEntry = Struct[[
	uint32_t crc;
	uint32_t offset;
	uint32_t inflatedLen;
]]

local ProcessedEntry = Struct[[
	uint32_t crc;
	uint32_t offset;
	uint32_t inflatedLen;
	uint32_t deflatedLen;
]]

local Signature	= BinUtil.toFileSignature("PFS ")

local PFS = Class("PFS")

function PFS.new(path)
    path = Config.getEQPath() .."/".. path
	local data, fileLen = File.openBinary(path)
	if not data then
		error("Could not open '".. path .."'")
	end

	local p = Header:sizeof()

	local function tooShort()
		if p > fileLen then
			error("File is too short for the length of data indicated: '".. path .."'")
		end
	end

	tooShort()
	local header = Header:cast(data)
	if header.signature ~= Signature then
		error("File is not a valid PFS archive: '".. path .."'")
	end

	p = header.offset
	tooShort()
	local n = BinUtil.Uint32:cast(data + p)[0]

	p = p + BinUtil.Uint32:sizeof()
	tooShort()

	local pfs = {
		_path			= path,
		_rawData		= data,
		_decompressed	= {},
		_names			= {},
		_byName			= {},
		_byExt			= {},
	}

	PFS:instance(pfs)

	for i = 1, n do
		local src = DirEntry:cast(data + p)
		p = p + DirEntry:sizeof()
		tooShort()

		local ent = ProcessedEntry()
		ent.crc			= src.crc
		ent.offset		= src.offset
		ent.inflatedLen	= src.inflatedLen

		local memPos = p
		p = src.offset
		tooShort()

		local ilen = 0
		local totalLen = src.inflatedLen
		while ilen < totalLen do
			local bh = BlockHeader:cast(data + p)
			p = p + BlockHeader:sizeof()
			tooShort()
			p = p + bh.deflatedLen
			tooShort()
			ilen = ilen + bh.inflatedLen
		end
		ent.deflatedLen = p - src.offset

		p = memPos
		pfs[i] = ent
	end

	table.sort(pfs, function(a, b) return a.offset < b.offset end)

	-- Retrieve name data entry and release it so it will be gc'd after we are done with it
	n = #pfs
	local nameData = pfs:_decompressEntry(n)
	pfs[n] = nil
	pfs._decompressed[n] = nil

	n = BinUtil.Uint32:cast(nameData)[0]
	p = BinUtil.Uint32:sizeof()

	local function AddByExt(name, n)
		local ext = name:match("[^%.]+$")
		local t = pfs._byExt[ext]
		if not t then
			t = {}
			pfs._byExt[ext] = t
		end
		table.insert(t, n)
	end

	for i = 1, n do
		local len = BinUtil.Uint32:cast(nameData + p)[0]
		p = p + BinUtil.Uint32:sizeof()
		local name = BinUtil.Char:cast(nameData + p)
		p = p + len

		name = ffi.string(name, len - 1) -- Cut trailing null byte

		pfs._names[i] = name
		pfs._byName[name] = i
		AddByExt(name, i)
	end

	return pfs
end

function PFS:getEntry(i)
	local ent = self._decompressed[i]
	if ent then return ent, ffi.sizeof(ent) end

	return self:_decompressEntry(i)
end

function PFS:getEntryByName(name)
	local i = self._byName[name]
	if i then return self:getEntry(i) end
end

function PFS:_decompressEntry(i)
	local ent = self[i]
	if not ent then return end

	local data		= self._rawData + ent.offset
	local ilen		= ent.inflatedLen
	local read		= 0
	local pos		= 0
	local buffer	= BinUtil.Byte.Array(ilen)

	while read < ilen do
		local bh = BlockHeader:cast(data + pos)
		pos = pos + BlockHeader:sizeof()

		Zlib.decompressToBuffer(data + pos, bh.deflatedLen, buffer + read, ilen - read)

		read = read + bh.inflatedLen
		pos	= pos + bh.deflatedLen
	end

	self._decompressed[i] = buffer

	return buffer, ilen
end

function PFS:hasFile(name)
	return self._byName[name] ~= nil
end

function PFS:names()
	local names = self._names
	local i = 0
	return function()
		i = i + 1
		return names[i]
	end
end

return PFS
