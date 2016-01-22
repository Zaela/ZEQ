
local Class     = require "Class"
local ConvModel = require "ConvModel"

local ModelEQG = Class("ModelEQG", ConvModel)

function ModelEQG.new()
    local m = ConvModel()
    return ModelEQG:instance(m)
end

return ModelEQG
