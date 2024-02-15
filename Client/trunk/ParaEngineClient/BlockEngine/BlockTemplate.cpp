//-----------------------------------------------------------------------------
// Class: BlockTemplate
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2012.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockTemplate.h"
#include "ParaWorldAsset.h"
#include "BlockModelProvider.h"
#include "WireModelProvider.h"
#include "CarpetModelProvider.h"
#include "SlopeModelProvider.h"
#include "StairModelProvider.h"
#include "BlockWorld.h"
#include "SceneObject.h"
#include "util/regularexpression.h"
#include "StringHelper.h"

namespace ParaEngine
{
	const uint16_t BlockTemplate::g_maxRenderPriority = 0xf;

	BlockTemplate::BlockTemplate(uint16_t id, uint32_t attFlag, uint16_t category_id) :m_id(id), m_attFlag(attFlag), m_category_id(category_id), m_fPhysicalHeight(1.f), m_nTileSize(1),
		m_pNormalMap(nullptr), m_renderPriority(0), m_lightScatterStep(1), m_lightOpacity(1), m_pBlockModelFilter(NULL), m_bIsShadowCaster(true), m_associated_blockid(0),
		m_bProvidePower(false), m_nLightValue(0xf), m_fSpeedReductionPercent(1.f), m_renderPass(BlockRenderPass_Opaque), m_dwMapColor(Color::White), m_UnderWaterColor(0)
	{
		modelName = "";
		Init(attFlag, category_id);
	}

	BlockTemplate::~BlockTemplate()
	{
		SAFE_DELETE(m_pBlockModelFilter);
	}

	void BlockTemplate::Init(uint32_t attFlag, uint16_t category_id)
	{
		m_attFlag = attFlag;
		m_category_id = category_id;

		// init default parameters
		m_fPhysicalHeight = 1.f;
		m_renderPriority = 0;
		m_lightScatterStep = 1;
		m_lightOpacity = 1;
		m_associated_blockid = 0;
		m_bProvidePower = false;
		m_nLightValue = 0xf;
		m_fSpeedReductionPercent = 1.f;
		m_renderPass = BlockRenderPass_Opaque;

		m_textures0.resize(1);
		m_textures1.resize(1);

		// render priority: opaque object, alpha tested object, and then alpha blended object. 
		if (IsAlphaBlendedTexture() && !IsAlphaTestTexture())
			SetRenderPass(BlockRenderPass_AlphaBlended);
		else if (IsAlphaTestTexture())
			SetRenderPass(BlockRenderPass_AlphaTest);
		else
			SetRenderPass(BlockRenderPass_Opaque);

		if (IsMatchAttribute(batt_transparent))
		{
			if (GetRenderPass() == BlockRenderPass_AlphaBlended)
			{
				if (IsMatchAttribute(batt_liquid) && !IsMatchAttribute(batt_solid))
				{
					// liquid water has higher priority to render first among all alpha blending pass.
					m_renderPriority += 1;
					SetRenderPass(BlockRenderPass_ReflectedWater);
				}
			}
			else
			{
				// making liquid with lowest render priority so that it is rendered last, and more opaque objects have higher priority. 
				if (IsMatchAttribute(batt_liquid))
				{
					if (!IsMatchAttribute(batt_solid))
					{
						m_renderPriority += 1;
						// light in water scattered more than in air. 
						SetLightOpacity(2);
						// disable shadow for water
						m_bIsShadowCaster = false;
						SetRenderPass(BlockRenderPass_ReflectedWater);
					}
					else
					{
						// ice (solid liquid) is rendered after water. 
						// ice scatters light. 
						SetLightOpacity(2);
					}
				}
				else
				{
					m_renderPriority += 2;

					if (IsMatchAttribute(batt_solid))
					{
						m_renderPriority++;
					}
					if (IsMatchAttribute(batt_obstruction))
						m_renderPriority++;
					if (IsMatchAttribute(batt_customModel) && !IsMatchAttribute(batt_cubeModel))
						m_renderPriority++;
				}
			}
		}
		else
		{
			if (IsMatchAttribute(batt_solid))
				SetLightOpacity(15);
		}

		int32_t uvPattern = 0;

		m_nLightValue = IsMatchAttribute(batt_light) ? 0xf : 0;

		if (IsMatchAttribute(BlockTemplate::batt_threeSideTex))
			uvPattern = 3;
		else if (IsMatchAttribute(BlockTemplate::batt_fourSideTex))
			uvPattern = 4;
		else if (IsMatchAttribute(BlockTemplate::batt_sixSideTex))
			uvPattern = 6;

		if (IsMatchAttribute(BlockTemplate::batt_obstruction))
		{
			if (IsMatchAttribute(BlockTemplate::batt_climbable))
				SetPhysicalHeight(0.f);
			else
			{
				SetPhysicalHeight(1.f);
			}
		}
		else
		{
			SetPhysicalHeight(-1.f);
		}

		m_block_models.resize(1);
		SAFE_DELETE(m_pBlockModelFilter);

		GetBlockModel().LoadModelByTexture(uvPattern);
		GetBlockModel().SetCategoryID(GetCategoryID());
		if (!IsMatchAttribute(BlockTemplate::batt_cubeModel) && IsMatchAttribute(BlockTemplate::batt_customModel))
		{
			GetBlockModel().SetUniformLighting(true);
			GetBlockModel().SetIsCubeAABB(false);
		}
	}

