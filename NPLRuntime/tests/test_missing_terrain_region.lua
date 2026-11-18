-- Run in a disposable world with a terrain tile at (64, 64).
-- Haqi can register a movement region whose PNG is absent from the world.
local x, z = 64, 64
local tile = ParaTerrain.GetAttributeObjectAt(x, z)
assert(tile:IsValid(), "A loaded terrain tile is required")

local layer = "missing_region_regression"
local path = "%WORLD%/regions/missing_region_regression.png"
assert(not ParaIO.DoesAssetFileExist(ParaIO.DecodePath(path)), "The test image must be absent")

local previous = tile:GetField("CurrentRegionName", "")
tile:SetField("CurrentRegionName", layer)
tile:SetField("CurrentRegionFilepath", path)
assert(tile:GetField("CurrentRegionFilepath", "") == path, "Region registration failed")
tile:SetField("CurrentRegionName", previous)

assert(ParaTerrain.GetRegionValue(layer, x, z) == 0, "Missing region must read as zero")
log("MISSING_TERRAIN_REGION_PASS\n")
