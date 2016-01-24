
local ffi = require "ffi"

local math = math

ffi.cdef[[
typedef struct Mat4 {
    float m[16];
} Mat4;
]]

local Mat4 = ffi.typeof("Mat4")

local Matrix = {}
Matrix.__index = Matrix

function Matrix.identity()
    local mat = Mat4()
    mat.m[ 0] = 1
    mat.m[ 5] = 1
    mat.m[10] = 1
    mat.m[15] = 1
    return mat
end

function Matrix.copy(mat)
    local new = Mat4()
    ffi.copy(new, mat, ffi.sizeof(Mat4))
    return new
end

------------------------------------------------
-- Operators, creates a new matrix
------------------------------------------------

function Matrix.__mul(a, b)
    local mat = Mat4()
    local m = mat.m
    a, b = a.m, b.m

    m[0]  = a[0]*b[0]  + a[4]*b[1]  + a[8]*b[2]   + a[12]*b[3]
    m[1]  = a[1]*b[0]  + a[5]*b[1]  + a[9]*b[2]   + a[13]*b[3]
    m[2]  = a[2]*b[0]  + a[6]*b[1]  + a[10]*b[2]  + a[14]*b[3]
    m[3]  = a[3]*b[0]  + a[7]*b[1]  + a[11]*b[2]  + a[15]*b[3]

    m[4]  = a[0]*b[4]  + a[4]*b[5]  + a[8]*b[6]   + a[12]*b[7]
    m[5]  = a[1]*b[4]  + a[5]*b[5]  + a[9]*b[6]   + a[13]*b[7]
    m[6]  = a[2]*b[4]  + a[6]*b[5]  + a[10]*b[6]  + a[14]*b[7]
    m[7]  = a[3]*b[4]  + a[7]*b[5]  + a[11]*b[6]  + a[15]*b[7]

    m[8]  = a[0]*b[8]  + a[4]*b[9]  + a[8]*b[10]  + a[12]*b[11]
    m[9]  = a[1]*b[8]  + a[5]*b[9]  + a[9]*b[10]  + a[13]*b[11]
    m[10] = a[2]*b[8]  + a[6]*b[9]  + a[10]*b[10] + a[14]*b[11]
    m[11] = a[3]*b[8]  + a[7]*b[9]  + a[11]*b[10] + a[15]*b[11]

    m[12] = a[0]*b[12] + a[4]*b[13] + a[8]*b[14]  + a[12]*b[15]
    m[13] = a[1]*b[12] + a[5]*b[13] + a[9]*b[14]  + a[13]*b[15]
    m[14] = a[2]*b[12] + a[6]*b[13] + a[10]*b[14] + a[14]*b[15]
    m[15] = a[3]*b[12] + a[7]*b[13] + a[11]*b[14] + a[15]*b[15]

    return mat
end

------------------------------------------------
-- Methods, modifies matrix in-place
------------------------------------------------

local function toRad(x, y, z)
    return math.rad(x), math.rad(y), math.rad(z)
end

function Matrix:setRotation(x, y, z)
    self:setRotationRadians(toRad(x, y, z))
end

function Matrix:setRotationRadians(x, y, z)
    local cr = math.cos(x)
    local sr = math.sin(x)
    local cp = math.cos(y)
    local sp = math.sin(y)
    local cy = math.cos(z)
    local sy = math.sin(z)

    local m = self.m

    m[0]  = cp * cy
    m[1]  = cp * sy
    m[2]  = -sp

    local srsp = sr * sp
    local crsp = cr * sp

    m[4]  = srsp * cy - cr * sy
    m[5]  = srsp * sy + cr * cy
    m[6]  = sr * cp

    m[8]  = crsp * cy + sr * sy
    m[9]  = crsp * sy - sr * cy
    m[10] = cr * cp
end

function Matrix:setTranslation(x, y, z)
    local m = self.m

    m[12] = x
    m[13] = y
    m[14] = z
end

function Matrix.translation(v)
    local m = Matrix.identity()
    m:setTranslation(v.x, v.y, v.z)
    return m
end

------------------------------------------------
-- Rotation matrices
------------------------------------------------

function Matrix.angleX(angle)
    local a = math.rad(angle)
    local c = math.cos(a)
    local s = math.sin(a)

    local mat = Mat4()
    local m = mat.m

    m[ 0] = 1

    m[ 5] = c
    m[ 6] = s

    m[ 9] = -s
    m[10] = c

    m[15] = 1

    return mat
