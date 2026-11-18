-- Regression for font atlas replacement while glyph quads are still batched.
-- Load in a running graphical client with NPL.load("<path to this file>").
-- Expect seven readable labels and FONT_ATLAS_SCALE_PASS after 120 frames.
-- Remove the overlay with ParaUI.Destroy("font_atlas_scale_regression").
local name = "font_atlas_scale_regression";
ParaUI.Destroy(name);
local panel = ParaUI.CreateUIObject("button", name, "_fi", 0, 0, 0, 0);
panel.zorder = 10000;
panel:SetField("OwnerDraw", true);
local frames = 0;
panel:SetScript("ondraw", function()
    ParaPainter.SetClipping(false);
    ParaPainter.SetFont({family = "System", size = 14, bold = false});
    ParaPainter.SetPen("#ffffff");
    -- Reuse one font at different scales within one batch, as overhead labels do.
    -- The final two scales also exercise returning to previously released atlases.
    for i, scale in ipairs({0.4, 0.7, 1.0, 1.5, 2.3, 0.4, 1.5}) do
        ParaPainter.Save();
        ParaPainter.Translate(30, 30 + i * 60);
        ParaPainter.Scale(scale, scale);
        ParaPainter.DrawText(0, 0, "Camera name label scale " .. tostring(scale));
        ParaPainter.Restore();
    end
    ParaPainter.Flush();
    frames = frames + 1;
    if frames == 120 then
        log("FONT_ATLAS_SCALE_PASS\n");
    end
end);
panel:AttachToRoot();
