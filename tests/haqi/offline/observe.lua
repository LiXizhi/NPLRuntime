-- Chrome reads the live engine log between input events.
ParaGlobal.GetLogger(""):SetForceFlush(true)
local seen = {}
local function once(name, ok)
    if ok and not seen[name] then seen[name] = true; log("CHECK "..name.."\n") end
end
commonlib.Timer:new({callbackFunc=function()
    local aries = MyCompany.Aries
    local shop = aries.NPCShopPage
    once("shop", shop and shop.page and shop.page:IsVisible())
    local bag = aries.Desktop.CombatCharacterFrame
    if bag and bag.pagectrl and bag.pagectrl:IsVisible() then
        if tonumber(bag.CurTab) == 3 and not seen.equipment then
            local x,y = bag.pagectrl:GetParentUIObject():GetAbsPosition()
            local root = ParaUI.GetUIObject("root")
            log(string.format("CHECK bag_tab %.4f %.4f\n", (x+673)/root.width, (y+270)/root.height))
            once("equipment", true)
        end
        once("bag_open", tonumber(bag.CurTab) == 4)
    end
    local items = System.Item.ItemManager
    local potion = items.GetItemByGUID(1)
    local card = items.GetItemByGUID(2)
    once("inventory", potion and potion.gsid==16001 and potion.copies==3 and card and card.gsid==22101)
    local quests = aries.Quest.QuestClientLogics
    local item = quests.GetProvider():GetQuest(63014)
    if item then once("quest"..item.QuestState, true) end
    once("stable", ParaGlobal.timeGetTime() - System.User.LastAuthGameTime > 20000)
    -- The production quest UI rate-limits its first action for five seconds after boot.
    if commonlib.TimerManager.GetCurrentTime() <= 5100 then return end
    for _, action in ipairs({"TryAccept","TryDrop"}) do
        local path = "temp/check-"..action
        if ParaIO.DoesFileExist(path) then
            ParaIO.DeleteFile(path)
            quests[action]({id=63014})
        end
    end
end}):Change(100, 150)
