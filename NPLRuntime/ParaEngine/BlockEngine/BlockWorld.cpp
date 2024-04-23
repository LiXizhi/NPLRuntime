//-----------------------------------------------------------------------------
// Class:	Block World Provider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#ifdef USE_TINYXML2
#include <tinyxml2.h>
#else
#include <tinyxml.h>
#endif
#include "BlockModel.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockRegion.h"
#include "BlockLightGridBase.h"
#include "TextureEntity.h"
#include "BlockWorld.h"
#include "SceneObject.h"
#include "BipedObject.h"
#include "BlockMaterialManager.h"
using namespace ParaEngine;

/** default render distance in blocks */
#define DEFAULT_RENDER_BLOCK_DISTANCE	96

namespace ParaEngine
{
	float CBlockWorld::g_verticalOffset = 0;

	inline int64_t GetBlockSparseIndex(uint16_t bx, uint16_t by, uint16_t bz)
	{
		return (((uint64)bx) << 32) + (((uint64)by) << 16) + bz;
	}
}

CBlockWorld::CBlockWorld()
	:m_curChunkIdW(-1), m_activeChunkDim(0), m_lastChunkIdW(-1), m_lastChunkIdW_RegionCache(-1), m_lastViewCheckIdW(0), m_dwBlockRenderMethod(BLOCK_RENDER_FAST_SHADER), m_sunIntensity(1), m_isVisibleChunkDirty(true), m_curRegionIdX(0), m_curRegionIdZ(0),
	m_pLightGrid(new CBlockLightGridBase(this)), m_bReadOnlyWorld(false), m_bIsRemote(false), m_bIsServerWorld(false), m_bCubeModePicking(false), m_isInWorld(false), m_bSaveLightMap(false),
	m_bUseAsyncLoadWorld(true), m_bRenderBlocks(true), m_group_by_chunk_before_texture(false), m_is_linear_torch_brightness(false), m_maxCacheRegionCount(0),
	m_minWorldPos(0, 0, 0), m_maxWorldPos(0xffff, 0xffff, 0xffff), m_minRegionX(0), m_minRegionZ(0), m_maxRegionX(63), m_maxRegionZ(63)
{
	m_bAutoPhysics = true;

	// 256 blocks, so that it never wraps
	m_activeChunkDimY = 16;
	SetActiveChunkRadius(12);
	m_blockTemplatesArray.resize(256, 0);

	GenerateLightBrightnessTable(m_is_linear_torch_brightness);
	// resize region
	m_pRegions = new BlockRegionPtr[64 * 64];
	memset(m_pRegions, 0, sizeof(BlockRegionPtr) * 64 * 64);

	SetRenderDist(DEFAULT_RENDER_BLOCK_DISTANCE);

	m_selectedBlockMap.resize(10);
	for (unsigned int i = 0; i < m_selectedBlockMap.size(); ++i)
	{
		auto& select_group = m_selectedBlockMap[i];
		select_group.m_color = LinearColor(0.3f, 0.3f, 0.3f, 0.4f);

		if (i % 2 == 1)
		{
			select_group.m_fScaling = 1.01f;
			select_group.m_bEnableBling = true;
		}
		else
		{
			select_group.m_fScaling = 1.f;
		}
		if (i == 4 || i == 5)
		{
			select_group.m_bOnlyRenderClickableArea = true;
			select_group.m_fScaling = 0.99f;
			select_group.m_bEnableBling = false;
		}
		if (i == BLOCK_GROUP_ID_WIREFRAME)
		{
			select_group.m_bWireFrame = true;
			select_group.m_color = LinearColor(0.2f, 0.2f, 0.2f, 0.7f);
			select_group.m_fScaling = 1.01f;
		}
		else if (i == BLOCK_GROUP_ID_HIGHLIGHT)
		{
			select_group.m_bEnableBling = true;
			select_group.m_fScaling = 1.01f;
		}
	}

	RenderableChunk::StaticInit();
}

CBlockWorld::~CBlockWorld()
{
	m_isInWorld = false;
	SAFE_DELETE(m_pLightGrid);
	ClearAllBlockTemplates();
	SAFE_DELETE_ARRAY(m_pRegions);
}

void ParaEngine::CBlockWorld::EnterWorld(const string& sWorldDir, float x, float y, float z)
{
	if (m_isInWorld)
		return;

	if (!sWorldDir.empty())
		m_worldInfo.ResetWorldName(sWorldDir);
	else
		m_worldInfo.ResetWorldName(CWorldInfo::GetSingleton().GetDefaultWorldConfigName());

	//init data
	// only use more region cache on 64 bits system.
	m_maxCacheRegionCount = (sizeof(void*) > 4) ? 16 : 9;

	/** only cache 4 region for networked world*/
	if (IsRemote())
		m_maxCacheRegionCount = 4;

	m_minActiveChunkId_ws.SetValue(0);

	// LoadBlockTemplateData();
	LoadBlockMaterialData();

	OUTPUT_LOG("Enter Block World %s : max region cache %d\n", m_worldInfo.GetWorldName().c_str(), m_maxCacheRegionCount);
	GetLightGrid().OnEnterWorld();

	OnViewCenterMove(x, y, z);

	m_isInWorld = true;
}

