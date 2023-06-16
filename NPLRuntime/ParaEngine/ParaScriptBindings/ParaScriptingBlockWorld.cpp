//-----------------------------------------------------------------------------
// Class: ParaBlockWorld
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.2.6
// Desc: exposed in ParaScripting4.cpp
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "BlockEngine/BlockWorld.h"
#include "BlockEngine/BlockWorldManager.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ParaScriptingBlockWorld.h"

/**@define  user defined block index */
#define CUSTOM_BLOCK_ID_BEGIN 2000
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif

extern "C"
{
#include "lua.h"
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace ParaScripting;
using namespace ParaEngine;
using namespace luabind;

#include "memdebug.h"

#define GETBLOCKWORLD(pWorld, pWorld_)  \
	PE_ASSERT(luabind::type(pWorld_) == LUA_TLIGHTUSERDATA); \
	lua_State* L = pWorld_.interpreter(); \
	pWorld_.push(L); \
	CBlockWorld* pWorld = (CBlockWorld*)lua_touserdata(L, -1); \
	lua_pop(L, 1);

ParaScripting::ParaBlockWorld::ParaBlockWorld()
{
}

ParaScripting::ParaBlockWorld::~ParaBlockWorld()
{
}

luabind::object ParaScripting::ParaBlockWorld::GetWorld(const object& sWorldName_)
{
	std::string sWorldName;
	if (type(sWorldName_) == LUA_TSTRING)
	{
		sWorldName = object_cast<const char*> (sWorldName_);
	}
	void * pWorld = NULL;
	if (!sWorldName.empty())
		pWorld = CBlockWorldManager::GetSingleton()->CreateGetBlockWorld(sWorldName);
	else
		pWorld = BlockWorldClient::GetInstance();

	auto L = sWorldName_.interpreter();

	lua_pushlightuserdata(L, pWorld);
	luabind::object ret(luabind::from_stack(L, -1));
	lua_pop(L, 1);
	return ret;
}

void ParaScripting::ParaBlockWorld::EnterWorld(const object& pWorld_, const char* sWorldDir)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->EnterWorld(sWorldDir, 0, 0, 0);
}

void ParaScripting::ParaBlockWorld::LeaveWorld(const object& pWorld_)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->LeaveWorld();
}


bool ParaScripting::ParaBlockWorld::RegisterBlockTemplate(const object& pWorld_, uint16_t templateId, const object& params)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return RegisterBlockTemplate_(pWorld, templateId, params);
}

