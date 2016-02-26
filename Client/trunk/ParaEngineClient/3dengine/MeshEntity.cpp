//-----------------------------------------------------------------------------
// Class:	MeshEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "MeshObject.h"
#include "SceneState.h"
#include "ParaMeshXMLFile.h"
#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "MeshEntity.h"

#include "memdebug.h"

using namespace ParaEngine;

/** place holder for being loaded static models */
#define DEFAULT_STATIC_MODEL		"model/common/editor/scalebox.x"

HRESULT MeshEntity::InitDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;
	CreateMeshFromFile_Async(NULL, NULL, NULL);
	return S_OK;
}

HRESULT MeshEntity::RestoreDeviceObjects()
{
	if(IsValid() && GetPrimaryTechniqueHandle()>0) 
	{
		std::vector<MeshLOD>::iterator iCur, iEnd = m_MeshLODs.end();
		for (iCur = m_MeshLODs.begin(); iCur != iEnd; ++iCur)
		{
			MeshLOD& lod = (*iCur);
#ifdef USE_DIRECTX_RENDERER
			if(lod.m_pStaticMesh && (lod.m_pStaticMesh->m_pLocalMesh == NULL))
			{
				lod.m_pStaticMesh->InitDeviceObjects();
			}
#endif
		}
	}
	return S_OK;
}

HRESULT MeshEntity::InvalidateDeviceObjects()
{
	return S_OK;
}

HRESULT MeshEntity::DeleteDeviceObjects()
{
	m_bIsInitialized = false;

	std::vector<MeshLOD>::iterator iCur, iEnd = m_MeshLODs.end();
	for (iCur = m_MeshLODs.begin(); iCur != iEnd; ++iCur)
	{
		MeshLOD& lod = (*iCur);
		if(lod.m_pStaticMesh)
		{
			lod.m_pStaticMesh->Destroy();
		}
	}
	return S_OK;
}

void MeshEntity::Refresh( const char* sFilename/*=NULL*/,bool bLazyLoad /*= false*/ )
{
	m_bIsValid = true;
	if(sFilename != NULL && sFilename[0] != '\0')
	{
		SetLocalFileName(sFilename);
	}

	UnloadAsset();
	Cleanup(); // clean up LODs
	Init();
	if(!bLazyLoad)
		LoadAsset();
}

HRESULT MeshEntity::CreateMeshFromFile_Serial(RenderDevicePtr pDev, const char* sFileName)
{
	// Load Texture sequentially
	asset_ptr<MeshEntity> my_asset(this);
	CMeshLoader loader_( my_asset, sFileName );
	CMeshLoader* pLoader = &loader_;
	CMeshProcessor processor_( my_asset );
	CMeshProcessor* pProcessor = &processor_;

	pProcessor->m_pDevice = pDev;
	
	void* pLocalData;
	int Bytes;
	if( SUCCEEDED(pLoader->Load()) && 
		SUCCEEDED(pLoader->Decompress( &pLocalData, &Bytes )) && 
		SUCCEEDED(pProcessor->Process( pLocalData, Bytes )) && 
		SUCCEEDED(pProcessor->LockDeviceObject()) && 
		SUCCEEDED(pProcessor->CopyToResource()) && 
		SUCCEEDED(pProcessor->UnLockDeviceObject()) )
	{
		m_bIsValid = true;
	}
	else
	{
		pProcessor->SetResourceError();
	}
	pProcessor->Destroy();
	pLoader->Destroy();
	return S_OK;
}

HRESULT MeshEntity::CreateMeshFromFile_Async(void* pContext, RenderDevicePtr pDev, const char* sFileName)
{
	// temporarily making this mesh object invalid. 
	
	// Load Texture asynchronously
	asset_ptr<MeshEntity> my_asset(this);
	if(pContext == 0)
		pContext = &(CAsyncLoader::GetSingleton());
	CAsyncLoader* pAsyncLoader = ( CAsyncLoader* )pContext;
	if( pAsyncLoader )
	{
		CMeshLoader* pLoader = new CMeshLoader( my_asset, sFileName);
		CMeshProcessor* pProcessor = new CMeshProcessor( my_asset );
		pProcessor->m_pDevice = pDev;

		pAsyncLoader->AddWorkItem( pLoader, pProcessor, NULL, NULL );
	}
	return S_OK;
}

