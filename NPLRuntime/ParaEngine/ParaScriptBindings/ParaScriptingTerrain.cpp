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
#include "BlockEngine/ChunkGeneratorFast.h"
#include "BlockEngine/ProceduralNoise.h"
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

	void ParaTerrain::SetBlockTemplateIdByIdx(uint16_t x, int32_t y, uint16_t z, uint32_t templateId)
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

	uint32_t ParaTerrain::GetBlockTemplateIdByIdx(uint16_t x, int32_t y, uint16_t z)
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

	void ParaTerrain::SetBlockUserDataByIdx(uint16_t x, int32_t y, uint16_t z, uint32_t data)
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

	uint32_t ParaTerrain::GetBlockUserDataByIdx(uint16_t x, int32_t y, uint16_t z)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->GetBlockUserDataByIdx(x, y, z);
		return 0;
	}

	bool ParaTerrain::SetBlockMaterial(uint16_t x, int32_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->SetBlockMaterial(x, y, z, nFaceId, nMaterial);
		return false;
	}

	int32_t ParaTerrain::GetBlockMaterial(uint16_t x, int32_t y, uint16_t z, int16_t nFaceId)
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
		return GetBlocksInWorldRegion(BlockWorldClient::GetInstance(), startChunkX, startChunkY, startChunkZ, endChunkX, endChunkY, endChunkZ, matchType, result);
	}

	object ParaTerrain::GetBlocksInWorldRegion(CBlockWorld* mgr, int32_t startChunkX, int32_t startChunkY, int32_t startChunkZ, int32_t endChunkX, int32_t endChunkY, int32_t endChunkZ, uint32_t matchType, const object& result)
	{
		if (mgr && mgr->IsInBlockWorld())
		{
			if (type(result) == LUA_TTABLE)
			{
				uint32 verticalSectionFilter = 0;
				// just in case startChunkY is used as verticalSectionFilter
				if (startChunkY < 0 && startChunkY >= -65535 && startChunkY == endChunkY)
					verticalSectionFilter = -startChunkY;
				if (startChunkX < 0) startChunkX = 0;
				if (startChunkY < BlockConfig::MinSectionY) startChunkY = BlockConfig::MinSectionY;
				if (startChunkZ < 0) startChunkZ = 0;
				if (endChunkX > 4095) endChunkX = 4095;
				if (endChunkY > BlockConfig::MaxSectionY) endChunkY = BlockConfig::MaxSectionY;
				if (endChunkZ > 4095) endChunkZ = 4095;
				if (startChunkX > endChunkX || startChunkZ > endChunkZ || (!verticalSectionFilter && startChunkY > endChunkY))
				{
					result["count"] = 0;
					return result;
				}

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
				if (minChunkId.y < BlockConfig::MinSectionY) minChunkId.y = BlockConfig::MinSectionY;
				if (minChunkId.z < 0) minChunkId.z = 0;
				if (maxChunkX > 0xffff) maxChunkX = 0xffff;
				if (maxChunkY > BlockConfig::MaxSectionY)
					maxChunkY = BlockConfig::MaxSectionY;
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


	void ParaTerrain::SelectBlock1(uint16_t x, int32_t y, uint16_t z, bool isSelect, int nGroupID)
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

	void ParaTerrain::SelectBlock(uint16_t x, int32_t y, uint16_t z, bool isSelect)
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

	void ParaTerrain::SetDamagedBlock(uint16_t x, int32_t y, uint16_t z)
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

	int ParaTerrain::FindFirstBlock(uint16_t x, int32_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
			return mgr->FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
		return -1;
	}

	int ParaTerrain::GetFirstBlock(uint16_t x, int32_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
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

	static std::vector<int16_t> ReadSectionList(const object& sections)
{
	std::vector<int16_t> result;
	if (type(sections) == LUA_TTABLE)
	{
		for (luabind::iterator it(sections), end; it != end; ++it)
		{
			int y = luabind::object_cast<int>(*it);
			if (BlockConfig::IsSectionY(y)) result.push_back(static_cast<int16_t>(y));
		}
		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());
	}
	return result;
}

object ParaTerrain::GetChunkSections(uint32_t x, uint32_t z, const object& result)
{
	if (x >= 4096 || z >= 4096) return result;
	auto* region = BlockWorldClient::GetInstance()->GetRegion(x >> 5, z >> 5);
	if (region && !region->IsLocked())
	{
		int index = 0;
		for (int y : region->GetColumnSections(x, z))
			result[++index] = y;
	}
	return result;
}

uint32_t ParaTerrain::GetColumnLightHeights(uint32_t x, uint32_t z)
{
	if (x > 65535 || z > 65535) return 0;
	auto* height = BlockWorldClient::GetInstance()->GetHighestBlock(x, z);
	if (!height) return 0;
	const int first = BlockConfig::MinBlockY - 1;
	return uint32_t(height->m_nMaxHeight - first) << 16 | uint32_t(height->m_nSolidHeight - first);
}

void ParaTerrain::ApplyColumnLightHeights(uint32_t x, uint32_t z, uint32_t heights)
{
	auto* world = BlockWorldClient::GetInstance();
	if (x > 65535 || z > 65535 || !world->IsBuildableY(BlockConfig::MinBlockY)) return;
	const int first = BlockConfig::MinBlockY - 1;
	const int solid = first + (heights & 65535), top = first + (heights >> 16);
	if (solid > top || top > BlockConfig::MaxBlockY) return;
	Scoped_WriteLock<BlockReadWriteLock> lock(world->GetReadWriteLock());
	auto* region = world->GetRegion(x >> 9, z >> 9);
	if (!region || region->IsLocked()) return;
	region->SetColumnHeight(x & 511, z & 511, ChunkMaxHeight(solid, top));
}

std::string ParaTerrain::GetMapSectionData(uint32_t x, uint32_t z, const object& sections)
{
	if (x >= 4096 || z >= 4096) return {};
	auto* region = BlockWorldClient::GetInstance()->GetRegion(x >> 5, z >> 5);
	if (!region || region->IsLocked()) return {};
	auto list = ReadSectionList(sections);
	return region->GetMapChunkData(x, z, true, 0, &list);
}

object ParaTerrain::ApplyMapSectionData(uint32_t x, uint32_t z, const std::string& data, const object& result)
{
	if (x >= 4096 || z >= 4096) return result;
	auto* region = BlockWorldClient::GetInstance()->CreateGetRegion(x >> 5, z >> 5);
	if (region) region->ApplyMapChunkData(x, z, 0, data, result, true);
	return result;
}

object ParaTerrain::GetBlocksInSections(uint32_t x, uint32_t z, const object& sections, uint32_t matchType, const object& result)
{
	if (x >= 4096 || z >= 4096) return result;
	auto* region = BlockWorldClient::GetInstance()->GetRegion(x >> 5, z >> 5);
	for (const char* key : {"x", "y", "z", "tempId", "data"})
		result[key] = newtable(result.interpreter());
	int count = 0;
	if (region)
		for (int y : ReadSectionList(sections))
			region->GetBlocksInChunk(x, z, y, y, matchType, result, count);
	result["count"] = count;
	return result;
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


	void ParaTerrain::GetBlockFullData(uint16_t x, int32_t y, uint16_t z, uint16_t* pId, uint32_t* pUserData)
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

	void ParaTerrain::LoadBlockAsync(uint16_t x, int32_t y, uint16_t z, uint16_t blockId, uint32_t userData)
	{
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (mgr)
		{
			mgr->LoadBlockAsync(x, y, z, blockId, userData);
		}
	}

	namespace
	{
		template<typename T>
		T ReadFieldOrDefault(const object& tbl, const char* key, T defaultVal)
		{
			if (!tbl.is_valid() || type(tbl) != LUA_TTABLE)
				return defaultVal;
			object v = tbl[key];
			if (v.is_valid() && type(v) == LUA_TNUMBER)
				return static_cast<T>(object_cast<double>(v));
			return defaultVal;
		}

		bool ReadBoolOrDefault(const object& tbl, const char* key, bool defaultVal)
		{
			if (!tbl.is_valid() || type(tbl) != LUA_TTABLE)
				return defaultVal;
			object v = tbl[key];
			if (v.is_valid() && type(v) == LUA_TBOOLEAN)
				return object_cast<bool>(v);
			return defaultVal;
		}

		template<typename T, size_t N>
		void WriteFlatArray256(lua_State* L, int summaryTableIdx, const char* fieldName, const std::array<T, N>& values)
		{
			lua_getfield(L, summaryTableIdx, fieldName);
			if (!lua_istable(L, -1))
			{
				lua_pop(L, 1);
				lua_createtable(L, static_cast<int>(N), 0);
				lua_pushvalue(L, -1);
				lua_setfield(L, summaryTableIdx, fieldName);
			}
			for (size_t i = 0; i < N; ++i)
			{
				lua_pushinteger(L, static_cast<lua_Integer>(values[i]));
				lua_rawseti(L, -2, static_cast<int>(i + 1));
			}
			lua_pop(L, 1);
		}
	}

	bool ParaTerrain::ConfigureWorldGen(int profileId, const object& config)
	{
		if (!config.is_valid() || type(config) != LUA_TTABLE)
			return false;

		auto profile = std::make_shared<WorldGen::WorldGenProfile>();
		profile->seed = ReadFieldOrDefault<int32_t>(config, "seed", 12345);
		profile->presetMode = ReadFieldOrDefault<int32_t>(config, "presetMode", WorldGen::Preset_EpicOverworld);
		profile->seaLevel = ReadFieldOrDefault<int16_t>(config, "seaLevel", 64);
		profile->bedrockY = ReadFieldOrDefault<int16_t>(config, "bedrockY", -240);
		profile->lavaLevel = ReadFieldOrDefault<int16_t>(config, "lavaLevel", -216);
		profile->snowLineY = ReadFieldOrDefault<int16_t>(config, "snowLineY", 230);

		profile->baseHeight = ReadFieldOrDefault<float>(config, "baseHeight", 68.0f);
		profile->continentalFreq = ReadFieldOrDefault<float>(config, "continentalFreq", 0.00075f);
		profile->mountainHeight = ReadFieldOrDefault<float>(config, "mountainHeight", 310.0f);
		profile->peakBoost = ReadFieldOrDefault<float>(config, "peakBoost", 210.0f);
		profile->karstStrength = ReadFieldOrDefault<float>(config, "karstStrength", 1.0f);
		profile->canyonStrength = ReadFieldOrDefault<float>(config, "canyonStrength", 1.0f);
		profile->riverStrength = ReadFieldOrDefault<float>(config, "riverStrength", 1.0f);
		profile->overhangStrength = ReadFieldOrDefault<float>(config, "overhangStrength", 1.0f);

		profile->skyIslandChance = ReadFieldOrDefault<float>(config, "skyIslandChance", 0.22f);
		profile->skyIslandMinY = ReadFieldOrDefault<int16_t>(config, "skyIslandMinY", 260);
		profile->skyIslandMaxY = ReadFieldOrDefault<int16_t>(config, "skyIslandMaxY", 640);
		profile->skyWaterfallChance = ReadFieldOrDefault<float>(config, "skyWaterfallChance", 0.35f);

		profile->caveStrength = ReadFieldOrDefault<float>(config, "caveStrength", 1.0f);
		profile->sinkholeChance = ReadFieldOrDefault<float>(config, "sinkholeChance", 0.25f);
		profile->deepCavernMinY = ReadFieldOrDefault<int16_t>(config, "deepCavernMinY", -210);
		profile->deepCavernMaxY = ReadFieldOrDefault<int16_t>(config, "deepCavernMaxY", -36);

		object blocksTbl = config["blocks"];
		if (blocksTbl.is_valid() && type(blocksTbl) == LUA_TTABLE)
		{
			auto& b = profile->blocks;
			b.bedrock = ReadFieldOrDefault<uint16_t>(blocksTbl, "bedrock", b.bedrock);
			b.stone = ReadFieldOrDefault<uint16_t>(blocksTbl, "stone", b.stone);
			b.dirt = ReadFieldOrDefault<uint16_t>(blocksTbl, "dirt", b.dirt);
			b.grass = ReadFieldOrDefault<uint16_t>(blocksTbl, "grass", b.grass);
			b.sand = ReadFieldOrDefault<uint16_t>(blocksTbl, "sand", b.sand);
			b.sandstone = ReadFieldOrDefault<uint16_t>(blocksTbl, "sandstone", b.sandstone);
			b.clay = ReadFieldOrDefault<uint16_t>(blocksTbl, "clay", b.clay);
			b.gravel = ReadFieldOrDefault<uint16_t>(blocksTbl, "gravel", b.gravel);
			b.snow = ReadFieldOrDefault<uint16_t>(blocksTbl, "snow", b.snow);
			b.snowBlock = ReadFieldOrDefault<uint16_t>(blocksTbl, "snowBlock", b.snowBlock);
			b.ice = ReadFieldOrDefault<uint16_t>(blocksTbl, "ice", b.ice);
			b.brokenIce = ReadFieldOrDefault<uint16_t>(blocksTbl, "brokenIce", b.brokenIce);
			b.mossStone = ReadFieldOrDefault<uint16_t>(blocksTbl, "mossStone", b.mossStone);
			b.obsidian = ReadFieldOrDefault<uint16_t>(blocksTbl, "obsidian", b.obsidian);
			b.netherrack = ReadFieldOrDefault<uint16_t>(blocksTbl, "netherrack", b.netherrack);
			b.glowstone = ReadFieldOrDefault<uint16_t>(blocksTbl, "glowstone", b.glowstone);
			b.water = ReadFieldOrDefault<uint16_t>(blocksTbl, "water", b.water);
			b.lava = ReadFieldOrDefault<uint16_t>(blocksTbl, "lava", b.lava);
			b.mycelium = ReadFieldOrDefault<uint16_t>(blocksTbl, "mycelium", b.mycelium);
			b.colorBlock = ReadFieldOrDefault<uint16_t>(blocksTbl, "colorBlock", b.colorBlock);
			b.transparentColorBlock = ReadFieldOrDefault<uint16_t>(blocksTbl, "transparentColorBlock", b.transparentColorBlock);
		}

		object biomesTbl = config["biomes"];
		if (biomesTbl.is_valid() && type(biomesTbl) == LUA_TTABLE)
		{
			for (luabind::iterator it(biomesTbl), end; it != end; ++it)
			{
				object entry = *it;
				if (type(entry) != LUA_TTABLE) continue;
				WorldGen::BiomeDef bd;
				bd.id = ReadFieldOrDefault<uint8_t>(entry, "id", 1);
				bd.specialFlag = ReadFieldOrDefault<uint8_t>(entry, "specialFlag", 0);
				bd.useStrata = ReadBoolOrDefault(entry, "useStrata", false);
				bd.minTemp = ReadFieldOrDefault<float>(entry, "minTemp", 0.0f);
				bd.maxTemp = ReadFieldOrDefault<float>(entry, "maxTemp", 1.0f);
				bd.minMoist = ReadFieldOrDefault<float>(entry, "minMoist", 0.0f);
				bd.maxMoist = ReadFieldOrDefault<float>(entry, "maxMoist", 1.0f);
				bd.topBlockId = ReadFieldOrDefault<uint16_t>(entry, "topBlockId", profile->blocks.grass);
				bd.topBlockData = ReadFieldOrDefault<uint32_t>(entry, "topBlockData", 0);
				bd.fillerBlockId = ReadFieldOrDefault<uint16_t>(entry, "fillerBlockId", profile->blocks.dirt);
				bd.fillerBlockData = ReadFieldOrDefault<uint32_t>(entry, "fillerBlockData", 0);
				bd.cliffBlockId = ReadFieldOrDefault<uint16_t>(entry, "cliffBlockId", profile->blocks.stone);
				bd.cliffBlockData = ReadFieldOrDefault<uint32_t>(entry, "cliffBlockData", 0);
				bd.underwaterBlockId = ReadFieldOrDefault<uint16_t>(entry, "underwaterBlockId", profile->blocks.sand);
				bd.underwaterBlockData = ReadFieldOrDefault<uint32_t>(entry, "underwaterBlockData", 0);
				bd.fillerDepth = ReadFieldOrDefault<int16_t>(entry, "fillerDepth", 4);
				profile->biomes.push_back(bd);
				if (profile->biomes.size() >= 32) break;
			}
		}

		object strataTbl = config["strata"];
		if (strataTbl.is_valid() && type(strataTbl) == LUA_TTABLE)
		{
			for (luabind::iterator it(strataTbl), end; it != end; ++it)
			{
				object entry = *it;
				if (type(entry) != LUA_TTABLE) continue;
				WorldGen::StrataBand sb;
				sb.blockId = ReadFieldOrDefault<uint16_t>(entry, "blockId", profile->blocks.sandstone);
				sb.blockData = ReadFieldOrDefault<uint32_t>(entry, "blockData", 0);
				profile->strata.push_back(sb);
				if (profile->strata.size() >= 64) break;
			}
		}

		object oresTbl = config["ores"];
		if (oresTbl.is_valid() && type(oresTbl) == LUA_TTABLE)
		{
			for (luabind::iterator it(oresTbl), end; it != end; ++it)
			{
				object entry = *it;
				if (type(entry) != LUA_TTABLE) continue;
				WorldGen::OrePocketDef od;
				od.blockId = ReadFieldOrDefault<uint16_t>(entry, "blockId", 125);
				od.blockData = ReadFieldOrDefault<uint32_t>(entry, "blockData", 0);
				od.minY = ReadFieldOrDefault<int16_t>(entry, "minY", -240);
				od.maxY = ReadFieldOrDefault<int16_t>(entry, "maxY", 128);
				od.frequency = ReadFieldOrDefault<float>(entry, "frequency", 0.09f);
				od.threshold = ReadFieldOrDefault<float>(entry, "threshold", 0.72f);
				profile->ores.push_back(od);
				if (profile->ores.size() >= 24) break;
			}
		}

		WorldGen::ChunkGeneratorFast::SetProfile(profileId, profile);
		return true;
	}

	bool ParaTerrain::GenerateBaseColumn(int profileId, int chunkX, int chunkZ, const object& outSummary)
	{
		auto profile = WorldGen::ChunkGeneratorFast::GetProfile(profileId);
		if (!profile)
			return false;

		auto& buf = WorldGen::ChunkGeneratorFast::GetThreadBuffer();
		WorldGen::ChunkGeneratorFast::GenerateBaseColumn(*profile, chunkX, chunkZ, buf);

		if (outSummary.is_valid() && type(outSummary) == LUA_TTABLE)
		{
			lua_State* L = outSummary.interpreter();
			outSummary.push(L);
			const int tblIdx = lua_gettop(L);

			WriteFlatArray256(L, tblIdx, "surfaceY", buf.m_surfaceY);
			WriteFlatArray256(L, tblIdx, "surfaceId", buf.m_surfaceBlockId);
			WriteFlatArray256(L, tblIdx, "biomeId", buf.m_biomeId);
			WriteFlatArray256(L, tblIdx, "slopeGrad", buf.m_slopeGrad);
			WriteFlatArray256(L, tblIdx, "skyTopY", buf.m_skyTopY);
			WriteFlatArray256(L, tblIdx, "skyBottomY", buf.m_skyBottomY);
			WriteFlatArray256(L, tblIdx, "caveFloorY", buf.m_caveFloorY);
			WriteFlatArray256(L, tblIdx, "caveCeilY", buf.m_caveCeilY);
			WriteFlatArray256(L, tblIdx, "waterDepth", buf.m_waterDepth);

			lua_pop(L, 1);
		}
		return true;
	}

	object ParaTerrain::SampleWorldColumn(int profileId, int worldX, int worldZ, const object& outInfo)
	{
		auto profile = WorldGen::ChunkGeneratorFast::GetProfile(profileId);
		if (!profile || !outInfo.is_valid() || type(outInfo) != LUA_TTABLE)
			return outInfo;

		const auto sample = WorldGen::ChunkGeneratorFast::SampleWorldColumn(*profile, worldX, worldZ);
		outInfo["surfaceY"] = static_cast<int>(sample.surfaceY);
		outInfo["surfaceId"] = static_cast<int>(sample.surfaceBlockId);
		outInfo["biomeId"] = static_cast<int>(sample.biomeId);
		outInfo["slopeGrad"] = static_cast<int>(sample.slopeGrad);
		outInfo["skyTopY"] = static_cast<int>(sample.skyTopY);
		outInfo["skyBottomY"] = static_cast<int>(sample.skyBottomY);
		outInfo["caveFloorY"] = static_cast<int>(sample.caveFloorY);
		outInfo["caveCeilY"] = static_cast<int>(sample.caveCeilY);
		outInfo["waterDepth"] = static_cast<int>(sample.waterDepth);
		outInfo["isRiver"] = sample.isRiver;
		outInfo["isSinkhole"] = sample.isSinkhole;
		return outInfo;
	}

	void ParaTerrain::GenResetBuffer(int chunkX, int chunkZ)
	{
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().Reset(chunkX, chunkZ);
	}

	void ParaTerrain::GenSetBlock(int lx, int wy, int lz, uint16_t blockId, uint32_t blockData, bool onlyIfAir)
	{
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().SetBlock(lx, wy, lz, blockId, blockData, onlyIfAir);
	}

	uint16_t ParaTerrain::GenGetBlockId(int lx, int wy, int lz)
	{
		return WorldGen::ChunkGeneratorFast::GetThreadBuffer().GetBlockId(lx, wy, lz);
	}

	void ParaTerrain::GenFillBox(int x1, int y1, int z1, int x2, int y2, int z2, uint16_t blockId, uint32_t blockData, int mode)
	{
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().FillBox(x1, y1, z1, x2, y2, z2, blockId, blockData, mode);
	}

	void ParaTerrain::GenFillEllipsoid(float cx, float cy, float cz, float rx, float ry, float rz, uint16_t blockId, uint32_t blockData, bool onlyIfAir)
	{
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().FillEllipsoid(cx, cy, cz, rx, ry, rz, blockId, blockData, onlyIfAir);
	}

	void ParaTerrain::GenStampTree(int lx, int wy, int lz, int style, uint32_t packedTrunk, uint32_t packedLeaf, uint32_t packedExtra, int packedSize, uint32_t seed)
	{
		const uint16_t trunkId = static_cast<uint16_t>(packedTrunk & 0xffffu);
		const uint32_t trunkData = (packedTrunk >> 16) & 0xffffu;
		const uint16_t leafId = static_cast<uint16_t>(packedLeaf & 0xffffu);
		const uint32_t leafData = (packedLeaf >> 16) & 0xffffu;
		const uint16_t extraId = static_cast<uint16_t>(packedExtra & 0xffffu);
		const uint32_t extraData = (packedExtra >> 16) & 0xffffu;
		const int height = packedSize & 0xff;
		const int radius = (packedSize >> 8) & 0xff;
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().StampTree(lx, wy, lz, style,
			trunkId, trunkData, leafId, leafData, extraId, extraData, height, radius, seed);
	}

	void ParaTerrain::GenStampCrystal(int lx, int wy, int lz, uint16_t coreId, uint16_t shellId, uint32_t shellData, int height, int radius, int signedSeed)
	{
		const bool pointUp = signedSeed >= 0;
		const uint32_t seed = static_cast<uint32_t>(pointUp ? signedSeed : -signedSeed);
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().StampCrystalCluster(lx, wy, lz, coreId, shellId, shellData, height, radius, pointUp, seed);
	}

	std::string ParaTerrain::GenEncodeChunkV2()
	{
		std::string encoded;
		WorldGen::ChunkGeneratorFast::GetThreadBuffer().EncodeChunkV2(encoded);
		return encoded;
	}

	bool ParaTerrain::GenCommitColumn(uint32_t chunkX, uint32_t chunkZ)
	{
		if (chunkX >= 4096 || chunkZ >= 4096)
			return false;
		BlockWorldClient* mgr = BlockWorldClient::GetInstance();
		if (!mgr)
			return false;
		BlockRegion* region = mgr->CreateGetRegion(static_cast<uint16_t>(chunkX >> 5), static_cast<uint16_t>(chunkZ >> 5));
		if (!region)
			return false;
		return WorldGen::ChunkGeneratorFast::GetThreadBuffer().CommitToRegion(mgr, region, static_cast<uint16_t>(chunkX), static_cast<uint16_t>(chunkZ));
	}

	double ParaTerrain::Noise2D(int seed, double x, double z, int octaves, double lacunarity, double gain)
	{
		if (octaves <= 1)
			return static_cast<double>(WorldGen::ProceduralNoise::Noise2D(seed, x, z));
		return static_cast<double>(WorldGen::ProceduralNoise::Fbm2D(seed, x, z, octaves, lacunarity, gain));
	}

	double ParaTerrain::Noise3D(int seed, double x, double y, double z, int octaves, double lacunarity, double gain)
	{
		if (octaves <= 1)
			return static_cast<double>(WorldGen::ProceduralNoise::Noise3D(seed, x, y, z));
		return static_cast<double>(WorldGen::ProceduralNoise::Fbm3D(seed, x, y, z, octaves, lacunarity, gain));
	}

	double ParaTerrain::RidgedNoise2D(int seed, double x, double z, int octaves, double lacunarity, double gain)
	{
		return static_cast<double>(WorldGen::ProceduralNoise::Ridged2D(seed, x, z, octaves, lacunarity, gain));
	}

}

// for LuaJit, only for function that maybe called millions of time per second
extern "C" {

	PE_CORE_DECL void ParaTerrain_GetBlockFullData(uint16_t x, int32_t y, uint16_t z, uint16_t* pId, uint32_t* pUserData)
	{
		ParaScripting::ParaTerrain::GetBlockFullData(x, y, z, pId, pUserData);
	}


	PE_CORE_DECL float ParaTerrain_GetElevation(float x, float y)
	{
		return ParaScripting::ParaTerrain::GetElevation(x, y);
	}

	PE_CORE_DECL void ParaTerrain_SelectBlock(uint16_t x, int32_t y, uint16_t z, bool isSelect, int nGroupID)
	{
		return ParaScripting::ParaTerrain::SelectBlock1(x, y, z, isSelect, nGroupID);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockTemplateId(float x, float y, float z, uint16_t templateId)
	{
		ParaScripting::ParaTerrain::SetBlockTemplateId(x, y, z, templateId);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockTemplateIdByIdx(uint16_t x, int32_t y, uint16_t z, uint32_t templateId)
	{
		ParaScripting::ParaTerrain::SetBlockTemplateIdByIdx(x, y, z, templateId);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockTemplateId(float x, float y, float z)
	{
		return ParaScripting::ParaTerrain::GetBlockTemplateId(x, y, z);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockTemplateIdByIdx(uint16_t x, int32_t y, uint16_t z)
	{
		return ParaScripting::ParaTerrain::GetBlockTemplateIdByIdx(x, y, z);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockUserData(float x, float y, float z, uint32_t data)
	{
		ParaScripting::ParaTerrain::SetBlockUserData(x, y, z, data);
	}

	PE_CORE_DECL void ParaTerrain_SetBlockUserDataByIdx(uint16_t x, int32_t y, uint16_t z, uint32_t data)
	{
		ParaScripting::ParaTerrain::SetBlockUserDataByIdx(x, y, z, data);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockUserData(float x, float y, float z)
	{
		return ParaScripting::ParaTerrain::GetBlockUserData(x, y, z);
	}

	PE_CORE_DECL uint32_t ParaTerrain_GetBlockUserDataByIdx(uint16_t x, int32_t y, uint16_t z)
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

	PE_CORE_DECL int ParaTerrain_FindFirstBlock(uint16_t x, int32_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t attrFilter /*= 0xffffffff*/, int nCategoryID /*= -1*/)
	{
		return ParaScripting::ParaTerrain::FindFirstBlock(x, y, z, nSide, max_dist, attrFilter, nCategoryID);
	}

	PE_CORE_DECL int ParaTerrain_GetFirstBlock(uint16_t x, int32_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
	{
		return ParaScripting::ParaTerrain::GetFirstBlock(x, y, z, nBlockId, nSide, max_dist);
	}

	PE_CORE_DECL void ParaTerrain_LoadBlockAsync(uint16_t x, int32_t y, uint16_t z, uint16_t blockId, uint32_t userData)
	{
		ParaScripting::ParaTerrain::LoadBlockAsync(x, y, z, blockId, userData);
	}
	
	PE_CORE_DECL bool ParaTerrain_SetBlockMaterial(uint16_t x, int32_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
	{
		return ParaScripting::ParaTerrain::SetBlockMaterial(x, y, z, nFaceId, nMaterial);
	}

	PE_CORE_DECL int32_t ParaTerrain_GetBlockMaterial(uint16_t x, int32_t y, uint16_t z, int16_t nFaceId)
	{
		return ParaScripting::ParaTerrain::GetBlockMaterial(x, y, z, nFaceId);
	}
};
