
local Class     = require "Class"
local Struct    = require "Struct"
local ffi       = require "ffi"
local BinUtil   = require "BinUtil"

local FIF_BMP    = 0
local FIF_PNG    = 13
local FIF_DDS    = 24

local formatMap = {
    [FIF_BMP] = "bmp",
    [FIF_PNG] = "png",
    [FIF_DDS] = "dds",
}

local Pixel

if ffi.os == "Windows" then
    Pixel = Struct.named("Pixel", "uint8_t b, g, r, a;")
else
    Pixel = Struct.named("Pixel", "uint8_t r, g, b, a;")
end

ffi.cdef[[
__stdcall void*        FreeImage_OpenMemory(uint8_t*, uint32_t);
__stdcall int        FreeImage_GetFileTypeFromMemory(void*, int);
__stdcall void*        FreeImage_LoadFromMemory(int, void*, int);
__stdcall void*     FreeImage_OpenMemory(uint8_t*, uint32_t);
__stdcall void        FreeImage_CloseMemory(void*);
__stdcall long      FreeImage_TellMemory(void*);
__stdcall int       FreeImage_SeekMemory(void*, long, int);
__stdcall uint32_t  FreeImage_ReadMemory(void* buf, uint32_t, uint32_t, void*);
__stdcall void*        FreeImage_ConvertTo32Bits(void*);
__stdcall void        FreeImage_Unload(void*);
__stdcall uint8_t*    FreeImage_GetBits(void*);
__stdcall uint32_t    FreeImage_GetWidth(void*);
__stdcall uint32_t    FreeImage_GetHeight(void*);
__stdcall int        FreeImage_FlipVertical(void*);

__stdcall Pixel*    FreeImage_GetPalette(void*);
__stdcall int        FreeImage_Save(int, void*, const char*, int);
__stdcall int       FreeImage_SaveToMemory(int, void*, void*, int);
__stdcall void*        FreeImage_ConvertFromRawBits(uint8_t*, int, int, int, uint32_t, uint32_t, uint32_t, uint32_t, int);

__stdcall int       FreeImage_AcquireMemory(void*, uint8_t**, uint32_t*);

__stdcall uint32_t  FreeImage_ZLibCompress(uint8_t* dst, uint32_t dstsize, uint8_t* src, uint32_t srcsize); 
]]

local C = ffi.C--ffi.load("./libfreeimage-3.17.0.so")--ffi.C

local Image = Class("Image")

function Image.new(data, len)
    local img = {
        _data   = data,
        _len    = len,
    }
    
    return Image:instance(img)
end

function Image:open()
    local mem   = C.FreeImage_OpenMemory(self._data, self._len)
    local fmt   = C.FreeImage_GetFileTypeFromMemory(mem, 0)
    local ptr   = C.FreeImage_LoadFromMemory(fmt, mem, 0)
    
    C.FreeImage_CloseMemory(mem)
    if ptr == nil then error "FreeImage_LoadFromMemory failed" end
    
    if fmt ~= FIF_DDS then
        C.FreeImage_FlipVertical(ptr)
    end
    
    local img = C.FreeImage_ConvertTo32Bits(ptr)
    
    if img == nil then error "FreeImage_ConvertTo32Bits failed" end
    
    self._ptr       = img
    self._base      = ptr
    self._format    = formatMap[fmt]
end

function Image:close()
    if self._base == nil then return end
    
    C.FreeImage_Unload(self._base)
    self._base = nil
    
    if self._ptr == nil then return end
    
    C.FreeImage_Unload(self._ptr)
    self._ptr = nil
end

function Image:getFormat()
    return self._format
end

function Image:normalize()
    local ptr       = self._ptr
    local data      = C.FreeImage_GetBits(ptr)
    local width     = C.FreeImage_GetWidth(ptr)
    local height    = C.FreeImage_GetHeight(ptr)
    
    return data, width * height * 4, width, height
end

function Image:convertToPng()
    local buf = C.FreeImage_OpenMemory(nil, 0)
    C.FreeImage_SaveToMemory(FIF_PNG, self._ptr, buf, 0)

    -- make a copy that isn't owned by FreeImage
    local ptr = ffi.new("uint8_t*[1]")
    local len = BinUtil.Uint32.Arg()
    
    C.FreeImage_AcquireMemory(buf, ptr, len)
    
    len         = len[0]
    local copy  = BinUtil.Byte.Array(len)
    
    ffi.copy(copy, ptr[0], len)
    
    C.FreeImage_CloseMemory(buf)
    
    return copy, len
end

function Image:mask()
    if self:getFormat() ~= "bmp" then return end
    -- BMP images need to have the alpha of their masked pixels set manually
    
    local ptr       = self._ptr
    local data      = C.FreeImage_GetBits(ptr)
    local width     = C.FreeImage_GetWidth(ptr)
    local height    = C.FreeImage_GetHeight(ptr)

    -- Assuming the first color in the palette is the mask color
    local pixels    = Pixel:cast(data)
    local p         = C.FreeImage_GetPalette(self._base)
    
    for i = 0, (width * height) - 1 do
        local c = pixels[i]
        if c.r == p.r and c.g == p.g and c.b == p.b then
            c.r, c.g, c.b = 255, 255, 255
            c.a = 0
        else
            c.a = 255
        end
    end
    
    return data, width * height * 4
    
--[[
    local buf = C.FreeImage_OpenMemory(nil, 0)
    C.FreeImage_SaveToMemory(FIF_PNG, self._ptr, buf, 0)

    -- make a copy that isn't owned by FreeImage
    local ptr = ffi.new("uint8_t*[1]")
    local len = BinUtil.Uint32.Arg()
    
    C.FreeImage_AcquireMemory(buf, ptr, len)
    
    len         = len[0]
    local copy  = BinUtil.Byte.Array(len)
    
    ffi.copy(copy, ptr[0], len)
    
    C.FreeImage_CloseMemory(buf)
    
    return copy, len
--]]
end

function Image:getData()
    local ptr       = self._ptr
    local data      = C.FreeImage_GetBits(ptr)
    local width     = C.FreeImage_GetWidth(ptr)
    local height    = C.FreeImage_GetHeight(ptr)
    return data, width, height
end

-- FreeImage contains zlib, and provides some convenience interfaces to it
function Image.compress(data, len)
    local buflen    = len * 2
    local buf       = BinUtil.Byte.Array(buflen)
    data            = BinUtil.Byte:cast(data)
    
    len = C.FreeImage_ZLibCompress(buf, buflen, data, len)
    
    if len == 0 then error "Compression failed" end
    
    return buf, len
end

return Image