bool ParaScripting::ParaBlockWorld::RegisterBlockTemplate_(CBlockWorld* pWorld, uint16_t templateId, const object& params)
{
	uint32_t attFlag = 0;
	uint16_t category_id = 0;
	std::string sModelName, sNormalMap, sTexture0, sTexture2, sTexture3, sTexture4;
	Color dwMapColor = 0;
	int nOpacity = -1;
	uint16_t associated_blockid = 0;
	int nTorchLight = -1;
	float fSpeedReduction = 1.0f;
	bool bIsUpdating = false;
	bool bProvidePower = false;
	bool bCustomBlockModel = false;
	bool bIsVisible = true;
	int nTile = -1;
	Color under_water_color = 0;
	if (type(params) == LUA_TNUMBER)
	{
		attFlag = (uint32_t)(object_cast<double> (params));
	}
	else if (type(params) == LUA_TTABLE)
	{
		if (type(params["IsUpdating"]) == LUA_TBOOLEAN)
			bIsUpdating = object_cast<bool> (params["IsUpdating"]);
		if (type(params["customBlockModel"]) == LUA_TBOOLEAN)
			bCustomBlockModel = object_cast<bool> (params["customBlockModel"]);
		if (type(params["attFlag"]) == LUA_TNUMBER)
			attFlag = (uint32_t)(object_cast<double> (params["attFlag"]));
		if (type(params["modelName"]) == LUA_TSTRING)
			sModelName = object_cast<const char*>(params["modelName"]);
		if (type(params["normalMap"]) == LUA_TSTRING)
			sNormalMap = object_cast<const char*>(params["normalMap"]);
		if (type(params["categoryID"]) == LUA_TNUMBER)
			category_id = (uint16_t)(object_cast<double> (params["categoryID"]));
		if (type(params["associated_blockid"]) == LUA_TNUMBER)
			associated_blockid = (uint16_t)(object_cast<double> (params["associated_blockid"]));
		if (type(params["associated_blockid"]) == LUA_TNUMBER)
			associated_blockid = (uint16_t)(object_cast<double> (params["associated_blockid"]));
		if (type(params["ProvidePower"]) == LUA_TBOOLEAN)
			bProvidePower = object_cast<bool> (params["ProvidePower"]);
		if (type(params["torchLightValue"]) == LUA_TNUMBER)
			nTorchLight = (int)(object_cast<double> (params["torchLightValue"]));
		if (type(params["speedReduction"]) == LUA_TNUMBER)
			fSpeedReduction = (float)(object_cast<double> (params["speedReduction"]));
		if (type(params["texture"]) == LUA_TSTRING)
			sTexture0 = object_cast<const char*>(params["texture"]);
		if (type(params["texture2"]) == LUA_TSTRING)
			sTexture2 = object_cast<const char*>(params["texture2"]);
		if (type(params["texture3"]) == LUA_TSTRING)
			sTexture3 = object_cast<const char*>(params["texture3"]);
		if (type(params["texture4"]) == LUA_TSTRING)
			sTexture4 = object_cast<const char*>(params["texture4"]);
		if (type(params["mapcolor"]) == LUA_TSTRING)
			dwMapColor = Color::FromString(object_cast<const char*>(params["mapcolor"]));
		if (type(params["opacity"]) == LUA_TNUMBER)
			nOpacity = (int)(object_cast<double> (params["opacity"]));
		if (type(params["isVisible"]) == LUA_TBOOLEAN)
			bIsVisible = object_cast<bool> (params["isVisible"]);
		if (type(params["under_water_color"]) == LUA_TSTRING)
			under_water_color = Color::FromString(object_cast<const char*>(params["under_water_color"]));
		if (type(params["tile"]) == LUA_TNUMBER)
			nTile = (int)(object_cast<double> (params["tile"]));
	}

	if (bIsUpdating)
	{
		BlockTemplate* pTemplate = pWorld->GetBlockTemplate(templateId);
		if (pTemplate)
		{
			if (!sModelName.empty())
				pTemplate->LoadModel(sModelName);
			if (!sNormalMap.empty())
				pTemplate->SetNormalMap(sNormalMap.c_str());
			if (!sTexture0.empty())
				pTemplate->SetTexture0(sTexture0.c_str());
			if (!sTexture2.empty())
				pTemplate->SetTexture0(sTexture2.c_str(), 2);
			if (!sTexture3.empty())
				pTemplate->SetTexture0(sTexture3.c_str(), 3);
			if (!sTexture4.empty())
				pTemplate->SetTexture0(sTexture4.c_str(), 4);
			if (fSpeedReduction < 1.f)
				pTemplate->SetSpeedReductionPercent(fSpeedReduction);
			if (nOpacity > 0)
				pTemplate->SetLightOpacity(nOpacity);
			if ((DWORD)dwMapColor != 0)
				pTemplate->SetMapColor(dwMapColor);
			if ((DWORD)under_water_color != 0)
				pTemplate->setUnderWaterColor(under_water_color);
			if (nTile >= 0) {
				pTemplate->setTileSize(nTile);
			}


			bool bRefreshBlockTemplate = false;
			if (attFlag != 0 && pTemplate->GetAttFlag() != attFlag)	
			{
				pTemplate->Init(attFlag, pTemplate->GetCategoryID());
				bRefreshBlockTemplate = true;
			}

			bRefreshBlockTemplate = bRefreshBlockTemplate || pWorld->SetBlockVisible(templateId, bIsVisible, false) || bRefreshBlockTemplate;
			if (nTorchLight >= 0 && pTemplate->GetTorchLight() != nTorchLight)
			{
				pTemplate->SetTorchLight(nTorchLight);
				bRefreshBlockTemplate = true;
			}
			
			if (bRefreshBlockTemplate)
			{
				pWorld->RefreshBlockTemplate(templateId);
			}
			return true;
		}
	}
	else
	{
		BlockTemplate* pTemplate = pWorld->GetBlockTemplate(templateId);

		if (pTemplate != NULL && templateId >= CUSTOM_BLOCK_ID_BEGIN)
		{
			// we will allow user defined custom block to be overridden
			// make all chunks dirty, to reflect the changes. 
			pWorld->ClearBlockRenderCache();
			pTemplate->Init(attFlag, category_id);
		}
		else
			pTemplate = pWorld->RegisterTemplate(templateId, attFlag, category_id);

		if (pTemplate)
		{
			if (!sModelName.empty())
				pTemplate->LoadModel(sModelName);
			if (!sNormalMap.empty())
				pTemplate->SetNormalMap(sNormalMap.c_str());
			if (!sTexture0.empty())
				pTemplate->SetTexture0(sTexture0.c_str());
			if (!sTexture2.empty())
				pTemplate->SetTexture0(sTexture2.c_str(), 2);
			if (!sTexture3.empty())
				pTemplate->SetTexture0(sTexture3.c_str(), 3);
			if (!sTexture4.empty())
				pTemplate->SetTexture0(sTexture4.c_str(), 4);
			if (associated_blockid > 0)
				pTemplate->SetAssociatedBlock(associated_blockid);
			if (bProvidePower)
				pTemplate->setProvidePower(true);
			if (nTorchLight > 0)
				pTemplate->SetTorchLight(nTorchLight);
			if (fSpeedReduction < 1.f)
				pTemplate->SetSpeedReductionPercent(fSpeedReduction);
			if (nOpacity > 0)
				pTemplate->SetLightOpacity(nOpacity);
			if ((DWORD)dwMapColor != 0)
				pTemplate->SetMapColor(dwMapColor);
			if ((DWORD)under_water_color != 0)
				pTemplate->setUnderWaterColor(under_water_color);
			if (nTile >= 0) {
				pTemplate->setTileSize(nTile);
			}

			if (bCustomBlockModel && type(params["models"]) == LUA_TTABLE && sModelName!="stairs")
			{
				const object& models = params["models"];
				int nMaxDataId = -1;
				int nMinDataId = -1;
				for (int i = 1; (type(models[i]) == LUA_TTABLE); ++i)
				{
					const object& model = models[i];
					if (type(model["assetfile"]) == LUA_TSTRING)
					{
						std::string assetfile = object_cast<const char*>(model["assetfile"]);
						Matrix4 mat = Matrix4::IDENTITY;

						if (type(model["transform"]) == LUA_TSTRING)
						{
							std::string transform = object_cast<const char*>(model["transform"]);
							if (sscanf(transform.c_str(), "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23, &mat._31, &mat._32, &mat._33, &mat._41, &mat._42, &mat._43) >= 12)
							{
								mat._14 = 0;
								mat._24 = 0;
								mat._34 = 0;
								mat._44 = 1;
							}
							else
							{
								mat = Matrix4::IDENTITY;
							}
						}

						if (type(model["facing"]) == LUA_TNUMBER)
						{
							float fFacing = (float)(object_cast<double> (model["facing"]));
							if (fFacing != 0.f)
							{
								Matrix4 matRot;
								ParaMatrixRotationY(&matRot, fFacing);
								mat *= matRot;
							}
						}
						int nTextureIndex = 0;
						if (type(model["texture_index"]) == LUA_TNUMBER)
						{
							nTextureIndex = (int)(object_cast<double> (model["texture_index"]));
						}
						if (type(model["id_data"]) == LUA_TNUMBER)
						{
							int id_data = (int)(object_cast<double> (model["id_data"]));
							if (id_data > nMaxDataId)
								nMaxDataId = id_data;
							if (nMinDataId == -1 || nMinDataId > id_data)
								nMinDataId = id_data;
							pTemplate->CreateGetBlockModel(id_data).LoadModel(pTemplate, assetfile, mat, nTextureIndex);
						}
					}
				}
				if (nMaxDataId >= 0)
				{
					if (nMinDataId != 0)
					{
						// index 0 must contains the a valid face count since it is used to estimate render buffer size in some cases. 
						pTemplate->GetBlockModel(0).CloneRenderData(pTemplate->GetBlockModel(nMinDataId));
					}
					pTemplate->MakeCustomLinearModelProvider(nMaxDataId + 1);
				}
			}
			return true;
		}
	}
	return false;
}

void ParaScripting::ParaBlockWorld::SetBlockWorldYOffset(const object& pWorld_, float offset)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetVerticalOffset(offset);
}

void ParaScripting::ParaBlockWorld::SetBlockId(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, uint32_t templateId)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetBlockTemplateIdByIdx(x, y, z, templateId);
}