void CBlockWorld::LoadBlockMaterialData()
{
	CParaFile* pFile = NULL;

	std::string tempXml = m_worldInfo.GetBlockMaterialFileName(true);
	CParaFile file(tempXml.c_str());

	std::string gameSaveXml = m_worldInfo.GetBlockMaterialFileName(false);
	CParaFile gameSaveFile(gameSaveXml.c_str());

	if (file.isEof())
	{
		if (!gameSaveFile.isEof())
			pFile = &gameSaveFile;
	}
	else
	{
		pFile = &file;
	}

	if (pFile == NULL)
		return;

	try
	{
#ifdef USE_TINYXML2
		using namespace tinyxml2;
		typedef tinyxml2::XMLDocument XMLDocumentType;
		typedef XMLDeclaration XMLDeclarationType;
		typedef XMLElement XMLElementType;
		typedef XMLNode XMLNodeType;
#define XML_SUCCESS_ENUM_TYPE XMLError::XML_SUCCESS
#else
		typedef TiXmlDocument XMLDocumentType;
		typedef TiXmlDeclaration XMLDeclarationType;
		typedef TiXmlElement XMLElementType;
		typedef TiXmlNode XMLNodeType;
#define XML_SUCCESS_ENUM_TYPE TIXML_SUCCESS
#endif

#ifdef USE_TINYXML2
		XMLDocumentType doc(true, COLLAPSE_WHITESPACE);
		doc.Parse(pFile->getBuffer(), (int)(pFile->getSize()));
#else
		XMLDocumentType doc;
		doc.Parse(pFile->getBuffer(), 0, TIXML_DEFAULT_ENCODING);
#endif
		XMLElementType* pRoot = doc.RootElement();
		auto mgr = CGlobals::GetBlockMaterialManager();
		if (pRoot)
		{
			for (XMLNodeType* pChild = pRoot->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
#ifdef USE_TINYXML2
				if (pChild->ToElement())
#else
				if (pChild->Type() == TiXmlNode::ELEMENT)
#endif
				{
					XMLElementType* pElement = pChild->ToElement();
					if (pElement)
					{
						int32_t materialID = -1;
						if (pElement->QueryIntAttribute("Id", &materialID) == XML_SUCCESS_ENUM_TYPE)
						{
							CBlockMaterial* pMaterial = mgr->CreateGetBlockMaterialByID(materialID);
							if (!pMaterial) continue;
							CParameterBlock* pParamBlock = pMaterial->GetParamBlock();
							const char* materialName = pElement->Attribute("MaterialName");
							if (materialName) pParamBlock->CreateGetParameter("MaterialName")->SetValueByString(materialName, CParameter::PARAMETER_TYPE::PARAM_STRING);
							const char* baseColor = pElement->Attribute("BaseColor");
							if (baseColor) pParamBlock->CreateGetParameter("BaseColor")->SetValueByString(baseColor, CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
							const char* metallic = pElement->Attribute("Metallic");
							if (metallic) pParamBlock->CreateGetParameter("Metallic")->SetValueByString(metallic, CParameter::PARAMETER_TYPE::PARAM_FLOAT);
							const char* specular = pElement->Attribute("Specular");
							if (specular) pParamBlock->CreateGetParameter("Specular")->SetValueByString(specular, CParameter::PARAMETER_TYPE::PARAM_FLOAT);
							const char* roughness = pElement->Attribute("Roughness");
							if (roughness) pParamBlock->CreateGetParameter("Roughness")->SetValueByString(roughness, CParameter::PARAMETER_TYPE::PARAM_FLOAT);
							const char* emissiveColor = pElement->Attribute("EmissiveColor");
							if (emissiveColor) pParamBlock->CreateGetParameter("EmissiveColor")->SetValueByString(emissiveColor, CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
							const char* emissive = pElement->Attribute("Emissive");
							if (emissive) pParamBlock->CreateGetParameter("Emissive")->SetValueByString(emissive, CParameter::PARAMETER_TYPE::PARAM_STRING);
							const char* opacity = pElement->Attribute("Opacity");
							if (opacity) pParamBlock->CreateGetParameter("Opacity")->SetValueByString(opacity, CParameter::PARAMETER_TYPE::PARAM_FLOAT);
							const char* normal = pElement->Attribute("Normal");
							if (normal) pParamBlock->CreateGetParameter("Normal")->SetValueByString(normal, CParameter::PARAMETER_TYPE::PARAM_STRING);
							const char* diffuse = pElement->Attribute("Diffuse");
							if (diffuse) pParamBlock->CreateGetParameter("Diffuse")->SetValueByString(diffuse, CParameter::PARAMETER_TYPE::PARAM_STRING);
							const char* materialUV = pElement->Attribute("MaterialUV");
							if (materialUV) pParamBlock->CreateGetParameter("MaterialUV")->SetValueByString(materialUV, CParameter::PARAMETER_TYPE::PARAM_VECTOR4);
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error parsing block material file \n");
	}
}

void CBlockWorld::SaveBlockMaterialData()
{
	auto mgr = CGlobals::GetBlockMaterialManager();
#ifdef USE_TINYXML2
	using namespace tinyxml2;
	typedef tinyxml2::XMLDocument XMLDocumentType;
	typedef XMLDeclaration XMLDeclarationType;
	typedef XMLElement XMLElementType;
#define NewXMLElementType doc.NewElement
#else
	typedef TiXmlDocument XMLDocumentType;
	typedef TiXmlDeclaration XMLDeclarationType;
	typedef TiXmlElement XMLElementType;
#define NewXMLElementType new XMLElementType
#endif

	XMLDocumentType doc;
#ifdef USE_TINYXML2
	XMLDeclarationType* decl = doc.NewDeclaration(nullptr);
#else
	XMLDeclarationType* decl = new TiXmlDeclaration("1.0", "", "");
#endif
	doc.LinkEndChild(decl);

	XMLElementType* root = NewXMLElementType("BlockMaterials");
	doc.LinkEndChild(root);
	for (auto it = mgr->begin(); it != mgr->end(); it++)
	{
		CBlockMaterial* pMaterial = mgr->GetEntity(it->second->GetKey());
		CParameterBlock* pParamBlock = pMaterial->GetParamBlock();
		XMLElementType* pNewItem = NewXMLElementType("Material");
		pNewItem->SetAttribute("Id", pMaterial->GetKey());
		auto materialName = pParamBlock->GetParameter("MaterialName"); // vector4
		if (materialName) pNewItem->SetAttribute("MaterialName", materialName->GetValueByString());
		auto baseColor = pParamBlock->GetParameter("BaseColor"); // vector4
		if (baseColor) pNewItem->SetAttribute("BaseColor", baseColor->GetValueByString());
		auto metallic = pParamBlock->GetParameter("Metallic");   // float
		if (metallic) pNewItem->SetAttribute("Metallic", metallic->GetValueByString());
		auto specular = pParamBlock->GetParameter("Specular");   // float
		if (specular) pNewItem->SetAttribute("Specular", specular->GetValueByString());
		auto roughness = pParamBlock->GetParameter("Roughness");   // float
		if (roughness) pNewItem->SetAttribute("Roughness", roughness->GetValueByString());
		auto emissiveColor = pParamBlock->GetParameter("EmissiveColor"); // vector4
		if (emissiveColor) pNewItem->SetAttribute("EmissiveColor", emissiveColor->GetValueByString());
		auto emissive = pParamBlock->GetParameter("Emissive");   // texture
		if (emissive) pNewItem->SetAttribute("Emissive", emissive->GetValueByString());
		auto opacity = pParamBlock->GetParameter("Opacity");   // float
		if (opacity) pNewItem->SetAttribute("Opacity", opacity->GetValueByString());
		auto normal = pParamBlock->GetParameter("Normal");   // texture
		if (normal) pNewItem->SetAttribute("Normal", normal->GetValueByString());
		auto diffuse = pParamBlock->GetParameter("Diffuse");   // texture
		if (diffuse) pNewItem->SetAttribute("Diffuse", diffuse->GetValueByString());
		auto materialUV = pParamBlock->GetParameter("MaterialUV"); // vector4
		if (materialUV) pNewItem->SetAttribute("MaterialUV", materialUV->GetValueByString());
		root->LinkEndChild(pNewItem);
	}
	std::string fileName = m_worldInfo.GetBlockMaterialFileName(true);
	std::string sText;
	sText << doc;
	CParaFile file;
	if (file.CreateNewFile(fileName.c_str(), true))
		file.write(sText.c_str(), sText.size());
}

void CBlockWorld::SaveToFile(bool saveToTemp)
{
	// SaveBlockTemplateData();
	SaveBlockMaterialData();
	for (std::map<int, BlockRegion*>::iterator iter = m_regionCache.begin(); iter != m_regionCache.end(); iter++)
	{
		iter->second->SaveToFile();
	}
#ifdef PARAENGINE_CLIENT
	if (!saveToTemp)
	{
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATA ffd;

		std::string lastSaveDir = m_worldInfo.GetBlockGameSaveDir(true);
		lastSaveDir.append("*");
		hFind = FindFirstFile(lastSaveDir.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		std::string gameSaveDir = m_worldInfo.GetBlockGameSaveDir(false);
		lastSaveDir = m_worldInfo.GetBlockGameSaveDir(true);

		std::string current;
		std::string src;
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				ffd.cFileName;
				current.clear();
				current.assign(ffd.cFileName);

				int32_t size = current.size();
				if ((current[size - 3] == 'x' && current[size - 2] == 'm' && current[size - 1] == 'l')
					|| current[size - 3] == 'r' && current[size - 2] == 'a' && current[size - 1] == 'w')
				{
					current.clear();
					current.append(gameSaveDir);
					current.append(ffd.cFileName);

					src.clear();
					src.append(lastSaveDir);
					src.append(ffd.cFileName);
					CParaFile::CopyFile(src.c_str(), current.c_str(), true);
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);
	}
#endif
}

void ParaEngine::CBlockWorld::LeaveWorld()
{
	Scoped_WriteLock<BlockReadWriteLock> Lock_(GetReadWriteLock());

	m_curRegionIdX = 0;
	m_curRegionIdZ = 0;

	m_curChunkIdW.SetValue(-1);
	m_lastChunkIdW.SetValue(-1);
	m_lastChunkIdW_RegionCache.SetValue(-1);
	m_lastViewCheckIdW = Uint16x3(0);

	std::map<int, BlockRegion*>::iterator iter;
	while ((iter = m_regionCache.begin()) != m_regionCache.end())
	{
		BlockRegion* pRegion = iter->second;
		UnloadRegion(pRegion, false);
	}

	for (int i = 0; i < m_activeChunkDim; i++)
	{
		for (int j = 0; j < m_activeChunkDimY; j++)
		{
			for (int k = 0; k < m_activeChunkDim; k++)
			{
				GetActiveChunk(i, j, k).OnLeaveWorld();
			}
		}
	}

	m_visibleChunks.clear();

	ClearAllBlockTemplates();

	m_isInWorld = false;

	GetLightGrid().OnLeaveWorld();

	DeselectAllBlock(-BLOCK_GROUP_ID_MAX);

	CGlobals::GetBlockMaterialManager()->Cleanup();
}

bool ParaEngine::CBlockWorld::MatchTemplateAttribute(uint16_t templateId, BlockTemplate::BlockAttrubiteFlag flag)
{
	BlockTemplate* pTemplate = GetBlockTemplate(templateId);
	if (pTemplate)
	{
		return pTemplate->IsMatchAttribute(flag);
	}
	return false;
}

float ParaEngine::CBlockWorld::GetLightBrightnessFloat(uint8_t brightness)
{
	return m_lightBrightnessTableFloat[brightness];
}

uint8_t ParaEngine::CBlockWorld::GetLightBrightnessInt(uint8_t brightness)
{
	return m_lightBrightnessTableInt[brightness];
}

float ParaEngine::CBlockWorld::GetLightBrightnessLinearFloat(uint8_t brightness)
{
	return m_lightBrightnessLinearTableFloat[brightness];
}

float ParaEngine::CBlockWorld::GetSunIntensity()
{
	return m_sunIntensity;
}

void ParaEngine::CBlockWorld::UpdateAllActiveChunks()
{
	m_lastChunkIdW.SetValue(-1);
	m_lastChunkIdW_RegionCache.SetValue(-1);
	m_lastViewCheckIdW = Uint16x3(0);
	UpdateActiveChunk();
}

void ParaEngine::CBlockWorld::SetActiveChunkRadius(int nActiveChunkRadius)
{
	if (nActiveChunkRadius >= 3 && nActiveChunkRadius <= 64)
	{
		// m_activeChunkDim must be a odd value
		int nActiveChunkDim = 2 * (nActiveChunkRadius + 1) + 1;
		if (nActiveChunkDim != m_activeChunkDim)
		{
			if (nActiveChunkDim > m_activeChunkDim)
			{
				m_activeChunks.resize(nActiveChunkDim * m_activeChunkDimY * nActiveChunkDim, NULL);
				for (int i = 0; i < (int)(m_activeChunks.size()); ++i)
				{
					if (m_activeChunks[i] == NULL)
					{
						m_activeChunks[i] = new RenderableChunk();
					}
				}
				m_activeChunkDim = nActiveChunkDim;

				if (IsInBlockWorld())
					UpdateAllActiveChunks();
			}
		}
	}
	else
	{
		OUTPUT_LOG("warning: SetActiveChunkRadius %d is too big or too small\n", nActiveChunkRadius);
	}
}

void CBlockWorld::SetRenderDist(int nValue)
{
	SetActiveChunkRadius((int)((float)nValue / (float)BlockConfig::g_chunkBlockDim + 0.5f));
	m_nRenderDistance = min(nValue, m_activeChunkDim * BlockConfig::g_chunkBlockDim / 2);
	GetLightGrid().SetLightGridSize((int)(m_nRenderDistance * 2 / BlockConfig::g_chunkBlockDim) + 2);

	int nMinRegionCount = ((int)(m_nRenderDistance / 512) + 2) ^ 2;

	if (m_maxCacheRegionCount < nMinRegionCount)
		m_maxCacheRegionCount = nMinRegionCount;
}

int ParaEngine::CBlockWorld::GetRenderDist()
{
	return m_nRenderDistance;
}

const Uint16x3& ParaEngine::CBlockWorld::GetEyeBlockId()
{
	return m_curCamBlockId;
}


void ParaEngine::CBlockWorld::SetEyeBlockId(const Uint16x3& eyePos)
{
	m_curCamBlockId = eyePos;
	m_curCamChunkId.x = m_curCamBlockId.x / 16;
	m_curCamChunkId.y = m_curCamBlockId.y / 16;
	m_curCamChunkId.z = m_curCamBlockId.z / 16;
}

const Uint16x3& ParaEngine::CBlockWorld::GetEyeChunkId()
{
	return m_curCamChunkId;
}

const Int16x3 ParaEngine::CBlockWorld::GetMinActiveChunkId()
{
	return m_minActiveChunkId_ws;
}

void ParaEngine::CBlockWorld::GetCurrentCenterChunkId(Int16x3& oResult)
{
	oResult = m_curChunkIdW;
}

CBlockLightGridBase& ParaEngine::CBlockWorld::GetLightGrid()
{
	return *m_pLightGrid;
}

void ParaEngine::CBlockWorld::SetBlockRenderMethod(BlockRenderMethod method)
{
}


BlockRegion* ParaEngine::CBlockWorld::CreateGetRegion(uint16_t x, uint16_t y, uint16_t z)
{
	if (!m_isInWorld || y >= BlockConfig::g_regionBlockDimY)
		return NULL;

	uint16_t region_x = x >> 9;
	uint16_t region_z = z >> 9;
	return CreateGetRegion(region_x, region_z);
}

BlockRegion* CBlockWorld::CreateGetRegion(uint16_t region_x, uint16_t region_z)
{
	BlockRegion* pRegion = GetRegion(region_x, region_z);
	if (pRegion)
	{
		return pRegion;
	}
	else if (region_x <= m_maxRegionX && region_z <= m_maxRegionZ && region_x >= m_minRegionX && region_z >= m_minRegionZ)
	{
		pRegion = new BlockRegion(region_x, region_z, this);
		if (pRegion)
		{
			char name[128];
			snprintf(name, 128, "region_%d_%d", pRegion->GetRegionX(), pRegion->GetRegionZ());
			pRegion->SetIdentifier(name);
			OUTPUT_LOG("Load region: %d %d, total regions: %d\n", pRegion->GetRegionX(), pRegion->GetRegionZ(), (int)(m_regionCache.size()));
			{
				Scoped_WriteLock<BlockReadWriteLock> Lock_(GetReadWriteLock());
				m_pRegions[pRegion->GetPackedRegionIndex()] = pRegion;
				m_regionCache[pRegion->GetPackedRegionIndex()] = pRegion;
			}
			pRegion->Load();
		}
		return pRegion;
	}
	else
		return NULL;
}

void ParaEngine::CBlockWorld::ResetAllLight()
{
	for (auto& item : m_regionCache)
	{
		item.second->ClearAllLight();
	}

	auto& lightGrid = GetLightGrid();

	lightGrid.OnEnterWorld();
}

bool ParaEngine::CBlockWorld::UnloadRegion(uint16_t block_x, uint16_t block_y, uint16_t block_z, bool bAutoSave /*= true*/)
{
	uint16_t rx, ry, rz;
	BlockRegion* pRegion = GetRegion(block_x, block_y, block_z, rx, ry, rz);
	if (pRegion) {
		UnloadRegion(pRegion, bAutoSave);
		return true;
	}
	return false;
}

void CBlockWorld::UnloadRegion(BlockRegion* pRegion, bool bAutoSave)
{
	if (pRegion)
	{
		if (m_pRegions[pRegion->GetPackedRegionIndex()] == pRegion)
		{
			if (!IsRemote() && bAutoSave && !IsReadOnly() && pRegion->IsModified())
				pRegion->SaveToFile();
			OnUnLoadBlockRegion(pRegion->GetRegionX(), pRegion->GetRegionZ());
			Scoped_WriteLock<BlockReadWriteLock> Lock_(GetReadWriteLock());
			m_pRegions[pRegion->GetPackedRegionIndex()] = NULL;
			m_regionCache.erase(pRegion->GetPackedRegionIndex());
			pRegion->OnUnloadWorld();
		}
		else
		{
			OUTPUT_LOG("error: invalid UnloadRegion\n");
		}
		delete pRegion;
	}
}

BlockRegion* CBlockWorld::GetRegion(uint16_t x, uint16_t y, uint16_t z, uint16_t& rs_x, uint16_t& rs_y, uint16_t& rs_z)
{
	if (m_isInWorld && y < BlockConfig::g_regionBlockDimY)
	{
		uint16_t region_x = x >> 9;
		uint16_t region_z = z >> 9;

		if (region_x < 64 && region_z < 64)
		{
			BlockRegion* pRegion = m_pRegions[region_x + (region_z << 6)];
			if (pRegion)
			{
				rs_x = x & 0x1ff;
				rs_y = y & 0xff;
				rs_z = z & 0x1ff;
				return pRegion;
			}
		}
	}
	return NULL;
}

BlockRegion* CBlockWorld::GetRegion(uint16_t region_x, uint16_t region_z)
{
	if (region_x < 64 && region_z < 64)
		return m_pRegions[region_x + (region_z << 6)];
	else
		return NULL;
}

float CBlockWorld::GetVerticalOffset()
{
	return g_verticalOffset;
}

void CBlockWorld::SetVerticalOffset(float offset)
{
	g_verticalOffset = offset;
}

int32_t CBlockWorld::GetActiveChunkDim()
{
	return m_activeChunkDim;
}

int32_t CBlockWorld::GetActiveChunkDimY()
{
	return m_activeChunkDimY;
}


bool CBlockWorld::IsInBlockWorld()
{
	return m_isInWorld;
}

void CBlockWorld::SetReadOnly(bool bValue)
{
	m_bReadOnlyWorld = bValue;
}

bool CBlockWorld::IsReadOnly()
{
	return m_bReadOnlyWorld;
}


void ParaEngine::CBlockWorld::ClearAllBlockTemplates()
{
	for (auto& it : m_blockTemplates)
	{
		if (it.first < 256) {
			m_blockTemplatesArray[it.first] = NULL;
		}
		SAFE_DELETE(it.second);
	}
	m_blockTemplates.clear();
}


void CBlockWorld::SaveBlockTemplateData()
{
#ifdef USE_TINYXML2
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;

	XMLDeclaration* decl = doc.NewDeclaration(nullptr);
	doc.LinkEndChild(decl);

	XMLElement* root = doc.NewElement("BlockTemplates");
	doc.LinkEndChild(root);
	for (std::map<uint16_t, BlockTemplate*>::iterator it = m_blockTemplates.begin(); it != m_blockTemplates.end(); it++)
	{
		BlockTemplate* pTemplate = it->second;

		XMLElement* pNewItem = doc.NewElement("Template");
		pNewItem->SetAttribute("Id", pTemplate->GetID());
		pNewItem->SetAttribute("Att", pTemplate->GetAttFlag());
		pNewItem->SetAttribute("Priority", pTemplate->GetRenderPriority());
		TextureEntity* pTex = pTemplate->GetTexture0();
		if (pTex)
		{
			pNewItem->SetAttribute("Tex0", pTex->GetLocalFileName().c_str());

			pTex = pTemplate->GetTexture1();
			if (pTex)
			{
				pNewItem->SetAttribute("Tex1", pTex->GetLocalFileName().c_str());
			}

			pTex = pTemplate->GetNormalMap();
			if (pTex)
			{
				pNewItem->SetAttribute("NorMap", pTex->GetLocalFileName().c_str());
			}
		}
		root->LinkEndChild(pNewItem);
	}
	std::string fileName = m_worldInfo.GetBlockTemplateFileName(true);
	std::string sText;
	sText << doc;
	CParaFile file;
	if (file.CreateNewFile(fileName.c_str(), true))
		file.write(sText.c_str(), sText.size());
#else
	TiXmlDocument doc;

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	TiXmlElement* root = new TiXmlElement("BlockTemplates");
	doc.LinkEndChild(root);
	for (std::map<uint16_t, BlockTemplate*>::iterator it = m_blockTemplates.begin(); it != m_blockTemplates.end(); it++)
	{
		BlockTemplate* pTemplate = it->second;

		TiXmlElement* pNewItem = new TiXmlElement("Template");
		pNewItem->SetAttribute("Id", pTemplate->GetID());
		pNewItem->SetAttribute("Att", pTemplate->GetAttFlag());
		pNewItem->SetAttribute("Priority", pTemplate->GetRenderPriority());
		TextureEntity* pTex = pTemplate->GetTexture0();
		if (pTex)
		{
			pNewItem->SetAttribute("Tex0", pTex->GetLocalFileName());

			pTex = pTemplate->GetTexture1();
			if (pTex)
			{
				pNewItem->SetAttribute("Tex1", pTex->GetLocalFileName());
			}

			pTex = pTemplate->GetNormalMap();
			if (pTex)
			{
				pNewItem->SetAttribute("NorMap", pTex->GetLocalFileName());
			}
		}
		root->LinkEndChild(pNewItem);
	}
	std::string fileName = m_worldInfo.GetBlockTemplateFileName(true);
	std::string sText;
	sText << doc;
	CParaFile file;
	if (file.CreateNewFile(fileName.c_str(), true))
		file.write(sText.c_str(), sText.size());
#endif
}

void CBlockWorld::LoadBlockTemplateData()
{
	CParaFile* pFile = NULL;

	std::string tempXml = m_worldInfo.GetBlockTemplateFileName(true);
	CParaFile file(tempXml.c_str());

	std::string gameSaveXml = m_worldInfo.GetBlockTemplateFileName(false);
	CParaFile gameSaveFile(gameSaveXml.c_str());

	if (file.isEof())
	{
		if (!gameSaveFile.isEof())
			pFile = &gameSaveFile;
	}
	else
	{
		pFile = &file;
	}

	if (pFile == NULL)
		return;

	try
	{
#ifdef USE_TINYXML2
		using namespace tinyxml2;
		tinyxml2::XMLDocument doc(true, COLLAPSE_WHITESPACE);
		doc.Parse(pFile->getBuffer(), (int)(pFile->getSize()));
		XMLElement* pRoot = doc.RootElement();

		if (pRoot)
		{
			for (XMLNode* pChild = pRoot->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				if (pChild->ToElement())
				{
					XMLElement* pElement = pChild->ToElement();
					if (pElement)
					{
						int32_t templateID = 0;
						int32_t categoryID = 0;
						pElement->QueryIntAttribute("Category", &categoryID);

						if (pElement->QueryIntAttribute("Id", &templateID) == XMLError::XML_SUCCESS)
						{
							uint32_t templateAtt = 0;
							if (pElement->QueryIntAttribute("Att", (int32_t*)&templateAtt) == XMLError::XML_SUCCESS)
							{
								int32_t priority = 0;
								pElement->QueryIntAttribute("Priority", &priority);

								const char* tex0 = pElement->Attribute("Tex0");
								const char* tex1 = pElement->Attribute("Tex1");
								const char* normalMap = pElement->Attribute("NorMap");

								BlockTemplate* pTemplate = RegisterTemplate(templateID & 0xffff, templateAtt, categoryID);
								if (pTemplate)
								{
									pTemplate->SetRenderPriority(priority & 0xf);
									if (tex0)
										pTemplate->SetTexture0(tex0);
									if (tex1)
										pTemplate->SetTexture1(tex1);
									if (normalMap)
										pTemplate->SetNormalMap(normalMap);
								}
							}
						}
					}
				}
			}
		}
#else
		TiXmlDocument doc;
		doc.Parse(pFile->getBuffer(), 0, TIXML_DEFAULT_ENCODING);
		TiXmlElement* pRoot = doc.RootElement();

		if (pRoot)
		{
			for (TiXmlNode* pChild = pRoot->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				if (pChild->Type() == TiXmlNode::ELEMENT)
				{
					TiXmlElement* pElement = pChild->ToElement();
					if (pElement)
					{
						int32_t templateID = 0;
						int32_t categoryID = 0;
						pElement->QueryIntAttribute("Category", &categoryID);

						if (pElement->QueryIntAttribute("Id", &templateID) == TIXML_SUCCESS)
						{
							uint32_t templateAtt = 0;
							if (pElement->QueryIntAttribute("Att", (int32_t*)&templateAtt) == TIXML_SUCCESS)
							{
								int32_t priority = 0;
								pElement->QueryIntAttribute("Priority", &priority);

								int32_t temp;
								const char* tex0 = pElement->Attribute("Tex0", &temp);
								const char* tex1 = pElement->Attribute("Tex1", &temp);
								const char* normalMap = pElement->Attribute("NorMap", &temp);

								BlockTemplate* pTemplate = RegisterTemplate(templateID & 0xffff, templateAtt, categoryID);
								if (pTemplate)
								{
									pTemplate->SetRenderPriority(priority & 0xf);
									if (tex0)
										pTemplate->SetTexture0(tex0);
									if (tex1)
										pTemplate->SetTexture1(tex1);
									if (normalMap)
										pTemplate->SetNormalMap(normalMap);
								}
							}
						}
					}
				}
			}

		}
#endif
	}
	catch (...)
	{
		OUTPUT_LOG("error parsing block template file \n");
	}

}

RenderableChunk& CBlockWorld::GetActiveChunk(uint16_t curChunkWX, uint16_t curChunkWY, uint16_t curChunkWZ)
{
	curChunkWX = curChunkWX % m_activeChunkDim;
	curChunkWY = curChunkWY % m_activeChunkDimY;
	curChunkWZ = curChunkWZ % m_activeChunkDim;
	uint32_t index = curChunkWZ + curChunkWX * m_activeChunkDim + curChunkWY * m_activeChunkDim * m_activeChunkDim;
	return *(m_activeChunks[index]);
}


bool CBlockWorld::ReuseActiveChunk(int16_t curChunkWX, int16_t curChunkWY, int16_t curChunkWZ, BlockRegion* pRegion)
{
	if (curChunkWX >= 0 && curChunkWY >= 0 && curChunkWY < 16 && curChunkWZ >= 0 && pRegion)
	{
		int16_t localChunkX = curChunkWX & 0x1f;
		int16_t localChunkY = curChunkWY & 0xf;
		int16_t localChunkZ = curChunkWZ & 0x1f;

		uint16_t chunkIndex = PackChunkIndex(localChunkX, localChunkY, localChunkZ);
		GetActiveChunk(curChunkWX, curChunkWY, curChunkWZ).ReuseChunk(pRegion, chunkIndex);
		return true;
	}
	else
	{
		GetActiveChunk(curChunkWX, curChunkWY, curChunkWZ).ReuseChunk(NULL, -1);
		return false;
	}
}

BlockTemplate* CBlockWorld::GetBlockTemplate(uint16_t id)
{
	if (id < 256)
		return m_blockTemplatesArray[id];
	else
	{
		std::map<uint16_t, BlockTemplate*>::iterator it = m_blockTemplates.find(id);
		if (it == m_blockTemplates.end())
			return NULL;

		return (*it).second;
	}
}

BlockTemplate* CBlockWorld::RegisterTemplate(uint16_t id, uint32_t attFlag, uint16_t category_id)
{
	if (GetBlockTemplate(id))
		return NULL;

	BlockTemplate* newTemplate = new BlockTemplate(id, attFlag, category_id);
	m_blockTemplates.insert(std::pair<uint16_t, BlockTemplate*>(id, newTemplate));
	if (id < 256)
		m_blockTemplatesArray[id] = newTemplate;
	return newTemplate;
}

void CBlockWorld::SetBlockTemplateId(float x, float y, float z, uint16_t templateId)
{
	Uint16x3 blockIdx;
	BlockCommon::ConvertToBlockIndex(x, y, z, blockIdx.x, blockIdx.y, blockIdx.z);
	SetBlockId(blockIdx.x, blockIdx.y, blockIdx.z, templateId);
}

void CBlockWorld::SetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z, uint16_t templateId)
{
	SetBlockId(x, y, z, templateId);
}

uint16_t CBlockWorld::GetBlockTemplateId(float x, float y, float z)
{
	Uint16x3 blockIdx;
	BlockCommon::ConvertToBlockIndex(x, y, z, blockIdx.x, blockIdx.y, blockIdx.z);
	return GetBlockId(blockIdx.x, blockIdx.y, blockIdx.z);
}

uint16_t CBlockWorld::GetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z)
{
	return GetBlockId(x, y, z);
}

void CBlockWorld::SetBlockUserData(float x, float y, float z, uint32_t data)
{
	Uint16x3 blockIdx;
	BlockCommon::ConvertToBlockIndex(x, y, z, blockIdx.x, blockIdx.y, blockIdx.z);
	SetBlockData(blockIdx.x, blockIdx.y, blockIdx.z, data);
}

void CBlockWorld::SetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t data)
{
	SetBlockData(x, y, z, data);
}

uint32_t CBlockWorld::GetBlockUserData(float x, float y, float z)
{
	Uint16x3 blockIdx;
	BlockCommon::ConvertToBlockIndex(x, y, z, blockIdx.x, blockIdx.y, blockIdx.z);
	return GetBlockData(blockIdx.x, blockIdx.y, blockIdx.z);
}

uint32_t CBlockWorld::GetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z)
{
	return GetBlockData(x, y, z);
}

bool CBlockWorld::SetBlockVisible(uint16_t templateId, bool value, bool bRefreshWorld)
{
	BlockTemplate* pTemp = GetBlockTemplate(templateId);

	if (pTemp)
	{
		if (!value == pTemp->IsMatchAttribute(BlockTemplate::batt_invisible))
			return false;

		if (!value)
		{
			BlockTemplateVisibleData visibleData;
			visibleData.lightOpyValue = pTemp->GetLightOpacity();
			visibleData.isTransparent = pTemp->IsMatchAttribute(BlockTemplate::batt_transparent);
			visibleData.torchLight = pTemp->GetTorchLight();

			m_blockTemplateVisibleDatas[templateId] = visibleData;

			pTemp->SetAttribute(BlockTemplate::batt_transparent, true);
			pTemp->SetAttribute(BlockTemplate::batt_invisible, true);
			pTemp->SetLightOpacity(0);
			pTemp->SetTorchLight(0);
		}
		else
		{
			pTemp->SetAttribute(BlockTemplate::batt_invisible, false);
			auto visibleDataItr = m_blockTemplateVisibleDatas.find(templateId);
			if (visibleDataItr != m_blockTemplateVisibleDatas.end())
			{
				pTemp->SetAttribute(BlockTemplate::batt_transparent, visibleDataItr->second.isTransparent);
				pTemp->SetLightOpacity(visibleDataItr->second.lightOpyValue);
				pTemp->SetTorchLight(visibleDataItr->second.torchLight);

				m_blockTemplateVisibleDatas.erase(templateId);
			}
		}
		if (bRefreshWorld) {
			RefreshBlockTemplate(templateId);
			return false;
		}
		return true;
	}
	return false;
}

void ParaEngine::CBlockWorld::RefreshBlockTemplate(uint16_t templateId)
{
	for (auto& iter : m_regionCache)
	{
		iter.second->SetChunksDirtyByBlockTemplate(templateId);
	}
}

uint32_t ParaEngine::CBlockWorld::SetBlockId(uint16_t x, uint16_t y, uint16_t z, uint32_t nBlockID)
{
	if (y >= 256)
		return 0;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		BlockTemplate* pTemplate = (nBlockID > 0) ? GetBlockTemplate(nBlockID) : NULL;
		pRegion->SetBlockTemplateByIndex(lx, ly, lz, pTemplate);
		m_isVisibleChunkDirty = true;
		return 0;
	}
	return 0;
}

uint32_t ParaEngine::CBlockWorld::GetBlockId(uint16_t x, uint16_t y, uint16_t z)
{
	if (y >= 256)
		return 0;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		return pRegion->GetBlockTemplateIdByIndex(lx, ly, lz);
	}
	return 0;
}

uint32_t ParaEngine::CBlockWorld::SetBlockData(uint16_t x, uint16_t y, uint16_t z, uint32_t nBlockData)
{
	if (y >= 256)
		return 0;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		pRegion->SetBlockUserDataByIndex(lx, ly, lz, nBlockData);
		m_isVisibleChunkDirty = true;
		return 0;
	}
	return 0;
}

uint32_t ParaEngine::CBlockWorld::GetBlockData(uint16_t x, uint16_t y, uint16_t z)
{
	if (y >= 256)
		return 0;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		return pRegion->GetBlockUserDataByIndex(lx, ly, lz);
	}
	return 0;
}

