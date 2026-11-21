-- Local training controls added after the stock Haqi world and quest systems are ready.
local Fixture = commonlib.gettable("HaqiTest.Plaza")
local actors = {}
local target
local casting = false

function Fixture.Cast()
    if casting then return end
    casting = true
    System.os.GetUrl({url=Fixture.service.."/training/cast", postfields="cast=fire"}, function(err, msg, data)
        if err~=200 or not data or not data.spell then
            casting=false
            log("HAQI_TEST_CAST_REJECTED "..tostring(err).."\n")
            return
        end
        local player=ParaScene.GetPlayer()
        local x,y,z=player:GetPosition()
        NPL.load("(gl)script/apps/Aries/Combat/SpellCast.lua")
        log("HAQI_TEST_FIRE_BEGIN\n")
        MyCompany.Aries.Combat.SpellCast.EntitySpellCast(1, player, 1, target, 5,
            "config/Aries/Spells/Fire_SingleAttack_Level1.xml", x,y,z, {{data.damage}}, nil,
            function()
                casting = false
                ParaUI.GetUIObject("haqi_fixture_fire").text = "火系技能 · "..data.hp.." HP"
                log("HAQI_TEST_FIRE_COMPLETE hp="..data.hp.."\n")
            end, function() end, true, "fixture_fire")
    end)
end

function Fixture.Visit()
    local npc=MyCompany.Aries.Quest.NPC.GetNpcCharacterFromIDAndInstance(30568)
    assert(npc and npc:IsValid(),"Town merchant was not loaded")
    local x,y,z=npc:GetPosition()
    ParaScene.GetPlayer():SetPosition(x+3,y,z)
    ParaCamera.ThirdPerson(0,7,0.30,0.1)
    commonlib.TimerManager.SetTimeout(function()
        local out={}
        ParaScene.GetScreenPosFrom3DPoint(x,y+2.6,z,out)
        local root=ParaUI.GetUIObject("root")
        log(string.format("HAQI_TEST_PICK %.4f %.4f\n",out.x/root.width,out.y/root.height))
    end,750)
    log("HAQI_TEST_MERCHANT_NEARBY\n")
end

local function button(name,label,y,action)
    local b=ParaUI.CreateUIObject("button",name,"_rt",-132,y,124,28)
    b.text=label
    b.onclick=action
    b:AttachToRoot()
end

function Fixture.Start(profile)
    local player=ParaScene.GetPlayer()
    local x,y,z=player:GetPosition()
    local avatar=ParaAsset.LoadParaX("","character/v3/Elf/Female/ElfFemale.xml")
    for i, guest in ipairs(profile.guests) do
        local c=ParaScene.CreateCharacter("haqi_guest_"..i,avatar,"",true,0.35,3.2,1)
        c:SetPosition(x+guest.dx,ParaTerrain.GetElevation(x+guest.dx,z+guest.dz),z+guest.dz)
        ParaScene.Attach(c)
        local ch=c:ToCharacter()
        ch:SetBodyParams(i%3,1,0,1,1)
        ch:SetCharacterSlot(12,1027)
        ch:SetCharacterSlot(13,1029)
        ch:SetCharacterSlot(14,1031)
        ch:SetCharacterSlot(16,1032)
        actors[i]=c
    end
    target=actors[15]
    assert(target and #actors==36)
    Fixture.service=profile.service
    button("haqi_fixture_fire","训练：火系技能",200,";HaqiTest.Plaza.Cast();")
    button("haqi_fixture_npc","去找米歇尔",233,";HaqiTest.Plaza.Visit();")
    log(string.format("HAQI_TEST_SPAWN %.2f %.2f %.2f guests=%d\n",x,y,z,#actors))
end
