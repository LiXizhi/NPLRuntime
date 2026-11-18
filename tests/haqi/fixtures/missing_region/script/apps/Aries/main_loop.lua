-- Haqi bootstrapper for a disposable terrain world. The move PNG is deliberately absent.
NPL.this(function() end)

local function check()
    local heightmap = ParaIO.open("temp/haqi_region_test_flat.raw", "w")
    assert(heightmap:IsValid(), "could not create test heightmap")
    local heights = string.rep("\0", 129 * 129 * 4)
    heightmap:write(heights, #heights)
    heightmap:close()

    ParaScene.CreateWorld("Haqi missing terrain region", 512, "terrain/worldconfig.txt")
    local tile = ParaTerrain.GetAttributeObjectAt(64, 64)
    assert(tile:GetField("NumOfRegions", -1) == 3, "fixture did not register all three regions")
    tile:SetField("CurrentRegionName", "move")
    assert(tile:GetField("CurrentRegionFilepath", "") == "terrain/move_37_37.png", "wrong movement region")
    assert(ParaTerrain.GetRegionValue("move", 64, 64) == 0, "missing movement image")
    assert(ParaTerrain.GetRegionValue("broken", 64, 64) == 0, "unreadable image")
    assert(ParaTerrain.GetRegionValue("unregistered", 64, 64) == 0, "unregistered layer")
    assert(ParaTerrain.GetRegionValue("valid", 64, 64, "r") == 0x12, "valid image red channel")
    assert(ParaTerrain.GetRegionValue("valid", 576, 576, "b") == 0x56, "neighboring tile blue channel")
end

local ok, err = pcall(check)
if ok then
    log("TERRAIN_REGION_CHECK_PASS\n")
else
    log("TERRAIN_REGION_CHECK_FAIL " .. tostring(err) .. "\n")
end
