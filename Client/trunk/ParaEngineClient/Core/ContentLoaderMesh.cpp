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
#include "ParaMeshXMLFile.h"
#include "AsyncLoader.h"

#include "ContentLoaderMesh.h"

using namespace ParaEngine;

ParaEngine::CMeshLoader::CMeshLoader(asset_ptr<MeshEntity>& pAsset, const char* sFileName)
{
	m_asset = pAsset;
	if (sFileName)
		m_sFileName = sFileName;
}

ParaEngine::CMeshLoader::~CMeshLoader()
{
	CleanUp();
}

const char* ParaEngine::CMeshLoader::GetFileName()
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

HRESULT ParaEngine::CMeshLoader::Decompress(void** ppData, int* pcBytes)
{
	return S_OK;
}

HRESULT ParaEngine::CMeshLoader::CleanUp()
{
	return S_OK;
}

HRESULT ParaEngine::CMeshLoader::Destroy()
{
	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CMeshLoader::Load()
{
	// let us only get raw compressed data from local zip file. and let the worker thread to decompress it. 
	if (m_asset.get() == 0)
		return E_FAIL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CMeshProcessor
//
//////////////////////////////////////////////////////////////////////////
ParaEngine::CMeshProcessor::CMeshProcessor(asset_ptr<MeshEntity>& pAsset)
	:m_cBytes(0), m_pData(NULL), m_pDevice(NULL), m_pParamBlock(NULL), m_nTechniqueHandle(-1), m_pAssetFileEntry(NULL)
{
	m_asset = pAsset;
	m_aabb.SetInvalid();
}

ParaEngine::CMeshProcessor::~CMeshProcessor()
{
	CleanUp();
}

bool ParaEngine::CMeshProcessor::PopulateMesh()
{
	return true;
}

HRESULT ParaEngine::CMeshProcessor::LockDeviceObject()
{
	if (m_asset.get() != 0 && m_asset->Lock())
	{
		return S_OK;
	}
	return E_FAIL;
}

RenderDevicePtr ParaEngine::CMeshProcessor::GetRenderDevice()
{
	return (m_pDevice != 0) ? m_pDevice : CGlobals::GetRenderDevice();
}

HRESULT ParaEngine::CMeshProcessor::UnLockDeviceObject()
{
	if (m_asset.get() != 0)
	{
		RenderDevicePtr pd3dDevice = GetRenderDevice();
		if (pd3dDevice == 0)
			return E_FAIL;
		// copy params 
		if (m_pParamBlock && m_asset->m_pParamBlock == 0)
		{
			m_asset->m_pParamBlock = m_pParamBlock;
			m_pParamBlock = NULL;
		}

		// copy Mesh lods
		if (m_MeshLODs.size() > 0 && m_asset->m_MeshLODs.size() == 0)
		{
			m_asset->m_MeshLODs = m_MeshLODs;
			m_MeshLODs.clear();
		}

		// load data from mesh. 
		std::vector<MeshLOD> & pMeshLODs = m_asset->m_MeshLODs;
		std::vector<MeshLOD>::iterator iCur, iEnd = pMeshLODs.end();
		int i = 0;
		for (iCur = pMeshLODs.begin(); iCur != iEnd; ++iCur, ++i)
		{
			MeshLOD& lod = (*iCur);
			if (lod.m_pStaticMesh)
			{
				lod.m_pStaticMesh->Create(pd3dDevice, (const char*)NULL, (m_asset->GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL_TEX2));

				if (i == 0 && !(m_aabb.IsValid()))
				{
					// get the mesh header(using the first LOD level) if previous one failed. 
					MeshHeader& mesh_header = lod.m_pStaticMesh->GetMeshHeader();
					if (mesh_header.m_bIsValid)
					{
						m_aabb.SetMinMax(mesh_header.m_vMin, mesh_header.m_vMax);
						if (!mesh_header.m_bHasTex2)
							m_nTechniqueHandle = (mesh_header.m_bHasNormal ? TECH_SIMPLE_MESH_NORMAL : TECH_SIMPLE_MESH);
						else
							m_nTechniqueHandle = (TECH_SIMPLE_MESH_NORMAL_TEX2);
					}
				}
				lod.m_pStaticMesh->DeleteMeshFileData();
			}
		}

		// set technique
		if (m_nTechniqueHandle > 0 && m_asset->GetPrimaryTechniqueHandle() < 0)
		{
			m_asset->SetPrimaryTechniqueHandle(m_nTechniqueHandle);
		}

		// set aabb
		if (m_aabb.IsValid())
		{
			Vector3 vMin = m_aabb.GetMin();
			Vector3 vMax = m_aabb.GetMax();
			m_asset->SetAABB(&vMin, &vMax);
		}

		m_asset->UnLock();
		m_asset->RestoreDeviceObjects();
	}
	return S_OK;
}

HRESULT ParaEngine::CMeshProcessor::CleanUp()
{
	std::vector<MeshLOD>::iterator iCur, iEnd = m_MeshLODs.end();
	m_MeshLODs.clear();
	SAFE_DELETE(m_pParamBlock);
	return S_OK;
}

class SMeshCallBackData
{
public:
	SMeshCallBackData(MeshEntity * pAsset_) :pAsset(pAsset_){}

	MeshEntity * pAsset;

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

HRESULT ParaEngine::CMeshProcessor::Destroy()
{
	if (m_pAssetFileEntry != 0)
	{
		m_asset->SetState(AssetEntity::ASSET_STATE_SYNC_START);

		return m_pAssetFileEntry->SyncFile_Async(SMeshCallBackData(m_asset.get()));
	}

	CleanUp();
	return S_OK;
}

HRESULT ParaEngine::CMeshProcessor::Process(void* pData, int cBytes)
{
	m_pData = (char*)pData;
	m_cBytes = cBytes;
	return S_OK;
}

CParameterBlock* ParaEngine::CMeshProcessor::GetParamBlock(bool bCreateIfNotExist)
{
	if (m_pParamBlock)
	{
		return m_pParamBlock;
	}
	else
	{
		if (bCreateIfNotExist)
			m_pParamBlock = new CParameterBlock();
		return m_pParamBlock;
	}
}

bool ParaEngine::CMeshProcessor::CreateMeshLODLevel(float fromDepth, const string& sFilename)
{
	MeshLOD meshLOD;
	meshLOD.m_fromDepthSquared = fromDepth*fromDepth;
	meshLOD.m_sMeshFileName = sFilename;
#ifdef USE_DIRECTX_RENDERER
	meshLOD.m_pStaticMesh = new CParaXStaticMesh(sFilename.c_str());
#elif defined(USE_OPENGL_RENDERER)
	meshLOD.m_pStaticMesh = new CParaXStaticModel(sFilename.c_str());
#endif
	m_MeshLODs.push_back(meshLOD);

	std::sort(m_MeshLODs.begin(), m_MeshLODs.end(), MeshLodSortLess());
	return true;
}

// this is in IO thread. 
HRESULT ParaEngine::CMeshProcessor::CopyToResource()
{
	if (m_asset.get() == 0)
		return E_FAIL;
	bool bLoadHeaderFromXFile = false;

	if (m_asset->m_MeshLODs.size() == 0)
	{
		string filename = m_asset->GetLocalFileName();

		bLoadHeaderFromXFile = true;
		bool bSingleLOD = true;
		bool bIsXML = (CParaFile::GetFileExtension(filename) == "xml");
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
						bLoadHeaderFromXFile = false;
						m_aabb.SetMinMax(file.m_vMinPos, file.m_vMaxPos);
					}
					m_nTechniqueHandle = file.GetPrimaryShaderIndex();

					if (!file.m_paramBlock.IsEmpty())
					{
						CParameterBlock* pPerAssetParamBlock = GetParamBlock(true);
						if (pPerAssetParamBlock != 0)
						{
							(*pPerAssetParamBlock) = file.m_paramBlock;
						}
					}

					for (i = 0; i < file.GetSubMeshCount(); ++i)
					{
						CParaMeshXMLFile::CSubMesh* pSubLODMesh = file.GetSubMesh(i);
						CreateMeshLODLevel(pSubLODMesh->m_fToCameraDist, pSubLODMesh->m_sFileName);
						if (i == 0)
							filename = pSubLODMesh->m_sFileName;
					}
					if (file.GetSubMeshCount() >= 1)
						bSingleLOD = false;
					else
						OUTPUT_LOG("warning: LOD file %s does not contain any lod mesh references\n", filename.c_str());
				}
			}
			else
			{
				OUTPUT_LOG("failed loading mesh xml file: %s\n", filename.c_str());
				return E_FAIL;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// create the first level. 
		if (bSingleLOD)
			CreateMeshLODLevel(0.f, filename);
	}

	// preload file data to memory in the IO thread. 
	std::vector<MeshLOD> & pMeshLODs = m_MeshLODs.size() > 0 ? m_MeshLODs : m_asset->m_MeshLODs;
	std::vector<MeshLOD>::iterator iCur, iEnd = pMeshLODs.end();
	for (iCur = pMeshLODs.begin(); iCur != iEnd; ++iCur)
	{
		MeshLOD& lod = (*iCur);
		if (lod.m_pStaticMesh)
		{
			CParaFile myFile(lod.m_sMeshFileName.c_str());
			char* buffer = myFile.getBuffer();
			DWORD nFileSize = (DWORD)myFile.getSize();
			if (buffer == 0)
			{
				OUTPUT_LOG("warning: failed to load mesh file %s\n", lod.m_sMeshFileName.c_str());
				continue;
			}
			lod.m_pStaticMesh->SetMeshFileData(buffer, nFileSize);
			myFile.GiveupBufferOwnership();
		}
	}

	if (bLoadHeaderFromXFile && pMeshLODs.size() > 0)
	{
		MeshLOD& lod = pMeshLODs[0];
		if (lod.m_pStaticMesh)
		{
#ifdef USE_DIRECTX_RENDERER
			MeshHeader& mesh_header = lod.m_pStaticMesh->GetMeshHeader(CAsyncLoader::GetSingleton().GetFileParser());
			if (mesh_header.m_bIsValid)
			{
				m_aabb.SetMinMax(mesh_header.m_vMin, mesh_header.m_vMax);
				if (!mesh_header.m_bHasTex2)
					m_nTechniqueHandle = (mesh_header.m_bHasNormal ? TECH_SIMPLE_MESH_NORMAL : TECH_SIMPLE_MESH);
				else
					m_nTechniqueHandle = (TECH_SIMPLE_MESH_NORMAL_TEX2);
			}
#endif
		}
	}

	return S_OK;
}

void ParaEngine::CMeshProcessor::SetResourceError()
{
	if (m_asset.get() != 0)
	{
		m_asset->m_bIsValid = false;
	}
}