uint32_t ParaScripting::ParaBlockWorld::GetBlockId(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return pWorld->GetBlockTemplateIdByIdx(x, y, z);
}

void ParaScripting::ParaBlockWorld::SetBlockData(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, uint32_t data)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetBlockUserDataByIdx(x, y, z, data);
}

uint32_t ParaScripting::ParaBlockWorld::GetBlockData(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return pWorld->GetBlockUserDataByIdx(x, y, z);
}

luabind::object ParaScripting::ParaBlockWorld::GetBlocksInRegion(const object& pWorld_, int32_t startChunkX, int32_t startChunkY, int32_t startChunkZ, int32_t endChunkX, int32_t endChunkY, int32_t endChunkZ, uint32_t matchType, const object& result)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return object(result);
}

void ParaScripting::ParaBlockWorld::SetBlockWorldSunIntensity(const object& pWorld_, float value)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetSunIntensity(value);
}

int ParaScripting::ParaBlockWorld::FindFirstBlock(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return pWorld->FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
}

int ParaScripting::ParaBlockWorld::GetFirstBlock(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return pWorld->GetFirstBlock(x, y, z, nBlockId, nSide, max_dist);
}

void ParaScripting::ParaBlockWorld::SetTemplateTexture(const object& pWorld_, uint16_t templateId, const char* fileName)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetTemplateTexture(templateId, fileName);
}

