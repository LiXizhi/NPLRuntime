-- A disposable, offline Haqi scene for WebGL frame captures.
NPL.this(function() end)

local function writeFile(path, contents)
    local file = ParaIO.open(path, "w")
    assert(file:IsValid(), path)
    file:write(contents, #contents)
    file:close()
end

local function setup()
    writeFile("temp/gui_batch_height.raw", string.rep("\0", 129 * 129 * 4))
    writeFile("temp/gui_batch_tile.txt", [[
Heightmapfile = temp/gui_batch_height.raw
MainTextureFile = Texture/bench/ground.png
CommonTextureFile = Texture/bench/ground.png
Size = 512
ElevScale = 1
MaxBlockSize = 16
DetailThreshold = 50
DetailTextureMatrixSize = 64
NumOfDetailTextures = 1
]])
    writeFile("temp/gui_batch_world.txt", "type = lattice\nTileSize = 512\n([0-1],[0-1]) = %WORLD%/gui_batch_tile.txt\n")
    ParaScene.CreateWorld("GUI batching", 512, "temp/gui_batch_world.txt")
    ParaScene.EnableScene(true)
    ParaScene.GetAttributeObject():SetField("ShowHeadOnDisplay", true)
    ParaUI.GetUIObject("root"):SetField("UsePointTextureFiltering", true)

    local panel = ParaUI.CreateUIObject("container", "gui_batch_hud", "_lt", 12, 20, 375, 155)
    panel.background = ""
    panel.fastrender = false
    panel:SetField("SelfPaint", false)
    panel:AttachToRoot()
    local images = {"Texture/bench/icon0_32bits.png", "Texture/bench/icon1_32bits.png"}
    for i = 0, 47 do
        local b = ParaUI.CreateUIObject("button", "icon" .. i, "_lt",
            5 + (i % 12) * 32, 4 + math.floor(i / 12) * 25, 39, 31)
        b.background = images[(i % 2) + 1]
        if i == 11 then b.rotation = 0.2 end
        panel:AddChild(b)
    end

    -- The source contains opposite colored edges, so gutter or UV mistakes
    -- show up when magnified. The third draw samples outside the POT source.
    local samples = {images[1], images[2] .. "; 3 4 17 19", images[2] .. "; 0 0 64 64"}
    for i, sample in ipairs(samples) do
        local b = ParaUI.CreateUIObject("button", "uv" .. i, "_lt", 8 + (i - 1) * 72, 191, 65, 62)
        b.background = sample
        b:AttachToRoot()
    end
    for i = 0, 5 do
        local b = ParaUI.CreateUIObject("text", "label" .. i, "_lt", 12 + i * 64, 110, 60, 32)
        b.text = "哈奇 " .. i
        b:GetFont("text").color = "255 255 255"
        b.scalingx = 1 + (i % 3) * 0.2
        b.scalingy = b.scalingx
        panel:AddChild(b)
    end
    local asset = ParaAsset.LoadParaX("", "character/CC/02human/Camera/Camera.x")
    for i = 0, 5 do
        local c = ParaScene.CreateCharacter("nameplate" .. i, asset, "", true, 0.35, 0, 1)
        c:SetPosition(64 + i % 2, 0, 64 + (i - 2) * 1.3)
        ParaScene.Attach(c)
        c:SetHeadOnText("哈奇 Name " .. i, 0)
        c:SetHeadOnTextColor("255 255 255", 0)
        c:SetHeadOnOffset(0, i == 0 and -3 or 1.2, 0, 0)
        c:ShowHeadOnDisplay(true, 0)
        if i == 2 then c:ToCharacter():SetFocus() end
    end
    ParaCamera.ThirdPerson(0, 12, 0.35, 0)
end

local ok, err = pcall(setup)
log(ok and "GUI_BATCH_SCENE_READY\n" or ("GUI_BATCH_SCENE_FAILED " .. tostring(err) .. "\n"))