bool ParaEngine::CBlockWorld::SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
{
	if (y >= 256)
		return false;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		pRegion->SetBlockMaterial(lx, ly, lz, nFaceId, nMaterial);
		m_isVisibleChunkDirty = true;
		return true;
	}
	return false;
}

int32_t ParaEngine::CBlockWorld::GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId)
{
	if (y >= 256)
		return -1;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		return pRegion->GetBlockMaterial(lx, ly, lz, nFaceId);
	}
	return -1;
}

void ParaEngine::CBlockWorld::LoadBlockAsync(uint16_t x, uint16_t y, uint16_t z, uint16_t blockId, uint32_t userData)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		pRegion->LoadBlockAsync(lx, ly, lz, blockId, userData);
	}
}

Block* CBlockWorld::GetBlock(uint16_t x_ws, uint16_t y_ws, uint16_t z_ws)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x_ws, y_ws, z_ws, lx, ly, lz);

	if (pRegion)
	{
		return pRegion->GetBlock(lx, ly, lz);
	}
	return NULL;
}


Block* CBlockWorld::GetUnlockBlock(uint16_t x, uint16_t y, uint16_t z)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion && !pRegion->IsLocked())
	{
		return pRegion->GetBlock(lx, ly, lz);
	}
	return nullptr;
}

