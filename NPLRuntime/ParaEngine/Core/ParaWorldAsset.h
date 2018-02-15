#pragma once
#include "TextureEntity.h"
#include "SpriteFontEntity.h"
#include "DatabaseEntity.h"
#include "ParaXEntity.h"
#include "MeshEntity.h"
#include "MiscEntity.h"
#include "SequenceManager.h"
#include "DynamicVertexBufferManager.h"
#include "effect_file.h"
#include "EffectManager.h"

#ifdef USE_DIRECTX_RENDERER
#include "HTMLBrowserManager.h"
#ifdef USE_FLASH_MANAGER
#include "FlashTextureManager.h"
#endif
#include "VoxelMesh/VoxelTerrainManager.h"
#include "CadModel/CadModel.h"
#include <vector>
namespace ParaEngine
{
	class LatentOcclusionQueryBank;
}
#endif

namespace ParaEngine
{
	class CVertexBufferPoolManager;
	class BufferPickingManager;
	struct CBufferPicking;
	using namespace std;

	/**
	* the asset manager class in ParaEngine.
	*
	* it contains sub managers for mesh, animated mesh, texture, font, sprite, database, buffer, effect, flash texture, occlusion query, archives, etc.
	* it also provides the get and load functions for all above asset types.
	* All assets are reference counted and can be garbage collected on demand. All device related asset are automatically restored when device changes.
	*
	* @note: each asset uses its asset file path as the internal key; it may also contain a shotcut key; however, in most cases, the shotcut key is empty.
	* shortcut key is used to get certain asset with a shorter and user friendly name.
	*/
	class CParaWorldAsset : public IAttributeFields
	{
	public:
		CParaWorldAsset(void);
		virtual ~CParaWorldAsset(void);

