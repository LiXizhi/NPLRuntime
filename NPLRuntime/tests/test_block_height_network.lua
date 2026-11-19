-- Copy to script/test_block_height_network.lua in the Paracraft directory.
-- Run two clients on disposable _block_height_ worlds using this bootstrapper.
-- Pass heightPeer=server to the first and heightPeer=client to the second.
-- Add heightProtocol=legacy to both to exercise the old wire format with either binary.
NPL.load("(gl)script/apps/Aries/main_loop.lua");
local path = "script/test_block_height_network.lua";
local role = ParaEngine.GetAppCommandLineByParam("heightPeer", "");
local engine = commonlib.gettable("MyCompany.Aries.Game.BlockEngine");
local legacy = ParaEngine.GetAppCommandLineByParam("heightProtocol", "") == "legacy";
local floorY, topY = legacy and 0 or -256, legacy and 255 or 9000;
local prefix = "MyCompany.Aries.Game.Network.";
local booted, ready, accepted, bx, bz, timeout;
local peer = role == "server" and "heightClient" or "heightServer";

local function send(message)
    return NPL.activate(peer .. ":" .. path, message);
end

local function finish(ok, detail)
    LOG.std(nil, ok and "info" or "error", "blockheight_net", "%s %s", ok and "PASS" or "FAIL", detail);
    ParaGlobal.Exit(ok and 0 or 1);
end

local function checkLoginCapabilities()
    NPL.load("(gl)script/apps/Aries/Creator/Game/Network/NetLoginHandler.lua");
    local login = commonlib.gettable(prefix.."NetLoginHandler").handleLoginClient;
    local kicked, initialized = 0, 0;
    local handler = {
        IsAuthenticated = function() return true end,
        KickUser = function() kicked = kicked+1 end,
        InitializePlayerConnection = function() initialized = initialized+1 end,
        InitializeEnvironment = function() end
    };
    engine:SetLegacyHeightSession(true);
    login(handler, {});
    engine:SetLegacyHeightSession(false);
    login(handler, {});
    login(handler, {heightCapable=true});
    local capable = ParaTerrain.GetMapSectionData ~= nil;
    assert(engine.supportsExtendedHeight == capable);
    assert(kicked == (capable and 1 or 0) and initialized == (capable and 2 or 3), "height capability negotiation failed");
    if ParaEngine.hasFFI then
        local ffi = require("ffi");
        assert(ffi.sizeof("para_block_y_t") == (capable and 4 or 2));
        assert(tonumber(ffi.cast("para_block_y_t", -1)) == (capable and -1 or 65535));
    end
    engine:SetLegacyHeightSession(legacy);
    assert(engine.min_block_y == floorY and engine.max_block_y == topY);
    assert(engine.extendedHeightSession == (not legacy));
    for _, y in ipairs({floorY, 23, topY}) do
        local packed = engine:PackNetworkBlock(3,y,14);
        local expected = legacy and 3*4096+14*256+y or y*256+3*16+14;
        assert(packed == expected);
        local x, level, z = engine:UnpackNetworkBlock(packed,48,96);
        assert(x==51 and level==y and z==110);
    end
    for _, invalid in ipairs({floorY*256-1,(topY+1)*256,1.5,"0"}) do
        local _, level = engine:UnpackNetworkBlock(invalid,0,0);
        assert(level > topY);
    end
    LOG.std(nil,"info","blockheight_net","%s binary, %s protocol, FFI %s",
        capable and "height-capable" or "legacy",legacy and "legacy" or "extended",tostring(ParaEngine.hasFFI));
end