BlockTemplate* CBlockWorld::GetBlockTemplate(uint16_t x, uint16_t y, uint16_t z)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		return pRegion->GetBlockTemplateByIndex(lx, ly, lz);
	}
	return NULL;
}

BlockTemplate* CBlockWorld::GetBlockTemplate(Uint16x3& blockId)
{
	return GetBlockTemplate(blockId.x, blockId.y, blockId.z);
}

void CBlockWorld::SetChunkDirty(Uint16x3& chunkId_ws, bool isDirty)
{
	int16_t regionX = chunkId_ws.x >> 5;
	int16_t regionZ = chunkId_ws.z >> 5;

	BlockRegion* pRegion = GetRegion(regionX, regionZ);
	if (pRegion)
	{
		int16_t chunkX_rs = chunkId_ws.x & 0x1f;
		int16_t chunkZ_rs = chunkId_ws.z & 0x1f;

		uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, chunkId_ws.y, chunkZ_rs);
		pRegion->SetChunkDirty(packedChunkId_rs, isDirty);
	}
}

void ParaEngine::CBlockWorld::SetChunkLightDirty(Uint16x3& chunkId_ws)
{
	int16_t regionX = chunkId_ws.x >> 5;
	int16_t regionZ = chunkId_ws.z >> 5;

	BlockRegion* pRegion = GetRegion(regionX, regionZ);
	if (pRegion)
	{
		int16_t chunkX_rs = chunkId_ws.x & 0x1f;
		int16_t chunkZ_rs = chunkId_ws.z & 0x1f;

		uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, chunkId_ws.y, chunkZ_rs);
		pRegion->SetChunkLightDirty(packedChunkId_rs);
	}
}


ChunkMaxHeight* CBlockWorld::GetHighestBlock(uint16_t blockX_ws, uint16_t blockZ_ws)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(blockX_ws, 0, blockZ_ws, lx, ly, lz);
	if (pRegion)
	{
		return pRegion->GetHighestBlock(lx, lz);
	}
	return NULL;
}

void CBlockWorld::GetMaxBlockHeightWatchingSky(uint16_t blockX_ws, uint16_t blockZ_ws, ChunkMaxHeight* pResult)
{
	ChunkMaxHeight* pBlockHeight = NULL;
	pBlockHeight = GetHighestBlock(blockX_ws, blockZ_ws);
	if (pBlockHeight)
		pResult[0] = *pBlockHeight;

	pBlockHeight = GetHighestBlock(blockX_ws + 1, blockZ_ws);
	if (pBlockHeight)
		pResult[1] = *pBlockHeight;

	pBlockHeight = GetHighestBlock(blockX_ws - 1, blockZ_ws);
	if (pBlockHeight)
		pResult[2] = *pBlockHeight;

	pBlockHeight = GetHighestBlock(blockX_ws, blockZ_ws + 1);
	if (pBlockHeight)
		pResult[3] = *pBlockHeight;

	pBlockHeight = GetHighestBlock(blockX_ws, blockZ_ws - 1);
	if (pBlockHeight)
		pResult[4] = *pBlockHeight;
}

bool ParaEngine::CBlockWorld::IsChunkColumnInActiveRange(int16_t curChunkWX, int16_t curChunkWZ)
{
	return (curChunkWX >= m_minActiveChunkId_ws.x && curChunkWX < (m_minActiveChunkId_ws.x + m_activeChunkDim) &&
		curChunkWZ >= m_minActiveChunkId_ws.z && curChunkWZ < (m_minActiveChunkId_ws.z + m_activeChunkDim));
}

bool CBlockWorld::RefreshChunkColumn(int16_t curChunkWX, int16_t curChunkWZ)
{
	if (IsChunkColumnInActiveRange(curChunkWX, curChunkWZ))
	{
		int16_t regionX = curChunkWX >> 5;
		int16_t regionZ = curChunkWZ >> 5;
		BlockRegion* pRegion = GetRegion(regionX, regionZ);

		for (int16_t curChunkWY = 0; curChunkWY < 16; curChunkWY++)
		{
			ReuseActiveChunk(curChunkWX, curChunkWY, curChunkWZ, pRegion);
		}
		GetLightGrid().AddDirtyColumn(curChunkWX, curChunkWZ);
		m_isVisibleChunkDirty = true;
		return true;
	}
	return false;
}

void CBlockWorld::UpdateActiveChunk()
{
	Int16x3 curMinActiveChunkId(m_minActiveChunkId_ws);

	Int16x3 deltaOfs;
	Int16x3::Subtract(m_curChunkIdW, m_lastChunkIdW, deltaOfs);
	m_minActiveChunkId_ws.x = m_curChunkIdW.x - (int)((m_activeChunkDim) / 2);
	m_minActiveChunkId_ws.y = m_curChunkIdW.y - (int)((m_activeChunkDimY) / 2);
	m_minActiveChunkId_ws.z = m_curChunkIdW.z - (int)((m_activeChunkDim) / 2);


	// tricky: this will be set to fixed value, since we want to cache for all chunks vertically. 
	m_minActiveChunkId_ws.y = 0;
	curMinActiveChunkId.y = 0;
	deltaOfs.y = 0;

	if (deltaOfs.x == 0 && deltaOfs.y == 0 && deltaOfs.z == 0)
		return;


	Int16x3 deltaAbs(deltaOfs);
	deltaAbs.Abs();

	if (m_lastChunkIdW.x < 0 || deltaAbs.x >= m_activeChunkDim ||
		deltaAbs.y >= m_activeChunkDimY ||
		deltaAbs.z >= m_activeChunkDim)
	{
		for (int y = 0; y < m_activeChunkDimY; y++)
		{
			int16_t curChunkWY = m_minActiveChunkId_ws.y + y;

			for (int z = 0; z < m_activeChunkDim; z++)
			{
				int16_t curChunkWZ = m_minActiveChunkId_ws.z + z;
				for (int x = 0; x < m_activeChunkDim; x++)
				{
					int16_t curChunkWX = m_minActiveChunkId_ws.x + x;
					int16_t regionX = curChunkWX >> 5;
					int16_t regionZ = curChunkWZ >> 5;
					// check load regions
					BlockRegion* pRegion = CreateGetRegion(regionX, regionZ);

					if (pRegion)
					{
						ReuseActiveChunk(curChunkWX, curChunkWY, curChunkWZ, pRegion);
					}
					else
					{
						// OUTPUT_DEBUG("warning: no region at visible chunk\r\n");
					}
				}
			}
		}
	}
	else
	{
		Int16x3 startChunkId(m_minActiveChunkId_ws);

		if (deltaOfs.x >= 0)
			startChunkId.x = startChunkId.x + m_activeChunkDim - deltaOfs.x;

		if (deltaOfs.y >= 0)
			startChunkId.y = startChunkId.y + m_activeChunkDimY - deltaOfs.y;

		if (deltaOfs.z >= 0)
			startChunkId.z = startChunkId.z + m_activeChunkDim - deltaOfs.z;

		//update 3D ring array, try to simplify the update logic here 
		// x direction
		for (int16_t x = 0; x < deltaAbs.x; x++)
		{
			int16_t curChunkWX = startChunkId.x + x;

			for (int16_t z = 0; z < m_activeChunkDim; z++)
			{
				int16_t curChunkWZ = curMinActiveChunkId.z + z;

				int16_t regionX = curChunkWX >> 5;
				int16_t regionZ = curChunkWZ >> 5;
				BlockRegion* pRegion = CreateGetRegion(regionX, regionZ);

				for (int16_t y = 0; y < m_activeChunkDimY; y++)
				{
					int16_t curChunkWY = curMinActiveChunkId.y + y;

					ReuseActiveChunk(curChunkWX, curChunkWY, curChunkWZ, pRegion);
					if (!pRegion)
					{
						// OUTPUT_DEBUG("warning: no region at visible chunk\r\n");
					}
				}
			}
		}

		curMinActiveChunkId.x = m_minActiveChunkId_ws.x;

		// y direction
		for (int16_t y = 0; y < deltaAbs.y; y++)
		{
			int16_t curChunkWY = startChunkId.y + y;

			for (int z = 0; z < m_activeChunkDim; z++)
			{
				int16_t curChunkWZ = curMinActiveChunkId.z + z;

				for (int x = 0; x < m_activeChunkDim; x++)
				{
					int16_t curChunkWX = curMinActiveChunkId.x + x;

					int16_t regionX = curChunkWX >> 5;
					int16_t regionZ = curChunkWZ >> 5;
					BlockRegion* pRegion = CreateGetRegion(regionX, regionZ);
					ReuseActiveChunk(curChunkWX, curChunkWY, curChunkWZ, pRegion);
					if (!pRegion)
					{
						// OUTPUT_DEBUG("warning: no region at visible chunk\r\n");
					}
				}
			}
		}

		curMinActiveChunkId.y = m_minActiveChunkId_ws.y;

		// z direction
		for (int16_t z = 0; z < deltaAbs.z; z++)
		{
			int16_t curChunkWZ = startChunkId.z + z;

			for (int16_t x = 0; x < m_activeChunkDim; x++)
			{
				int16_t curChunkWX = curMinActiveChunkId.x + x;

				int16_t regionX = curChunkWX >> 5;
				int16_t regionZ = curChunkWZ >> 5;
				BlockRegion* pRegion = CreateGetRegion(regionX, regionZ);

				for (int16_t y = 0; y < m_activeChunkDimY; y++)
				{
					int16_t curChunkWY = curMinActiveChunkId.y + y;
					ReuseActiveChunk(curChunkWX, curChunkWY, curChunkWZ, pRegion);
					if (!pRegion)
					{
						// OUTPUT_DEBUG("warning: no region at visible chunk\r\n");
					}
				}
			}
		}
	}
}

