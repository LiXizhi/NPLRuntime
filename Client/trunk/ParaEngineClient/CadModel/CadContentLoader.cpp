//-----------------------------------------------------------------------------
// Class:	
// Authors:	Clayman@paraengine.com
// Emails:	
// Date:	2007.11
// Revised: 2007.3
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <string>
#include "ICadModelImporter.h"
#include "CadContentLoader.h"
#include "CadImporterFactory.h"


namespace ParaEngine
{
	//////////////////////////////////////////////////////////////////////////
	//
	// CadModelLoader
	//
	//////////////////////////////////////////////////////////////////////////
	CadModelLoader::CadModelLoader(asset_ptr<CadModel>& pAsset,const char* fileName)
	{
		m_asset = pAsset;
		if(fileName)
			m_fileName = fileName;
	}

	CadModelLoader::~CadModelLoader()
	{
		CleanUp();
	}

	const char* CadModelLoader::GetFileName()
	{
		if(!m_fileName.empty())
		{
			return m_fileName.c_str();
		}
		else if(m_asset.get() != 0)
		{
			return m_asset->GetLocalFileName().c_str();
		}
		return NULL;
	}

	HRESULT CadModelLoader::Decompress(void** ppData,int* pcBytes)
	{
		return S_OK;
	}

	HRESULT CadModelLoader::CleanUp()
	{
		return S_OK;
	}

	HRESULT CadModelLoader::Destroy()
	{
		CleanUp();
		return S_OK;
	}