	void BlockTemplate::SetTexture0(const char* texName, int nIndex)
	{
		if (texName)
		{
			if (nIndex == 0 && (IsMatchAttribute(BlockTemplate::batt_pos_tiling) || IsMatchAttribute(BlockTemplate::batt_random_tiling)))
			{
				regex r("^.+_x(\\d+)\\..+$");

				cmatch num;
				if (regex_search(texName, num, r))
				{
					std::string str(num[1].first, num[1].second - num[1].first);
					setTileSize(StringHelper::StrToInt(str.c_str()));
				}
			}

			if ((int)m_textures0.size() <= nIndex)
				m_textures0.resize(nIndex + 1);
			m_textures0[nIndex] = CGlobals::GetAssetManager()->LoadTexture("", texName, TextureEntity::StaticTexture);
		}
		else
		{
			if (nIndex < (int)m_textures0.size())
				m_textures0[nIndex].reset();
		}
	}

	void BlockTemplate::SetTexture1(const char* texName)
	{
		if (texName)
		{
			m_secondTexName = texName;
			m_textures1[0] = CGlobals::GetAssetManager()->LoadTexture("", m_secondTexName, TextureEntity::StaticTexture);
		}
		else
		{
			m_secondTexName.clear();
			m_textures1[0].reset();
		}
	}

	void BlockTemplate::SetNormalMap(const char* texName)
	{
		if (texName)
		{
			m_normalMapName = texName;
			m_pNormalMap = CGlobals::GetAssetManager()->LoadTexture("", m_normalMapName, TextureEntity::StaticTexture);
		}
		else
		{
			m_pNormalMap = nullptr;
		}
	}

	void BlockTemplate::GetBoundingBoxVertices(Vector3 * pVertices, int* pNumber)
	{
		GetBlockModel().GetBoundingBoxVertices(pVertices, pNumber);
	}

	BlockModel& BlockTemplate::CreateGetBlockModel(int nIndex /*= 0*/)
	{
		if (m_pBlockModelFilter != 0)
			return m_pBlockModelFilter->GetBlockModel(nIndex);
		else
		{
			if (nIndex >= (int)m_block_models.size())
				m_block_models.resize(nIndex + 1);
			return m_block_models[nIndex];
		}
	}

	BlockModel& BlockTemplate::GetBlockModelByData(uint32 nData)
	{
		if (m_pBlockModelFilter != 0)
			return m_pBlockModelFilter->GetBlockModelByData(nData & 0xff);
		else
			return m_block_models[0];
	}

	int BlockTemplate::GetFaceShape(int nSide, int32_t nData)
	{
		if (IsMatchAttribute(batt_cubeModel) && !IsMatchAttribute(batt_transparent))
		{
			if (IsMatchAttribute(batt_solid))
				return 0xf;
			else if (m_pBlockModelFilter != 0)
			{
				return GetBlockModelByData(nData).GetFaceShape(nSide);
			}
		}
		return 0;
	}