void CBlockWorld::RefreshAllLightsInColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws)
{
	uint16_t regionX = chunkX_ws / BlockConfig::g_regionChunkDimX;
	uint16_t regionZ = chunkZ_ws / BlockConfig::g_regionChunkDimZ;

	BlockRegion* pRegion = GetRegion(regionX, regionZ);
	if (pRegion)
	{
		pRegion->RefreshAllLightsInColumn(chunkX_ws, chunkZ_ws);
	}
}

void CBlockWorld::NotifyBlockHeightMapChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight)
{
	GetLightGrid().NotifyBlockHeightChanged(blockIdX_ws, blockIdZ_ws, prevBlockHeight);
}


bool CBlockWorld::GetBlockBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nSize, int nLightType)
{
	return GetLightGrid().GetBrightness(blockId_ws, brightness, nSize, nLightType);
}

bool CBlockWorld::GetBlockMeshBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nLightType)
{
	BlockTemplate* pBlock = GetBlockTemplate(blockId_ws);
	if (pBlock && !pBlock->IsTransparent())
	{
		// if the block is solid, we will return the max brightness of the block's 6 neighbors. 
		uint8_t brightnesses[14];
		int nSize = 7;
		if (GetBlockBrightness(blockId_ws, brightnesses, nSize, nLightType))
		{
			uint8_t max_value = brightnesses[1];
			for (int i = 2; i < nSize; i++)
			{
				if (max_value < brightnesses[i])
					max_value = brightnesses[i];
			}
			brightness[0] = max_value;
			if (nLightType == 2)
			{
				max_value = brightnesses[1 + nSize];
				for (int i = 2; i < nSize; i++)
				{
					if (max_value < brightnesses[i + nSize])
						max_value = brightnesses[i + nSize];
				}
				brightness[1] = max_value;
			}
			return true;
		}
		else
			return false;
	}
	else
	{
		// if the block in Not solid, we will simply return the block's brightness. 
		return (GetBlockBrightness(blockId_ws, brightness, 1, nLightType));
	}
}


float CBlockWorld::GetBlockBrightnessReal(Uint16x3& blockId_ws, float* pBrightness)
{
	uint8_t brightness = 0;
	GetBlockMeshBrightness(blockId_ws, &brightness, -1);
	float fBrightness = GetLightBrightnessFloat(brightness);
	if (pBrightness != NULL)
		pBrightness[0] = fBrightness;
	return fBrightness;
}

float CBlockWorld::GetBlockBrightnessReal(const Vector3& vPos, float* pBrightness)
{
	Uint16x3 blockId_ws(0, 0, 0);
	BlockCommon::ConvertToBlockIndex(vPos.x, vPos.y, vPos.z, blockId_ws.x, blockId_ws.y, blockId_ws.z);
	return GetBlockBrightnessReal(blockId_ws, pBrightness);
}

void CBlockWorld::SetLightBlockDirty(Uint16x3& blockId_ws, bool isSunLight)
{
	GetLightGrid().SetLightDirty(blockId_ws, isSunLight, 1);
}

int32_t CBlockWorld::GetBlocksInRegion(Uint16x3& startChunk_ws, Uint16x3& endChunk_ws, uint32_t matchType, const luabind::adl::object& result, uint32_t verticalSectionFilter)
{
	int32_t blockCount = 0;
	for (uint16_t x = startChunk_ws.x; x <= endChunk_ws.x; x++)
	{
		uint16_t regionX = x >> 5;
		for (uint16_t z = startChunk_ws.z; z <= endChunk_ws.z; z++)
		{
			uint16_t regionZ = z >> 5;

			BlockRegion* pRegion = GetRegion(regionX, regionZ);
			if (pRegion)
			{
				if (verticalSectionFilter == 0)
					pRegion->GetBlocksInChunk(x, z, startChunk_ws.y, endChunk_ws.y, matchType, result, blockCount);
				else
					pRegion->GetBlocksInChunk(x, z, verticalSectionFilter, matchType, result, blockCount);
			}
		}
	}
	return blockCount;
}


void CBlockWorld::SetCubeModePicking(bool bIsCubeModePicking)
{
	m_bCubeModePicking = bIsCubeModePicking;
}

bool CBlockWorld::IsCubeModePicking()
{
	return m_bCubeModePicking;
}

bool CBlockWorld::Pick(const Vector3& rayOrig, const Vector3& dir, float length, PickResult& result, uint32_t filter)
{
	if (!m_isInWorld)
		return false;
	//////////////////////////////////////////////////////////////
	//
	// use 3D DDA algorithm to find hit block more detail see 
	// http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_4_Spatial_Subdivisions.shtml
	//
	//////////////////////////////////////////////////////////////

	Uint16x3 tempBlockId;
	BlockCommon::ConvertToBlockIndex(rayOrig.x, rayOrig.y, rayOrig.z, tempBlockId.x, tempBlockId.y, tempBlockId.z);
	int32_t startBlockIdX = tempBlockId.x;
	int32_t startBlockIdY = tempBlockId.y;
	int32_t startBlockIdZ = tempBlockId.z;

	int32_t curBlockIdX = startBlockIdX;
	int32_t curBlockIdY = startBlockIdY;
	int32_t curBlockIdZ = startBlockIdZ;

	//ray tracing direction
	int32_t blockStepX;
	int32_t blockStepY;
	int32_t blockStepZ;

	//setup 3d dda init value
	Vector3 nextBlockPos;
	if (dir.x > 0)
	{
		blockStepX = 1;
		nextBlockPos.x = (curBlockIdX + 1) * BlockConfig::g_blockSize;
	}
	else
	{
		blockStepX = -1;
		nextBlockPos.x = curBlockIdX * BlockConfig::g_blockSize;
	}

	if (dir.y > 0)
	{
		blockStepY = 1;
		nextBlockPos.y = (curBlockIdY + 1) * BlockConfig::g_blockSize;
	}
	else
	{
		blockStepY = -1;
		nextBlockPos.y = curBlockIdY * BlockConfig::g_blockSize;
	}

	if (dir.z > 0)
	{
		blockStepZ = 1;
		nextBlockPos.z = (curBlockIdZ + 1) * BlockConfig::g_blockSize;
	}
	else
	{
		blockStepZ = -1;
		nextBlockPos.z = curBlockIdZ * BlockConfig::g_blockSize;
	}

	// distance we can travel along the ray before hitting a block boundary, in either of the three axis.
	Vector3 errDist;
	// the delta distance to travel in the three axis, before we move to next block. This is a constant;
	Vector3 delta;

	float maxRayDist = 100000;
	if (dir.x != 0)
	{
		float invX = 1.0f / dir.x;
		errDist.x = (nextBlockPos.x - rayOrig.x) * invX;
		delta.x = BlockConfig::g_blockSize * blockStepX * invX;
	}
	else
		errDist.x = maxRayDist;

	if (dir.y != 0)
	{
		float invY = 1.0f / dir.y;
		errDist.y = (nextBlockPos.y - rayOrig.y + GetVerticalOffset()) * invY;
		delta.y = BlockConfig::g_blockSize * blockStepY * invY;
	}
	else
		errDist.y = maxRayDist;

	if (dir.z != 0)
	{
		float invZ = 1.0f / dir.z;
		errDist.z = (nextBlockPos.z - rayOrig.z) * invZ;
		delta.z = BlockConfig::g_blockSize * blockStepZ * invZ;
	}
	else
		errDist.z = maxRayDist;

	int16_t curRegionX = -1;
	int16_t curRegionZ = -1;
	BlockRegion* curRegion = NULL;
	int32_t side;
	while (true)
	{
		//find the smallest value of traveledDist and going alone that direction
		float distTraveled = 0;
		if (errDist.x < errDist.y)
		{
			if (errDist.x < errDist.z)
			{
				distTraveled = errDist.x;
				curBlockIdX += blockStepX;
				errDist.x += delta.x;
				side = 0;
			}
			else
			{
				distTraveled = errDist.z;
				curBlockIdZ += blockStepZ;
				errDist.z += delta.z;
				side = 2;
			}
		}
		else
		{
			if (errDist.y < errDist.z)
			{
				distTraveled = errDist.y;
				curBlockIdY += blockStepY;
				errDist.y += delta.y;
				side = 4;
			}
			else
			{
				distTraveled = errDist.z;
				curBlockIdZ += blockStepZ;
				errDist.z += delta.z;
				side = 2;
			}
		}

		uint16_t regionX = curBlockIdX >> 9;
		uint16_t regionZ = curBlockIdZ >> 9;
		if (regionX != curRegionX || regionZ != curRegionZ)
		{
			curRegionX = regionX;
			curRegionZ = regionZ;
			curRegion = GetRegion(curRegionX, curRegionZ);
		}

		if (curRegion == NULL || curBlockIdX < 0 || curBlockIdY < 0 || curBlockIdZ < 0)
			return false;

		Block* pBlock = curRegion->GetBlock(curBlockIdX & 0x1ff, curBlockIdY & 0xff, curBlockIdZ & 0x1ff);
		BlockTemplate* pBlockTemplate = NULL;
		if (pBlock != 0 && (pBlockTemplate = pBlock->GetTemplate()) != 0 && ((pBlockTemplate->GetAttFlag() & filter) > 0))
		{
			const double blockSize = BlockConfig::g_blockSize;
			float rayLength = -1;

			if (side == 0 && blockStepX <= 0)
				side = 1;
			if (side == 2 && blockStepZ <= 0)
				side = 3;
			if (side == 4 && blockStepY <= 0)
				side = 5;

			if (pBlockTemplate->GetBlockModel().IsCubeAABB() || IsCubeModePicking())
			{
				rayLength = distTraveled;
			}
			else
			{
				// use AABB for non-cube model
				CShapeAABB aabb;
				pBlockTemplate->GetAABB(this, curBlockIdX, curBlockIdY, curBlockIdZ, &aabb);
				Vector3 vOrig = rayOrig - Vector3((float)(blockSize * curBlockIdX), (float)(blockSize * curBlockIdY + GetVerticalOffset()), (float)(blockSize * curBlockIdZ));

				float fHitDist = -1;
				int nHitSide = 0;
				if (aabb.IntersectOutside(&fHitDist, &vOrig, &dir, &nHitSide))
				{
					rayLength = fHitDist;
					side = nHitSide;
				}
			}

			if (rayLength >= 0)
			{
				m_selectBlockIdW.x = curBlockIdX;
				m_selectBlockIdW.y = curBlockIdY;
				m_selectBlockIdW.z = curBlockIdZ;

				float collsionX = rayOrig.x + rayLength * dir.x;
				float collsionY = rayOrig.y + rayLength * dir.y;
				float collsionZ = rayOrig.z + rayLength * dir.z;

				result.X = collsionX;
				result.Y = collsionY;
				result.Z = collsionZ;

				result.BlockX = curBlockIdX;
				result.BlockY = curBlockIdY;
				result.BlockZ = curBlockIdZ;

				result.Side = side;
				result.Distance = rayLength;
				return true;
			}
		}
		if (distTraveled > length)
			return false;
	}
	return false;
}

