-- Run in an Emscripten graphical client with NPL.load("<path to this file>").
-- Regression: desktop resolution presets submitted by Haqi's settings page
-- must not change the browser-owned drawing size or GUI hit-test coordinates.
-- Expect WEB_HIGH_EFFECT_GUI_PASS and readable labels. Click both buttons;
-- their labels must change. Close the overlay to restore the original effect.
local name = "web_high_effect_gui_regression";
ParaUI.Destroy(name);
local att = ParaEngine.GetAttributeObject();
local originalEffect = att:GetField("Effect Level", 0);
local originalSize = att:GetField("ScreenResolution", {0, 0});
local panel = ParaUI.CreateUIObject("container", name, "_fi", 0, 0, 0, 0);
panel.background = "";
panel.zorder = 30000;
panel:SetField("ClickThrough", true);
panel:AttachToRoot();

local buttons = {};
for i, alignment in ipairs({"_lt", "_rb"}) do
    local button = ParaUI.CreateUIObject("button", name .. i, alignment,
        i == 1 and 24 or -364, i == 1 and 24 or -94, 340, 70);
    button.text = "Click test / 中文文字 " .. i;
    button:SetScript("onclick", function()
        button.text = "CLICK PASS / 点击成功 " .. i;
    end);
    panel:AddChild(button);
    buttons[i] = button;
end
local close = ParaUI.CreateUIObject("button", "close", "_rt", -184, 24, 160, 40);
close.text = "Close regression";
close:SetScript("onclick", function()
    att:SetField("Effect Level", originalEffect);
    ParaUI.Destroy(name);
end);
panel:AddChild(close);

local frames = 0;
local presets = {{2, 1280, 720}, {0, 800, 600}, {2, 1920, 1080}};
panel:SetScript("onframemove", function()
    frames = frames + 1;
    local phase = math.floor((frames - 1) / 45) + 1;
    if frames <= 135 and (frames - 1) % 45 == 0 then
        local preset = presets[phase];
        att:SetField("Effect Level", preset[1]);
        att:SetField("ScreenResolution", {preset[2], preset[3]});
        att:CallField("UpdateScreenMode");
    elseif frames <= 135 and frames % 45 == 0 then
        local size = att:GetField("ScreenResolution", {0, 0});
        assert(att:GetField("Effect Level", 0) == presets[phase][1],
            "Requested effect level was not preserved");
        assert(size[1] == originalSize[1] and size[2] == originalSize[2],
            "Desktop preset changed the browser drawing size");
        for _, button in ipairs(buttons) do
            local x, y, w, h = button:GetAbsPosition();
            -- GetUIObjectAtPoint takes logical GUI coordinates in 2D mode.
            local hit = ParaUI.GetUIObjectAtPoint(
                math.floor(x + w / 2), math.floor(y + h / 2));
            assert(hit.id == button.id, "GUI hit test missed a visible button");
        end
        if frames == 135 then
            log("WEB_HIGH_EFFECT_GUI_PASS\n");
        end
    end
end);