const char* ParaScripting::ParaBlockWorld::GetTemplatePhysicsProperty(const object& pWorld_, uint16_t templateId)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return pWorld->GetTemplatePhysicsProperty(templateId);
}

void ParaScripting::ParaBlockWorld::SetTemplatePhysicsProperty(const object& pWorld_, uint16_t templateId, const char* property)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SetTemplatePhysicsProperty(templateId, property);
}

luabind::object ParaScripting::ParaBlockWorld::GetVisibleChunkRegion(const object& pWorld_, const object& result)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	if ((pWorld) && pWorld->IsInBlockWorld())
	{
		if (type(result) == LUA_TTABLE)
		{
			Int16x3 minChunkId = pWorld->GetMinActiveChunkId();
			int32_t maxChunkX = (int32_t)minChunkId.x + pWorld->GetActiveChunkDim() - 1;
			int32_t maxChunkY = (int32_t)minChunkId.y + pWorld->GetActiveChunkDimY() - 1;
			int32_t maxChunkZ = (int32_t)minChunkId.z + pWorld->GetActiveChunkDim() - 1;
			;

			if (minChunkId.x < 0) minChunkId.x = 0;
			if (minChunkId.y < 0) minChunkId.y = 0;
			if (minChunkId.z < 0) minChunkId.z = 0;
			if (maxChunkX > 0xffff) maxChunkX = 0xffff;
			if (maxChunkY > BlockConfig::g_regionChunkDimY)
				maxChunkY = BlockConfig::g_regionChunkDimY - 1;
			if (maxChunkZ > 0xffff) maxChunkZ = 0xffff;

			result["minX"] = minChunkId.x;
			result["minY"] = minChunkId.y;
			result["minZ"] = minChunkId.z;
			result["maxX"] = maxChunkX;
			result["maxY"] = maxChunkY;
			result["maxZ"] = maxChunkZ;
		}
	}
	return object(result);
}

