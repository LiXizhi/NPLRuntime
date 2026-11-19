-- Copy into the Paracraft script directory and select this file as bootstrapper.
-- Use a disposable worlds/DesignHouse/_block_height_ copy of Project 530.
NPL.load("(gl)script/apps/Aries/main_loop.lua");
local started, frame, stage = false, 0, 1;
local times, distances = {}, {96,256,512};
local x0, y0, z0, last, timerLibrary;
local frameClock = ParaGlobal.getAccurateTime;
-- Avoid rounding a short Windows main-loop wait up to the default timer tick.
local function configureTimer()
    local ok, ffi = pcall(require,"ffi");
    if ok and ffi.os == "Windows" then
        ffi.cdef[[
            unsigned int __stdcall timeBeginPeriod(unsigned int);
            unsigned int __stdcall timeEndPeriod(unsigned int);
            int __stdcall QueryPerformanceCounter(int64_t*);
            int __stdcall QueryPerformanceFrequency(int64_t*);
        ]];
        local counter, frequency = ffi.new("int64_t[1]"), ffi.new("int64_t[1]");
        if ffi.C.QueryPerformanceFrequency(frequency) ~= 0 then
            local scale = 1 / tonumber(frequency[0]);
            frameClock = function()
                ffi.C.QueryPerformanceCounter(counter);
                return tonumber(counter[0]) * scale;
            end
        end
        local library = ffi.load("winmm");
        if library.timeBeginPeriod(1) == 0 then timerLibrary = library end
    end
end
local function checkPlayerBounds(logic, engine, player)
    if engine.min_block_y ~= -256 or engine.max_block_y ~= 9000 then return end
    local x, y, z = player:GetPosition();
    local mode = commonlib.gettable("MyCompany.Aries.Game.GameLogic.GameMode");
    local oldMode, oldDead = mode:GetMode(), logic.OnDead;
    local oldTerrain = logic.options.has_real_terrain;
    local oldAutoFly = logic.options.auto_disable_fly_when_on_ground;
    local entity = commonlib.gettable("MyCompany.Aries.Game.EntityManager").GetFocus();
    local flying = entity:IsFlying();
    local deaths = 0;
    logic.OnDead = function() deaths = deaths + 1 end;
    logic.options.auto_disable_fly_when_on_ground = false;
    local ok, message = pcall(function()
        for _, terrain in ipairs({false,true}) do
            logic.options.has_real_terrain = terrain;
            mode:SetInnerMode("game");
            player:SetPosition(x, engine:realY(-257), z);
            logic.CheckCurrentPlayerLocation();
            mode:SetInnerMode("editor");
            player:SetField("VerticalSpeed", -5);
            logic.CheckCurrentPlayerLocation();
            local _, feet = player:GetPosition();
            local speed = player:GetField("VerticalSpeed",-1);
            assert(feet >= engine:realY(-256) and speed == 0 and entity:IsFlying(),
                string.format("feet=%.3f floor=%.3f velocity=%.3f flight=%s",feet,engine:realY(-256),speed,tostring(entity:IsFlying())));
            local standing = engine:realY(9001);
            for _, delta in ipairs({0.05, 1.25}) do
                player:SetPosition(x, standing + delta, z);
                logic.CheckCurrentPlayerLocation();
                local _, after = player:GetPosition();
                assert(math.abs(after - standing - delta) < 0.01);
            end
        end
        assert(deaths == 2);
    end);
    mode:SetInnerMode(oldMode);
    logic.OnDead = oldDead;
    logic.options.has_real_terrain, logic.options.auto_disable_fly_when_on_ground = oldTerrain, oldAutoFly;
    player:SetPosition(x,y,z);
    if entity:IsFlying() ~= flying then logic.ToggleFly() end
    LOG.std(nil,ok and "info" or "error","blockheight_bench","player bounds %s",ok and "passed" or tostring(message));
    if not ok then
        ParaGlobal.Exit(1);
        error(message);
    end
end

local function checkVerticalTeleport(logic, engine, player)
    NPL.load("(gl)script/apps/Aries/Creator/Game/Tasks/TeleportPlayerTask.lua");
    local teleport = commonlib.gettable("MyCompany.Aries.Game.Tasks.TeleportPlayer");
    local entity = logic.GetPlayer();
    local originalX, originalY, originalZ = player:GetPosition();
    local bx, _, bz = engine:block(originalX,originalY,originalZ);
    bx = bx+5;
    local x, _, z = engine:real(bx,0,bz);
    for _, y in ipairs({-256,-240,9000}) do ParaTerrain.SetBlockTemplateByIdx(bx,y,bz,1) end
    for _, test in ipairs({{8950,true,9001},{-230,false,-255}}) do
        entity:SetPosition(x,engine:realY(test[1]),z);
        teleport:new({mode="vertical",isUpward=test[2],playerEntity=entity,add_to_history=false}):Run();
        local _, feet = entity:GetPosition();
        if math.abs(feet-engine:realY(test[3]))>0.05 then
            LOG.std(nil,"error","blockheight_bench","vertical teleport failed from %d to %.3f",test[1],feet);
            ParaGlobal.Exit(1);
            error("vertical teleport failed");
        end
    end
    entity:SetPosition(originalX,originalY,originalZ);
    LOG.std(nil,"info","blockheight_bench","vertical teleports passed");
