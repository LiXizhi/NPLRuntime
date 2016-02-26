#pragma once
#include "IAttributeFields.h"
#include "VertexFVF.h"

#include <set>
#include <map>

namespace ParaEngine
{
	/**
	* the unique key object for asset entity.
	*/
	typedef std::string AssetKey;

	/**
	* Base class for managed asset entity in ParaEngine.
	* We allow each entity to have one name shortcut of string type,
	* so that the entity can be easily referenced in script files through this name. 
	* we allow each asset to be associated with only one name, if multiple names are assigned to the
	* the name entity, the latest assigned names will override previous names. This could be limitation.
	* and will be removed in later version.
	* However, in the game engine runtime, scene node stores asset entity as pointer to them.
	* Entity is by default lazily initialized, unless they are called to be initialized.
	*/
	struct AssetEntity : public IAttributeFields
	{
	public:
		AssetEntity();
		AssetEntity(const AssetKey& key);
		virtual ~AssetEntity();
		ATTRIBUTE_DEFINE_CLASS(AssetEntity);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(AssetEntity, GetLocalFileName_s, const char**)		{ *p1 = cls->GetLocalFileName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(AssetEntity, SetLocalFileName_s, const char*)		{ cls->SetLocalFileName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(AssetEntity, GetRemoteAssetURL_s, const char**)		{ static std::string s;  s = cls->GetRemoteAssetURL().c_str(); *p1 = s.c_str(); return S_OK; }

		ATTRIBUTE_METHOD1(AssetEntity, IsValid_s, bool*)		{ *p1 = cls->IsValid(); return S_OK; }
		ATTRIBUTE_METHOD1(AssetEntity, IsLoaded_s, bool*)		{ *p1 = cls->IsLoaded(); return S_OK; }
		ATTRIBUTE_METHOD1(AssetEntity, IsLocked_s, bool*)		{ *p1 = cls->IsLocked(); return S_OK; }
		ATTRIBUTE_METHOD1(AssetEntity, IsInitialized_s, bool*)		{ *p1 = cls->IsInitialized(); return S_OK; }
		
		ATTRIBUTE_METHOD1(AssetEntity, GetState_s, int*)		{ *p1 = (int)cls->GetState(); return S_OK; }
		ATTRIBUTE_METHOD1(AssetEntity, SetState_s, int)		{ cls->SetState((AssetState)p1); return S_OK; }

	public:

		/** the asset state. an asset may be local or remote. if remote, it may take time for the asset to sync with the server. 
		* if an asset does not exist, we will try to fetch it via the central file server. 
		*/
		enum AssetState
		{
			/** asset is always created in normal state. */
			ASSET_STATE_NORMAL,
			/** the asset is local, which means that we have found a local disk version of the file. 
			we always try to find in local disk first*/
			ASSET_STATE_LOCAL,
			/** the asset is always remote. usually the asset key string begins with http:// */
			ASSET_STATE_REMOTE,
			/** asset is from the local cache. we have found the asset in disk cache, this means that we have sync it before. 
			* if file is not local, we will try to find in cache. */
			ASSET_STATE_CACHE,
			/** if file can not be found in local disk or in local cache, we will put it in sync start state, and make a url request to fetch from the server. 
			* During the fetching period, the asset is in this state, and we usually use a dummy placeholder for display. 
			*/
			ASSET_STATE_SYNC_START,
			/** if a remote file is downloaded to cache and loaded. it will be put to sync succeed state. This is the same as cache state. 
			*/
			ASSET_STATE_SYNC_SUCCEED,
			/** if a remote file can not be downloaded. We will put it in sync fail state. This will prevent us from checking with the remote server again. 
			*/
			ASSET_STATE_SYNC_FAIL,
			/** the file is not found or failed to load 
			*/
			ASSET_STATE_FAILED_TO_LOAD,
		};

		/** each asset type has a unique asset type number */
		enum AssetType
		{
			base=0,
			texture,
			mesh,
			multianimation,
			spritevertex,
			font,
			sound,
			mdx,
			parax,
			database,
			effectfile,
			dllplugin,
			datagrid,
			cadMesh,
			image,
		};
		virtual AssetType GetType(){return base;};

	public:
		virtual const std::string& GetIdentifier();

		/** call this function to automatically sync with the remote server according to current asset state 
		* @param bForceSync: if true, cache version is ignored. default to false. 
		* @param bLazyLoading: default to true. if true, it will immediately refresh texture when data is available. otherwise, we will wait until the next round. 
		* @return: whether a cache version is found. if not, a remote request is made. 
		*/
		virtual bool AutoSync(bool bForceSync = false, bool bLazyLoading=true);
		
		/** refresh the asset from local disk. Call this function to refresh the asset if the asset file has been modified since last load. 
		* When we finished synchronizing with the remote server, we will also call this function to refresh from file. 
		* @param sFilename: the new local file name to use. if NULL, the GetAssetFileName method will be used.
		*	calling this function will always causes the asset to reload from local disk file. 
		* @param bLazyLoad: default to false. if false, asset is only loaded when needed. 
		*/
		virtual void Refresh(const char* sFilename=NULL,bool bLazyLoad = false);

		/** the actual local file name. it could be the same as the asset key name or a dummy texture name, or a local cache texture name
		* in all conditions, this is either empty or a local disk file name. This function is overridden for asset that support remote sync. 
		* For local asset, this is the same as the key string. 
		*/
		virtual const string& GetLocalFileName();
		virtual void SetLocalFileName(const char* sFileName);

		/** get the remote server url of this asset. this is computed by concatenating asset server url and asset key */
		virtual string GetRemoteAssetURL();

		/** get effect parameter block with this object. 
		* @param bCreateIfNotExist: 
		*/
		virtual CParameterBlock* GetParamBlock(bool bCreateIfNotExist = false){return NULL;};


		/** whether this is a valid resource object. An invalid object may result from a non-exist resource file.*/
		bool IsValid() {return m_bIsValid;};

		/** whether initialized. */
		bool IsInitialized();
		
		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded. 
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninitialized list.  
		*/
		virtual bool IsLoaded();

		/**
		* call this function to safely release this asset. If there is no further reference to this object,
		* it will actually delete itself (with "delete this"). So never keep a pointer to this class after you 
		* have released it. A macro like SAFE_RELEASE() is advised to be used.
		*/
		virtual int Release();

		/** get the asset server url. In case we can not find an up-to-date asset, we will try to update from the server. 
		* if "", asset server will be disabled. 
		*/
		static const string& GetAssetServerUrl();

		/** set the asset server url. In case we can not find an up-to-date asset, we will try to update from the server. 
		* @param pStr: such as "http://assets.paraengine.com/"; if this is "", asset server will be disabled. 
		*/
		static void SetAssetServerUrl(const char* pStr);

	public:
		/** return the key object. */
		AssetKey& GetKey()
		{
			return m_key;
		}
		/** return the asset state. */
		AssetState GetState()
		{
			return m_assetState;
		}
		/** set the asset state. This is usually called automatically when asset is synced with the server. */
		void SetState(AssetState state)
		{
			m_assetState = state;
		}
		/** get the attribute object of this asset. */
		virtual IAttributeFields* GetAttributeObject();

		virtual HRESULT InitDeviceObjects(){m_bIsInitialized =true;return S_OK;};
		virtual HRESULT RestoreDeviceObjects(){return S_OK;};
		virtual HRESULT InvalidateDeviceObjects(){return S_OK;};
		virtual HRESULT DeleteDeviceObjects(){m_bIsInitialized =false;return S_OK;};
		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual HRESULT RendererRecreated() { return S_OK; };

		/** Clean up additional resources. 
		* this function will only be called before the destructor function.*/
		virtual void Cleanup(){};

		/** load asset. 
		* this function will only load the asset if it has not been initialized.
		* since ParaEngine uses lazy loading, it is highly advised that user calls this
		* function as often as possible to ensure that the asset is active; otherwise,
		* they may get invalid resource pointers. Some derived class will call this function
		* automatically, during resource pointer retrieval function.
		* E.g. During each frame render routine, call this function if the asset is used.*/
		void LoadAsset(){
			if(!m_bIsInitialized)
			{
				InitDeviceObjects();
				RestoreDeviceObjects();
				//m_bIsInitialized = true;
			}
		};
		/** unload asset. 
		* Normally this function is called automatically by resource manager. So never call this function
		* manually, unless you really mean it. */
		void UnloadAsset(){
			if(m_bIsInitialized)
			{
				InvalidateDeviceObjects();
				DeleteDeviceObjects();
				//m_bIsInitialized = false;
			}
		}
		/**
		* if its reference count is zero, unload this asset object.
		* any reference holder of this object can call this function to free its resources,
		* if they believe that it will not be needed for quite some time in future.
		*/
		void GarbageCollectMe(){
			if(m_refcount<=0)
			{
				UnloadAsset();
				m_refcount = 0;
			}
		}

		/** whether this is a locked resource resource. A resource is locked because we are loading it from disk to memory or we are downloading it or we are composing it. */
		inline bool IsLocked() {return m_bIsLocked;}

		/** Lock the object. Once an object is locked, it can not be initialized, or reinitialized. 
		* @param nMethod: how we are going to lock the asset. if 0(default), the asset should use place holder when used during the lock session. 
		* @return true if the asset is not previously locked. 
		*/
		virtual bool Lock(int nMethod=0);

		/** UnLock the object. 
		*/
		virtual void UnLock();

		/** Get AABB bounding box of the asset object. if the asset contains an OOB, it will return true. */
		virtual bool GetBoundingBox(Vector3* pMin, Vector3* pMax) { return false; };
	public:
		/** this is the unique key object. */
		AssetKey m_key;

		/** whether this entity is initialized;Entity is by default lazily initialized */
		bool		m_bIsInitialized;	

		/** whether this is a valid resource object. */
		bool		m_bIsValid;

	private:
		/** asset state: whether asset is being sync or not*/
		AssetState m_assetState;

		/** whether this is a locked resource. A resource is locked because we are loading it from disk to memory or we are downloading it or we are composing it. */
		bool		m_bIsLocked;

		/** the actual local file name. it could be the same as the asset key name or a dummy texture name, or a local cache texture name
		* in all conditions, this is either empty or a local disk file name. 
		*/
		string m_localfilename;
	};
}

#include "AssetManager.h"