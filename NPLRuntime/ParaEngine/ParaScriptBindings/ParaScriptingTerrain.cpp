//-----------------------------------------------------------------------------
// Class: ParaTerrain	(in ParaScripting3.cpp)
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12
// Revised: 2005.12
//-----------------------------------------------------------------------------

#include "ParaEngine.h"

#include "NPLHelper.h"
#include "terrain/GlobalTerrain.h"
#include "ParaWorldAsset.h"
#include "ParaScriptingTerrain.h"


#include <luabind/luabind.hpp>
#include <luabind/object.hpp>


#include "ParaScriptingBlockWorld.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockRegion.h"
#include "ViewportManager.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "2dengine/GUIRoot.h"

#include "memdebug.h"



using namespace ParaEngine;

namespace ParaScripting
{
	ParaTerrain::ParaTerrain(void)
	{
	}

	ParaTerrain::~ParaTerrain(void)
	{
	}

	void ParaTerrain::UpdateTerrain()
	{
		return CGlobals::GetGlobalTerrain()->Update();
	}

	void ParaTerrain::UpdateTerrain1(bool bForceUpdate)
	{
		return CGlobals::GetGlobalTerrain()->Update(bForceUpdate);
	}

	float ParaTerrain::GetElevation(float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->GetElevation(x, y);
	}

	DWORD ParaTerrain::GetRegionValue(const char* sLayerName, float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->GetRegionValue(sLayerName, x, y);
	}
	DWORD ParaTerrain::GetRegionValue4(const char* sLayerName, float x, float y, const char* argb)
	{
		// assuming that value is 32 bits. 
		DWORD color = CGlobals::GetGlobalTerrain()->GetRegionValue(sLayerName, x, y);
		DWORD value = 0;

		if (argb[0] == '\0')
		{
			value = color;
		}
		else
		{
			char c;
			for (int i = 0; (c = argb[i]) != '\0'; ++i)
			{
				if (c == 'a')
				{
					value += color >> 24;
				}
				else if (c == 'r')
				{
					value += (color & 0x00ff0000) >> 16;
				}
				else if (c == 'g')
				{
					value += (color & 0x0000ff00) >> 8;
				}
				else if (c == 'b')
				{
					value += (color & 0x000000ff);
				}
			}
		}
		return value;
	}

	string ParaTerrain::GetTerrainOnloadScript(float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->GetOnloadScript(x, y);
	}

	const char * ParaTerrain::GetTerrainOnloadScript__(float x, float y)
	{
		static string g_str;
		g_str = GetTerrainOnloadScript(x, y);
		return g_str.c_str();
	}

	string ParaTerrain::GetTerrainElevFile(float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->GetTerrainElevFile(x, y);
	}

	const char* ParaTerrain::GetTerrainElevFile__(float x, float y)
	{
		static string g_str;
		g_str = GetTerrainElevFile(x, y);
		return g_str.c_str();
	}

	void ParaTerrain::SaveTerrain(bool bHeightMap, bool bTextures)
	{
		CGlobals::GetGlobalTerrain()->SaveTerrain(bHeightMap, bTextures);
	}

	void ParaTerrain::ReloadTerrain(bool bHeightMap, bool bTextures)
	{
		//TODO:
		OUTPUT_LOG("ParaTerrain::ReloadTerrain is not implemented yet.\r\n");
	}

