#pragma once

#include "MeshEntity.h"

namespace ParaEngine
{
	class CParaXModel;

	struct ParaXEntity : public AssetEntity
	{
	public:
		ParaXEntity(const AssetKey& key);
		ParaXEntity();
		virtual ~ParaXEntity();
		ATTRIBUTE_DEFINE_CLASS(ParaXEntity);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);
		
		ATTRIBUTE_METHOD1(ParaXEntity, GetFileName_s, const char**)	{ *p1 = cls->GetFileName().c_str(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaXEntity, DumpTextureUsage_s, const char**)	{ *p1 = cls->DumpTextureUsage(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaXEntity, GetPolyCount_s, int*)	{ *p1 = cls->GetPolyCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaXEntity, GetPhysicsCount_s, int*)	{ *p1 = cls->GetPhysicsCount(); return S_OK; }
		

	public:
		friend class CParaXProcessor;

		virtual void Cleanup();
		virtual AssetEntity::AssetType GetType(){ return AssetEntity::parax; };

		CAnimInstanceBase* CreateAnimInstance();

		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();

		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded. 
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninialized list.  
		*/
		virtual bool IsLoaded();

		/** name of the model file(*.x) name holding the parax object
		get the mesh file name of the lowest level mesh. */
		const std::string& GetFileName();

		virtual IAttributeFields* GetAttributeObject();

		/**
		* refresh this texture surface with a local file. 
		* @param sFilename: if NULL or empty the old texture file(sTextureFileName) will be used. 
		* @param bLazyLoad if true it will be lazy loaded.
		*/
		void Refresh(const char* sFilename=NULL,bool bLazyLoad = false);

		/** get polycount of this mesh object */
		int GetPolyCount();
		/** get physics polycount of this mesh object */
		int GetPhysicsCount();
		/** get texture usage such as the number of textures and their sizes. */
		const char* DumpTextureUsage();

		/** Adds a new level-of-detail entry to this Mesh.
		@remarks
		As an alternative to generating lower level of detail versions of a mesh, you can
		use your own manually modeled meshes as lower level versions. This lets you 
		have complete control over the LOD, and in addition lets you scale down other
		aspects of the model which cannot be done using the generated method; for example, 
		you could use less detailed materials and / or use less bones in the skeleton if
		this is an animated mesh. Therefore for complex models you are likely to be better off
		modeling your LODs yourself and using this method, whilst for models with fairly
		simple materials and no animation you can just use the generateLodLevels method.
		@param fromDepth The z value from which this Lod will apply.
		@param meshName The name of the mesh which will be the lower level detail version.
		*/
		void CreateMeshLODLevel(float fromDepth, const std::string& sFilename);

		/** Changes the alternate mesh to use as a manual LOD at the given index.
		@remarks
		Note that the index of a LOD may change if you insert other LODs. If in doubt,
		use getLodIndex().
		@param index: The index of the level to be changed
		@param sFilename: The name of the mesh which will be the lower level detail version.
		*/
		void UpdateManualLodLevel(int index, const std::string& sFilename);

		/** Retrieves the level of detail index for the given depth value. 
		*/
		int GetLodIndex(float depth) const;

		/** Retrieves the level of detail index for the given squared depth value. 
		@remarks
		Internally, the LODs are stored at squared depths to avoid having to perform
		square roots when determining the lod. This method allows you to provide a
		squared length depth value to avoid having to do your own square roots.
		*/
		int GetLodIndexSquaredDepth(float squaredDepth) const;

		/** Removes all LOD data from this Mesh. only the lowest level remains */
		void RemoveLodLevels(void);

		/** get the highest level mesh if LOD is enabled. 
		* @param nLODIndex: default to 0, where the lowest LOD level mesh is returned.
		*/
		CParaXModel* GetModel(int nLODIndex=0);
		
		/** init the parax model. */
		void Init(const char* filename=NULL);

		/** this function is mostly used internally. It will load the mesh from disk, unpack it and init the entity. 
		* when this function returns, the texture will be already loaded to device pool. 
		* @param pDev: if NULL, the default render device is used 
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used. 
		*/
		HRESULT CreateModelFromFile_Serial(RenderDevicePtr pDev=NULL, const char* sFileName=NULL);

		/** this function is mostly used internally. 
		* this function will return immediately. It will append the texture request to AsyncLoaders's IO queue. 
		* @param pContext: this should be a pointer to CAsyncLoader
		* @param pDev: if NULL, the default render device is used 
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used. 
		*/
		HRESULT CreateModelFromFile_Async(void* pContext, RenderDevicePtr pDev = NULL, const char* sFileName = NULL);
		
		void SetPrimaryTechniqueHandle(int nHandle);
		int GetPrimaryTechniqueHandle();

		/** Get AABB bounding box of the asset object. if the asset contains an OOB, it will return true. */
		virtual bool GetBoundingBox(Vector3* pMin, Vector3* pMax);
	private:
		/// mesh objects in LOD list. each mesh may contain materials and textures, but you can simply 
		/// ignore them. The default setting is rendering with materials. See CParaXStaticMesh for more details
		std::vector<MeshLOD> m_MeshLODs;

		/** the primary technique handle*/
		int m_nTechniqueHandle;
	};

	typedef AssetManager<ParaXEntity>  ParaXEntityManager;
}