-- Loaded only by the local test shell after the real Aries login module.
do
    log("HAQI_TEST_ENTER\n")
    NPL.load("(gl)script/ide/Json.lua")
    local file = ParaIO.open("haqi-test/session.json", "r")
    assert(file:IsValid(), "Log in through the local test launcher")
    local profile = commonlib.Json.Decode(file:GetText())
    file:close()
    assert(profile and profile.world == "61HaqiTown", "missing offline test session")
    local assetManager = ParaEngine.GetAttributeObject():GetChild("AssetManager")
    assetManager:SetField("EnableAssetManifest", false)
    assetManager:SetField("UseLocalFileFirst", true)
    assert(ParaAsset.OpenArchive("haqi-test/plaza.zip", false), "missing local world archive")
    -- Use the real auth wrapper and its post-login initialization against a local reply.
    local auth = paraworld.auth.AuthUser
    local rest = GameServer.rest.client
    function rest:SendRequest(url, request, callback, rawCallback)
        local path
        if url == auth.GetUrl() then
            path = "/auth"
        elseif url == paraworld.inventory.GetItemsInBag.GetUrl() then
            path = "/bags/"..request.bag
        end
        if path then
            System.os.GetUrl(profile.service..path, function(status, headers, data)
                assert(status == 200 and data, "local game service unavailable")
                rawCallback({data = commonlib.Json.Encode(data)})
            end)
        elseif rawCallback then
            rawCallback({data = '{"errorcode":1}'})
        elseif callback then
            callback({errorcode=1})
        end
    end
    local function loggedIn(result)
        assert(result.issuccess and System.User.IsAuthenticated and System.User.nid == profile.nid)
        System.App.profiles.ProfileManager.UserInfos[profile.nid] = {
            nid = profile.nid, nickname = profile.nickname, userid = tostring(profile.nid), emoney=0,
        }
        paraworld.users.GetUserAndDragonInfo = function(msg, queue, reply)
            if tonumber(msg.nid or profile.nid) == profile.nid then
                local user = System.App.profiles.ProfileManager.UserInfos[profile.nid]
                reply(msg.id and profile.pet or {user = user, dragon = profile.pet})
            else
                reply({})
            end
        end
        NPL.load("(gl)script/apps/Aries/Creator/Game/game_logic.lua")
        NPL.load("(gl)script/apps/IMServer/IMserver_client.lua")
        NPL.load("(gl)script/apps/Aries/Team/TeamClientLogics.lua")
        local jc=IMServer.JabberClientManager.CreateJabberClient(tostring(profile.nid).."@fixture.invalid")
        MyCompany.Aries.Team.TeamClientLogics:SetJC(jc)
        NPL.load("(gl)script/apps/Aries/Pet/DragonPet_New.lua")
        local pet=MyCompany.Aries.Pet.DragonPet_New:new({nid=profile.nid})
        pet:SetBean(profile.pet)
        MyCompany.Aries.Pet.my_dragon_pet_instance=pet
        MyCompany.Aries.Pet.SetDragonInstance(profile.nid,pet)
        System.Item.ItemManager.SyncGlobalStore()
        log("HAQI_TEST_SESSION "..profile.world.."\n")
        local world=MyCompany.Aries.WorldManager:GetWorldInfo(profile.world)
        assert(world and world.born_pos, "missing configured birth point")
        local born=world.born_pos
        local function enterWorld()
            System.App.Commands.Call(System.App.Commands.GetLoadWorldCommand(), {
                name=world.name, worldpath=world.worldpath, is_standalone=true,
                PosX=born.x, PosY=born.y, PosZ=born.z, PosFacing=born.facing,
                on_finish=function(res)
                    assert(ParaWorld.GetWorldDirectory():lower() == world.worldpath:lower().."/", "world load failed")
                    log("HAQI_TEST_WORLD "..ParaWorld.GetWorldDirectory().."\n")
                    System.App.Commands.Call("File.ConnectAriesQuest", {name=world.name})
                    MyCompany.Aries.Quest.QuestClientLogics.DoInitRemoteQuest_Handler({
                        issuccess=true, quests_list=profile.quests or {},
                    })
                    HaqiTest.Plaza.ConnectQuests(profile)
                    HaqiTest.Plaza.Start(profile)
                    log("HAQI_TEST_NPCS_READY\n")
                end,
            })
        end

        local remaining = 3
        for _, bag in ipairs({0, 12, 24}) do
            System.Item.ItemManager.GetItemsInBag(bag, "offline", function()
                remaining = remaining - 1
                if remaining == 0 then enterWorld() end
            end)
        end
    end
    -- Creative Space uses the WorldShare filters registered by the stock login stage.
    local keepwork = NPL.load("(gl)Mod/WorldShare/api/Keepwork/KeepworkKeepworksApi.lua")
    function keepwork:CurrentTime(reply)
        reply({timestamp = os.time() * 1000}, 200)
    end
    MyCompany.Aries.MainLogin.LoadPackages({
        next_step = function()
            auth({username = profile.nickname}, "local-test-login", loggedIn)
        end,
    })
end