	void ParaTerrain::Paint_(int nDetailTextureID, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
	{
		//TODO:
		OUTPUT_LOG("ParaTerrain::Paint_ is not implemented yet.\r\n");
	}

	void ParaTerrain::Paint(const char* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
	{
		TextureEntity* pDetail = NULL;
		if (detailTexture != NULL && detailTexture[0] != '\0')
			pDetail = CGlobals::GetAssetManager()->LoadTexture("", detailTexture, TextureEntity::StaticTexture);
		CGlobals::GetGlobalTerrain()->Paint((TextureEntity*)(pDetail), brushRadius, brushIntensity, maxIntensity, erase, x, y);
	}

	void ParaTerrain::Flatten(float x, float y, float radius, int flatten_op, float elevation, float factor)
	{
		CGlobals::GetGlobalTerrain()->Flatten(x, y, radius, flatten_op, elevation, factor);
	}

	void ParaTerrain::Spherical(float x, float y, float radius, float offset)
	{
		CGlobals::GetGlobalTerrain()->Spherical(x, y, radius, offset);
	}

	void ParaTerrain::DigCircleFlat(float x, float y, float radius, float fFlatPercentage, float factor)
	{
		CGlobals::GetGlobalTerrain()->DigCircleFlat(x, y, radius, fFlatPercentage, factor);
	}

	void ParaTerrain::GaussianHill(float x, float y, float radius, float hscale, float standard_deviation, float smooth_factor)
	{
		CGlobals::GetGlobalTerrain()->GaussianHill(x, y, radius, hscale, standard_deviation, smooth_factor);
	}

	void ParaTerrain::RadialScale(float x, float y, float scale_factor, float min_dist, float max_dist, float smooth_factor)
	{
		CGlobals::GetGlobalTerrain()->RadialScale(x, y, scale_factor, min_dist, max_dist, smooth_factor, 1);
	}

	void ParaTerrain::Roughen_Smooth(float x, float y, float radius, bool roughen, bool big_grid, float factor)
	{
		CGlobals::GetGlobalTerrain()->Roughen_Smooth(x, y, radius, roughen, big_grid, factor);
	}
	void ParaTerrain::Ramp(float x1, float y1, float x2, float y2, float radius, float borderpercentage, float factor)
	{
		CGlobals::GetGlobalTerrain()->Ramp(x1, y1, x2, y2, radius, borderpercentage, factor);
	}

	void ParaTerrain::AddHeightField(float x, float y, const char* filename, int nSmoothPixels)
	{
		CGlobals::GetGlobalTerrain()->AddHeightField(x, y, filename, nSmoothPixels);
	}
	void ParaTerrain::MergeHeightField(float x, float y, const char* filename, int mergeOperation/*=0*/, float weight1/*=1.0*/, float weight2/*=1.0*/, int nSmoothPixels/*=7*/)
	{
		CGlobals::GetGlobalTerrain()->MergeHeightField(x, y, filename, mergeOperation, weight1, weight2, nSmoothPixels);
	}

	bool ParaTerrain::IsModified()
	{
		return CGlobals::GetGlobalTerrain()->IsModified();
	}

	void ParaTerrain::SetContentModified(float x, float y, bool bIsModified)
	{
		CGlobals::GetGlobalTerrain()->SetContentModified(x, y, bIsModified);
	}

	void ParaTerrain::SetContentModified4(float x, float y, bool bIsModified, DWORD dwModifiedBits)
	{
		CGlobals::GetGlobalTerrain()->SetContentModified(x, y, bIsModified, dwModifiedBits);
	}

	void ParaTerrain::SetAllLoadedModified(bool bIsModified, DWORD dwModifiedBits)
	{
		CGlobals::GetGlobalTerrain()->SetAllLoadedModified(bIsModified, dwModifiedBits);
	}

	void ParaTerrain::EnableLighting(bool bEnable)
	{
		CGlobals::GetGlobalTerrain()->EnableLighting(bEnable);
	}

	int ParaTerrain::GetTextureCount(float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->GetTextureCount(x, y);
	}

	ParaAssetObject ParaTerrain::GetTexture(float x, float y, int nIndex)
	{
		return ParaAssetObject(CGlobals::GetGlobalTerrain()->GetTexture(x, y, nIndex));
	}
	void ParaTerrain::GetTexture__(ParaAssetObject* pOut, float x, float y, int nIndex)
	{
		if (pOut)
		{
			*pOut = GetTexture(x, y, nIndex);
		}
	}

	void ParaTerrain::GetTexturesInCell(float x, float y, const object& out)
	{
		std::vector<int> indices;
		CGlobals::GetGlobalTerrain()->GetTexturesInCell(x, y, indices);

		for (int i = 0; i < (int)(indices.size()); ++i)
		{
			out[i + 1] = indices[i];
		}
	}

	bool ParaTerrain::RemoveTextureInCell(float x, float y, int nIndex)
	{
		return CGlobals::GetGlobalTerrain()->RemoveTextureInCell(x, y, nIndex);
	}

	bool ParaTerrain::ReplaceTexture(float x, float y, int nIndex, ParaAssetObject& TextureAsset)
	{
		if (TextureAsset.IsValid())
		{
			if (TextureAsset.m_pAsset->GetType() == AssetEntity::texture)
				return CGlobals::GetGlobalTerrain()->ReplaceTexture(x, y, nIndex, (TextureEntity*)(TextureAsset.m_pAsset));
			else
			{
				OUTPUT_LOG("warning: ParaTerrain::ReplaceTexture() must use a texture asset.\r\n");
			}
		}
		else
		{
			return CGlobals::GetGlobalTerrain()->ReplaceTexture(x, y, nIndex, NULL);
		}

		return false;
	}

	bool ParaTerrain::ReplaceTexture_(float x, float y, int nIndex, const object& TextureAsset_)
	{
		const char* TextureAsset = NPL::NPLHelper::LuaObjectToString(TextureAsset_);
		if (TextureAsset == NULL)
		{
			return CGlobals::GetGlobalTerrain()->ReplaceTexture(x, y, nIndex, NULL);
		}
		else
		{
			TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", TextureAsset, TextureEntity::StaticTexture);
			return CGlobals::GetGlobalTerrain()->ReplaceTexture(x, y, nIndex, pTex);
		}
	}

	ParaAttributeObject ParaTerrain::GetAttributeObject()
	{
		return ParaAttributeObject(CGlobals::GetGlobalTerrain());
	}
	void ParaTerrain::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = GetAttributeObject();
	}


	ParaAttributeObject ParaTerrain::GetBlockAttributeObject()
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		return ParaAttributeObject(mgr);
	}


