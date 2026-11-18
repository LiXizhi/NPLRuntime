-- Reuse/eviction regression for scaled font atlases. Load in a graphical client.
-- Warm and rewarm draws must not allocate textures or upload glyphs. The stress
-- phase requests twelve sizes; renderer size buckets may merge these atlases.
-- A browser harness can count texture allocations/uploads between the markers.
NPL.load("(gl)script/ide/ParaEngineExtension.lua");
local name = "font_atlas_cache_regression";
ParaUI.Destroy(name);
local panel = ParaUI.CreateUIObject("button", name, "_fi", 0, 0, 0, 0);
panel.zorder = 10000;
panel:SetField("OwnerDraw", true);
local frames = 0;
local reuseScales = {0.4, 0.7, 1.0, 1.5, 2.3, 0.4, 1.5};
local evictionScales = {};
for size = 8, 19 do
    evictionScales[#evictionScales + 1] = size / 14;
end
local samples = {[1] = "cold", [60] = "warm", [121] = "evict", [180] = "evict_repeat", [240] = "rewarm"};
panel:SetScript("ondraw", function()
    frames = frames + 1;
    ParaPainter.SetClipping(false);
    ParaPainter.SetFont({family = "System", size = 14, bold = true});
    ParaPainter.SetPen("#ffffff");
    -- Exclude pending scene/UI work from the measured text draw.
    ParaPainter.Flush();
    local sample = samples[frames];
    if sample then log("FONT_ATLAS_CACHE_BEGIN:" .. sample .. "\n"); end
    local scales = (frames > 120 and frames <= 180) and evictionScales or reuseScales;
    for i, scale in ipairs(scales) do
        ParaPainter.Save();
        ParaPainter.Translate(30, 30 + i * 45);
        ParaPainter.Scale(scale, scale);
        ParaPainter.DrawText(0, 0, "中文名字 Camera label " .. tostring(i));
        ParaPainter.Restore();
    end
    ParaPainter.Flush();
    if sample then log("FONT_ATLAS_CACHE_END:" .. sample .. "\n"); end
    if frames == 240 then log("FONT_ATLAS_CACHE_PASS\n"); end
end);
panel:AttachToRoot();
