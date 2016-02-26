//-----------------------------------------------------------------------------
// Class:	
// Authors:	Clayman@paraengine.com
// Emails:	
// Date:	2007.11
// Revised: 2007.3
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CadModel.h"
#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "CadContentLoader.h"

namespace ParaEngine
{
	const D3DVERTEXELEMENT9 VertexPositionNormal::g_VertexDesc[3] =
	{
		{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},
		{0,12,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL,0},
		D3DDECL_END(),
	};

	VertexPositionNormal::VertexPositionNormal():Position(0,0,0),Normal(0,1,0){}

	VertexPositionNormal::VertexPositionNormal(const Vector3 &pos,const Vector3 &normal)
		:Position(pos),Normal(normal){}

	VertexPositionNormal::VertexPositionNormal(float pX,float pY,float pZ,float nX,float nY,float nZ)
		:Position(pX,pY,pZ),Normal(nX,nY,nZ){}


	const D3DVERTEXELEMENT9 VertexPosition::g_VertexDesc[2] =
	{
		{0,0,D3DDECLTYPE_FLOAT3,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},
		D3DDECL_END(),
	};

	VertexPosition::VertexPosition():Position(0,0,0){}

	VertexPosition::VertexPosition(const Vector3 &pos)
		:Position(pos){}

	VertexPosition::VertexPosition(float pX,float pY,float pZ)
		:Position(pX,pY,pZ){}


	/////////////////////////////////////////////////////////
	///CadModelPart
	/////////////////////////////////////////////////////////
	CadModelPart::CadModelPart(CadModel *pParent,IDirect3DVertexBuffer9* vb,IDirect3DIndexBuffer9* ib,
		IDirect3DVertexDeclaration9* layout, bool sharedBuffer,D3DPRIMITIVETYPE primitiveType,
		uint16_t vertexStride,uint32_t vertexOfs, uint32_t indexOfs,uint32_t primitiveCount,uint32_t vertexCount):
		m_useSharedBuffer(sharedBuffer),
		m_pVertexBuffer(vb),
		m_pIndexBuffer(ib),
		m_pVertexLayout(layout),
		m_pParent(pParent),
		m_vertexStride(vertexStride),
		m_vertexOffset(vertexOfs),
		m_indexOffset(indexOfs),
		m_primitiveCount(primitiveCount),
		m_primitiveType(primitiveType),
		m_vertexCount(vertexCount),
		m_effectPassId(0),
		m_diffuseColor(1,141.0f/255.0f,25.0f/255.0f,1)
	{
	}
		

	CadModelPart::~CadModelPart()
	{
		if(!m_useSharedBuffer)
		{
			SAFE_RELEASE(m_pVertexBuffer);
			SAFE_RELEASE(m_pIndexBuffer);
			SAFE_RELEASE(m_pVertexLayout);
		}
	}

	//todo: replace IDirect3DDevice9 with sth like IDevice.
	void CadModelPart::SubmitGeometry(IDirect3DDevice9* pDevice)
	{
		pDevice->SetVertexDeclaration(m_pVertexLayout);
		pDevice->SetStreamSource(0,m_pVertexBuffer,m_vertexOffset*m_vertexStride,m_vertexStride);
		pDevice->SetIndices(m_pIndexBuffer);
		pDevice->DrawIndexedPrimitive(m_primitiveType,0,0,m_vertexCount,m_indexOffset,m_primitiveCount);
	}


	//////////////////////////////////////////////////////////////////////////
	///CadModel
	//////////////////////////////////////////////////////////////////////////
	const std::string CadModel::g_gridModelName("ReferenceGrid.iges");
	
	CParameterBlock* CadModel::GetParamBlockByIndex_(int16_t index)
	{
		if(index >= (int)(m_paramBlocks.size()))
		{
			return &m_paramBlocks[0];
		}
		else
		{
			return &m_paramBlocks[index];
		}
	}

