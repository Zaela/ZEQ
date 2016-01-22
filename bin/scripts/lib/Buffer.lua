
local BinUtil	= require "BinUtil"
local Class		= require "Class"
local ffi		= require "ffi"

local Buffer = Class("Buffer")

function Buffer.new(type)
	local buf = {
		_capacity	= 32,
		_count		= 0,
		_array		= BinUtil.Byte.Array(32),
	}

	return Buffer:instance(buf)
end

function Buffer:push(data, len)
	local count	= self._count
	local n		= len + count

	if n >= self._capacity then
		self:_grow(n)
	end

	ffi.copy(self._array + count, data, len)
	self._count = count + len
end

function Buffer:_grow(n)
	local cap = self._capacity

	while n >= cap do
		cap = cap * 2
	end

	local new = BinUtil.Byte.Array(cap)
	if self._count > 0 then
		ffi.copy(new, self._array, self._count)
	end

	self._array = new
	self._capacity = cap
end

function Buffer:get()
	return self._array, self._count
end

function Buffer:length()
	return self._count
end

return Buffer