	ParaAttributeObject ParaTerrain::GetAttributeObjectAt(float x, float y)
	{
		return ParaAttributeObject(CGlobals::GetGlobalTerrain()->GetAttributeObjectAt(x, y));
	}
	void ParaTerrain::GetAttributeObjectAt_(ParaAttributeObject& output, float x, float y)
	{
		output = GetAttributeObjectAt(x, y);
	}

	void ParaTerrain::SnapPointToVertexGrid(float x, float y, float* vertex_x, float* vertex_y)
	{
		CGlobals::GetGlobalTerrain()->SnapPointToVertexGrid(x, y, vertex_x, vertex_y);
	}

	float ParaTerrain::GetVertexSpacing()
	{
		return CGlobals::GetGlobalTerrain()->GetVertexSpacing();
	}

	void ParaTerrain::UpdateHoles(float x, float y)
	{
		CGlobals::GetGlobalTerrain()->UpdateHoles(x, y);
	}

	bool ParaTerrain::IsHole(float x, float y)
	{
		return CGlobals::GetGlobalTerrain()->IsHole(x, y);
	}

	void ParaTerrain::SetHole(float x, float y, bool bIsHold)
	{
		CGlobals::GetGlobalTerrain()->SetHole(x, y, bIsHold);
	}

	void ParaTerrain::PaintTerrainData(float x, float y, float radius, unsigned int data, unsigned int dataMask, unsigned int bitOffset)
	{
		CGlobals::GetGlobalTerrain()->PaintTerrainData(x, y, radius, data, dataMask, bitOffset);
	}

	uint32 ParaTerrain::GetTerrainData(float x, float y, uint32 dataMask, unsigned char bitOffset)
	{
		return CGlobals::GetGlobalTerrain()->GetTerrainData(x, y, dataMask, bitOffset);
	}

	void ParaTerrain::SetVisibleDataMask(uint32 mask, uint8 bitOffset)
	{
		CGlobals::GetGlobalTerrain()->SetVisibleDataMask(mask, bitOffset);
	}

	void ParaTerrain::EnterBlockWorld(float x, float y, float z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->EnterWorld("", x, y, z);
	}

