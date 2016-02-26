//-----------------------------------------------------------------------------
// Class:	ParaXEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "ParaMeshXMLFile.h"
#include "ParaXAnimInstance.h"
#include "ParaXSerializer.h"
#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "ParaXEntity.h"

#include "memdebug.h"

using namespace ParaEngine;

/** place holder for being loaded parax models */
#define DEFAULT_PARAX_MODEL			"character/common/tag/tag.x"

ParaEngine::ParaXEntity::ParaXEntity(const AssetKey& key) :AssetEntity(key), m_nTechniqueHandle(-1)
{

}

ParaEngine::ParaXEntity::ParaXEntity()
{

}

ParaEngine::ParaXEntity::~ParaXEntity()
{

}

int ParaXEntity::GetPrimaryTechniqueHandle()
{
	return m_nTechniqueHandle;
}

void ParaXEntity::SetPrimaryTechniqueHandle(int nHandle)
{
	m_nTechniqueHandle = nHandle;
}

HRESULT ParaXEntity::InitDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;

	m_bIsInitialized = true;
	CreateModelFromFile_Async(NULL, NULL, NULL);
	return S_OK;
}

HRESULT ParaXEntity::DeleteDeviceObjects()
{
	m_bIsInitialized = false;

	std::vector<MeshLOD>::iterator iCur, iEnd = m_MeshLODs.end();
	for (iCur = m_MeshLODs.begin(); iCur != iEnd; ++iCur)
	{
		MeshLOD& lod = (*iCur);
		if(lod.m_pParaXMesh)
		{
			lod.m_pParaXMesh->DeleteDeviceObjects();
			lod.m_pParaXMesh.reset();
		}
	}
	return S_OK;
}

CAnimInstanceBase* ParaXEntity::CreateAnimInstance()
{
	CParaXAnimInstance* pAI = new CParaXAnimInstance();
	pAI->Init(this);
	return pAI;
}


void ParaXEntity::Refresh( const char* sFilename/*=NULL*/,bool bLazyLoad /*= false*/ )
{
	// TODO: not implemented yet. ParaX does not support remote sync at the moment
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

HRESULT ParaXEntity::CreateModelFromFile_Serial(RenderDevicePtr pDev, const char* sFileName)
{
	// Load Texture sequentially
	asset_ptr<ParaXEntity> my_asset(this);
	CParaXLoader loader_( my_asset, sFileName );
	CParaXLoader* pLoader = &loader_;
	CParaXProcessor processor_( my_asset );
	CParaXProcessor* pProcessor = &processor_;

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
	}
	else
	{
		pProcessor->SetResourceError();
	}
	pProcessor->Destroy();
	pLoader->Destroy();
	return S_OK;
}

HRESULT ParaXEntity::CreateModelFromFile_Async(void* pContext, RenderDevicePtr pDev, const char* sFileName)
{
	// Load Texture asynchronously
	asset_ptr<ParaXEntity> my_asset(this);
	if(pContext == 0)
		pContext = &(CAsyncLoader::GetSingleton());
	CAsyncLoader* pAsyncLoader = ( CAsyncLoader* )pContext;
	if( pAsyncLoader )
	{
		CParaXLoader* pLoader = new CParaXLoader( my_asset, sFileName);
		CParaXProcessor* pProcessor = new CParaXProcessor( my_asset );
		pProcessor->m_pDevice = pDev;

		pAsyncLoader->AddWorkItem( pLoader, pProcessor, NULL, NULL );
	}
	return S_OK;
}

void ParaXEntity::Init(const char* sFilename)
{
	if(sFilename)
		SetLocalFileName(sFilename);
}

const string& ParaXEntity::GetFileName()
{
	if(m_MeshLODs.size()>0)
	{
		return m_MeshLODs[0].m_sMeshFileName;
	}
	else
	{
		return GetLocalFileName();
	}
}

CParaXModel* ParaXEntity::GetModel( int nLODIndex/*=0*/ )
{
	LoadAsset();
	if(IsValid() && ((int)m_MeshLODs.size())>nLODIndex && nLODIndex>=0)
		return m_MeshLODs[nLODIndex].m_pParaXMesh.get();
	else
		return NULL;
}


bool ParaXEntity::IsLoaded()
{
	return GetModel()!=0;
}

void ParaXEntity::CreateMeshLODLevel( float fromDepth, const string& sFilename )
{
	MeshLOD meshLOD;
	meshLOD.m_fromDepthSquared = fromDepth*fromDepth;
	meshLOD.m_sMeshFileName = sFilename;
	
	m_MeshLODs.push_back(meshLOD);

	std::sort(m_MeshLODs.begin(), m_MeshLODs.end(), MeshLodSortLess());
}

void ParaXEntity::UpdateManualLodLevel( int index, const string& sFilename )
{
	if((int)m_MeshLODs.size()>index)
	{
		MeshLOD& lod = m_MeshLODs[index];
		if(lod.m_sMeshFileName != sFilename)
		{
			lod.m_sMeshFileName = sFilename;
			lod.m_pParaXMesh.reset();
		}
	}
}

int ParaXEntity::GetLodIndex( float depth ) const
{
	return GetLodIndexSquaredDepth(depth * depth);
}

int ParaXEntity::GetLodIndexSquaredDepth( float squaredDepth ) const
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

void ParaXEntity::RemoveLodLevels( void )
{
	if(m_MeshLODs.size()>1)
	{
		m_MeshLODs.resize(1);
	}
}

void ParaXEntity::Cleanup()
{
	m_MeshLODs.clear();
};


IAttributeFields* ParaXEntity::GetAttributeObject()
{
	return this;
}

bool ParaEngine::ParaXEntity::GetBoundingBox(Vector3* pMin, Vector3* pMax)
{
	CParaXModel* pMesh = GetModel();
	if (pMesh != NULL)
	{
		*pMin = pMesh->GetHeader().minExtent;
		*pMax = pMesh->GetHeader().maxExtent;
		return true;
	}
	return false;
}

int ParaEngine::ParaXEntity::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	return (int)m_MeshLODs.size();
}

IAttributeFields* ParaEngine::ParaXEntity::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	return GetModel(nRowIndex);
}

int ParaEngine::ParaXEntity::GetPolyCount()
{
	CParaXModel* pModel = GetModel();
	return (pModel) ? pModel->GetPolyCount() : 0;
}

int ParaEngine::ParaXEntity::GetPhysicsCount()
{
	CParaXModel* pModel = GetModel();
	return (pModel) ? pModel->GetPhysicsCount() : 0;
}

const char* ParaEngine::ParaXEntity::DumpTextureUsage()
{
	CParaXModel* pModel = GetModel();
	return (pModel) ? pModel->DumpTextureUsage() : CGlobals::GetString().c_str();
}

int ParaEngine::ParaXEntity::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	AssetEntity::InstallFields(pClass, bOverride);

	pClass->AddField("FileName", FieldType_String, NULL, (void*)GetFileName_s, NULL, NULL, bOverride);
	pClass->AddField("TextureUsage", FieldType_String, NULL, (void*)DumpTextureUsage_s, NULL, NULL, bOverride);
	pClass->AddField("PolyCount", FieldType_Int, NULL, (void*)GetPolyCount_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsCount", FieldType_Int, NULL, (void*)GetPhysicsCount_s, NULL, NULL, bOverride);
	return S_OK;
}


