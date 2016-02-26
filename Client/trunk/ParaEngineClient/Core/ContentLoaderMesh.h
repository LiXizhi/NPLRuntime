#pragma once

#include "MeshEntity.h"
#include "ShapeAABB.h"
#include "IDataLoader.h"

namespace ParaEngine
{
	struct AssetFileEntry;

	/**
	* CMeshLoader implementation of IDataLoader
	* it will first search locally. If not found or version expired (as indicated in the assets_manifest file),
	* we will download from the asset update server.
	*/
	class CMeshLoader : public IDataLoader
	{
	private:
		friend struct MeshEntity;

		/** the asset file that this Mesh loader will update to*/
		asset_ptr<MeshEntity> m_asset;
		// if this is "", m_asset->GetLocalFileName() is used. 
		std::string m_sFileName;

	public:
		/**
		* @param sFileName: if this is "", m_asset->GetLocalFileName() is used.
		*/
		CMeshLoader(asset_ptr<MeshEntity>& pAsset, const char* sFileName = NULL);
		~CMeshLoader();

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
	* CMeshProcessor implementation of IDataProcessor
	*/
	class CMeshProcessor : public IDataProcessor
	{
	private:
		friend struct MeshEntity;

		/** the asset file that this Mesh loader will update to*/
		asset_ptr<MeshEntity> m_asset;
		/// if this is NULL, the default device is used. 
		RenderDevicePtr m_pDevice;

		char* m_pData;
		int m_cBytes;

		/// mesh objects in LOD list. each mesh may contain materials and textures, but you can simply 
		/// ignore them. The default setting is rendering with materials. See CParaXStaticMesh for more details
		std::vector<MeshLOD> m_MeshLODs;

		/** effect param block */
		CParameterBlock* m_pParamBlock;

		/** the primary technique handle*/
		int m_nTechniqueHandle;

		// aabb of the object
		CShapeAABB	m_aabb;

		/** the loader failed because the asset file the following file needs to be downloaded from the asset web server. */
		AssetFileEntry* m_pAssetFileEntry;
	private:
		bool PopulateMesh();

		CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);
		bool CreateMeshLODLevel(float fromDepth, const string& sFilename);

	public:
		CMeshProcessor(asset_ptr<MeshEntity>& pAsset);
		~CMeshProcessor();

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
	};
}