luabind::object ParaScripting::ParaBlockWorld::Pick(const object& pWorld_, float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const object& result, uint32_t filter /*= 0xffffffff*/)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	if (pWorld)
	{
		PickResult pickResult;
		Vector3 rayOrig(rayX, rayY, rayZ);
		Vector3 rayDir(dirX, dirY, dirZ);

		if (pWorld->Pick(rayOrig, rayDir, fMaxDistance, pickResult, filter))
		{
			if (type(result) == LUA_TTABLE)
			{
				result["x"] = pickResult.X;
				result["y"] = pickResult.Y;
				result["z"] = pickResult.Z;
				result["blockX"] = pickResult.BlockX;
				result["blockY"] = pickResult.BlockY;
				result["blockZ"] = pickResult.BlockZ;
				result["side"] = pickResult.Side;
				result["length"] = pickResult.Distance;
			}
			return object(result);
		}
	}

	if (type(result) == LUA_TTABLE)
	{
		result["length"] = fMaxDistance + 10000;
	}
	return object(result);
}

luabind::object ParaScripting::ParaBlockWorld::MousePick(const object& pWorld_, float fMaxDistance, const object& result, uint32_t filter /*= 0xffffffff*/)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	OUTPUT_LOG("MousePick Not implemented\n");
	return object(result);
}

void ParaScripting::ParaBlockWorld::SelectBlock(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, bool isSelect)
{
	SelectBlock1(pWorld_, x, y, z, isSelect, 0);
}

void ParaScripting::ParaBlockWorld::SelectBlock1(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z, bool isSelect, int nGroupID)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	if (isSelect)
		pWorld->SelectBlock(x, y, z, nGroupID);
	else
		pWorld->DeselectBlock(x, y, z, nGroupID);
}

void ParaScripting::ParaBlockWorld::DeselectAllBlock1(const object& pWorld_, int nGroupID)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->DeselectAllBlock(nGroupID);
}

void ParaScripting::ParaBlockWorld::DeselectAllBlock(const object& pWorld_)
{
	DeselectAllBlock1(pWorld_, -1);
}

void ParaScripting::ParaBlockWorld::SetDamagedBlock(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z)
{
	OUTPUT_LOG("SetDamagedBlock Not implemented\n");
}

void ParaScripting::ParaBlockWorld::SetDamageDegree(const object& pWorld_, float damagedDegree)
{
	OUTPUT_LOG("SetDamagedBlock Not implemented\n");
}

void ParaScripting::ParaBlockWorld::LoadRegion(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->CreateGetRegion(x, y, z);
}

void ParaScripting::ParaBlockWorld::UnloadRegion(const object& pWorld_, uint16_t x, uint16_t y, uint16_t z)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->UnloadRegion(x, y, z, false);
}

int ParaScripting::ParaBlockWorld::GetVersion(const object& pWorld_)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return 0;
}

void ParaScripting::ParaBlockWorld::SaveBlockWorld(const object& pWorld_, bool saveToTemp)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	pWorld->SaveToFile(saveToTemp);
}