end

function Matrix.angleY(angle)
    local a = math.rad(angle)
    local c = math.cos(a)
    local s = math.sin(a)

    local mat = Mat4()
    local m = mat.m

    m[ 0] = c
    m[ 2] = -s

    m[ 5] = 1

    m[ 8] = s
    m[10] = c

    m[15] = 1

    return mat
end

function Matrix.angleZ(angle)
    local a = math.rad(angle)
    local c = math.cos(a)
    local s = math.sin(a)

    local mat = Mat4()
    local m = mat.m

    m[ 0] = c
    m[ 1] = s

    m[ 4] = -s
    m[ 5] = c

    m[10] = 1

    m[15] = 1

    return mat
end

function Matrix.angleYZ(y, z)
    return Matrix.angleY(y) * Matrix.angleZ(z)
end

function Matrix.angleXYZ(x, y, z)
    return Matrix.angleX(x) * Matrix.angleY(y) * Matrix.angleZ(z)
end

-- Transposed, technically
function Matrix.fromQuaternion(q)
	local mat = Mat4()
	local m = mat.m

	m[ 0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z
	m[ 4] = 2.0 * q.x * q.y + 2.0 * q.z * q.w
	m[ 8] = 2.0 * q.x * q.z - 2.0 * q.y * q.w

	m[ 1] = 2.0 * q.x * q.y - 2.0 * q.z * q.w
	m[ 5] = 1.0 - 2.0 * q.x * q.x - 2.0 * q.z * q.z
	m[ 9] = 2.0 * q.z * q.y + 2.0 * q.x * q.w

	m[ 2] = 2.0 * q.x * q.z + 2.0 * q.y * q.w
	m[ 6] = 2.0 * q.z * q.y - 2.0 * q.x * q.w
	m[10] = 1.0 - 2.0 * q.x * q.x - 2.0 * q.y * q.y

	m[15] = 1.0

	return mat
end

------------------------------------------------
-- Camera matrices
------------------------------------------------

function Matrix.lookAt(pos, targ, up)
    local zaxis = targ - pos
    zaxis:normalize()

    local xaxis = zaxis:crossProduct(up)
    xaxis:normalize()

    local yaxis = xaxis:crossProduct(zaxis)

    local mat = Mat4()
    local m = mat.m

    m[ 0] = xaxis.x
    m[ 1] = yaxis.x
    m[ 2] = zaxis.x

    m[ 4] = xaxis.y
    m[ 5] = yaxis.y
    m[ 6] = zaxis.y

    m[ 8] = xaxis.z
    m[ 9] = yaxis.z
    m[10] = zaxis.z

    m[12] = -xaxis:dotProduct(pos)
    m[13] = -yaxis:dotProduct(pos)
    m[14] = -zaxis:dotProduct(pos)
    m[15] = 1

    return mat
end

function Matrix.perspective(fov, aspectRatio, nearZ, farZ)
    fov = math.rad(fov)
    local f = 1.0 / math.tan(fov * 0.5)

    local mat = Mat4()
    local m = mat.m

    m[ 0] = f / aspectRatio
    m[ 5] = f
    m[10] = (farZ + nearZ) / (nearZ - farZ)
    m[11] = -1
    m[14] = (2 * farZ * nearZ) / (nearZ - farZ)

    return mat
end

------------------------------------------------
-- Transformations
------------------------------------------------

-- Rotate, translate
function Matrix:transformVector(vec)
    local m = self.m

    local x = vec.x * m[ 0] + vec.y * m[ 4] + vec.z * m[ 8] + m[12]
    local y = vec.x * m[ 1] + vec.y * m[ 5] + vec.z * m[ 9] + m[13]
    local z = vec.x * m[ 2] + vec.y * m[ 6] + vec.z * m[10] + m[14]

    vec.x, vec.y, vec.z = x, y, z
end

function Matrix.scale(x, y, z)
    local mat = Mat4()
    local m = mat.m

    m[ 0] = x
    m[ 5] = y or x
    m[10] = z or x
    m[15] = 1

    return mat
end

------------------------------------------------

function Matrix:ptr()
    return self.m
end

ffi.metatype(Mat4, Matrix)

return Matrix