bool CBlockWorld::IsObstructionBlock(uint16_t x, uint16_t y, uint16_t z)
{
	if (!m_isInWorld)
		return 0;

	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);

	if (pRegion)
	{
		uint32_t templateId = pRegion->GetBlockTemplateIdByIndex(lx, ly, lz);
		if (templateId > 0)
		{
			BlockTemplate* temp = GetBlockTemplate(templateId);
			if (temp && temp->IsMatchAttribute(BlockTemplate::batt_obstruction))
				return true;
		}
	}
	return false;
}

void CBlockWorld::SetTemplatePhysicsProperty(uint16_t id, const char* property)
{
	BlockTemplate* pTemplate = GetBlockTemplate(id);
	if (pTemplate)
	{
		pTemplate->SetPhysicsProperty(property);
	}
}

const char* CBlockWorld::GetTemplatePhysicsProperty(uint16_t id)
{
	BlockTemplate* pTemplate = GetBlockTemplate(id);
	if (pTemplate)
	{
		pTemplate->GetPhysicsProperty();
	}
	return "";
}

void CBlockWorld::SetTemplateTexture(uint16_t id, const char* textureName)
{
	BlockTemplate* pTemplate = GetBlockTemplate(id);
	if (pTemplate)
	{
		std::string sTextureName = textureName;

#ifdef USE_TEXTURE_NAME_FOR_BLOCK_TEXTURING
		if (sTextureName.find("_three") != std::string::npos)
		{
			if (pTemplate->IsMatchAttribute(BlockTemplate::batt_singleSideTex))
			{
				pTemplate->SetAttribute(BlockTemplate::batt_singleSideTex, false);
				pTemplate->SetAttribute(BlockTemplate::batt_threeSideTex, true);
				pTemplate->SetAttribute(BlockTemplate::batt_fourSideTex, false);
				pTemplate->GetBlockModel().LoadModelByTexture(3);
				ClearBlockRenderCache();
			}
		}
		else if (sTextureName.find("_four") != std::string::npos)
		{
			if (pTemplate->IsMatchAttribute(BlockTemplate::batt_singleSideTex))
			{
				pTemplate->SetAttribute(BlockTemplate::batt_singleSideTex, false);
				pTemplate->SetAttribute(BlockTemplate::batt_threeSideTex, false);
				pTemplate->SetAttribute(BlockTemplate::batt_fourSideTex, true);
				pTemplate->GetBlockModel().LoadModelByTexture(4);
				ClearBlockRenderCache();
			}
		}
		else
		{
			if (pTemplate->IsMatchAttribute(BlockTemplate::batt_threeSideTex))
			{
				pTemplate->SetAttribute(BlockTemplate::batt_singleSideTex, true);
				pTemplate->SetAttribute(BlockTemplate::batt_threeSideTex, false);
				pTemplate->SetAttribute(BlockTemplate::batt_fourSideTex, false);
				pTemplate->GetBlockModel().LoadModelByTexture(0);
				ClearBlockRenderCache();
			}
		}
#endif
		pTemplate->SetTexture0(textureName);
	}
}

void CBlockWorld::SuspendLightUpdate()
{
	GetLightGrid().SuspendLightUpdate();
}

void CBlockWorld::ResumeLightUpdate()
{
	GetLightGrid().ResumeLightUpdate();
}

bool CBlockWorld::IsLightUpdateSuspended()
{
	return GetLightGrid().IsLightUpdateSuspended();
}

void CBlockWorld::SetChunkColumnTimeStamp(uint16_t x, uint16_t z, uint16_t nTimeStamp)
{
	uint16_t y = 0;
	BlockRegion* pRegion = GetRegion(x, y, z, x, y, z);
	if (pRegion)
	{
		pRegion->SetChunkColumnTimeStamp(x, z, nTimeStamp);
	}
}

int32_t CBlockWorld::GetChunkColumnTimeStamp(uint16_t x, uint16_t z)
{
	uint16_t y = 0;
	BlockRegion* pRegion = GetRegion(x, y, z, x, y, z);
	if (pRegion)
	{
		return pRegion->GetChunkColumnTimeStamp(x, z);
	}
	return -1;
}



int CBlockWorld::FindFirstBlock(uint16_t x, uint16_t y, uint16_t z, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/, uint32_t filter /*= 0xffffffff*/, int nCategoryID)
{
	int bx = x;
	int by = y;
	int dist_offset = 0;
	int bz = z;

	if (nSide == 5)
	{
		// if we are finding downward, we can accelerate by using the heightmap;
		auto pHeightPair = GetHighestBlock(x, z);
		if (pHeightPair)
		{
			int nMaxY = pHeightPair->GetMaxHeight();
			if (y > nMaxY)
			{
				if (filter != 0)
				{
					// start searching from highest block.
					by = nMaxY + 1;
					dist_offset = y - by;
				}
				else
				{
					return 1;
				}
			}
		}
		else
		{
			// all column is air. 
			if (filter == 0)
				return 1;
			else
				return -1;
		}
	}
	for (uint32_t nDist = 1; nDist <= max_dist; ++nDist)
	{
		BlockCommon::GetBlockPosBySide(bx, by, bz, nSide);
		if (by >= 0 && by <= 256)
		{
			BlockTemplate* pTemplate = GetBlockTemplate(bx, by, bz);
			if (pTemplate)
			{
				if (pTemplate->IsMatchAttribute(filter) && (nCategoryID < 0 || pTemplate->GetCategoryID() == nCategoryID))
				{
					return nDist + dist_offset;
				}
			}
			else if (filter == 0)
			{
				return nDist + dist_offset;
			}
		}
	}
	return -1;
}


int CBlockWorld::GetFirstBlock(uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide /*= 4*/, uint32_t max_dist /*= 32*/)
{
	int bx = x;
	int by = y;
	int dist_offset = 0;
	int bz = z;

	if (nSide == 5)
	{
		// if we are finding downward, we can accelerate by using the heightmap;
		BlockTemplate* pBlock = GetBlockTemplate(nBlockId);
		if (pBlock && pBlock->IsMatchAttribute(BlockTemplate::batt_solid))
		{
			auto pHeightPair = GetHighestBlock(x, z);
			if (pHeightPair && pHeightPair->GetMaxHeight() > 0)
			{
				if (y > pHeightPair->GetMaxHeight())
				{
					// start searching from highest block.
					by = pHeightPair->GetMaxHeight() + 1;
					dist_offset = y - by;
				}
			}
			else
			{
				// all column is air. 
				return -1;
			}
		}
	}
	for (uint32_t nDist = 1; nDist <= max_dist; ++nDist)
	{
		BlockCommon::GetBlockPosBySide(bx, by, bz, nSide);
		if (by >= 0 && by <= 256)
		{
			BlockTemplate* pTemplate = GetBlockTemplate(bx, by, bz);
			if (pTemplate && pTemplate->GetID() == nBlockId)
			{
				return nDist + dist_offset;
			}
		}
	}
	return -1;
}


BlockIndex CBlockWorld::GetBlockIndex(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);
	if (pRegion)
	{
		BlockChunk* pChunk = pRegion->GetChunk(CalcPackedChunkID(lx, ly, lz), bCreateIfNotExist);
		if (pChunk)
		{
			return BlockIndex(pChunk, CalcPackedBlockID(lx, ly, lz));
		}
	}
	return BlockIndex((BlockChunk*)NULL, 0);
}

BlockChunk* CBlockWorld::GetChunk(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist)
{
	uint16_t lx, ly, lz;
	BlockRegion* pRegion = GetRegion(x, y, z, lx, ly, lz);
	if (pRegion)
	{
		return pRegion->GetChunk(CalcPackedChunkID(lx, ly, lz), bCreateIfNotExist);
	}
	return NULL;
}

LightData* CBlockWorld::GetLightData(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist)
{
	BlockIndex index = GetBlockIndex(x, y, z, bCreateIfNotExist);
	if (index.m_pChunk != NULL)
		return index.m_pChunk->GetLightData(index.m_nChunkBlockIndex);
	else
		return NULL;
}

int CBlockWorld::GetDirtyColumnCount()
{
	return GetLightGrid().GetDirtyColumnCount();
}

int ParaEngine::CBlockWorld::GetDirtyBlockCount()
{
	return GetLightGrid().GetDirtyBlockCount();
}

void CBlockWorld::GenerateLightBrightnessTable(bool bUseLinearBrightness)
{
	float fLight = 0.0f;

	for (int nIntensity = 0; nIntensity <= 15; ++nIntensity)
	{
		float fLightDist = 1.0f - nIntensity / 15.0f;
		if (bUseLinearBrightness)
			m_lightBrightnessTableFloat[nIntensity] = nIntensity / 15.0f;
		else
			m_lightBrightnessTableFloat[nIntensity] = (1.0f - fLightDist) / (fLightDist * 3.0f + 1.0f) * (1.0f - fLight) + fLight;
		m_lightBrightnessTableInt[nIntensity] = min((int)(m_lightBrightnessTableFloat[nIntensity] * 255), 255);
		m_lightBrightnessLinearTableFloat[nIntensity] = nIntensity / 15.0f;
	}
	for (int nIntensity = 16; nIntensity < sizeof(m_lightBrightnessTableInt); ++nIntensity)
	{
		m_lightBrightnessTableFloat[nIntensity] = 1.f;
		m_lightBrightnessTableInt[nIntensity] = 255;
		m_lightBrightnessLinearTableFloat[nIntensity] = 1.f;
	}
	if (m_is_linear_torch_brightness != bUseLinearBrightness)
	{
		ClearBlockRenderCache();
		m_is_linear_torch_brightness = bUseLinearBrightness;
	}
}

void CBlockWorld::UpdateRegionCache()
{
	if (!IsServerWorld())
	{
		int nDeltaChunkMoved = Math::Max(abs(m_curChunkIdW.x - m_lastChunkIdW_RegionCache.x), abs(m_curChunkIdW.z - m_lastChunkIdW_RegionCache.z));
		// at least walk cross one chunk to update region cache. 
		if (nDeltaChunkMoved >= 2)
		{
			m_lastChunkIdW_RegionCache = m_curChunkIdW;

			std::map<int, BlockRegion*>::iterator iter;

			bool bHasUnloadedRegion = true;
			int nRegionUnloaded = 0;
			while (m_regionCache.size() > m_maxCacheRegionCount && bHasUnloadedRegion)
			{
				BlockRegion* pFarthestRegion = NULL;
				int nFarthestRegionDist = 0;
				bHasUnloadedRegion = false;
				for (iter = m_regionCache.begin(); iter != m_regionCache.end(); iter++)
				{
					BlockRegion* pRegion = iter->second;
					if (pRegion)
					{
						// int nDistToCurrent = Math::Max(abs(m_curRegionIdX - pRegion->GetRegionX()), abs(m_curRegionIdZ - pRegion->GetRegionZ()));
						// all nine regions need to be in cache, regardless of m_maxCacheRegionCount
						// if (nDistToCurrent > 1)

						Uint16x3 center;
						pRegion->GetCenterBlockWs(&center);
						int nDistToCurrent = Math::Max(abs((int)m_curCenterBlockId.x - (int)center.x), abs((int)m_curCenterBlockId.z - (int)center.z));
						if (nDistToCurrent >= 256 + GetRenderDist())
						{
							// only remove unmodified region or remote region. 
							if (IsRemote() || !(pRegion->IsModified()))
							{
								if (nDistToCurrent > nFarthestRegionDist)
								{
									nFarthestRegionDist = nDistToCurrent;
									pFarthestRegion = pRegion;
								}
							}
						}
					}
				}
				if (pFarthestRegion)
				{
					OUTPUT_LOG("unload out of range region: %d %d\n", pFarthestRegion->GetRegionX(), pFarthestRegion->GetRegionZ());
					UnloadRegion(pFarthestRegion);
					bHasUnloadedRegion = true;
					nRegionUnloaded++;
				}
			}

			if (nRegionUnloaded > 0)
			{
				OUTPUT_LOG("%d region unloaded. Current region in memory: %d\n", nRegionUnloaded, (int)m_regionCache.size());
			}
		}
	}
	else
	{

	}
}