object ParaScripting::ParaBlockWorld::GetBlockModelInfo(int template_id, int blockData,const luabind::object& result)
{
	static std::map<int, luabind::object> _map;
	int key = template_id * 10000 + blockData;
	if (_map.find(key) != _map.end()) {
		return _map[key];
	}
	BlockTemplate* block_template = BlockWorldClient::GetInstance()->GetBlockTemplate((uint16)template_id);
	BlockModel& blockModel = block_template->GetBlockModelByData(blockData);
	int vertNum = blockModel.GetVerticesCount();
	Vector3 pt, normal;
	float u, v;
	result["m_Vertices"] = luabind::newtable(result.interpreter());
	for (int i = 0; i < vertNum; i++) {
		BlockVertexCompressed &vert = blockModel.Vertices()[i];
		vert.GetPosition(pt);
		vert.GetNormal(normal);
		vert.GetTexcoord(u, v);
		int idx = i + 1;
		result["m_Vertices"][idx] = luabind::newtable(result.interpreter());
		result["m_Vertices"][idx]["position"] = luabind::newtable(result.interpreter());
		result["m_Vertices"][idx]["position"]["x"] = pt.x;
		result["m_Vertices"][idx]["position"]["y"] = pt.y;
		result["m_Vertices"][idx]["position"]["z"] = pt.z;

		result["m_Vertices"][idx]["normal"] = luabind::newtable(result.interpreter());
		result["m_Vertices"][idx]["normal"]["x"] = normal.x;
		result["m_Vertices"][idx]["normal"]["y"] = normal.y;
		result["m_Vertices"][idx]["normal"]["z"] = normal.z;

		result["m_Vertices"][idx]["texcoord"] = luabind::newtable(result.interpreter());
		result["m_Vertices"][idx]["texcoord"]["u"] =u;
		result["m_Vertices"][idx]["texcoord"]["v"] = v;

		result["m_Vertices"][idx]["color"] = vert.color;
		result["m_Vertices"][idx]["color2"] = vert.color2;
	}
	result["m_bUseAO"] = blockModel.IsUseAmbientOcclusion();
	result["m_bUniformLighting"] = blockModel.IsUniformLighting();
	result["m_bDisableFaceCulling"] = blockModel.IsDisableFaceCulling();
	result["m_bUseSelfLighting"] = blockModel.IsUsingSelfLighting();
	result["m_bIsCubeAABB"] = blockModel.IsCubeAABB();
	result["m_nFaceCount"] = blockModel.GetFaceCount();
	result["m_nTextureIndex"] = blockModel.GetTextureIndex();

	_map[key] = object(result);
	return _map[key];
}

ParaScripting::ParaAttributeObject ParaScripting::ParaBlockWorld::GetBlockAttributeObject(const object& pWorld_)
{
	GETBLOCKWORLD(pWorld, pWorld_);
	return ParaAttributeObject(pWorld);
}

// for LuaJit, only for function that maybe called millions of time per second
extern "C" {

	PE_CORE_DECL void ParaBlockWorld_SetBlockId(void* pWorld, uint16_t x, uint16_t y, uint16_t z, uint32_t templateId)
	{
		((CBlockWorld*)pWorld)->SetBlockTemplateIdByIdx(x, y, z, templateId);
	}

	PE_CORE_DECL uint32_t ParaBlockWorld_GetBlockId(void* pWorld, uint16_t x, uint16_t y, uint16_t z)
	{
		return ((CBlockWorld*)pWorld)->GetBlockTemplateIdByIdx(x, y, z);
	}

	PE_CORE_DECL void ParaBlockWorld_SetBlockData(void* pWorld, uint16_t x, uint16_t y, uint16_t z, uint32_t data)
	{
		((CBlockWorld*)pWorld)->SetBlockUserDataByIdx(x, y, z, data);
	}

	PE_CORE_DECL uint32_t ParaBlockWorld_GetBlockData(void* pWorld, uint16_t x, uint16_t y, uint16_t z)
	{
		return ((CBlockWorld*)pWorld)->GetBlockUserDataByIdx(x, y, z);
	}

	PE_CORE_DECL int ParaBlockWorld_FindFirstBlock(void* pWorld, uint16_t x, uint16_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
	{
		return ((CBlockWorld*)pWorld)->FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
	}

	PE_CORE_DECL int ParaBlockWorld_GetFirstBlock(void* pWorld, uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
	{
		return ((CBlockWorld*)pWorld)->GetFirstBlock(x, y, z, nBlockId, nSide, max_dist);
	}
};