local function checkTerrainFloor(x, z)
    local block = commonlib.gettable("MyCompany.Aries.Game.block");
    local isHole = ParaTerrain.IsHole;
    ParaTerrain.IsHole = function() return true end;
    local ok, detail = pcall(function()
        for y = floorY, floorY+8 do ParaTerrain.SetBlockTemplateByIdx(x,y,z,0) end
        for _, y in ipairs({floorY, floorY+2}) do
            ParaTerrain.SetBlockTemplateByIdx(x,y,z,1);
            assert(engine:GetBlockId(x,y,z) == 1, "test column did not load at Y "..y);
            engine:SetBlock(x,y+65536,z,0);
            engine:SetBlockData(x,y-65536,z,39,0);
            assert(engine:GetBlockId(x,y,z) == 1 and engine:GetBlockData(x,y,z) == 0, "invalid Y aliased onto a valid block");
            local _, modified = block.AutoFillUndergroundColumn(x,floorY+8,z);
            assert(modified == 0, "underground fill missed Y "..y.." (search distance "..ParaTerrain.FindFirstBlock(x,floorY,z,4,topY-floorY)..")");
            ParaTerrain.SetBlockTemplateByIdx(x,y,z,0);
        end
    end);
    ParaTerrain.IsHole = isHole;
    assert(ok,detail);
end

local function getBatchFilters(cx, cz)
    local chunks = commonlib.vector:new();
    local chunk = commonlib.gettable("MyCompany.Aries.Game.World.ChunkCpp");
    chunks:add(chunk:new():Init(nil,cx,cz));
    chunks:add(chunk:new():Init(nil,cx+1,cz));
    local packets = commonlib.gettable(prefix.."Packets");
    local batch = packets.PacketMapChunks:new():Init(chunks,not legacy and 562,1);
    return batch.chunkExistFlag;
end