		ATTRIBUTE_DEFINE_CLASS(CParaWorldAsset);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CParaWorldAsset, IsAsyncLoading_s, bool*)	{ *p1 = cls->IsAsyncLoading(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaWorldAsset, SetAsyncLoading_s, bool)	{ cls->SetAsyncLoading(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CParaWorldAsset, IsAssetManifestEnabled_s, bool*)	{ *p1 = cls->IsAssetManifestEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaWorldAsset, EnableAssetManifest_s, bool)	{ cls->EnableAssetManifest(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CParaWorldAsset, IsUseLocalFileFirst_s, bool*)	{ *p1 = cls->IsUseLocalFileFirst(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaWorldAsset, SetUseLocalFileFirst_s, bool)	{ cls->SetUseLocalFileFirst(p1); return S_OK; }

		ATTRIBUTE_METHOD(CParaWorldAsset, DeleteTempDiskTextures_s)	{ cls->DeleteTempDiskTextures(); return S_OK; }
	public:
		static CParaWorldAsset* GetSingleton();

		TextureAssetManager& GetTextureManager();
		CVertexBufferPoolManager& GetVertexBufferPoolManager();
		TextureEntity* GetTexture(const string& sIdentifier);
		TextureEntity* GetDefaultTexture(int nTextureID);
		/**
		* @param sIdentifier: usually same as sTextureFileName
		* @param sTextureFileName: if the file name ends with  _a{0-9}{0-9}{0-9}.xxx, it will be regarded as a texture sequence.
		*  and the nSurfaceType will be ignored and forced to TextureSequence
		* @param nSurfaceType: default to TextureEntity::StaticTexture
		* @return
		*/
		TextureEntity* LoadTexture(const string&  sIdentifier, const string&  sTextureFileName, TextureEntity::_SurfaceType nSurfaceType = TextureEntity::StaticTexture);

		SpriteFontAssetManager& GetFontManager();
		SpriteFontEntity* GetFont(const string& sIdentifier);
		SpriteFontEntity* LoadGDIFont(const string&  sIdentifier, const string&  pstrFont, DWORD dwSize, bool bIsBold = true);

		DatabaseAssetManager& GetDatabaseManager();
		DatabaseEntity* GetDatabase(const string& sIdentifier);
		DatabaseEntity* LoadDatabase(const string&  sIdentifier, const string&  fileName);

		MeshEntityManager& GetMeshManager();
		MeshEntity* GetMesh(const string& sIdentifier);
		MeshEntity* LoadMesh(const string&  sIdentifier, const string&  sMeshFileName);

		ParaXEntityManager& GetParaXManager();
		ParaXEntity* GetParaX(const string& sIdentifier);
		ParaXEntity* LoadParaX(const string&  sIdentifier, const string&  fileName);
		/** the asset are feched from the current model asset database.*/
		ParaXEntity* LoadParaXByID(int nAssetID);

		CSequenceManager& GetSequenceManager(){ return m_SequenceManager; };
		SequenceEntity* LoadSequence(const string&  sName);

		EffectManager& GetEffectManager(){ return m_EffectsManager; };
		CEffectFile* LoadEffectFile(const string&  sIdentifier, const string&  sEffectFile);

		DynamicVertexBufferEntity* GetDynamicBuffer(DynamicVBAssetType nBufferType);

		BufferPickingManager& GetBufferPickingManager();
		CBufferPicking* GetBufferPick(const string& sIdentifier);
		CBufferPicking* LoadBufferPick(const string&  sIdentifier);

#ifdef USE_DIRECTX_RENDERER
		inline CHTMLBrowserManager& GetHTMLBrowserManager(){ return m_HTMLBrowserManager; };
		inline VoxelTerrainManager& GetVoxelTerrainManager(){ return m_VoxelTerrainManager; };
#ifdef USE_FLASH_MANAGER
		inline CFlashTextureManager& GetFlashManager(){ return m_FlashManager; };
#endif	
		//-- get resource primitives
		D3DXSpriteEntity* GetD3DXSprite(const string& sIdentifier);
		CadModel* LoadCadModel(const string&  sIdentifier, const string&  sMeshFileName);

		D3DXSpriteEntity* LoadD3DXSprite(const string&  sIdentifier, int nFrames, int nRow, int nCol);

		/** create an X file parser. One only needs one parser per thread. One needs to manually call SAFE_RELEASE() with the returned object.
		* for the render thread, use GetParaXFileParser()
		*/
		void CreateXFileParser(LPD3DXFILE* ppParser);
		/** get the x file parser with ParaX file templates registered */
		LPD3DXFILE GetParaXFileParser();

		LPDIRECT3DVERTEXBUFFER9 GetShadowSquareVB(){ return m_pShadowSquareVB; }

		/** this will return NULL, if the device does not support occlusion query. */
		LPDIRECT3DQUERY9		GetOcclusionQuery(){ return m_pOcclusionQuery; }
		/** this will return NULL, if the device does not support occlusion query.
		* TODO: currently there is only one query bank, implement multiple banks for different ID.
		* @param nID:
		*	- 0 is reserved for scene object.
		*	- 1 is reserved for ocean object.
		*/
		LatentOcclusionQueryBank* GetOcclusionQueryBank(int nID = 0);

		/** print all asset file to a given file. Each asset is on a single line, in the following format:
		[AssetFileName]
		* @param pOutputFile: to which file object the result is written to
		* @param dwSelection: bitwise on which assets to export, 1 is for texture, 2 is for Mesh, 4 is for ParaXEntity. Default to 0xffffffff
		* @return: the number of results printed are returned.
		*/
		int PrintToFile(CParaFile* pOutputFile, DWORD dwSelection = 0xffffffff);
#endif

	public:
		/** refresh asset if it is already loaded. it will search for all refreshable asset type, such as textures and mesh, etc.
		* if found, it will call the Refresh() method on the asset entity and return true, or return false.
		*/
		bool RefreshAsset(const char* filename);

		/**
		* called before every render frame
		* @param fElapsedTime in seconds
		*/
		void RenderFrameMove(float fElapsedTime);

		/** initialize all assets created so far to accelerate loading during game play. */
		void LoadAsset();
		/** uninitialize all assets created so far to save some memory */
		void UnloadAsset();

		/** unload a given asset by key name. This function is rarely used, except for some specify situation,
		* such as Loading a new replacement file that overwrite existing asset entity.
		* currently only texture and model files are supported.
		* @param keyname: the asset key, we will automatically determine the asset type by its extension (assuming key is filename)
		* @return: true if we have found one and unloaded the asset. Please note, we will only unload if the asset is not being downloaded (asynchronously).
		*/
		bool UnloadAssetByKeyName(const string& keyname);

		/** Garbage Collect(free resources of) all unused entity.*/
		void GarbageCollectAll();

		void	Cleanup();

		HRESULT InitDeviceObjects();	// device independent
		HRESULT RestoreDeviceObjects(); // device dependent
		HRESULT InvalidateDeviceObjects();
		HRESULT DeleteDeviceObjects();
		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		HRESULT RendererRecreated();

		/**
		* given a source asset, it will first see if the asset already exist.
		* if the file does not exist, it will search if there is a mapping in the m_AssetMap,
		* if there is no valid, it will search the disk using the file name and extensions; if the file name is found somewhere else, it will automatically generate a new mapping item in m_AssetMap,
		* m_AssetMap will be read and written to disk file assetmap.txt at loading and exiting time
		* @note: Normally this function is only called when m_bUseAssetSearch is true. Both the asset load and get function in the asset manager classes need to call this function.
		* @param AssetFile: [in|out] asset file to search for, the result file will be assigned to it as well.
		* @param searchDir: where to search the file if it is found.
		* @return: return true if found.
		*/
		bool DoAssetSearch(string& AssetFile, const char* searchDir);

		/** expose attribute model for all asset manager types */
		void CreateAttributeModel();

		/** whether to enable async loading for all subsequently loaded assets, such as texture, etc. Enabled by default. */
		bool IsAsyncLoading() const;
		void SetAsyncLoading(bool val);

		/** whether to enable asset manifest files */
		bool IsAssetManifestEnabled() const;
		void EnableAssetManifest(bool val);

		/** if true, asset manifest's GetFile() will return null, if a local disk or zip file is found even there is an entry in the assetmanifest. */
		bool IsUseLocalFileFirst() const;
		void SetUseLocalFileFirst(bool val);
	private:
		void SaveAssetFileMapping();
		void DeleteTempDiskTextures();

	public:
		typedef boost::signals2::signal<void()>  DeviceEvent_Callback_t;

		DeviceEvent_Callback_t OnInitDeviceObjects;
		DeviceEvent_Callback_t OnRestoreDeviceObjects;
		DeviceEvent_Callback_t OnInvalidateDeviceObjects;
		DeviceEvent_Callback_t OnDeleteDeviceObjects;
		DeviceEvent_Callback_t OnRendererRecreated;
		DeviceEvent_Callback_t OnCleanup;

	protected:
		/**
		* this will be set to true when the manager class is created if there is a fill called temp/assetmap.txt
		* if this is true, all mesh and paraX mesh files will be checked for existence
		* if the file does not exist, it will search if there is a mapping in the m_AssetMap,
		* if there is no valid, it will search the disk using the file name and extensions and automatically generate a new mapping item,
		* m_AssetMap will be read and written to disk file temp/assetmap.txt at loading and exiting time
		*/
		bool m_bUseAssetSearch;

		/**
		* this is only used when m_bUseAssetSearch is true.
		* m_AssetMap will be read and written to disk file temp/assetmap.txt at loading and exiting time
		* file format: on each line, it is string=string, where the second string may be none.
		*/
		map<string, string>	m_AssetMap;

	private:
		/** array of all asset manager weak references. */
		vector<IAttributeFields*> m_attribute_models;

		CSequenceManager						m_SequenceManager;
		EffectManager							m_EffectsManager;
		DynamicVertexBufferManager				m_DynamicVBManager;
		bool m_bAsyncLoading;

#ifdef USE_DIRECTX_RENDERER
		/// dynamic buffer for MDX
		/// shadow square, a square that is the same size of the back buffer for shadow rendering
		LPDIRECT3DVERTEXBUFFER9 m_pShadowSquareVB;
		/// for occlusion query
		LPDIRECT3DQUERY9		m_pOcclusionQuery;
		vector<LatentOcclusionQueryBank*> m_pOcclusionQueryBanks;

		AssetManager<D3DXSpriteEntity>			m_D3DXSpriteManager;
#ifdef USE_FLASH_MANAGER
		CFlashTextureManager					m_FlashManager;
#endif
		CHTMLBrowserManager						m_HTMLBrowserManager;
		VoxelTerrainManager						m_VoxelTerrainManager;
		AssetManager<CadModel>					m_CadModelManager;
		LPD3DXFILE								m_pDXFile;	// for parsing X file
#endif
	};

}