void CBlockWorld::OnViewCenterMove(float viewCenterX, float viewCenterY, float viewCenterZ)
{
	if (!m_isInWorld)
		return;

	BlockCommon::ConvertToBlockIndex(viewCenterX, viewCenterY, viewCenterZ, m_curCenterBlockId.x, m_curCenterBlockId.y, m_curCenterBlockId.z);

	m_curRegionIdX = m_curCenterBlockId.x >> 9;
	m_curRegionIdZ = m_curCenterBlockId.z >> 9;

	m_lastChunkIdW = m_curChunkIdW;

	m_curChunkIdW.x = m_curCenterBlockId.x >> 4;
	m_curChunkIdW.y = m_curCenterBlockId.y >> 4;
	m_curChunkIdW.z = m_curCenterBlockId.z >> 4;


	if (m_curChunkIdW.x != m_lastChunkIdW.x || m_curChunkIdW.y != m_lastChunkIdW.y || m_curChunkIdW.z != m_lastChunkIdW.z)
	{
		UpdateRegionCache();
		UpdateActiveChunk();
		GetLightGrid().OnWorldMove(m_curChunkIdW.x, m_curChunkIdW.z);
	}

	int nDeltaBlocksMoved = Math::Max(abs((int)m_curCenterBlockId.x - (int)m_lastViewCheckIdW.x), abs((int)m_curCenterBlockId.z - (int)m_lastViewCheckIdW.z));
	// at least walk cross one chunk to update region cache. 
	if (nDeltaBlocksMoved >= 16)
	{
		m_lastViewCheckIdW = m_curCenterBlockId;
		// ClearOutOfRangeActiveChunkData();
	}

	m_isVisibleChunkDirty = true;
}

void CBlockWorld::UpdateVisibleChunks(bool bIsShadowPass)
{

}

void CBlockWorld::SetSunIntensity(float intensity)
{
	if (intensity < 0) intensity = 0;
	if (intensity > 1) intensity = 1;

	float fDelta = m_sunIntensity - intensity;
	if (fDelta != 0.f)
	{
		m_sunIntensity = intensity;

		if (!IsInBlockWorld())
		{
			return;
		}

		// it will cause all buffer to rebuild, we will only change if light value changed a lot. 
		if (fDelta > 0.15f && GetBlockRenderMethod() == BLOCK_RENDER_FIXED_FUNCTION)
		{
			for (int x = 0; x < m_activeChunkDim; x++)
			{
				for (int z = 0; z < m_activeChunkDim; z++)
				{
					for (int y = 0; y < m_activeChunkDimY; y++)
					{
						RenderableChunk& chunk = GetActiveChunk(x, y, z);
						BlockChunk* pChunk = chunk.GetChunk();
						if (pChunk && pChunk->IsInfluenceBySunLight())
						{
							chunk.SetChunkDirty(true);
						}
					}
				}
			}
		}
	}
}


void CBlockWorld::SelectBlock(uint16_t x, uint16_t y, uint16_t z, int nGroupID)
{
	if (nGroupID >= 0 && nGroupID < BLOCK_GROUP_ID_MAX)
	{
		int64_t	nIndex = GetBlockSparseIndex(x, y, z);

		auto& blocks = m_selectedBlockMap[nGroupID].GetBlocks();
		if (blocks.find(nIndex) == blocks.end())
		{
			blocks[nIndex] = 0;
			UpdateSelectedBlockNearbyValues(x, y, z, nGroupID);
		}
	}
}

void CBlockWorld::DeselectBlock(uint16_t x, uint16_t y, uint16_t z, int nGroupID)
{
	if (nGroupID >= 0 && nGroupID < BLOCK_GROUP_ID_MAX)
	{
		int64_t	nIndex = GetBlockSparseIndex(x, y, z);

		auto itCur = m_selectedBlockMap[nGroupID].GetBlocks().find(nIndex);
		if (itCur != m_selectedBlockMap[nGroupID].GetBlocks().end())
		{
			m_selectedBlockMap[nGroupID].GetBlocks().erase(itCur);
			UpdateSelectedBlockNearbyValues(x, y, z, nGroupID);
		}
	}
}

void CBlockWorld::UpdateSelectedBlockNearbyValues(uint16_t x, uint16_t y, uint16_t z, int nGroupID)
{
	UpdateSelectedBlockValue(x, y, z, nGroupID);
	UpdateSelectedBlockValue(x, y + 1, z, nGroupID);
	UpdateSelectedBlockValue(x, y, z - 1, nGroupID);
	UpdateSelectedBlockValue(x, y - 1, z, nGroupID);
	UpdateSelectedBlockValue(x - 1, y, z, nGroupID);
	UpdateSelectedBlockValue(x + 1, y, z, nGroupID);
	UpdateSelectedBlockValue(x, y, z + 1, nGroupID);
}

void CBlockWorld::UpdateSelectedBlockValue(uint16_t x, uint16_t y, uint16_t z, int nGroupID)
{
	if (nGroupID >= 0 && nGroupID < BLOCK_GROUP_ID_MAX)
	{
		int64_t	nIndex = GetBlockSparseIndex(x, y, z);

		auto& blocks = m_selectedBlockMap[nGroupID].GetBlocks();
		if (blocks.find(nIndex) != blocks.end())
		{
			uint16_t v = 0;
			if (blocks.find(GetBlockSparseIndex(x, y + 1, z)) == blocks.end()) {
				v = v | 0x1;
			}
			if (blocks.find(GetBlockSparseIndex(x, y, z - 1)) == blocks.end()) {
				v = v | (0x1 << 1);
			}
			if (blocks.find(GetBlockSparseIndex(x, y - 1, z)) == blocks.end()) {
				v = v | (0x1 << 2);
			}
			if (blocks.find(GetBlockSparseIndex(x - 1, y, z)) == blocks.end()) {
				v = v | (0x1 << 3);
			}
			if (blocks.find(GetBlockSparseIndex(x + 1, y, z)) == blocks.end()) {
				v = v | (0x1 << 4);
			}
			if (blocks.find(GetBlockSparseIndex(x, y, z + 1)) == blocks.end()) {
				v = v | (0x1 << 5);
			}
			blocks[nIndex] = v;
		}
	}
}

void CBlockWorld::DeselectAllBlock(int nGroupID)
{
	if (nGroupID < 0)
	{
		nGroupID = min(-nGroupID, BLOCK_GROUP_ID_MAX - 1);
		for (int i = 0; i <= nGroupID; i++)
		{
			m_selectedBlockMap[i].GetBlocks().clear();
		}
	}
	else if (nGroupID < BLOCK_GROUP_ID_MAX)
	{
		m_selectedBlockMap[nGroupID].GetBlocks().clear();
	}
}

ParaEngine::BlockRenderMethod ParaEngine::CBlockWorld::GetBlockRenderMethod()
{
	return m_dwBlockRenderMethod;
}

void CBlockWorld::OnGenerateTerrain(int nRegionX, int nRegionY, int nChunkX, int nChunkZ)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_GeneratorScript);
	if (pCallback) {
		char sMsg[512];
		snprintf(sMsg, 512, "msg={region_x=%d, region_y=%d,chunk_x=%d, chunk_z=%d};", nRegionX, nRegionY, nChunkX, nChunkZ);
		std::string script = sMsg;
		script += pCallback->GetCode();
		pCallback->ActivateLocalNow(script);
	}
}


int ParaEngine::CBlockWorld::OnBeforeLoadBlockRegion(int x, int y)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_BeforeLoadBlockRegion);
	if (pCallback) {
		char sMsg[100];
		snprintf(sMsg, 100, "msg={x=%d,y=%d};", x, y);
		std::string script = sMsg;
		script += pCallback->GetCode();
		return pCallback->ActivateLocalNow(script);
	}
	return S_OK;
}


int CBlockWorld::OnLoadBlockRegion(int x, int y)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_LoadBlockRegion);
	if (pCallback) {
		char sMsg[100];
		snprintf(sMsg, 100, "msg={x=%d,y=%d};", x, y);
		std::string script = sMsg;
		script += pCallback->GetCode();
		pCallback->ActivateLocalNow(script);
	}
	return S_OK;
}

int CBlockWorld::OnUnLoadBlockRegion(int x, int y)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_UnLoadBlockRegion);
	if (pCallback) {
		char sMsg[100];
		snprintf(sMsg, 100, "msg={x=%d,y=%d};", x, y);
		std::string script = sMsg;
		script += pCallback->GetCode();
		pCallback->ActivateAsync(script);
	}
	return S_OK;
}

int ParaEngine::CBlockWorld::OnSaveBlockRegion(int x, int y)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_SaveRegionCallbackScript);
	if (pCallback) {
		char sMsg[100];
		snprintf(sMsg, 100, "msg={x=%d,y=%d,type=\"raw\"};", x, y);
		std::string script = sMsg;
		script += pCallback->GetCode();
		pCallback->ActivateLocalNow(script);
	}
	return S_OK;
}


void ParaEngine::CBlockWorld::AddRenderTask(BlockRenderTask* pRenderTask)
{

}

CWorldInfo& ParaEngine::CBlockWorld::GetWorldInfo()
{
	return m_worldInfo;
}

bool ParaEngine::CBlockWorld::IsSaveLightMap() const
{
	return m_bSaveLightMap;
}

void ParaEngine::CBlockWorld::SaveLightMap(bool val)
{
	m_bSaveLightMap = val;
}


ParaEngine::mutex& ParaEngine::CBlockWorld::GetBlockWorldMutex()
{
	return m_blockworld_mutex;
}

void ParaEngine::CBlockWorld::LockWorld()
{
	m_blockworld_mutex.lock();
}

void ParaEngine::CBlockWorld::UnlockWorld()
{
	m_blockworld_mutex.unlock();
}

void ParaEngine::CBlockWorld::ClearBlockRenderCache()
{

}

bool ParaEngine::CBlockWorld::DoChunksNearChunkExist(uint16_t x, uint16_t y, uint16_t z, uint16_t radius)
{
	return CheckChunkColumnsExist(x - radius, y - radius, z - radius, x + radius, y + radius, z + radius);
}