	void ParaTerrain::LeaveBlockWorld()
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->LeaveWorld();
	}

	bool ParaTerrain::IsInBlockWorld()
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->IsInBlockWorld();
		return false;
	}

	void ParaTerrain::SaveBlockWorld(bool saveToTemp)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->SaveToFile(saveToTemp);
	}

	void ParaTerrain::DeleteAllBlocks()
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->DeleteAllBlocks();
	}

	void ParaTerrain::SetBlockTemplateId(float x, float y, float z, uint16_t templateId)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetBlockTemplateId(x, y, z, templateId);
	}

	void ParaTerrain::SetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t templateId)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetBlockTemplateIdByIdx(x, y, z, templateId);
	}

	uint32_t ParaTerrain::GetBlockTemplateId(float x, float y, float z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockTemplateId(x, y, z);
		return 0;
	}

	uint32_t ParaTerrain::GetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockTemplateIdByIdx(x, y, z);
		return 0;
	}

	void ParaTerrain::SetBlockUserData(float x, float y, float z, uint32_t data)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetBlockUserData(x, y, z, data);
	}

	void ParaTerrain::SetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t data)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetBlockUserDataByIdx(x, y, z, data);
	}

	uint32_t ParaTerrain::GetBlockUserData(float x, float y, float z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockUserData(x, y, z);
		return 0;
	}

	uint32_t ParaTerrain::GetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockUserDataByIdx(x, y, z);
		return 0;
	}

	bool ParaTerrain::SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->SetBlockMaterial(x, y, z, nFaceId, nMaterial);
		return false;
	}

	int32_t ParaTerrain::GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockMaterial(x, y, z, nFaceId);
		return -1;
	}
	
	bool ParaTerrain::RegisterBlockTemplate(uint16_t templateId, const object& params)
	{
		return ParaBlockWorld::RegisterBlockTemplate_(BlockWorldClient::GetInstance(), templateId, params);
	}

	void ParaTerrain::SetTemplateTexture(uint16_t templateId, const char* fileName)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->SetTemplateTexture(templateId, fileName);
	}

	void ParaTerrain::SetBlockWorldYOffset(float offset)
	{
		BlockWorldClient::SetVerticalOffset(offset);
	}

	object ParaTerrain::Pick(float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const object& result, uint32_t filter)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			PickResult pickResult;
			Vector3 rayOrig(rayX, rayY, rayZ);
			Vector3 rayDir(dirX, dirY, dirZ);
			rayDir.normalise();
			if (mgr->Pick(rayOrig, rayDir, fMaxDistance, pickResult, filter))
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

	object ParaTerrain::GetBlocksInRegion(int32_t startChunkX, int32_t startChunkY, int32_t startChunkZ, int32_t endChunkX, int32_t endChunkY, int32_t endChunkZ, uint32_t matchType, const object& result)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr && mgr->IsInBlockWorld())
		{
			if (type(result) == LUA_TTABLE)
			{
				uint32 verticalSectionFilter = 0;
				// just in case startChunkY is used as verticalSectionFilter
				if (startChunkY < 0 && startChunkY == endChunkY)
					verticalSectionFilter = -startChunkY;
				if (startChunkX < 0) startChunkX = 0;
				if (startChunkY < 0) startChunkY = 0;
				if (startChunkZ < 0) startChunkZ = 0;
				if (endChunkX >= 0xffff) endChunkX = 0xfffe;
				if (endChunkY >= BlockConfig::g_regionChunkDimY) endChunkY = BlockConfig::g_regionChunkDimY - 1;
				if (endChunkZ >= 0xffff) endChunkZ = 0xfffe;

				Uint16x3 start(startChunkX, startChunkY, startChunkZ);
				Uint16x3 end(endChunkX, endChunkY, endChunkZ);

				if (type(result["x"]) != LUA_TTABLE)
				{
					object table = newtable(result.interpreter());
					result["x"] = table;
				}
				if (type(result["y"]) != LUA_TTABLE)
				{
					object table = newtable(result.interpreter());
					result["y"] = table;
				}
				if (type(result["z"]) != LUA_TTABLE)
				{
					object table = newtable(result.interpreter());
					result["z"] = table;
				}
				if (type(result["tempId"]) != LUA_TTABLE)
				{
					object table = newtable(result.interpreter());
					result["tempId"] = table;
				}
				if (type(result["data"]) != LUA_TTABLE)
				{
					object table = newtable(result.interpreter());
					result["data"] = table;
				}

				int32_t count = mgr->GetBlocksInRegion(start, end, matchType, result, verticalSectionFilter);
				result["count"] = count;
				return object(result);
			}
		}

		if (type(result) == LUA_TTABLE)
		{
			result["count"] = 0;
		}
		return object(result);
	}

	object ParaTerrain::GetVisibleChunkRegion(const object& result)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr && mgr->IsInBlockWorld())
		{
			if (type(result) == LUA_TTABLE)
			{
				Int16x3 minChunkId = mgr->GetMinActiveChunkId();
				int32_t maxChunkX = (int32_t)minChunkId.x + mgr->GetActiveChunkDim() - 1;
				int32_t maxChunkY = (int32_t)minChunkId.y + mgr->GetActiveChunkDimY() - 1;
				int32_t maxChunkZ = (int32_t)minChunkId.z + mgr->GetActiveChunkDim() - 1;
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

	luabind::object ParaTerrain::MousePick(float fMaxDistance, const object& result, uint32_t filter/*=0xffffffff*/)
	{
		if (!CGlobals::GetGUI()->GetMouseInClient() || !CGlobals::GetViewportManager())
			return object(result);

		int nScreenX, nScreenY;
		CGlobals::GetGUI()->GetMousePosition(&(nScreenX), &(nScreenY));

		Matrix4 matWorld;
		matWorld = Matrix4::IDENTITY;

		POINT ptCursor;
		float fScaleX = 1.f, fScaleY = 1.f;
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
		ptCursor.x = (fScaleX == 1.f) ? nScreenX : (int)(nScreenX*fScaleX);
		ptCursor.y = (fScaleY == 1.f) ? nScreenY : (int)(nScreenY*fScaleY);

		int x = ptCursor.x;
		int y = ptCursor.y;
		int nWidth, nHeight;
		CGlobals::GetViewportManager()->GetPointOnViewport(x, y, &nWidth, &nHeight);
		ptCursor.x = x;
		ptCursor.y = y;
		if (x < 0 || y < 0)
			return object(result);

		Vector3 vPickRayOrig, vPickRayDir;
		CGlobals::GetScene()->GetCurrentCamera()->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor, nWidth, nHeight, &matWorld);

		vPickRayOrig += CGlobals::GetScene()->GetRenderOrigin();
		return ParaTerrain::Pick(vPickRayOrig.x, vPickRayOrig.y, vPickRayOrig.z, vPickRayDir.x, vPickRayDir.y, vPickRayDir.z, fMaxDistance, result, filter);
	}


	void ParaTerrain::SelectBlock1(uint16_t x, uint16_t y, uint16_t z, bool isSelect, int nGroupID)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			if (isSelect)
				mgr->SelectBlock(x, y, z, nGroupID);
			else
				mgr->DeselectBlock(x, y, z, nGroupID);
		}
	}

	void ParaTerrain::SelectBlock(uint16_t x, uint16_t y, uint16_t z, bool isSelect)
	{
		SelectBlock1(x, y, z, isSelect, 0);
	}

	void ParaTerrain::DeselectAllBlock()
	{
		DeselectAllBlock1(-1);
	}

	void ParaTerrain::DeselectAllBlock1(int nGroupID)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->DeselectAllBlock(nGroupID);
	}

	void ParaTerrain::SetDamagedBlock(uint16_t x, uint16_t y, uint16_t z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetDamagedBlock(x, y, z);
	}

	void ParaTerrain::SetDamageDegree(float damagedDegree)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetDamagedBlockDegree(damagedDegree);
	}

	void ParaTerrain::SetBlockWorldSunIntensity(float value)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			mgr->SetSunIntensity(value);
	}

	int ParaTerrain::FindFirstBlock(uint16_t x, uint16_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
		return -1;
	}

	int ParaTerrain::GetFirstBlock(uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetFirstBlock(x, y, z, nBlockId, nSide, max_dist);
		return -1;
	}

	int32_t ParaTerrain::GetChunkColumnTimeStamp(uint32_t chunkX, uint32_t chunkZ)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr && !mgr->IsChunkLocked(chunkX, chunkZ))
			return mgr->GetChunkColumnTimeStamp((uint16_t)chunkX, (uint16_t)chunkZ);
		else
			return -1;
	}

	void ParaTerrain::SetChunkColumnTimeStamp(uint32_t chunkX, uint32_t chunkZ, uint32_t nTimeStamp)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->SetChunkColumnTimeStamp((uint16_t)chunkX, (uint16_t)chunkZ, (uint16_t)nTimeStamp);
	}

	const std::string& ParaTerrain::GetMapChunkData(uint32_t chunkX, uint32_t chunkZ, bool bIncludeInit, uint32_t verticalSectionFilter)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			BlockRegion* pRegion = mgr->CreateGetRegion((uint16_t)(chunkX >> 5), (uint16_t)(chunkZ >> 5));
			if (pRegion)
			{
				return pRegion->GetMapChunkData(chunkX, chunkZ, bIncludeInit, verticalSectionFilter);
			}
		}
		return CGlobals::GetString();
	}

	object ParaTerrain::ApplyMapChunkData(uint32_t chunkX, uint32_t chunkZ, uint32_t verticalSectionFilter, const std::string& chunkData, const object& out)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			BlockRegion* pRegion = mgr->CreateGetRegion((uint16_t)(chunkX >> 5), (uint16_t)(chunkZ >> 5));
			if (pRegion)
			{
				pRegion->ApplyMapChunkData(chunkX, chunkZ, verticalSectionFilter, chunkData, out);
			}
			else
			{
				OUTPUT_LOG("error: ApplyMapChunkData called when region is not loaded. ");
			}
		}
		return out;
	}


	void ParaTerrain::GetBlockFullData(uint16_t x, uint16_t y, uint16_t z, uint16_t* pId, uint32_t* pUserData)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();

		*pId = 0;
		*pUserData = 0;

		if (mgr)
		{
			auto pBlock = mgr->GetUnlockBlock(x, y, z);
			if (pBlock)
			{
				*pId = pBlock->GetTemplateId();
				*pUserData = pBlock->GetUserData();
			}
		}

	}

	void ParaTerrain::LoadBlockAsync(uint16_t x, uint16_t y, uint16_t z, uint16_t blockId, uint32_t userData)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			mgr->LoadBlockAsync(x, y, z, blockId, userData);
		}
	}

}

