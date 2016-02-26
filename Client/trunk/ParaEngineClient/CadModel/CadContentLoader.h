
#pragma once
#include <string>
#include "ParaEngine.h"
#include "IDataLoader.h"
#include "AssetEntity.h"
#include "AssetManifest.h"
#include "ShapeAABB.h"
#include "CadModel.h"
#include "ICadModelImporter.h"

namespace ParaEngine
{
	class CadModel;
	class CadModelPart;

	class CadModelLoader:public IDataLoader
	{
	public:
		CadModelLoader(asset_ptr<CadModel>& pAsset,const char* fileName = NULL);
		~CadModelLoader();

		const char* GetFileName();
		HRESULT Decompress(void** ppData,int* pcBytes);
		HRESULT CleanUp();
		HRESULT Destroy();
		HRESULT Load();

	private:
		asset_ptr<CadModel> m_asset;
		std::string m_fileName;
	};

	class CadModelProcessor:public IDataProcessor
	{
	public:
		friend CadModel;
		CadModelProcessor(asset_ptr<CadModel>& pAsset);
		~CadModelProcessor();

		IDirect3DDevice9* GetRenderDevice();
		HRESULT CleanUp();

		HRESULT LockDeviceObject();
		HRESULT UnLockDeviceObject();
		HRESULT Destroy();
		HRESULT Process(void* pData,int bytes);
		HRESULT CopyToResource();
		void SetResourceError();

	private:
		asset_ptr<CadModel> m_asset;
		IDirect3DDevice9* m_pDevice;
		char* m_pData;
		int m_bytes;
		AssetFileEntry* m_pAssetFileEntry;
		CadModelData* m_modelData;

		HRESULT CreateGridData();
	};
}