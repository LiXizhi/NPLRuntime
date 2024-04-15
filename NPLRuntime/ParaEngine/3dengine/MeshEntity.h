#pragma once
#include "AssetEntity.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXStaticModel.h"

namespace ParaEngine
{
	/** A way of recording the way each LODs is recorded a static Mesh or ParaXMesh object. */
	struct MeshLOD
	{
	public:
		MeshLOD() :m_fromDepthSquared(0.f){}

		/// squared Z value from which this LOD will apply
		float m_fromDepthSquared;

		/// Only relevant if m_bIsLodManual is true, the name of the alternative mesh to use
		std::string m_sMeshFileName;

		/// Hard link to mesh to avoid looking up each time
		ref_ptr<CParaXStaticBase> m_pStaticMesh;
		CParaXModelPtr m_pParaXMesh;
	};

	struct MeshLodSortLess
	{
		bool operator() (const MeshLOD& mesh1, const MeshLOD& mesh2) const
		{
			// sort ascending by depth
			return mesh1.m_fromDepthSquared < mesh2.m_fromDepthSquared;
		}
	};

	//--------------------------------------------------------
	/// MeshEntity distinguish one template from other
	//--------------------------------------------------------
	struct MeshEntity : public AssetEntity
	{
	public:
		friend class CMeshProcessor;
		virtual AssetEntity::AssetType GetType(){return AssetEntity::mesh;};

		/**
		* refresh this texture surface with a local file. 
		* @param sFilename: if NULL or empty the old texture file(sTextureFileName) will be used. 
		* @param bLazyLoad if true it will be lazy loaded.
		*/
		void Refresh(const char* sFilename=NULL,bool bLazyLoad = false);

		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded. 
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninialized list.  
		*/
		virtual bool IsLoaded();

		/** get the mesh file name of the lowest level mesh. */
		const string& GetFileName();
		
		void SetPrimaryTechniqueHandle(int nHandle);
		int GetPrimaryTechniqueHandle();
		/** get effect parameter block with this object. 
		* @param bCreateIfNotExist: 
		*/
		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false);

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
		void CreateMeshLODLevel(float fromDepth, const string& sFilename);

		/** Changes the alternate mesh to use as a manual LOD at the given index.
		@remarks
		Note that the index of a LOD may change if you insert other LODs. If in doubt,
		use getLodIndex().
		@param index: The index of the level to be changed
		@param sFilename: The name of the mesh which will be the lower level detail version.
		*/
		void UpdateManualLodLevel(int index, const string& sFilename);

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
		/** this function is mostly used internally. It will load the mesh from disk, unpack it and init the entity. 
		* when this function returns, the texture will be already loaded to device pool. 
		* @param pDev: if NULL, the default render device is used 
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used. 
		*/
		HRESULT CreateMeshFromFile_Serial(RenderDevicePtr pDev=NULL, const char* sFileName=NULL);

		/** this function is mostly used internally. 
		* this function will return immediately. It will append the texture request to AsyncLoaders's IO queue. 
		* @param pContext: this should be a pointer to CAsyncLoader
		* @param pDev: if NULL, the default render device is used 
		* @param sFileName: if NULL, m_asset->GetLocalFileName() is used. 
		*/
		HRESULT CreateMeshFromFile_Async(void* pContext, RenderDevicePtr pDev = NULL, const char* sFileName = NULL);

		
	public:
		MeshEntity(const AssetKey& key)
			:AssetEntity(key),m_nTechniqueHandle(-1), m_pParamBlock(NULL), m_vMin(0,0,0), m_vMax(0,0,0)
		{
		}
		
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual void Cleanup();
		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual HRESULT RendererRecreated() override;

		virtual IAttributeFields* GetAttributeObject();

		/** get the highest level mesh if LOD is enabled. 
		* @param nLODIndex: default to 0, where the lowest LOD level mesh is returned.
		*/
		CParaXStaticBase* GetMesh(int nLODIndex = 0);
		ref_ptr<CParaXStaticBase> CreateMesh(const char* sFilename);

		/**
		* init the asset entity object.
		* @param sFilename: if NULL, the key string name is used. 
		*/
		void Init(const char* sFilename=NULL);

		/** set AABB */
		void SetAABB(const Vector3* vMin, const Vector3* vMax){ m_vMin = *vMin; m_vMax = *vMax; }

		const Vector3& GetAABBMin(){return m_vMin;}
		const Vector3& GetAABBMax(){return m_vMax;}

		/** Get AABB bounding box of the asset object. if the asset contains an OOB, it will return true. */
		virtual bool GetBoundingBox(Vector3* pMin, Vector3* pMax);
	public:
		Vector3 m_vMin;
		Vector3 m_vMax;

	private:
		/// mesh objects in LOD list. each mesh may contain materials and textures, but you can simply 
		/// ignore them. The default setting is rendering with materials. See CParaXStaticMesh for more details
		std::vector<MeshLOD> m_MeshLODs;

		/** the primary technique handle*/
		int m_nTechniqueHandle;

		/** effect param block */
		CParameterBlock* m_pParamBlock;
	};

	typedef AssetManager<MeshEntity> MeshEntityManager;
}