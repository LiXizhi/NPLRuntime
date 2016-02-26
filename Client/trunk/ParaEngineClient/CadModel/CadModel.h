#pragma once

#include "AssetEntity.h"
#include "ParameterBlock.h"
#include "CadModelNode.h"
#include "CadContentLoader.h"

namespace ParaEngine
{
	class VertexPositionNormal
	{
	public:
		Vector3 Position;
		Vector3 Normal;
		static const D3DVERTEXELEMENT9 g_VertexDesc[3];

		VertexPositionNormal();
		VertexPositionNormal(const Vector3 &pos,const Vector3 &normal);
		VertexPositionNormal(float pX,float pY,float pZ,float nX,float nY,float nZ);
	};

	class VertexPosition
	{
	public:
		Vector3 Position;
		static const D3DVERTEXELEMENT9 g_VertexDesc[2];

		VertexPosition();
		VertexPosition(const Vector3 &pos);
		VertexPosition(float pX,float pY,float pZ);
	};
	
	class CadModelProcessor;
	class CadModelNode;
	class CadModel;

	class CadModelPart
	{
		friend CadModelProcessor;

	public:
		CadModelPart(CadModel *m_pParent,IDirect3DVertexBuffer9* vb,IDirect3DIndexBuffer9* ib,IDirect3DVertexDeclaration9* layout,
			bool sharedBuffer,D3DPRIMITIVETYPE primitiveType,uint16_t vertexStride,uint32_t vertexOfs,
			uint32_t indexOfs,uint32_t primitiveCount,uint32_t vertexCount);
		~CadModelPart();

		CParameterBlock* GetParamBlock();
		const LinearColor& GetDiffuseColor(){return m_diffuseColor;}
		int32 GetEffectPassId(){return m_effectPassId;}
		void SubmitGeometry(IDirect3DDevice9* pDevice);

	private:
		char* m_name;
		CadModel *m_pParent;		

		bool m_useSharedBuffer;
		uint16_t m_paramBlockIndex;
		uint16_t m_vertexStride;
		uint32_t m_vertexOffset;
		uint32_t m_indexOffset;
		uint32_t m_primitiveCount;
		uint32_t m_vertexCount;

		
		int m_nTechniqueHandle;
		uint8 m_effectPassId;
		LinearColor m_diffuseColor;
		
		D3DPRIMITIVETYPE m_primitiveType;
		IDirect3DVertexDeclaration9* m_pVertexLayout;
		IDirect3DVertexBuffer9* m_pVertexBuffer;
		IDirect3DIndexBuffer9* m_pIndexBuffer;
	
	};


	class CadModel :public AssetEntity
	{
		friend CadModelNode;
		friend CadModelProcessor;
	public:
		CadModel(const AssetKey& key)
			:AssetEntity(key),m_pVertexLayout(NULL),m_pSharedIndexBuffer(NULL),m_pSharedVertexBuffer(NULL)
		{

		}

		virtual AssetEntity::AssetType GetType(){return AssetEntity::cadMesh;};

		void Refresh(const char* fileName=NULL,bool loayLoad=false);

		virtual bool IsLoaded();

		const std::string& GetFileName();

		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual void Cleanup();
		void Init(const char* fileName=NULL);

		CParameterBlock* GetParamBlockByIndex_(int16_t index);
		HRESULT CreateFromFile_Async(void* pContext, IDirect3DDevice9* pDev=NULL, const char* sFileName=NULL);

		CadModelPart* CreateCadModelPart(IDirect3DVertexBuffer9* sharedVB,IDirect3DIndexBuffer9* sharedIB,IDirect3DVertexDeclaration9* sharedLayout,
			D3DPRIMITIVETYPE primitiveType,uint16_t vertexStride,uint32_t vertexOfs,uint32_t indexOfs,uint32_t primitiveCount,uint32_t vertexCount);

		int GetPrimaryTechniqueHandle(){return m_nTechniqueHandle;}

		void SetPrimaryTechniqueHandle(int nHandle){m_nTechniqueHandle = nHandle;}

		const static std::string g_gridModelName;

		static bool IsStepFile(const char* fileName);
		static bool IsIgesFile(const char* fileName);

	private:
		std::vector<CadModelPart*> m_modelParts;
		std::vector<CParameterBlock> m_paramBlocks;
		IDirect3DVertexDeclaration9* m_pVertexLayout;
		IDirect3DIndexBuffer9* m_pSharedIndexBuffer;
		IDirect3DVertexBuffer9* m_pSharedVertexBuffer;
		int m_nTechniqueHandle;
		Vector3 m_vMin;
		Vector3 m_vMax;
	};
}