// for LuaJit, only for function that maybe called millions of time per second
extern "C" {

	PE_CORE_DECL void ParaTerrain_GetBlockFullData(uint16_t x, uint16_t y, uint16_t z, uint16_t* pId, uint32_t* pUserData)
	{
		ParaScripting::ParaTerrain::GetBlockFullData(x, y, z, pId, pUserData);
	}


	PE_CORE_DECL float ParaTerrain_GetElevation(float x, float y)
	{
		return ParaScripting::ParaTerrain::GetElevation(x, y);
	}

	PE_CORE_DECL void ParaTerrain_SelectBlock(uint16_t x, uint16_t y, uint16_t z, bool isSelect, int nGroupID)
	{
		return ParaScripting::ParaTerrain::SelectBlock1(x, y, z, isSelect, nGroupID);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockTemplateId(float x, float y, float z, uint16_t templateId)
	{
		ParaScripting::ParaTerrain::SetBlockTemplateId(x, y, z, templateId);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t templateId)
	{
		ParaScripting::ParaTerrain::SetBlockTemplateIdByIdx(x, y, z, templateId);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockTemplateId(float x, float y, float z)
	{
		return ParaScripting::ParaTerrain::GetBlockTemplateId(x, y, z);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z)
	{
		return ParaScripting::ParaTerrain::GetBlockTemplateIdByIdx(x, y, z);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockUserData(float x, float y, float z, uint32_t data)
	{
		ParaScripting::ParaTerrain::SetBlockUserData(x, y, z, data);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t data)
	{
		ParaScripting::ParaTerrain::SetBlockUserDataByIdx(x, y, z, data);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockUserData(float x, float y, float z)
	{
		return ParaScripting::ParaTerrain::GetBlockUserData(x, y, z);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z)
	{
		return ParaScripting::ParaTerrain::GetBlockUserDataByIdx(x, y, z);
	}

	PE_CORE_DECL void ParaTerrain_UpdateHoles(float x, float y)
	{
		ParaScripting::ParaTerrain::UpdateHoles(x, y);
	}

	PE_CORE_DECL bool ParaTerrain_IsHole(float x, float y)
	{
		return ParaScripting::ParaTerrain::IsHole(x, y);
	}

	PE_CORE_DECL void ParaTerrain_SetHole(float x, float y, bool bIsHold)
	{
		ParaScripting::ParaTerrain::SetHole(x, y, bIsHold);
	}

	PE_CORE_DECL int ParaTerrain_FindFirstBlock(uint16_t x, uint16_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
	{
		return ParaScripting::ParaTerrain::FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
	}

	PE_CORE_DECL int ParaTerrain_GetFirstBlock(uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
	{
		return ParaScripting::ParaTerrain::GetFirstBlock(x, y, z, nBlockId, nSide, max_dist);
	}

	PE_CORE_DECL void ParaTerrain_LoadBlockAsync(uint16_t x, uint16_t y, uint16_t z, uint16_t blockId, uint32_t userData)
	{
		ParaScripting::ParaTerrain::LoadBlockAsync(x, y, z, blockId, userData);
	}
	
	PE_CORE_DECL bool ParaTerrain_SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
	{
		return ParaScripting::ParaTerrain::SetBlockMaterial(x, y, z, nFaceId, nMaterial);
	}

	PE_CORE_DECL int32_t ParaTerrain_GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId)
	{
		return ParaScripting::ParaTerrain::GetBlockMaterial(x, y, z, nFaceId);
	}
};