	BlockModel& BlockTemplate::GetBlockModel(int nIndex)
	{
		if (m_pBlockModelFilter != 0)
			return m_pBlockModelFilter->GetBlockModel(nIndex);
		else
			return (nIndex < (int)m_block_models.size()) ? m_block_models[nIndex] : m_block_models[0];
	}

	BlockModel& BlockTemplate::GetBlockModel(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks)
	{
		if (m_pBlockModelFilter != 0)
			return m_pBlockModelFilter->GetBlockModel(pBlockManager, GetID(), bx, by, bz, nBlockData, neighborBlocks);
		else {
			return m_block_models[0];
		}
	}

	void BlockTemplate::LoadModel(const std::string& sModelName)
	{
		this->modelName = sModelName;
		GetBlockModel().LoadModel(sModelName);

		if (IsMatchAttribute(BlockTemplate::batt_customModel))
			GetBlockModel().SetUniformLighting(true);

		if (sModelName == "wire")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CWireModelProvider(this);
		}
		else if (sModelName == "slope")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CSlopeModelProvider(this);
		}
		else if (sModelName == "grass")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CGrassModelProvider(this);

			// modify 16 different scaling and 
			m_block_models.resize(16, GetBlockModel());

			const float random_numbers[] = { 1.f,0.3f,0.8f, 0.5f,0.2f,0.9f, 0.1f, 0.15f, 0.4f, 0.55f, 0.8f, 0.95f, 0.25f, 0.05f, 0.75f, 0.65f, 0.25f, 0.85f, 1.f, 0.6f, 0.9f, };
			for (int i = 0; i < 16; i++)
			{
				BlockModel& block_model = m_block_models[i];
				float fScaling = 0.8f + random_numbers[i] * 0.2f;
				Vector3 vOffset((random_numbers[i] - 0.5f)*0.5f, 0, (random_numbers[16 - i] - 0.5f)*0.5f);
				block_model.Transform(vOffset, fScaling);
			}
		}
		else if (sModelName == "stairs")
		{
			// TODO: in future: currently it is not a cubeModel yet
			// SetLightOpacity(15);
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CStairModelProvider(this);
		}
		else if (sModelName == "slab")
		{
			SetLightOpacity(5);
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 2);

			// modify 16 different scaling and 
			m_block_models.resize(2, GetBlockModel());
			m_block_models[0].LoadModel("slab_top");
			m_block_models[1].LoadModel("slab_bottom");

			SetPhysicalHeight(0.5f);
		}
		else if (sModelName == "vine")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 24);

			// modify 
			m_block_models.resize(24, GetBlockModel());
			for (int dir = 0; dir < 4; dir++)
			{
				char sName[] = "vine00";
				sName[5] = '0' + dir;
				for (int i = 0; i < 6; i++)
				{
					sName[4] = '0' + i;
					m_block_models[i + dir * 6].LoadModel(sName);
				}
			}
		}
		else if (sModelName == "halfvine")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 6);

			// modify 
			m_block_models.resize(6, GetBlockModel());
			char sName[] = "halfvine0";
			for (int i = 0; i < 6; i++)
			{
				sName[8] = '0' + i;
				m_block_models[i].LoadModel(sName);
			}
		}
		else if (sModelName == "plate")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 6);

			// modify 
			m_block_models.resize(6, GetBlockModel());
			char sName[] = "plate0";
			for (int i = 0; i < 6; i++)
			{
				sName[5] = '0' + i;
				m_block_models[i].LoadModel(sName);
			}
		}
		else if (sModelName == "carpet")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CCarpetModelProvider(this);
		}
		else if (sModelName.find("plant") == 0)
		{
			// four state plant: 128*32 textures: mature,  growing, tiny, withered
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 4);

			// modify 
			m_block_models.resize(4, GetBlockModel());
			char sName[] = "cross0/4";
			for (int i = 0; i < 4; i++)
			{
				sName[5] = '0' + i;
				m_block_models[i].LoadModel(sName);
			}
		}
		else if (sModelName.find("seed_plant") == 0)
		{
			// four state plant: 128*32 textures: mature,  growing, seed, withered
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			m_pBlockModelFilter = new CLinearModelProvider(this, 4);

			// modify 
			m_block_models.resize(4, GetBlockModel());
			m_block_models[0].LoadModel("cross0/4");
			m_block_models[1].LoadModel("cross1/4");
			m_block_models[2].LoadModel("cross2/4");
			m_block_models[3].LoadModel("seed3/4"); // this is flat seed here
		}
		else if (sModelName == "codeblock")
		{
			// set model filter
			SAFE_DELETE(m_pBlockModelFilter);
			const int nDataCount = 16;
			m_pBlockModelFilter = new CLinearModelProvider(this, nDataCount);
			m_block_models.resize(nDataCount, GetBlockModel());
			m_block_models[0].SetTextureIndex(2);
		}
	}

	void BlockTemplate::SetAssociatedBlock(uint16_t associated_blockid)
	{
		m_associated_blockid = associated_blockid;
	}

	bool BlockTemplate::IsAssociatedBlockID(uint16_t block_id)
	{
		return (GetID() == block_id) || ((m_associated_blockid == block_id) && (m_associated_blockid > 0));
	}

	void BlockTemplate::setProvidePower(bool bValue)
	{
		m_bProvidePower = bValue;
	}

	bool BlockTemplate::isBlockNormalCube()
	{
		return IsMatchAttribute(batt_solid | batt_cubeModel) && !m_bProvidePower;
	}

	void BlockTemplate::SetTorchLight(uint8_t value)
	{
		if (value >= 0 && value <= 15)
			m_nLightValue = value;
	}

	void BlockTemplate::GetAABB(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz, CShapeAABB* pOutAABB)
	{
		if (GetBlockModel().IsCubeAABB())
		{
			GetBlockModel().GetAABB(pOutAABB);
		}
		else
		{
			Block* pBlock = pBlockManager->GetBlock(bx, by, bz);
			if (pBlock)
			{
				if (IsMatchAttribute(BlockTemplate::batt_cubeModel))
				{
					GetBlockModel(pBlockManager, bx, by, bz, pBlock->GetUserData()).GetAABB(pOutAABB);
				}
				else if (IsMatchAttribute(BlockTemplate::batt_customModel))
				{
					const double blockSize = BlockConfig::g_blockSize;
					const double fHalfBlockSize = blockSize / 2;
					Vector3 vPos(Vector3((float)(blockSize*bx + fHalfBlockSize), (float)(blockSize*by + pBlockManager->GetVerticalOffset() + fHalfBlockSize), (float)(blockSize*bz + fHalfBlockSize)));

					char tmp[256];
					snprintf(tmp, 255, "%d,%d,%d", bx, by, bz);
					CBaseObject* pObject = CGlobals::GetScene()->GetObject(tmp, vPos, false);
					if (pObject)
					{
						pObject->GetViewClippingObject()->GetAABB(pOutAABB);
						pOutAABB->GetCenter() -= (vPos - Vector3((float)(fHalfBlockSize), (float)(fHalfBlockSize), (float)(fHalfBlockSize)));
					}
					else
					{
						GetBlockModel().GetAABB(pOutAABB);
					}
				}
				else
				{
					GetBlockModel().GetAABB(pOutAABB);
				}
			}
			else
				GetBlockModel().GetAABB(pOutAABB);
		}
	}

	float BlockTemplate::GetPhysicalHeight()
	{
		return m_fPhysicalHeight;
	}

	float BlockTemplate::GetPhysicalHeight(CBlockWorld* pBlockManager, uint16_t bx, uint16_t by, uint16_t bz)
	{
		if (m_fPhysicalHeight > 0.f && IsMatchAttribute(BlockTemplate::batt_cubeModel) && (m_pBlockModelFilter || m_block_models.size() > 1))
		{
			Block* pBlock = pBlockManager->GetBlock(bx, by, bz);
			if (pBlock)
			{
				CShapeAABB aabb;
				GetBlockModel(pBlockManager, bx, by, bz, pBlock->GetUserData()).GetAABB(&aabb);
				// get max y height value.
				return aabb.GetMax(1);
			}
		}
		return m_fPhysicalHeight;
	}

	void BlockTemplate::SetPhysicalHeight(float fHeight)
	{
		m_fPhysicalHeight = fHeight;
	}

	void BlockTemplate::SetAttribute(DWORD dwAtt, bool bTurnOn /*= true*/)
	{
		if (bTurnOn)
			m_attFlag |= dwAtt;
		else
			m_attFlag &= (~dwAtt);
	}

	float BlockTemplate::GetSpeedReductionPercent() const
	{
		return m_fSpeedReductionPercent;
	}

	void BlockTemplate::SetSpeedReductionPercent(float val)
	{
		m_fSpeedReductionPercent = val;
	}

	void BlockTemplate::SetLightOpacity(int32 nValue)
	{
		m_lightOpacity = nValue;
		m_lightScatterStep = nValue;
	}

	void BlockTemplate::MakeCustomLinearModelProvider(int nModelCount)
	{
		// set model filter
		SAFE_DELETE(m_pBlockModelFilter);
		m_pBlockModelFilter = new CLinearModelProvider(this, nModelCount);
	}

	TextureEntity* BlockTemplate::GetTexture0(uint32 nUserData)
	{
		int nIndex = GetBlockModelByData(nUserData).GetTextureIndex();
		return nIndex < (int32)(m_textures0.size()) ? m_textures0[nIndex].get() : m_textures0[0].get();
	}

	TextureEntity* BlockTemplate::GetTexture1()
	{
		return m_textures1[0].get();
	}

	void BlockTemplate::SetMapColor(Color val)
	{
		m_dwMapColor = val;
	}

	Color BlockTemplate::GetMapColor() const
	{
		return m_dwMapColor;
	}

	DWORD BlockTemplate::GetBlockColor(int32_t blockData)
	{
		DWORD dwBlockColor = Color::White;
		if (HasColorData())
		{
			dwBlockColor = 0xff000000 | Color::convert16_32((uint16)blockData);
		}
		else if (IsColorData8Bits())
		{
			dwBlockColor = 0xff000000 | (~Color::convert8_32((uint8)(blockData >> 8)));
		}
		else
		{
			dwBlockColor = GetMapColor();
		}
		return dwBlockColor;
	}

	DWORD BlockTemplate::GetDiffuseColor(int32_t blockData)
	{
		DWORD dwBlockColor = Color::White;
		if (HasColorData())
		{
			dwBlockColor = 0xff000000 | Color::convert16_32((uint16)blockData);
		}
		else if (IsColorData8Bits())
		{
			dwBlockColor = 0xff000000 | (~Color::convert8_32((uint8)(blockData >> 8)));
		}
		return dwBlockColor;
	}

	bool BlockTemplate::isSolidBlock()
	{
		return IsMatchAttribute(batt_solid);
	}

	void BlockTemplate::setUnderWaterColor(const Color & val)
	{
		m_UnderWaterColor = val;
	}

	const Color & BlockTemplate::getUnderWaterColor()const
	{
		return m_UnderWaterColor;
	}

	int BlockTemplate::getTileSize() const
	{
		return m_nTileSize;
	}

	void BlockTemplate::setTileSize(int nTile)
	{
		if (m_nTileSize != nTile)
		{
			m_nTileSize = nTile;
			if (IsMatchAttribute(BlockTemplate::batt_pos_tiling))
			{
				SetAttribute(batt_pos_tiling, m_nTileSize > 1);
				SetAttribute(batt_random_tiling, false);
			}
			else if (IsMatchAttribute(BlockTemplate::batt_random_tiling))
			{
				SetAttribute(batt_pos_tiling, false);
				SetAttribute(batt_random_tiling, m_nTileSize > 1);
			}
			else
			{
				//any case should this happen?
				SetAttribute(batt_pos_tiling, false);
				SetAttribute(batt_random_tiling, false);
			}
		}
	}

}
