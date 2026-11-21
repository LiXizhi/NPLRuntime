-- Run the game's quest eligibility and state transitions against the fixture inventory.
-- Only storage and transport change. Reward settlement requires the live game server.
function HaqiTest.Plaza.ConnectQuests(profile)
    local client = MyCompany.Aries.Quest.QuestClientLogics
    local player = client.GetProvider()
    local realm = MyCompany.Aries.Quest.QuestProvider:new({
        remote = true, nid = profile.nid, load_version = "kids",
        template_quests = player:GetTemplateQuests(),
    })
    realm:Reset()
    for _, quest in ipairs(profile.quests) do realm:AddQuest(commonlib.deepcopy(quest)) end
    function realm:GetUserRequestAttr(attrs) return player:GetUserRequestAttr(attrs) end
    function realm:Role_Equals(role) return player:Role_Equals(role) end
    function realm:SaveServerData(id, quest, callback)
        System.os.GetUrl({url = profile.service.."/quests", postfields = commonlib.Json.Encode(quest)},
            function(status, headers, result)
                assert(status == 200 and result and result.issuccess, "local quest save failed")
                callback(result)
            end)
    end
    local prefix = "MyCompany.Aries.Quest.QuestServerLogics."
    client.CallServer = function(method, msg)
        if method == prefix.."TryAccept_Handler" then
            realm:TryAccept({id = msg.id}, client.TryAccept_Handler)
        elseif method == prefix.."TryDrop_Handler" then
            realm:TryDrop({id = msg.id}, client.TryDrop_Handler)
        else
            log("Haqi local realm: unsupported quest operation "..tostring(method).."\n")
        end
    end
end
