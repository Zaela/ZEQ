
local ffi		= require "ffi"
local Struct	= require "Struct"

local C			= ffi.C
local io		= io
local math		= math
local tonumber	= tonumber

local BinUtil = {
	Void	= {Ptr = ffi.typeof("void*")},
	Int		= Struct.primitive("int"),
	Int16	= Struct.primitive("int16_t"),
	Int64	= Struct.primitive("int64_t"),
	Uint16	= Struct.primitive("uint16_t"),
	Uint32	= Struct.primitive("uint32_t"),
	Uint64	= Struct.primitive("uint64_t"),
	Float	= Struct.primitive("float"),
	Double	= Struct.primitive("double"),
	Char	= Struct.primitive("char"),
	Byte	= Struct.primitive("uint8_t"),
	IntPtr	= Struct.primitive("intptr_t"),
	UintPtr	= Struct.primitive("uintptr_t"),
	-----------------------------------------------
	Long	= Struct.primitive("long"),
	ULong	= Struct.primitive("unsigned long"),
}

-- converts a pointer type to its (signed) address as a lua number
function BinUtil.toAddress(ptr)
	return tonumber(ffi.cast(BinUtil.IntPtr.Type, ptr))
end
BinUtil.ptrToInt = BinUtil.toAddress

-- converts a numeric address to a void pointer
function BinUtil.addrToPtr(addr)
	return ffi.cast(BinUtil.Void.Ptr, addr)
end

-- turns a 4-character file signature string into a uint32_t
function BinUtil.toFileSignature(str)
	return ffi.cast(BinUtil.Uint32.Ptr, str)[0]
end

-- does (stack-limited) quicksort over a binary array of an arbitrary (supplied) type
function BinUtil.sortArray(array, numElements, compFunc, cType)
	local temp = cType()
	local size = ffi.sizeof(cType)

	local function swap(a, b)
		if a == b then return end -- same address
		ffi.copy(temp, array[a], size) -- can't do copy-assignment because it would overwrite temp variable-wise
		array[a] = array[b]
		array[b] = temp
	end

	local function partition(low, high)
		local pivotIndex = math.floor((low + high) / 2) -- random may be better than middle

		swap(pivotIndex, high)

		local mem = low
		for i = low, high - 1 do
			if compFunc(array[i], array[high]) then
				swap(mem, i)
				mem = mem + 1
			end
		end

		swap(mem, high)
		return mem
	end

	local function quicksort(low, high)
		if low < high then
			local p = partition(low, high)
			quicksort(low, p - 1)
			quicksort(p + 1, high)
		end
	end

	quicksort(0, numElements - 1)
end

return BinUtil