end

local physical;
-- Step from the center of a floor block into the neighboring air column.
local function checkFloorFall(logic, engine, player)
    local entity = logic.GetPlayer();
    local mode = commonlib.gettable("MyCompany.Aries.Game.GameLogic.GameMode");
    if not physical.fallStarted then
        if physical.case > 4 then
            logic.OnDead = physical.onDead;
            LOG.std(nil,"info","blockheight_bench","floor physics passed all four cases");
            ParaGlobal.Exit(0);
            return;
        end
        physical.lethal = physical.case % 2 == 1;
        logic.options.has_real_terrain = physical.case > 2;
        logic.options.auto_disable_fly_when_on_ground = false;
        mode:SetInnerMode(physical.lethal and "game" or "editor");
        local bx, _, bz = engine:block(player:GetPosition());
        local x, top, z = engine:real_top(bx,-256,bz);
        ParaTerrain.SetBlockTemplateByIdx(bx,-256,bz,1);
        for dx=1,3 do ParaTerrain.SetBlockTemplateByIdx(bx+dx,-256,bz,0) end
        entity:ToggleFly(false);
        entity:SetPosition(x,top+0.04,z);
        player:SetField("VerticalSpeed",0);
        physical.edgeX = engine:real(bx+2,-256,bz);
        physical.top, physical.z, physical.dead = top, z, false;
        physical.fallStarted = ParaGlobal.getAccurateTime();
        physical.stepped = false;
        logic.OnDead = function() physical.dead=true end;
    end
    local elapsed = ParaGlobal.getAccurateTime() - physical.fallStarted;
    if not physical.stepped and elapsed > 0.4 then
        local _,feet = player:GetPosition();
        if math.abs(feet-physical.top) >= 0.15 then
            LOG.std(nil,"error","blockheight_bench","floor support failed case=%d feet=%.3f top=%.3f",physical.case,feet,physical.top);
            ParaGlobal.Exit(1);
            return;
        end
        entity:SetPosition(physical.edgeX,feet,physical.z);
        player:SetField("VerticalSpeed",-1);
        physical.stepped = true;
    end
    local _,feet = player:GetPosition();
    if physical.stepped and (physical.lethal and physical.dead or
            not physical.lethal and entity:IsFlying() and feet>=engine:realY(-256)) then
        LOG.std(nil,"info","blockheight_bench","floor physics passed lethal=%s terrain=%s",
            tostring(physical.lethal), tostring(logic.options.has_real_terrain));
        physical.case, physical.fallStarted = physical.case+1, nil;
    elseif elapsed > 6 then
        LOG.std(nil,"error","blockheight_bench","floor physics failed case=%d feet=%.3f floor=%.3f speed=%.3f",physical.case,feet,engine:realY(-256),player:GetField("VerticalSpeed",0));
        ParaGlobal.Exit(1);
    end
end

local function checkCeilingJump(logic,engine,player)
    if physical and physical.case then checkFloorFall(logic,engine,player); return end
    local entity=logic.GetPlayer();
    if not physical then physical={warmup=180} end
    if physical.warmup and physical.warmup>0 then physical.warmup=physical.warmup-1; return end
    if not physical.top then
        checkVerticalTeleport(logic,engine,player);
        local bx,_,bz=engine:block(player:GetPosition());
        for x=bx-1,bx+1 do for z=bz-1,bz+1 do ParaTerrain.SetBlockTemplateByIdx(x,9000,z,1) end end
        local x,_,z=engine:real(bx,9000,bz);
        local top=engine:realY(9001);
        entity:ToggleFly(false);
        entity:SetPosition(x,top+0.04,z);
        player:SetField("VerticalSpeed",0);
        assert(ParaTerrain.GetBlockTemplateByIdx(bx,9000,bz)==1);
        physical={frame=0,top=top,peak=top};
    end
    physical.frame=physical.frame+1;
    local _,y=player:GetPosition();
    if physical.frame==90 then
        physical.standing=y;
        entity:Jump();
    elseif physical.frame>90 then
        physical.peak=math.max(physical.peak,y);
    end
    if physical.frame==270 then
        local ok=math.abs(physical.standing-physical.top)<0.12 and physical.peak>physical.top+0.12 and math.abs(y-physical.top)<0.12;
        LOG.std(nil,ok and "info" or "error","blockheight_bench","physics ceiling %s top=%.3f standing=%.3f peak=%.3f landed=%.3f",
            ok and "passed" or "failed",physical.top,physical.standing,physical.peak,y);
        if ok then physical.case, physical.onDead = 1, logic.OnDead else ParaGlobal.Exit(1) end
    end