	HRESULT CadModel::InitDeviceObjects()
	{
		if(m_bIsInitialized)
			return S_OK;

		m_bIsInitialized = true;
		CreateFromFile_Async(NULL,NULL,NULL);
		return S_OK;
	}

	HRESULT CadModel::RestoreDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CadModel::InvalidateDeviceObjects()
	{
		return S_OK;
	}

	HRESULT CadModel::DeleteDeviceObjects()
	{
		SAFE_RELEASE(m_pSharedVertexBuffer);
		SAFE_RELEASE(m_pSharedIndexBuffer);
		SAFE_RELEASE(m_pVertexLayout);
		return S_OK;
	}

	void CadModel::Refresh(const char* fileName/* =NULL */,bool lazyLoad/* =false */)
	{
		m_bIsValid = true;
		if(fileName != NULL && fileName[0] != '\0')
		{
			SetLocalFileName(fileName);
		}

		UnloadAsset();
		Cleanup();
		Init();
		if(!lazyLoad)
			LoadAsset();
	}

	HRESULT CadModel::CreateFromFile_Async(void* pContext, IDirect3DDevice9* pDev, const char* sFileName)
	{
		asset_ptr<CadModel> asset(this);
		if(pContext == 0)
			pContext = &(CAsyncLoader::GetSingleton());
		CAsyncLoader *pAsyncLoader = (CAsyncLoader*)pContext;
		if(pAsyncLoader)
		{
			CadModelLoader* pLoader = new CadModelLoader(asset,sFileName);
			CadModelProcessor* pProcessor = new CadModelProcessor(asset);
			pProcessor->m_pDevice = pDev;
			pAsyncLoader->AddWorkItem(pLoader,pProcessor,NULL,NULL);
		}
		return S_OK;
	}

	void CadModel::Init(const char* fileName)
	{
		if(fileName)
			SetLocalFileName(fileName);
	}

	void CadModel::Cleanup()
	{
		m_modelParts.clear();
		DeleteDeviceObjects();
	}

	bool CadModel::IsLoaded()
	{
		return m_modelParts.size() > 0;
	}

	const std::string& CadModel::GetFileName()
	{
		static string empty;
		return empty;
	}

	CParameterBlock* CadModel::GetParamBlock(bool bCreateIfNotExist /* = false */)
	{
		return NULL;
	}

	CadModelPart* CadModel::CreateCadModelPart(IDirect3DVertexBuffer9* sharedVB,IDirect3DIndexBuffer9* sharedIB,IDirect3DVertexDeclaration9* sharedLayout, 
		D3DPRIMITIVETYPE primitiveType,uint16_t vertexStride,uint32_t vertexOfs,uint32_t indexOfs,uint32_t primitiveCount,uint32_t vertexCount)
	{
		bool sharedBuffer = false;
		if(sharedVB != NULL)
			sharedBuffer = true;

		CadModelPart* part = new CadModelPart(this,sharedVB,sharedIB,sharedLayout,sharedBuffer,primitiveType,vertexStride,vertexOfs,indexOfs,primitiveCount,vertexCount);
		m_modelParts.push_back(part);
		return part;
	}

	bool CadModel::IsIgesFile(const char* fileName)
	{
		if(fileName != NULL)
		{
			int len = (int)strlen(fileName);
			if( (fileName[len-4] == 'i' && fileName[len-3] == 'g'&& fileName[len-2] == 'e'
				&& fileName[len-1]=='s') 
				||
				(fileName[len-3] == 'i' && fileName[len-2] == 'g'&& fileName[len-1]=='s')
				)
			{
				return true;
			}
		}
		return false;
	}

	bool CadModel::IsStepFile(const char* fileName)
	{
		if(fileName != NULL)
		{
			int len = (int)strlen(fileName);
			if( (fileName[len-4] == 's' && fileName[len-3] == 't'
				&& fileName[len-2] == 'e' && fileName[len-1] == 'p')
				||
				(fileName[len-4] == 's' && fileName[len-3] == 't' && fileName[len-1] == 'p')
				)
			{
				return true;
			}
		}
		return false;
	}
}
