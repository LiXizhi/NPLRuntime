-- Run in a Haqi WebGL world with a visible, customized player.
NPL.load("(gl)script/ide/timer.lua")

local player = ParaScene.GetPlayer()
local layers = {
    {slot = 1, prefix = "temp/composeskin/"},
    {slot = 7, prefix = "temp/composeface/"},
}

local function start()
    for _, layer in ipairs(layers) do
        local texture = player:GetReplaceableTexture(layer.slot)
        assert(texture:IsValid() and texture:IsLoaded(), "missing avatar texture in slot " .. layer.slot)
        assert(texture:GetKeyName():find(layer.prefix, 1, true) == 1, "expected a composed texture in slot " .. layer.slot)
        layer.key = texture:GetKeyName()
        layer.width, layer.height = texture:GetWidth(), texture:GetHeight()
        assert(layer.width > 0 and layer.height > 0, "empty avatar texture in slot " .. layer.slot)
    end
    for _, layer in ipairs(layers) do
        player:GetReplaceableTexture(layer.slot):UnloadAsset()
    end

    local started = ParaGlobal.timeGetTime()
    local timer = commonlib.Timer:new({callbackFunc = function(self)
        local ok, done = pcall(function()
            for _, layer in ipairs(layers) do
                local texture = player:GetReplaceableTexture(layer.slot)
                assert(texture:IsValid(), "invalid reloaded texture in slot " .. layer.slot)
                if not texture:IsLoaded() then
                    assert(ParaGlobal.timeGetTime() - started < 15000, "reload timed out in slot " .. layer.slot)
                    return false
                end
                assert(texture:GetKeyName() == layer.key and texture:GetWidth() == layer.width
                    and texture:GetHeight() == layer.height, "wrong texture after reload in slot " .. layer.slot)
            end
            return true
        end)
        if not ok or done then
            self:Change()
            log(ok and "HAQI_AVATAR_RELOAD_PASS\n" or ("HAQI_AVATAR_RELOAD_FAIL " .. tostring(done) .. "\n"))
        end
    end})
    timer:Change(100, 100)
end

local ok, err = pcall(start)
if not ok then
    log("HAQI_AVATAR_RELOAD_FAIL " .. tostring(err) .. "\n")
end