end

local function bench()
    local logic = commonlib.gettable("MyCompany.Aries.Game.GameLogic");
    local engine = commonlib.gettable("MyCompany.Aries.Game.BlockEngine");
    if not logic.is_started or not logic.GetPlayer or not logic.GetPlayer() then return end
    local player = ParaScene.GetPlayer();
    local attr = ParaTerrain.GetBlockAttributeObject();
    local scene = math.floor((stage-1)/3);
    if physical then checkCeilingJump(logic,engine,player); return end
    if not x0 then
        -- This replay places test geometry. Run it only in a disposable copy of Project 530.
        if not ParaWorld.GetWorldDirectory():match("/_block_height_[%w_]+/?$") then
            error("Block height flight requires a disposable _block_height_ world");
        end
        configureTimer();
        ParaEngine.GetAttributeObject():SetField("RefreshTimer", 0);
        x0,y0,z0 = player:GetPosition();
        checkPlayerBounds(logic,engine,player);
        if ParaEngine.GetAppCommandLineByParam("heightSafetyOnly","") == "true" then
            checkCeilingJump(logic,engine,player);
            return;
        end
        logic.options:SetMaxViewDist(512,false);
        LOG.std(nil,"info","blockheight_bench","world ready %s", ParaWorld.GetWorldDirectory());
    end
    if frame == 0 then
        times = {};
        local distance = distances[(stage-1)%3+1];
        logic.options:SetRenderDist(distance,false);
        assert(attr:GetField("RenderDist",0) == distance);
        assert(ParaCamera.GetAttributeObject():GetField("FarPlane",0) >= distance);
        if stage == 4 or stage == 7 then
            local bx,by,bz = engine:block(x0,y0,z0);
            local y = stage == 4 and 160 or math.min(attr:GetField("MaxBlockY",255)-30,8960);
            local count = 0;
            local function place(x,level,z)
                ParaTerrain.SetBlockTemplateByIdx(x,level,z,1);
                if ParaTerrain.GetBlockTemplateByIdx(x,level,z) == 1 then count = count + 1 end
            end
            if stage == 4 then
                for i=-32,31 do for j=-32,31 do
                    for k = 0,3 do place(bx+i,y+k,bz+j) end
                end end
            else
                for i=-16,16 do for j=-16,16 do
                    place(bx+i*16,y,bz+j*16);
                end end
            end
            local expected = stage == 4 and 16384 or 1089;
            LOG.std(nil,"info","blockheight_bench","fixture y=%d blocks=%d/%d",y,count,expected);
            assert(count == expected, "Benchmark geometry was not placed");
        end
    end
    local routeY = scene == 0 and y0 or engine:realY(scene == 1 and 168 or math.min(attr:GetField("MaxBlockY",255)-22,8968));
    local phase = (frame%180) * math.pi/90;
    player:SetPosition(x0+math.sin(phase)*48,routeY+7+math.cos(phase*2)*4,z0+math.cos(phase)*48);
    ParaCamera.GetAttributeObject():SetField("CameraRotY", phase);
    local now = frameClock();
    if frame >= 180 and last then times[#times+1] = (now-last)*1000 end
    last = now;
    frame = frame+1;
    if frame == 360 then
        table.sort(times);
        LOG.std(nil,"info","blockheight_bench","RESULT %d %d %.1f sections=%d renderables=%d peakMiB=%.1f",scene,attr:GetField("RenderDist",0),times[math.ceil(#times*0.95)],attr:GetField("TotalChunksInMemory",0),attr:GetField("TotalRenderableChunksInMemory",0),ParaEngine.GetAttributeObject():GetField("PeakMemoryUse",0)/1048576);
        stage,frame=stage+1,0;
        if stage > 9 then
            if timerLibrary then timerLibrary.timeEndPeriod(1) end
            ParaUI.Destroy("blockheight_bench");
            ParaGlobal.Exit(0);
        end
    end
end
NPL.this(function()
    NPL.activate("(gl)script/apps/Aries/main_loop.lua", {});
    if not started then
        started = true;
        local ui = ParaUI.CreateUIObject("container","blockheight_bench","_lt",0,0,1,1);
        ui.background=""; ui:AttachToRoot(); ui:SetScript("onframemove",bench);
        LOG.std(nil,"info","blockheight_bench","bootstrap");
    end
end)
