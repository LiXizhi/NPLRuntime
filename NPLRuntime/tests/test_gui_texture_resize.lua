-- Run in a graphical client. Image-block head-on templates are persistent
-- buttons whose texture can change size without a new background assignment.
local name = "_textureGuiResizeRegression"
local texture = ParaAsset.LoadTexture(name, name, 11)
local function upload(w, h)
    local pixels = string.rep("#ff4080c0", w*h)
    texture:GetAttributeObject():SetField("LoadImageFromString",
        string.format("paintrect 0 0 %d %d %s", w-1, h-1, pixels))
end

local panel = ParaUI.CreateUIObject("container", "gui_texture_resize_test", "_lt", 0, 0, 1, 1)
panel.visible = false
panel:AttachToRoot()
local function background(id, suffix)
    local button = ParaUI.CreateUIObject("button", id, "_lt", 0, 0, 90, 90)
    panel:AddChild(button)
    button.background = name .. suffix
    return button
end
local function check(button, x, y, w, h)
    local rect = string.format("%d %d %d %d", x, y, w, h)
    assert(button.background == name .. ";" .. rect, button.background)
    assert(button:GetTexture(nil).rect == rect, rect)
end
local ok, err = pcall(function()
    upload(128, 128)
    local whole = background("whole", "")
    local crop = background("crop", ";4 6 12 15")
    local fixed = background("explicit_full", "")
    fixed:GetTexture(nil).rect = "0 0 128 128"
    local nine = background("nine", ":2 3 4 5")
    local nineFull = background("full_nine", ";0 0 0 0:2 3 4 5")
    local nineCrop = background("cropped_nine", ";4 6 24 32:2 3 4 5")
    for _, size in ipairs({{128,128}, {64,64}, {192,96}, {64,64}}) do
        local w,h = size[1],size[2]
        upload(w,h)
        check(whole, 0,0,w,h)
        check(crop, 4,6,12,15)
        check(fixed, 0,0,128,128)
        check(nine, 2,3,w-6,h-8)
        check(nineFull, 2,3,w-6,h-8)
        check(nineCrop, 6,9,18,24)
    end
end)
ParaUI.Destroy(panel.id)
texture:UnloadAsset()
assert(ok, err)
log("GUI_TEXTURE_RESIZE_PASS\n")
