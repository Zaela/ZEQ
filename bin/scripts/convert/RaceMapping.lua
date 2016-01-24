
local RaceMapping = {}

local gender0 = {
    [  1]   = {"global_chr.s3d", "HUM"},
}

local gender1 = {

}

local gender2 = {
    [  1]   = "aro.eqg",
}

function RaceMapping.getFileFor(race, gender)
    if gender == 2 then
        return gender2[race]
    elseif gender == 1 then
        return gender1[race]
    else
        return gender0[race]
    end
end

return RaceMapping