	HRESULT CadModelLoader::Load()
	{
		if(m_asset.get() == 0)
			return E_FAIL;
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	// CadModelProcessor
	//
	//////////////////////////////////////////////////////////////////////////
	CadModelProcessor::CadModelProcessor(asset_ptr<CadModel>& pAsset)
		:m_bytes(0),m_pData(NULL),m_pDevice(NULL),m_pAssetFileEntry(NULL),m_modelData(NULL)
	{
		m_asset = pAsset;
	}

	CadModelProcessor::~CadModelProcessor()
	{
		CleanUp();
	}

	HRESULT CadModelProcessor::LockDeviceObject()
	{
		if(m_asset.get()!=0 && m_asset->Lock())
		{
			return S_OK;
		}
		return E_FAIL;
	}

	IDirect3DDevice9* CadModelProcessor::GetRenderDevice()
	{
		return (m_pDevice!=0)?m_pDevice:CGlobals::GetRenderDevice();
	}

	HRESULT CadModelProcessor::UnLockDeviceObject()
	{
		if(m_asset.get()!=0)
		{
			std::string fileName = m_asset->GetLocalFileName();
			if(fileName.compare(CadModel::g_gridModelName)==0)
			{
				//Hard code to create a reference grid used in icad scene
				//we should really refactor this code to a generic way.
				CreateGridData();
				return S_OK;
			}

			IDirect3DDevice9* pDevice = GetRenderDevice();
			if(pDevice == 0)
				return E_FAIL;

			int32_t vertexCount = 0;
			int32_t indexCount = 0;
			for (size_t i=0;i<m_modelData->SubPartInfo.size();i++)
			{
				vertexCount += m_modelData->SubPartInfo[i].vertexCount;
				indexCount += m_modelData->SubPartInfo[i].primitiveCount * 3;
			}
			
			uint32_t vbSize = vertexCount * sizeof(VertexPositionNormal);
			pDevice->CreateVertexBuffer(vbSize,0,0,D3DPOOL_MANAGED,&(m_asset->m_pSharedVertexBuffer),NULL);
			void *pData;
			IDirect3DVertexBuffer9* vb = m_asset->m_pSharedVertexBuffer;
			vb->Lock(0,0,&pData,0);
			memcpy(pData,&m_modelData->VertexData[0],vbSize);
			vb->Unlock();

			uint32_t ibSize = indexCount * sizeof(uint32_t);
			pDevice->CreateIndexBuffer(ibSize,0,D3DFMT_INDEX32,D3DPOOL_MANAGED,&(m_asset->m_pSharedIndexBuffer),NULL);
			pData = NULL;
			IDirect3DIndexBuffer9* ib = m_asset->m_pSharedIndexBuffer;
			ib->Lock(0,0,&pData,0);
			memcpy(pData,&m_modelData->IndexData[0],ibSize);
			ib->Unlock();

			//todo:optimize to use single vertex declaration object across all cad model
			IDirect3DVertexDeclaration9* pVertexLayout;
			pDevice->CreateVertexDeclaration(VertexPositionNormal::g_VertexDesc,&pVertexLayout);

			m_asset->m_pVertexLayout = pVertexLayout;
			for(size_t i=0;i<m_modelData->SubPartInfo.size();i++)
			{
				SubPartInfo& info = m_modelData->SubPartInfo[i];
				m_asset->CreateCadModelPart(m_asset->m_pSharedVertexBuffer,m_asset->m_pSharedIndexBuffer,pVertexLayout,
					D3DPT_TRIANGLELIST,sizeof(VertexPositionNormal),info.vertexOffset,info.indexOffset,info.primitiveCount,
					info.vertexCount);
			}

			m_asset->m_vMax.x = m_modelData->AABBMax.x;
			m_asset->m_vMax.y = m_modelData->AABBMax.y;
			m_asset->m_vMax.z = m_modelData->AABBMax.z;

			m_asset->m_vMin.x = m_modelData->AABBMin.x;
			m_asset->m_vMin.y = m_modelData->AABBMin.y;
			m_asset->m_vMin.z = m_modelData->AABBMin.z;

			m_asset->m_nTechniqueHandle = TECH_SIMPLE_CAD_MODEL;
		}

		m_asset->UnLock();
		m_asset->RestoreDeviceObjects();
		return S_OK;
	}

	HRESULT CadModelProcessor::CleanUp()
	{
		if(m_modelData != NULL)
		{
			ICadModelImporter* pImporter = CadImporterFactory::GetImporter();
			pImporter->ReleaseModelData(m_modelData);
			m_modelData = NULL;
		}
		return S_OK;
	}

	class CadModelCallBackData
	{
	public:
		CadModelCallBackData(CadModel * pAsset_):pAsset(pAsset_){}

		CadModel * pAsset;

		void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
		{
			if(nResult == 0)
			{
				pAsset->SetState(AssetEntity::ASSET_STATE_NORMAL);
				pAsset->m_bIsInitialized = false;
				pAsset->InitDeviceObjects();
			}
		}
	};

	HRESULT CadModelProcessor::Destroy()
	{
		if(m_pAssetFileEntry != 0)
		{
			m_asset->SetState(AssetEntity::ASSET_STATE_SYNC_START);
			return m_pAssetFileEntry->SyncFile_Async(CadModelCallBackData(m_asset.get()));
		}
		CleanUp();
		return S_OK;
	}

	HRESULT CadModelProcessor::Process(void* pData,int bytes)
	{
		m_pData = (char*)pData;
		m_bytes = bytes;
		return S_OK;
	}

	HRESULT CadModelProcessor::CopyToResource()
	{
		if(m_asset.get() == 0)
			return E_FAIL;


		std::string fileName = m_asset->GetLocalFileName();
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(fileName);
		if(pEntry)
		{
			if(pEntry->DoesFileExist())
			{
				fileName = pEntry->GetLocalFileName();
			}
			else
			{
				m_pAssetFileEntry = pEntry;
				return E_PENDING;
			}
		}

		if(fileName.compare(CadModel::g_gridModelName)==0)
			return S_OK;

		ICadModelImporter* pImporter = CadImporterFactory::GetImporter();
		if(pImporter == NULL)
			return E_FAIL;

		if(pImporter->LoadFromFile(fileName.c_str(),30,&m_modelData))
			return S_OK;

		return E_FAIL;
	}

	void CadModelProcessor::SetResourceError()
	{
		if(m_asset.get()!=0)
		{
			m_asset->m_bIsValid = false;
		}
	}

	HRESULT CadModelProcessor::CreateGridData()
	{
		if(m_asset.get()!=0)
		{
			uint32_t gridCount = 24;
			float gridSpace = 0.5;

			IDirect3DDevice9* pDevice = GetRenderDevice();
			if(pDevice == 0)
				return E_FAIL;

			int32_t vertexCount = (gridCount + 1)*4;
			int32_t indexCount = (gridCount + 1)*4;

			VertexPosition* pVerts = new VertexPosition[vertexCount];
			uint16_t* pIndices = new uint16_t[indexCount];

			float halfGridWidth = gridCount*gridSpace*0.5f;
			int32_t idx = 0;
			for(uint32_t i=0;i<(gridCount+1);i++)
			{
				Vector3& pos0 = pVerts[idx].Position;
				pos0.x = halfGridWidth;
				pos0.y = 0;
				pos0.z = halfGridWidth - gridSpace*i;
				pIndices[idx] = idx;
				idx++;

				Vector3& pos1 = pVerts[idx].Position;
				pos1.x = -halfGridWidth;
				pos1.y = 0;
				pos1.z = halfGridWidth - gridSpace*i;
				pIndices[idx] = idx;
				idx++;

				Vector3& pos2 = pVerts[idx].Position;
				pos2.x = halfGridWidth - gridSpace*i;
				pos2.y = 0;
				pos2.z = halfGridWidth;
				pIndices[idx] = idx;
				idx++;

				Vector3& pos3 = pVerts[idx].Position;
				pos3.x = halfGridWidth - gridSpace*i;
				pos3.y = 0;
				pos3.z = -halfGridWidth;
				pIndices[idx] = idx;
				idx++;
			}
			
			uint32_t vbSize = vertexCount * sizeof(VertexPosition);
			pDevice->CreateVertexBuffer(vbSize,0,0,D3DPOOL_MANAGED,&(m_asset->m_pSharedVertexBuffer),NULL);
			void *pData;
			IDirect3DVertexBuffer9* vb = m_asset->m_pSharedVertexBuffer;
			vb->Lock(0,0,&pData,0);
			memcpy(pData,pVerts,vbSize);
			vb->Unlock();

			uint32_t ibSize = indexCount * sizeof(uint16_t);
			pDevice->CreateIndexBuffer(ibSize,0,D3DFMT_INDEX16,D3DPOOL_MANAGED,&(m_asset->m_pSharedIndexBuffer),NULL);
			pData = NULL;
			IDirect3DIndexBuffer9* ib = m_asset->m_pSharedIndexBuffer;
			ib->Lock(0,0,&pData,0);
			memcpy(pData,pIndices,ibSize);
			ib->Unlock();

			delete[] pVerts;
			delete[] pIndices;

			//todo:optimize to use single vertex declaration object across all cad model
			IDirect3DVertexDeclaration9* pVertexLayout;
			pDevice->CreateVertexDeclaration(VertexPosition::g_VertexDesc,&pVertexLayout);
			m_asset->m_pVertexLayout = pVertexLayout;

			CadModelPart* pModelPart = m_asset->CreateCadModelPart(m_asset->m_pSharedVertexBuffer,m_asset->m_pSharedIndexBuffer,pVertexLayout,
				D3DPT_LINELIST,sizeof(VertexPosition),0,0,(gridCount+1)*2,vertexCount);

			pModelPart->m_diffuseColor = LinearColor(0.5f,0.5f,0.5f,1.0f);
			pModelPart->m_effectPassId = 1;

			m_asset->m_vMax.x = halfGridWidth;
			m_asset->m_vMax.y = 0;
			m_asset->m_vMax.z = halfGridWidth;

			m_asset->m_vMin.x = -halfGridWidth;
			m_asset->m_vMin.y = 0;
			m_asset->m_vMin.z = -halfGridWidth;
			m_asset->m_nTechniqueHandle = TECH_SIMPLE_CAD_MODEL;
		}

		m_asset->UnLock();
		m_asset->RestoreDeviceObjects();
		return S_OK;
	}
}