void MeshEntity::Init(const char* sFilename)
{
	if(sFilename)
		SetLocalFileName(sFilename);
}

void MeshEntity::Cleanup()
{
	m_MeshLODs.clear();
	SAFE_DELETE(m_pParamBlock);
}

int MeshEntity::GetPrimaryTechniqueHandle()
{
	return m_nTechniqueHandle;
}

void MeshEntity::SetPrimaryTechniqueHandle(int nHandle)
{
	m_nTechniqueHandle = nHandle;
}

CParameterBlock* MeshEntity::GetParamBlock( bool bCreateIfNotExist /*= false*/ )
{
	if(m_pParamBlock) 
	{
		return m_pParamBlock;
	}
	else
	{
		if(bCreateIfNotExist)
			m_pParamBlock = new CParameterBlock();
		return m_pParamBlock;
	}
}

const string& MeshEntity::GetFileName()
{
	if(m_MeshLODs.size()>0)
	{
		return m_MeshLODs[0].m_sMeshFileName;
	}
	else
	{
		static string empty;
		return empty;
	}
}

bool MeshEntity::IsLoaded()
{
	return GetMesh()!=0;
}

CParaXStaticModelRawPtr MeshEntity::GetMesh(int nLODIndex)
{
	LoadAsset();
	if(IsValid() && ((int)m_MeshLODs.size())>nLODIndex && nLODIndex>=0)
		return m_MeshLODs[nLODIndex].m_pStaticMesh.get();
	else
		return NULL;
}


ParaEngine::CParaXStaticModelPtr ParaEngine::MeshEntity::CreateMesh(const char* sFilename)
{
#ifdef USE_DIRECTX_RENDERER
	return  CParaXStaticModelPtr(new CParaXStaticMesh(sFilename));
#elif defined(USE_OPENGL_RENDERER)
	return CParaXStaticModelPtr(new CParaXStaticModel(sFilename));
#else
	return CParaXStaticModelPtr();
#endif
}


void MeshEntity::CreateMeshLODLevel( float fromDepth, const string& sFilename )
{
	MeshLOD meshLOD;
	meshLOD.m_fromDepthSquared = fromDepth*fromDepth;
	meshLOD.m_sMeshFileName = sFilename;
	meshLOD.m_pStaticMesh = CreateMesh(sFilename.c_str());

	m_MeshLODs.push_back(meshLOD);

	std::sort(m_MeshLODs.begin(), m_MeshLODs.end(), MeshLodSortLess());
}

void MeshEntity::UpdateManualLodLevel( int index, const string& sFilename )
{
	if((int)m_MeshLODs.size()>index)
	{
		MeshLOD& lod = m_MeshLODs[index];
		if(lod.m_sMeshFileName != sFilename)
		{
			lod.m_sMeshFileName = sFilename;
			lod.m_pStaticMesh = CreateMesh(sFilename.c_str());
		}
	}
}

int MeshEntity::GetLodIndex( float depth ) const
{
	return GetLodIndexSquaredDepth(depth * depth);
}

int MeshEntity::GetLodIndexSquaredDepth( float squaredDepth ) const
{
	if(m_MeshLODs.size()>1)
	{
		std::vector<MeshLOD>::const_iterator i, iend = m_MeshLODs.end();
		int index = 0;
		for (i = m_MeshLODs.begin(); i != iend; ++i, ++index)
		{
			if (i->m_fromDepthSquared > squaredDepth)
			{
				return index;
			}
		}
	}
	// If we fall all the way through, use the highest value
	return (int)(m_MeshLODs.size() - 1);
}

void MeshEntity::RemoveLodLevels( void )
{
	if(m_MeshLODs.size()>1)
	{
		m_MeshLODs.resize(1);
	}
}

IAttributeFields* MeshEntity::GetAttributeObject()
{
	return GetMesh();
}

bool ParaEngine::MeshEntity::GetBoundingBox(Vector3* pMin, Vector3* pMax)
{
	*pMin = GetAABBMin();
	*pMax = GetAABBMax();
	return true;
}