bool ParaEngine::CBlockWorld::CheckChunkColumnsExist(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
	if (maxY >= 0 && minY < 256)
	{
		minX >>= 4;
		minZ >>= 4;
		maxX >>= 4;
		maxZ >>= 4;

		for (int cx = minX; cx <= maxX; ++cx)
		{
			for (int cz = minZ; cz <= maxZ; ++cz)
			{
				if (!ChunkColumnExists(cx, cz))
				{
					return false;
				}
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool ParaEngine::CBlockWorld::ChunkColumnExists(uint16_t chunkX, uint16_t chunkZ)
{
	ChunkLocation chunkPos(chunkX, chunkZ);
	uint16_t y = 0;
	uint16_t x = chunkPos.GetCenterWorldX();
	uint16_t z = chunkPos.GetCenterWorldZ();

	BlockRegion* pRegion = GetRegion(x, y, z, x, y, z);
	if (pRegion && !pRegion->IsLocked())
	{
		return pRegion->GetChunkColumnTimeStamp(x, z) > 0;
	}
	return false;
}


bool ParaEngine::CBlockWorld::IsChunkLocked(uint32 worldX, uint32 worldZ)
{
	int16_t regionX = (int16_t)(worldX >> 9);
	int16_t regionZ = (int16_t)(worldZ >> 9);

	BlockRegion* pRegion = GetRegion(regionX, regionZ);
	if (pRegion && !pRegion->IsLocked())
	{
		return false;
	}
	return true;
}

bool ParaEngine::CBlockWorld::IsRemote()
{
	return m_bIsRemote;
}

void ParaEngine::CBlockWorld::SetIsRemote(bool bValue)
{
	m_bIsRemote = bValue;
}


void ParaEngine::CBlockWorld::SetIsServerWorld(bool bValue)
{
	m_bIsServerWorld = bValue;
}

bool ParaEngine::CBlockWorld::IsServerWorld()
{
	return m_bIsServerWorld;
}

void ParaEngine::CBlockWorld::SetLightCalculationStep(uint32 nTicks)
{
	GetLightGrid().SetLightCalculationStep(nTicks);
}

uint32 ParaEngine::CBlockWorld::GetLightCalculationStep()
{
	return GetLightGrid().GetLightCalculationStep();
}

void ParaEngine::CBlockWorld::SetRenderBlocks(bool bValue)
{
	if (m_bRenderBlocks != bValue)
	{
		m_bRenderBlocks = bValue;
		SetVisibleChunkDirty(true);
	}
}

bool ParaEngine::CBlockWorld::IsRenderBlocks()
{
	return m_bRenderBlocks;
}

bool ParaEngine::CBlockWorld::IsUseAsyncLoadWorld() const
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	return false;
#else
	return m_bUseAsyncLoadWorld;
#endif
}

void ParaEngine::CBlockWorld::SetUseAsyncLoadWorld(bool val)
{
	m_bUseAsyncLoadWorld = val;
}

void ParaEngine::CBlockWorld::OnFrameMove()
{
	for (auto& iter : m_regionCache)
	{
		iter.second->OnFrameMove();
	}
}

int ParaEngine::CBlockWorld::GetNumOfLockedBlockRegion()
{
	int nCount = 0;
	for (auto& iter : m_regionCache)
	{
		if (iter.second->IsLocked())
			nCount++;
	}
	return nCount;
}


int ParaEngine::CBlockWorld::GetNumOfBlockRegion()
{
	return (int)m_regionCache.size();
}

int ParaEngine::CBlockWorld::GetTotalNumOfLoadedChunksInLockedBlockRegion()
{
	int nCount = 0;
	for (auto& iter : m_regionCache)
	{
		if (iter.second->IsLocked())
			nCount += iter.second->GetChunksLoaded();
	}
	return nCount;
}

bool ParaEngine::CBlockWorld::IsVisibleChunkDirty() const
{
	return m_isVisibleChunkDirty;
}

void ParaEngine::CBlockWorld::SetVisibleChunkDirty(bool val)
{
	m_isVisibleChunkDirty = val;
}

BlockReadWriteLock& ParaEngine::CBlockWorld::GetReadWriteLock()
{
	return m_readWriteLock;
}


bool ParaEngine::CBlockWorld::IsGroupByChunkBeforeTexture() const
{
	return m_group_by_chunk_before_texture;
}

void ParaEngine::CBlockWorld::SetGroupByChunkBeforeTexture(bool val)
{
	m_group_by_chunk_before_texture = val;
}

const ParaEngine::Int32x3& ParaEngine::CBlockWorld::GetMinWorldPos() const
{
	return m_minWorldPos;
}

void ParaEngine::CBlockWorld::SetMinWorldPos(const ParaEngine::Int32x3& val)
{
	m_minWorldPos = val;
	m_minRegionX = (std::max)((int32)0, m_minWorldPos.x >> 9);
	m_minRegionZ = (std::max)((int32)0, m_minWorldPos.z >> 9);
}

const ParaEngine::Int32x3& ParaEngine::CBlockWorld::GetMaxWorldPos() const
{
	return m_maxWorldPos;
}

void ParaEngine::CBlockWorld::SetMaxWorldPos(const ParaEngine::Int32x3& val)
{
	m_maxWorldPos = val;
	m_maxRegionX = (std::min)((int32)63, m_maxWorldPos.x >> 9);
	m_maxRegionZ = (std::min)((int32)63, m_maxWorldPos.z >> 9);
}

uint16_t ParaEngine::CBlockWorld::GetMaxCacheRegionCount() const
{
	return m_maxCacheRegionCount;
}

void ParaEngine::CBlockWorld::SetMaxCacheRegionCount(uint16_t val)
{
	if (val >= 4)
	{
		m_maxCacheRegionCount = val;
	}
}


RenderableChunk* ParaEngine::CBlockWorld::GetRenderableChunk(const Int16x3& chunkPos)
{
	if (IsChunkColumnInActiveRange(chunkPos.x, chunkPos.z))
	{
		RenderableChunk* pChunk = &GetActiveChunk(chunkPos.x, chunkPos.y, chunkPos.z);
		if (pChunk->IsDirty() || pChunk->GetChunkPosWs() != chunkPos)
			pChunk = NULL;
		return pChunk;
	}
	return NULL;
}


void ParaEngine::CBlockWorld::ClearOutOfRangeActiveChunkData()
{
	int nChunkColumnRemoved = 0;
	int32 centerX = m_curCamBlockId.x;
	int32 centerZ = m_curCamBlockId.z;

	for (int z = 0; z < m_activeChunkDim; z++)
	{
		int16_t curChunkWZ = m_minActiveChunkId_ws.z + z;
		for (int x = 0; x < m_activeChunkDim; x++)
		{
			int16_t curChunkWX = m_minActiveChunkId_ws.x + x;
			int32 nDist = (int32)(pow(centerX - (curChunkWX * 16 + 8), 2) + pow(centerZ - (curChunkWZ * 16 + 8), 2));

			if (nDist > 0)
				nDist = (int)((Math::Sqrt((float)nDist) + 12)); // 16/2 * 1.414 = 12
			if (nDist > GetRenderDist())
			{
				//if (!GetActiveChunk(curChunkWX, 0, curChunkWZ).IsDirty())
				{
					for (int y = 0; y < m_activeChunkDimY; y++)
					{
						int16_t curChunkWY = m_minActiveChunkId_ws.y + y;

						GetActiveChunk(curChunkWX, curChunkWY, curChunkWZ).ClearChunkData();
					}
					nChunkColumnRemoved++;
				}
			}
		}
	}
	if (nChunkColumnRemoved > 0)
	{
		// OUTPUT_LOG1("nChunkColumnRemoved: %d\n", nChunkColumnRemoved);
	}
}

IAttributeFields* ParaEngine::CBlockWorld::GetChildAttributeObject(const char* sName)
{
	for (auto& iter : m_regionCache)
	{
		if (iter.second->GetIdentifier() == sName)
			return iter.second;
	}
	return NULL;
}

IAttributeFields* ParaEngine::CBlockWorld::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nRowIndex < (int)m_regionCache.size())
	{
		auto iter = m_regionCache.begin();
		std::advance(iter, nRowIndex);
		return iter->second;
	}
	return NULL;
}

int ParaEngine::CBlockWorld::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return (int)m_regionCache.size();
}

const std::string& ParaEngine::CBlockWorld::GetIdentifier()
{
	return m_sName;
}

void ParaEngine::CBlockWorld::SetIdentifier(const std::string& sID)
{
	m_sName = sID;
}

bool ParaEngine::CBlockWorld::IsAutoPhysics()
{
	return m_bAutoPhysics;
}

void ParaEngine::CBlockWorld::SetAutoPhysics(bool bValue)
{
	m_bAutoPhysics = bValue;
}

bool ParaEngine::CBlockWorld::IsAsyncLightCalculation()
{
	return m_pLightGrid->IsAsyncLightCalculation();
}

void ParaEngine::CBlockWorld::SetAsyncLightCalculation(bool val)
{
	m_pLightGrid->SetAsyncLightCalculation(val);
}

int ParaEngine::CBlockWorld::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass != NULL);

	pClass->AddField("BlockRenderMethod", FieldType_Int, (void*)SetBlockRenderMethod_s, (void*)GetBlockRenderMethod_s, NULL, NULL, bOverride);

	pClass->AddField("ResumeLightUpdate", FieldType_void, (void*)ResumeLightUpdate_s, NULL, NULL, "", bOverride);
	pClass->AddField("SuspendLightUpdate", FieldType_void, (void*)SuspendLightUpdate_s, NULL, NULL, "", bOverride);

	pClass->AddField("ResetAllLight", FieldType_void, (void*)ResetAllLight_s, NULL, NULL, "", bOverride);

	pClass->AddField("LockWorld", FieldType_void, (void*)LockWorld_s, NULL, NULL, "", bOverride);
	pClass->AddField("UnlockWorld", FieldType_void, (void*)UnlockWorld_s, NULL, NULL, "", bOverride);

	pClass->AddField("IsLightUpdateSuspended", FieldType_Bool, NULL, (void*)IsLightUpdateSuspended_s, NULL, NULL, bOverride);

	pClass->AddField("SetChunkColumnTimeStamp", FieldType_Float_Float_Float, (void*)SetChunkColumnTimeStamp_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("RenderDist", FieldType_Int, (void*)SetRenderDist_s, (void*)GetRenderDist_s, NULL, NULL, bOverride);

	pClass->AddField("DirtyColumnCount", FieldType_Int, NULL, (void*)GetDirtyColumnCount_s, NULL, NULL, bOverride);
	pClass->AddField("DirtyBlockCount", FieldType_Int, NULL, (void*)GetDirtyBlockCount_s, NULL, NULL, bOverride);
	pClass->AddField("IsReadOnly", FieldType_Bool, (void*)SetReadOnly_s, (void*)IsReadOnly_s, NULL, NULL, bOverride);
	pClass->AddField("IsRemote", FieldType_Bool, (void*)SetIsRemote_s, (void*)IsRemote_s, NULL, NULL, bOverride);
	pClass->AddField("IsServerWorld", FieldType_Bool, (void*)SetIsServerWorld_s, (void*)IsServerWorld_s, NULL, NULL, bOverride);
	pClass->AddField("SaveLightMap", FieldType_Bool, (void*)SetSaveLightMap_s, (void*)IsSaveLightMap_s, NULL, NULL, bOverride);
	pClass->AddField("UseAsyncLoadWorld", FieldType_Bool, (void*)SetUseAsyncLoadWorld_s, (void*)IsUseAsyncLoadWorld_s, NULL, NULL, bOverride);
	pClass->AddField("UseLinearTorchBrightness", FieldType_Bool, (void*)UseLinearTorchBrightness_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("GeneratorScript", FieldType_String, (void*)SetGeneratorScript_s, (void*)GetGeneratorScript_s, NULL, NULL, bOverride);

	pClass->AddField("OnBeforeLoadBlockRegion", FieldType_String, (void*)SetBeforeLoadBlockRegion_s, (void*)GetBeforeLoadBlockRegion_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("OnLoadBlockRegion", FieldType_String, (void*)SetLoadBlockRegion_s, (void*)GetLoadBlockRegion_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("OnUnLoadBlockRegion", FieldType_String, (void*)SetUnLoadBlockRegion_s, (void*)GetUnLoadBlockRegion_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("OnSaveRegionCallbackScript", FieldType_String, (void*)SetSaveRegionCallbackScript_s, (void*)GetSaveRegionCallbackScript_s, NULL, NULL, bOverride);

	pClass->AddField("LightCalculationStep", FieldType_Int, (void*)SetLightCalculationStep_s, (void*)GetLightCalculationStep_s, NULL, NULL, bOverride);
	pClass->AddField("IsAsyncLightCalculation", FieldType_Bool, (void*)SetAsyncLightCalculation_s, (void*)IsAsyncLightCalculation_s, NULL, NULL, bOverride);
	
	pClass->AddField("RenderBlocks", FieldType_Bool, (void*)SetRenderBlocks_s, (void*)IsRenderBlocks_s, NULL, NULL, bOverride);
	pClass->AddField("NumOfLockedBlockRegion", FieldType_Int, (void*)NULL, (void*)GetNumOfLockedBlockRegion_s, NULL, NULL, bOverride);
	pClass->AddField("NumOfBlockRegion", FieldType_Int, (void*)NULL, (void*)GetNumOfBlockRegion_s, NULL, NULL, bOverride);
	pClass->AddField("MaxCacheRegionCount", FieldType_Int, (void*)SetMaxCacheRegionCount_s, (void*)GetMaxCacheRegionCount_s, NULL, NULL, bOverride);
	pClass->AddField("TotalNumOfLoadedChunksInLockedBlockRegion", FieldType_Int, (void*)NULL, (void*)GetTotalNumOfLoadedChunksInLockedBlockRegion_s, NULL, NULL, bOverride);
	pClass->AddField("SunIntensity", FieldType_Float, (void*)SetSunIntensity_s, (void*)GetSunIntensity_s, NULL, NULL, bOverride);

	pClass->AddField("MinWorldPos", FieldType_Vector3, (void*)SetMinWorldPos_s, (void*)GetMinWorldPos_s, NULL, NULL, bOverride);
	pClass->AddField("MaxWorldPos", FieldType_Vector3, (void*)SetMaxWorldPos_s, (void*)GetMaxWorldPos_s, NULL, NULL, bOverride);
	pClass->AddField("TotalChunksInMemory", FieldType_Int, (void*)0, (void*)GetTotalChunksInMemory_s, NULL, NULL, bOverride);
	pClass->AddField("TotalRenderableChunksInMemory", FieldType_Int, (void*)0, (void*)GetTotalRenderableChunksInMemory_s, NULL, NULL, bOverride);
	pClass->AddField("IsAutoPhysics", FieldType_Bool, (void*)SetAutoPhysics_s, (void*)IsAutoPhysics_s, NULL, NULL, bOverride);

	return S_OK;
}


