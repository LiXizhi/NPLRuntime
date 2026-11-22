-- The desktop driver supplies p and submits this body through run_npl_code.
NPL.load("(gl)script/apps/Aries/Creator/Game/Code/Creation/World.lua")
local Game = MyCompany.Aries.Game
local World = Game.Code.Creation.World
local B = Game.BlockEngine
assert(World.Same(p.identity, World.Identity()), "world or session changed")
local path = GameLogic.GetWorldDirectory() .. p.filename
assert(ParaIO.DoesFileExist(path, true), "missing world-local GLB")
local asset = ParaAsset.LoadParaX("", path)
assert(asset and asset:IsValid(), "GLB asset unavailable")
asset:LoadAsset()
if not asset:IsLoaded() then return {state="loading"} end
local box = asset:GetBoundingBox({})
assert(Game.Code.Creation.Geometry.ModelBoundsValid(box), "GLB has no renderable bounds")
if not p.at then return {state="ready", bounds=box, blockSize=B.blocksize} end

assert(not GameLogic.isRemote and not GameLogic.isServer and GameLogic.GameMode:CanEditBlock(), "local edit mode required")
local x,y,z = unpack(p.at)
local world = World:new()
assert(y>=0 and y<=255 and world:Loaded(x,z), "carrier outside loaded world")
local id = Game.block_types.names.BlockModel
local entity = Game.EntityManager.GetBlockEntity(x,y,z)
if B:GetBlockId(x,y,z) == 0 and not entity then
    -- Use the native yaw convention to transform the four footprint corners.
    local matrix = commonlib.gettable("mathlib.Matrix4"):new():identity()
    local rotation = commonlib.gettable("mathlib.Quaternion"):new()
    rotation:FromEulerAnglesSequence(0,0,p.facing,"zxy")
    rotation:ToRotationMatrix(matrix)
    local ox,oy,oz = B:real_bottom(x,y,z)
    local lo = {math.huge,oy+box.min_y*p.scale,math.huge}
    local hi = {-math.huge,oy+box.max_y*p.scale,-math.huge}
    for _,dx in ipairs({box.min_x,box.max_x}) do
        for _,dz in ipairs({box.min_z,box.max_z}) do
            local wx = ox + (dx*matrix[1]+dz*matrix[9])*p.scale
            local wz = oz + (dx*matrix[3]+dz*matrix[11])*p.scale
            lo[1],hi[1] = math.min(lo[1],wx),math.max(hi[1],wx)
            lo[3],hi[3] = math.min(lo[3],wz),math.max(hi[3],wz)
        end
    end
    local eps = B.blocksize*1e-6
    local a = {B:block_float(lo[1]+eps,lo[2]+eps,lo[3]+eps)}
    local b = {B:block_float(hi[1]-eps,hi[2]-eps,hi[3]-eps)}
    for k=1,3 do a[k]=math.floor(a[k]); b[k]=math.max(a[k],math.floor(b[k])) end
    local authoring = Game.Code.Creation.Authoring
    local footprint = {min={math.min(x,a[1]),math.min(y,a[2]),math.min(z,a[3])},
                       max={math.max(x,b[1]),math.max(y,b[2]),math.max(z,b[3])}}
    assert(not authoring.Intersects(authoring.Get(p),footprint), "another chat reserved this site")
    assert((b[1]-a[1]+1)*(b[2]-a[2]+1)*(b[3]-a[3]+1)<=65536, "model footprint too large")
    for bx=a[1],b[1] do for bz=a[3],b[3] do
        assert(world:Loaded(bx,bz), "unloaded model footprint")
        for by=a[2],b[2] do
            assert(by>=0 and by<=255 and B:GetBlockId(bx,by,bz)==0, "model intersects a block or world boundary")
        end
    end end
    assert(not world:EntitiesIntersectReal(lo,hi), "model intersects an entity")
    cmd(string.format('/setblock %d %d %d %d 0 {attr={filename="%s",scale=%.12g,facing=%.12g}}',
        x,y,z,id,p.filename,p.scale,p.facing))
    entity = Game.EntityManager.GetBlockEntity(x,y,z)
end
assert(B:GetBlockId(x,y,z)==id and entity and entity:GetModelFile()==p.filename, "occupied or mismatched carrier")
assert(math.abs(entity:GetScaling()-p.scale)<1e-5 and math.abs(entity:GetFacing()-p.facing)<1e-5,
    "model transform differs")
local obj = entity:GetInnerObject()
local actual = obj and obj:IsValid() and obj:GetPrimaryAsset()
if not actual or not actual:IsLoaded() then return {state="instance_loading"} end
assert(actual:GetKeyName()==asset:GetKeyName(), "renderer selected another asset")
return {state="placed", position=p.at, filename=entity:GetModelFile(), scale=entity:GetScaling(),
    facing=entity:GetFacing(), bounds=actual:GetBoundingBox({})}
