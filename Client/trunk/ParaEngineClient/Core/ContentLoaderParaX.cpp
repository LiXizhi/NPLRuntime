//-----------------------------------------------------------------------------
// Class:	Content loaders
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: for async asset streaming. It uses architecture proposed by the content streaming sample in DirectX 9&10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseObject.h"
#include "AssetManifest.h"
#include "MeshEntity.h"
#include "ParaXEntity.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/FBXParser.h"
#include "BMaxModel/BMaxParser.h"
#include "ParaXSerializer.h"
#include "ParaMeshXMLFile.h"
#include "AsyncLoader.h"
#include "util/StringHelper.h"
#include "ContentLoaderParaX.h"

using namespace ParaEngine;

/**@def enable automatic lod calculation for bmax model */
#define ENABLE_BMAX_AUTO_LOD

//////////////////////////////////////////////////////////////////////////
//
// CParaXLoader
//
//////////////////////////////////////////////////////////////////////////

ParaEngine::CParaXLoader::CParaXLoader(asset_ptr<ParaXEntity>& pAsset, const char* sFileName)
{
	m_asset = pAsset;
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CParaXLoader::~CParaXLoader()
{

}

const char* ParaEngine::CParaXLoader::GetFileName()
{
	if (!m_sFileName.empty())
	{
		return m_sFileName.c_str();
	}
	else if (m_asset.get() != 0)
	{
		return m_asset->GetLocalFileName().c_str();
	}
	return NULL;
}

HRESULT ParaEngine::CParaXLoader::Decompress(void** ppData, int* pcBytes)
{
	return S_OK;
}
HRESULT ParaEngine::CParaXLoader::CleanUp()
{
	m_asset.reset();
	return S_OK;
}

HRESULT ParaEngine::CParaXLoader::Destroy()
{
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CParaXLoader::Load()
{
	// let us only get raw compressed data from local zip file. and let the worker thread to decompress it. 
	if (m_asset.get() == 0)
		return E_FAIL;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CParaXProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CParaXProcessor::CParaXProcessor(asset_ptr<ParaXEntity>& pAsset)
	:m_cBytes(0), m_pData(NULL), m_pDevice(NULL), m_pAssetFileEntry(NULL), m_nTechniqueHandle(TECH_CHARACTER)
{
	m_asset = pAsset;
}

ParaEngine::CParaXProcessor::~CParaXProcessor()
{

}

bool ParaEngine::CParaXProcessor::CreateMeshLODLevel(float fromDepth, const string& sFilename)
{
	MeshLOD meshLOD;
	meshLOD.m_fromDepthSquared = fromDepth*fromDepth;
	meshLOD.m_sMeshFileName = sFilename;

	m_MeshLODs.push_back(meshLOD);

	std::sort(m_MeshLODs.begin(), m_MeshLODs.end(), MeshLodSortLess());
	return true;
}

HRESULT ParaEngine::CParaXProcessor::LockDeviceObject()
{
	if (m_asset.get() != 0 && m_asset->Lock())
	{
		return S_OK;
	}
	return E_FAIL;
}

RenderDevicePtr ParaEngine::CParaXProcessor::GetRenderDevice()
{
	return (m_pDevice != 0) ? m_pDevice : CGlobals::GetRenderDevice();
}

HRESULT ParaEngine::CParaXProcessor::UnLockDeviceObject()
{
	if (m_asset.get() != 0)
	{
		if (m_MeshLODs.size() > 0 && m_asset->m_MeshLODs.size() == 0)
		{
			m_asset->m_MeshLODs = m_MeshLODs;
			m_MeshLODs.clear();
		}

		// load data from mesh. 
		std::vector<MeshLOD> & pMeshLODs = m_asset->m_MeshLODs;
		std::vector<MeshLOD>::iterator iCur, iEnd = pMeshLODs.end();
		for (iCur = pMeshLODs.begin(); iCur != iEnd; ++iCur)
		{
			MeshLOD& lod = (*iCur);
			if (lod.m_pParaXMesh)
			{
				lod.m_pParaXMesh->InitDeviceObjects();
			}
		}

		if (m_asset->m_MeshLODs.size() > 0 && m_asset->GetPrimaryTechniqueHandle() < 0)
		{
			m_asset->SetPrimaryTechniqueHandle(m_nTechniqueHandle);
		}

		m_asset->UnLock();
	}
	return S_OK;
}

HRESULT ParaEngine::CParaXProcessor::CleanUp()
{
	return S_OK;
}

class SParaXCallBackData
{
public:
	SParaXCallBackData(ParaXEntity * pAsset_) :pAsset(pAsset_){}

	ParaXEntity * pAsset;

	void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
	{
		if (nResult == 0)
		{
			pAsset->SetState(AssetEntity::ASSET_STATE_NORMAL);
			pAsset->m_bIsInitialized = false;
			pAsset->InitDeviceObjects();
		}
	}
};

HRESULT ParaEngine::CParaXProcessor::Destroy()
{
	if (m_pAssetFileEntry != 0)
	{
		m_asset->SetState(AssetEntity::ASSET_STATE_SYNC_START);

		// we need to download from the web server. 
		return m_pAssetFileEntry->SyncFile_Async(SParaXCallBackData(m_asset.get()));
	}
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CParaXProcessor::Process(void* pData, int cBytes)
{
	m_pData = (char*)pData;
	m_cBytes = cBytes;
	return S_OK;
}

HRESULT ParaEngine::CParaXProcessor::CopyToResource()
{
	if (m_asset.get() == 0)
		return E_FAIL;
	std::string sRootFileExt;
	if (m_asset->m_MeshLODs.size() == 0)
	{
		string filename = m_asset->GetLocalFileName();

		bool bLoadHeaderFromXFile = true;
		bool bSingleLOD = true;
		sRootFileExt = CParaFile::GetFileExtension(filename);
		StringHelper::make_lower(sRootFileExt);
		bool bIsXML = (sRootFileExt == "xml");
		string sParentDirectory = CParaFile::GetParentDirectoryFromPath(filename);

		// check in manifest
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
		if (pEntry)
		{
			if (pEntry->DoesFileExist())
			{
				filename = pEntry->GetLocalFileName();
			}
			else
			{
				m_pAssetFileEntry = pEntry;
				return E_PENDING;
			}
		}

		if (bIsXML)
		{
			sRootFileExt = "";
			CParaMeshXMLFile file;
			if (file.LoadFromFile(filename, sParentDirectory))
			{
				if (file.GetType() == CParaMeshXMLFile::TYPE_MESH_LOD)
				{
					int i;
					// check if all sub files are available locally.
					for (i = 0; i < file.GetSubMeshCount(); ++i)
					{
						CParaMeshXMLFile::CSubMesh* pSubLODMesh = file.GetSubMesh(i);
						AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(pSubLODMesh->m_sFileName);
						if (pEntry)
						{
							if (pEntry->DoesFileExist())
							{
								pSubLODMesh->m_sFileName = pEntry->GetLocalFileName();
							}
							else
							{
								m_pAssetFileEntry = pEntry;
								return E_PENDING;
							}
						}
					}

					if (file.m_bHasBoundingBox)
					{
					}

					for (i = 0; i < file.GetSubMeshCount(); ++i)
					{
						CParaMeshXMLFile::CSubMesh* pSubLODMesh = file.GetSubMesh(i);
						CreateMeshLODLevel(pSubLODMesh->m_fToCameraDist, pSubLODMesh->m_sFileName);
					}
					if (file.GetSubMeshCount() == 0)
					{
						OUTPUT_LOG("warning: LOD file %s does not contain any lod mesh references\n", filename.c_str());
						return E_FAIL;
					}
				}
			}
			else
			{
				OUTPUT_LOG("failed loading mesh xml file: %s\n", filename.c_str());
				return E_FAIL;
			}
		}
		else
		{
			CreateMeshLODLevel(0.f, filename);
		}
	}

	try
	{
		// preload file data to memory in the IO thread. 
		std::vector<MeshLOD> & pMeshLODs = m_MeshLODs.size() > 0 ? m_MeshLODs : m_asset->m_MeshLODs;
		for (auto iCur = pMeshLODs.begin(); iCur != pMeshLODs.end(); ++iCur)
		{
			if ( ! (iCur->m_pParaXMesh) )
			{
				CParaFile myFile(iCur->m_sMeshFileName.c_str());

				if (myFile.isEof())
				{
					OUTPUT_LOG("warning: ParaX model file not found %s\n", iCur->m_sMeshFileName.c_str());
					return E_FAIL;
				}
				std::string sExt;
				if (sRootFileExt.empty()) {
					sExt = CParaFile::GetFileExtension(iCur->m_sMeshFileName);
					StringHelper::make_lower(sExt);
				}
				else {
					sExt = sRootFileExt;
				}
					
				if (sExt == "bmax")
				{
					// block max model. 
					BMaxParser p;
					ParaXEntity* pParaEntity = dynamic_cast<ParaXEntity*>(m_asset.get());
					if (pParaEntity != nullptr) {
						p.SetMergeCoplanerBlockFace(pParaEntity->GetMergeCoplanerBlockFace());
					}
					p.Load(myFile.getBuffer(), myFile.getSize());
					iCur->m_pParaXMesh = p.ParseParaXModel();
					auto pParaXMesh = iCur->m_pParaXMesh;

#ifdef ENABLE_BMAX_AUTO_LOD
					// generate LOD
					bool bGenerateLOD = true;
					if (iCur == pMeshLODs.begin())
					{
						if (pMeshLODs.size() > 1)
						{
							for (int i = 1; i < (int)pMeshLODs.size(); ++i)
							{
								if (!pMeshLODs[i].m_sMeshFileName.empty())
								{
									bGenerateLOD = false;
								}
							}
							if (bGenerateLOD)
							{
								pMeshLODs.resize(1);
							}
						}
					}
					if (bGenerateLOD)
					{
						// each LOD at least cut triangle count in half and no bigger than a given count. 
						// by default, generate lod in range 30, 60, 90, 120 meters;
						// TODO: currently it is hard-coded, shall we read LOD settings from bmax file, so that users
						// have find-control on the settings. 
						struct LODSetting {
							int nMaxTriangleCount;
							float fMaxDistance;
						};
						const LODSetting nLodsMaxTriangleCounts[] = { {2000, 60.f}, {500, 90.f}, {100, 120.f} };
						
						// from second lod
						for (int i = 0; pParaXMesh && i < sizeof(nLodsMaxTriangleCounts)/sizeof(LODSetting); i++)
						{
							if ((int)pParaXMesh->GetPolyCount() >= nLodsMaxTriangleCounts[i].nMaxTriangleCount)
							{
								MeshLOD lod;
								lod.m_pParaXMesh = p.ParseParaXModel((std::min)(nLodsMaxTriangleCounts[i].nMaxTriangleCount, (int)(pParaXMesh->GetPolyCount() / 2) - 4));
								lod.m_fromDepthSquared = Math::Sqr(nLodsMaxTriangleCounts[i].fMaxDistance);
								if (lod.m_pParaXMesh)
								{
									if (pMeshLODs.size() == 1)
									{
										iCur->m_fromDepthSquared = Math::Sqr(nLodsMaxTriangleCounts[0].fMaxDistance*0.5f);
									}
									pParaXMesh = lod.m_pParaXMesh;
									pMeshLODs.push_back(lod);
									iCur = pMeshLODs.end() - 1;
								}
							}
						}
					}
#endif
				}

#ifdef SUPPORT_FBX_MODEL_FILE
				else if (sExt == "fbx" || sExt == "gltf" || sExt == "glb")
				{
					// static or animated fbx model
					FBXParser parser(iCur->m_sMeshFileName);
					iCur->m_pParaXMesh = parser.ParseParaXModel(myFile.getBuffer(), myFile.getSize(), sExt.c_str());
				}
#endif
				else
				{
					CParaXSerializer serializer;
					serializer.SetFilename(iCur->m_sMeshFileName);
					iCur->m_pParaXMesh = (CParaXModel*)serializer.LoadParaXMesh(myFile);
				}
				if (iCur->m_pParaXMesh == 0)
				{
					// lod.m_pParaXMesh = new CParaXModel(ParaXHeaderDef());
					OUTPUT_LOG("warning: cannot load ParaX model file %s\n", iCur->m_sMeshFileName.c_str());
					return E_FAIL;
				}

				if (!iCur->m_pParaXMesh->IsValid())
				{
					return E_FAIL;
				}
				else
				{
					m_nTechniqueHandle = iCur->m_pParaXMesh->IsBmaxModel() ? TECH_BMAX_MODEL : TECH_CHARACTER;
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG("warning: failed initialize ParaX model %s\n", m_asset->GetLocalFileName().c_str());
		return E_FAIL;
	}
	return S_OK;
}

void ParaEngine::CParaXProcessor::SetResourceError()
{
	if (m_asset.get() != 0)
	{
		m_asset->m_bIsValid = false;
	}
}