local function receive(message)
    if not ready then return end
    if role == "server" and message.kind == "hello" then
        if not accepted then
            assert(message.tid and not message.nid);
            NPL.accept(message.tid, "heightClient");
            accepted = true;
        end
        assert(legacy or message.heightCapable);
        checkTerrainFloor(bx,bz);
        ParaTerrain.SetBlockTemplateByIdx(bx,floorY,bz,1);
        ParaTerrain.SetBlockTemplateByIdx(bx,topY,bz,1);
        ParaTerrain.SetBlockUserDataByIdx(bx,topY,bz,27);
        local cx, cz = math.floor(bx/16), math.floor(bz/16);
        local filters = getBatchFilters(cx,cz);
        if legacy then
            local chunk = commonlib.gettable("MyCompany.Aries.Game.World.ChunkCpp"):new():Init(nil,cx,cz);
            send({kind="sections", x=bx, z=bz, filters=filters, mask=32769, data=chunk:GetMapChunkData(true,32769)});
            return;
        end
        send({kind="sections", x=bx, z=bz, filters=filters, lower=ParaTerrain.GetMapSectionData(cx,cz,{-16}),
            upper=ParaTerrain.GetMapSectionData(cx,cz,{562}), heights=ParaTerrain.GetColumnLightHeights(bx,bz)});
    elseif role == "client" and message.kind == "sections" and not accepted then
        accepted = true;
        bx,bz = message.x,message.z;
        for i=1,2 do
            local filter = message.filters[i];
            if legacy then
                assert(filter == 65535, "legacy batch changed its section mask");
            else
                assert(#filter == 2 and filter[1] == 561 and filter[2] == 562, "batch lost its section list");
            end
        end
        local cx,cz = math.floor(bx/16),math.floor(bz/16);
        local chunk = commonlib.gettable("MyCompany.Aries.Game.World.ChunkCpp"):new():Init(nil,cx,cz);
        ParaTerrain.SetBlockTemplateByIdx(bx,topY,bz,0);
        if legacy then
            assert(chunk:ApplyMapChunkData("",{0}) == false);
            chunk:FillChunk(message.data,message.mask);
        else
            chunk:FillChunk(message.lower,{-16});
            assert(engine:GetBlockId(bx,floorY,bz)==1 and engine:GetBlockId(bx,topY,bz)==0);
            assert(ParaTerrain.GetColumnLightHeights(bx,bz)==message.heights, "missing remote roof summary");
            chunk:FillChunk(message.upper,{562});
        end
        local id, data = engine:GetBlockIdAndData(bx,topY,bz);
        assert(engine:GetBlockId(bx,floorY,bz)==1 and id==1 and data==27);
        checkTerrainFloor(bx-bx%16+(bx+1)%16,bz);
        local indices = {engine:PackNetworkBlock(bx%16,floorY,bz%16),engine:PackNetworkBlock(bx%16,topY,bz%16)};
        local packet = commonlib.gettable(prefix.."Packets.PacketBlockMultiChange"):new():Init(cx,cz,indices);
        assert((packet.lightHeights ~= nil) == (not legacy) and packet.dataList[2]==27);
        send({kind="edit", chunkX=cx, chunkZ=cz, blockList=indices, idList={0,0}, dataList={0,0}});
    elseif role == "server" and message.kind == "edit" and message.nid == "heightClient" then
        NPL.load("(gl)script/apps/Aries/Creator/Game/Network/NetServerHandler.lua");
        commonlib.gettable(prefix.."NetServerHandler").handleBlockMultiChange({},message);
        assert(engine:GetBlockId(bx,floorY,bz)==0 and engine:GetBlockId(bx,topY,bz)==0);
        send({kind="verified"});
    elseif role == "client" and message.kind == "verified" then
        send({kind="done"});
        finish(true,"streamed both endpoints over loopback and server applied both edits");
    elseif role == "server" and message.kind == "done" and message.nid == "heightClient" then
        finish(true,"received acknowledgement");
    end
end

NPL.this(function()
    if msg and msg.kind then
        local ok, detail = pcall(receive,msg);
        if not ok then finish(false,tostring(detail)) end
        return;
    end
    NPL.activate("(gl)script/apps/Aries/main_loop.lua", {});
    if booted then return end
    booted = true;
    assert(role == "server" or role == "client");
    NPL.AddPublicFile(path,31093);
    local timer = commonlib.Timer:new({callbackFunc=function()
        local logic = commonlib.gettable("MyCompany.Aries.Game.GameLogic");
        if not logic.is_started or not logic.GetPlayer() then return end
        if not ready then
            assert(ParaWorld.GetWorldDirectory():match("/_block_height_[%w_]+/?$"));
            NPL.load("(gl)script/apps/Aries/Creator/Game/World/ChunkCpp.lua");
            NPL.load("(gl)script/apps/Aries/Creator/Game/Network/Packets/PacketBlockMultiChange.lua");
            NPL.load("(gl)script/apps/Aries/Creator/Game/Network/Packets/PacketLoginClient.lua");
            NPL.load("(gl)script/apps/Aries/Creator/Game/Network/Packets/PacketMapChunkData.lua");
            NPL.load("(gl)script/apps/Aries/Creator/Game/Network/Packets/PacketMapChunk.lua");
            NPL.load("(gl)script/apps/Aries/Creator/Game/Network/Packets/PacketMapChunks.lua");
            local ok, detail = pcall(checkLoginCapabilities);
            if not ok then finish(false,tostring(detail)); return end
            local x,y,z = ParaScene.GetPlayer():GetPosition();
            local by;
            bx,by,bz = engine:block(x,y,z);
            NPL.StartNetServer("127.0.0.1",role=="server" and "18371" or "18372");
            if role=="client" then NPL.AddNPLRuntimeAddress({host="127.0.0.1",port="18371",nid=peer}) end
            ready, timeout = true, commonlib.TimerManager.GetCurrentTime()+45000;
        end
        if role=="client" and not accepted then
            local login = commonlib.gettable(prefix.."Packets.PacketLoginClient"):new():Init();
            send({kind="hello", heightCapable=login.heightCapable});
        end
        if commonlib.TimerManager.GetCurrentTime()>timeout then finish(false,"loopback timed out") end
    end});
    timer:Change(100,200);
end)
