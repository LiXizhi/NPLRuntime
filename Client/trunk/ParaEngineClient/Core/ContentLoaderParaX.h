#pragma once
#include "IDataLoader.h"

namespace ParaEngine
{
	struct AssetFileEntry;

	/**
	* CParaXLoader implementation of IDataLoader
	* it will first search locally. If not found or version expired (as indicated in the assets_manifest file),
	* we will download from the asset update server.
	*/
	class CParaXLoader : public IDataLoader
	{
	private:
		friend struct ParaXEntity;

		/** the asset file that this ParaX loader will update to*/
		asset_ptr<ParaXEntity> m_asset;
		// if this is "", m_asset->GetLocalFileName() is used. 
		std::string m_sFileName;

	public:
		/**
		* @param sFileName: if this is "", m_asset->GetLocalFileName() is used.
		*/
		CParaXLoader(asset_ptr<ParaXEntity>& pAsset, const char* sFileName = NULL);
		~CParaXLoader();

		/** clean up everything */
		HRESULT CleanUp();
		// overrides
	public:
		/** get file name */
		const char* GetFileName();
		/** Decompress is called by one of the processing threads to decompress the data.*/
		HRESULT Decompress(void** ppData, int* pcBytes);
		/** Destroy is called by the graphics thread when it has consumed the data. */
		HRESULT Destroy();
		/** Load is called from the IO thread to load data. */
		HRESULT Load();
	};

	/**
	* CParaXProcessor implementation of IDataProcessor
	*/
	class CParaXProcessor : public IDataProcessor
	{
	public:
		CParaXProcessor(asset_ptr<ParaXEntity>& pAsset);
		~CParaXProcessor();

		/** Get render device */
		RenderDevicePtr GetRenderDevice();

		/** clean up everything */
		HRESULT CleanUp();
		// overrides
	public:
		HRESULT LockDeviceObject();
		HRESULT UnLockDeviceObject();
		HRESULT Destroy();
		HRESULT Process(void* pData, int cBytes);
		HRESULT CopyToResource();
		void    SetResourceError();

	private:
		bool CreateMeshLODLevel(float fromDepth, const std::string& sFilename);

	private:
		friend struct ParaXEntity;

		/** the asset file that this ParaX loader will update to*/
		asset_ptr<ParaXEntity> m_asset;
		/// if this is NULL, the default device is used. 
		RenderDevicePtr m_pDevice;

		char* m_pData;
		int m_cBytes;

		/// mesh objects in LOD list. each mesh may contain materials and textures, but you can simply 
		/// ignore them. The default setting is rendering with materials. See CParaXStaticMesh for more details
		std::vector<MeshLOD> m_MeshLODs;

		/** the loader failed because the asset file the following file needs to be downloaded from the asset web server. */
		AssetFileEntry* m_pAssetFileEntry;
		int m_nTechniqueHandle;
	};
}
