//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.9
// Description:	API for common objects. 
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingGlobal.h"
#include <string>

namespace ParaEngine{
	struct AssetEntity;
	class CParameterBlock;
}

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;
	using namespace luabind;
	class ParaAssetObject;

	/** 
	* @ingroup ParaAsset
	*  a list of CParameter{name, value} pairs of anything.  
	*  usually used for DirectX effect parameter block. 
	*  value can be integer, float, vector3, vector4, matrix, TextureEntity, etc. 
	*/
	class PE_CORE_DECL ParaParamBlock
	{
	public:
		CParameterBlock * m_pParamBlock;
		ParaParamBlock():m_pParamBlock(NULL){};
		ParaParamBlock(CParameterBlock * pBlock):m_pParamBlock(pBlock){};

		/**
		* check if the object is valid
		*/
		bool IsValid();

		/** clear all parameters*/
		void Clear();

		void SetInt(const char* sParamName, int value);
		void SetBoolean(const char* sParamName, bool value);
		void SetFloat(const char* sParamName, float value);
		void SetVector2(const char* sParamName, float x, float y);
		void SetVector3(const char* sParamName, float x, float y, float z);
		void SetVector4(const char* sParamName, float x, float y, float z, float w);

		/** set matrix by a string of 4*3 number of float values separated by comma (see below): \n
		*	"mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43"
		*	If a blank string("") is specified, identity matrix is set
		*/
		void SetMatrix43(const char* sParamName, const char* matrix);
		/** setting known parameters to its predefined or current value. 
		@param sValue: known values such as "mat4Project", "mat4ProjectionInverse", "mat4Projection", "mat4ModelView", "mat4ModelViewInverse", "mat4ShadowMapTex", "vec3cameraPosition"
		*/
		void SetParam(const char* sParamName, const char* sValue);
		
		/**
		* set texture with the given texture index, 
		* @param nTextureIndex: usually [0,9], which texture register to use in case of effect file parameter. 
		* @param sFilePath: the file name of the texture. 
		*/
		void SetTexture(int nTextureIndex, const char* sFilePath);
		void SetTexture_(const char* sParamName, const char* sFilePath);

		/** same as SetTexture, except that ParaAssetObject is an object.*/
		void SetTextureObj(int nTextureIndex, const ParaAssetObject& assetObject);
	};

	/**
	* @ingroup ParaAsset
	* it represents an asset entity.
	*/
	class PE_CORE_DECL ParaAssetObject
	{
	public:
		AssetEntity * m_pAsset;
		
		ParaAssetObject():m_pAsset(NULL){};
		ParaAssetObject(AssetEntity * pEntity):m_pAsset(pEntity){};
		
		AssetEntity * GetAssetEntity();
		TextureEntity * GetTextureEntity();

		/** static helper functions:
		* @param texture: it can be string or a ParaAssetObject
		*/
		static TextureEntity * GetTexture(const object& texture);
		static TextureEntity * GetTextureWithParams(const object& texture);

		/**
		* check if the object is valid
		*/
		bool IsValid();

		/**
		* most assets are loaded asynchronously. This allows us to check if an asset is loaded. 
		* For example, we can LoadAsset() for a number of assets that need preloading. and then use a timer to check if they are initialized and remove from the uninialized list.  
		*/
		bool IsLoaded();

		/** whether the two objects are equal*/
		bool equals(const ParaAssetObject obj) const;
		
		/** reload the asset from file. Please note that for scene nodes which are currently using the asset entities,
		* they are not automatically updated. For example, the physics which depends on a mesh entity, will not be
		* automatically updated, once the mesh entity is reloaded. This function is almost solely used for debugging. 
		* @return: return true if the mesh is updated. 
		*/
		bool Reload();

		/** unload the asset from video and system memory. This is usually used for animated or one time texture entity.
		* Please note, asset is unloaded, but can still be used by other scene objects.The use of an unloaded object will cause
		* the object to be loaded again.*/
		void UnloadAsset();

		/**
		* Preload the asset to video and system memory, even though there is no scene object using the object in the previous frames.
		*/
		void LoadAsset();

		/**
		* currently, this function only takes effects on texture entity
		* refresh this entity with a local file. 
		* @param sFilename: if NULL or empty, the old file will be used. 
		*/
		void Refresh();
		void Refresh1(const char* sFilename);

		/**
		* call this function to safely release this asset. If there is no further reference to this object,
		* it will actually delete itself (with "delete this"). So never keep a pointer to this class after you 
		* have released it. A macro like SAFE_RELEASE() is advised to be used.
		*/
		void Release();

		/** get the reference count */
		int GetRefCount();

		/**
		* if its reference count is zero, unload this asset object.
		* any reference holder of this object can call this function to free its resources,
		* if they believe that it will not be needed for quite some time in future.
		*/
		void GarbageCollectMe();
		
		/** get the key name. this is usually the file name of the entity.  
		* return "" if it is not valid.*/
		string GetKeyName();
		/** this function shall never be called from the scripting interface. Solely used for managed exporting. */
		const char* GetKeyName_();

		/** get the file name. this is always the file name of the entity.  
		* return "" if it is not valid.*/
		string GetFileName();
		/** this function shall never be called from the scripting interface. Solely used for managed exporting. */
		const char* GetFileName_();


		/** get the asset type: 
			enum AssetType
			{
			base=0,
			texture=1,
			mesh=2,
			multianimation=3,
			spritevertex,
			font,
			sound,
			mdx,
			parax,
			database,
			effectfile,
			dllplugin,
			};
		* @return -1 is returned, if the asset is invalid.
		*/
		int GetType();

		/** set an integer handle to this asset. This is usually used by effect file asset. We can later assign mesh's primary technique handler using this value. 
		* please note that handles are not automatically created for most custom asset, one needs to manually create one. 
		* call this function multiple times with different handle, will associate the same asset with multiple handles. 
		* @param nHandle: TODO: if nHandle is -1, the system will automatically allocate a free handle for it and returned. 
		* @return: handle of this asset after successful set. 
		*/
		int SetHandle(int nHandle);

		/** Get the integer handle to this asset. if there are multiple handles, the first (smallest) handle is returned. 
		* if handle is not available. it will return -1 (INVALID handle). */
		int GetHandle();

		/** get the attribute object associated with the current asset object, such as getting the poly count, etc */
		ParaAttributeObject GetAttributeObject();

		//////////////////////////////////////////////////////////////////////////
		//
		// for effect entity 
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the parameter block. 
		* currently only effect and mesh entity file asset has effect param block. 
		* Currently the effect parameters can be set via ParaParamBlock interface from the scripting interface. 
		* we offer three levels of effect parameters: per effect file, per asset file, per mesh object. Effect parameters are also applied in that order. 
		* e.g. per effect file affects all object rendering with the effect file; per asset file affects all objects that use the mesh asset file; per mesh object affects only the mesh object. 
		*/
		ParaParamBlock GetParamBlock();

		/** only applies to effect entity: begin effect */
		bool Begin();
		/** only applies to effect entity: begin effect pass */
		bool BeginPass(int nPass);
		/** only used for effect file asset. 
		@param nIndex: texture stage
		@param filename: if "", it will set to NULL
		*/
		void SetTexture(int nIndex, const char* filename);

		/** only applies to effect entity: end effect pass */
		bool EndPass();
		/** only applies to effect entity: end effect*/
		bool End();

		/** this apply to changes.*/
		bool CommitChanges();

		//////////////////////////////////////////////////////////////////////////
		//
		// for TextureEntity
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* For animated textures. set the FPS for animation textures. this provides a short cut to animated textures
		* @param nFPS frames per seconds. default value is 15 FPS
		*/
		void SetTextureFPS(float FPS);
		/**
		* For animated textures. whether to enable texture animation. this provides a short cut to animated textures
		* @param bEnable default value is true. Set this to false, if one wants to manually animate the texture, such as controlling from the scripting interface.
		*/
		void EnableTextureAutoAnimation(bool bEnable);
		/**
		* For animated textures. set the current frame number. this provides a short cut to animated textures
		* @param nFrame 
		*/
		void SetCurrentFrameNumber(int nFrame);
		/**
		* For animated textures. Get the current frame number. this provides a short cut to animated textures
		* @return frame number is returned
		*/
		int GetCurrentFrameNumber();

		/**
		* For animated textures. Get the total frames in the animated texture. this provides a short cut to animated textures
		* @return frame number is returned
		*/
		int GetFrameCount();

		/**
		* get the texture width
		* @return 
		*/
		int GetWidth();
		/**
		* get the texture height
		* @return 
		*/
		int GetHeight();

		/** set the texture info (size) of the asset */
		void SetSize(int nWidth, int nHeight);

		/** get the bounding box (AABB) of the mesh or parax entity in object space. This function returns nothing if asset is not mesh or character entity. 
		* @param box: [in|out] a script table to receive the output. in the format: {min_x, min_y, min_z, max_x, max_y, max_z}
		*/
		object GetBoundingBox(const object& box);
		
		//////////////////////////////////////////////////////////////////////////
		//
		// for static mesh entity
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID. 
		* but it does not mean that all ID contains valid replaceable textures.  
		* This function can be used to quickly decide whether the model contains replaceable textures. 
		* Generally we allow 32 replaceable textures per model.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		int GetNumReplaceableTextures();

		/**
		* get the default replaceable texture by its ID. The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return invalid asset, if replaceable texture is not set before or ID is invalid.
		*/
		ParaAssetObject GetDefaultReplaceableTexture(int ReplaceableTextureID); 

		void SetCallback(int callbackType,const char * func);
	};


	/** 
	* @ingroup ParaAsset
	* ParaAsset namespace contains a list of HAPI functions to manage resources(asset)
	* used in game world composing, such as 3d models, textures, 
	* animations, sound, etc. Resources are used to create scene objects.
	* assets of the same type must have different names. Assets must be initialized before
	* they can be used, and this should be manually done in scripts by calling Init().
	*/
	class PE_CORE_DECL ParaAsset
	{
	public:
		/**
		* open the archive file(zip or pkg file) for further resource file searching. If any file can not be located
		* on the disk file system, it will go on searching for it in the archive file.
		* files in the archive file will be relative to the ParaEngine SDK root directory.
		* @param strFileName: the package file path name
		*/
		static bool OpenArchive(const char* strFileName);


		/**
		* Generate a pkg file which is equivalent to the specified zip file. 
		* This function can only be called, when a zip file can be successfully loaded. 
		* @param srcZip: the zip file name from which to generate. 
		* @param destPkg : destination file. if nil, it just rename the srcZip
		* @return true if successful. It will overwrite existing file. Output file is at the save directory and filename but with pkg file extension. 
		*/
		static bool GeneratePkgFile(const char* srcZip, const char* destPkg );

		/**
		* Generate a pkg file which is equivalent to the specified zip file.
		* This function can only be called, when a zip file can be successfully loaded.
		* @param srcZip: the zip file name from which to generate.
		* @param destPkg : destination file. if nil, it just rename the srcZip
		* @param nVersion: the pkg file version number. version 2 is recommended.
		* @return true if successful. It will overwrite existing file. Output file is at the save directory and filename but with pkg file extension.
		*/
		static bool GeneratePkgFile_(const char* srcZip, const char* destPkg, int nVersion);

		/**
		* open the archive file(zip or pkg file) for further resource file searching. If any file can not be located
		* on the disk file system, it will go on searching for it in the archive file.
		* @param strFileName: the package file path name
		* @param bUseRelativePath: if this is true, files in the archive file will be relative to the parent directory of archive path. 
		*/
		static bool OpenArchive2(const char* strFileName, bool bUseRelativePath);

		/** add archive to manager 
		* @param strFileName: path of the zip or pkg file. 
		* @param sRootDir: files in the archive will be regarded as relative to this this root directory. If this is "", there is no root directory set. 
		* such as "model/", "script/", characters after the last slash is always stripped off. 
		*/
		static bool OpenArchiveEx(const char* strFileName, const char* sRootDir);

		/** close an archive. When done with an archive, one should always close it. 
		Too many opened archives will compromise the IO performances. */
		static void CloseArchive(const string&  path);

		/** when an asset is not found, we will try to find it via this url.  
		* e.g. if asset is "model/test.dds", and asset url is "http://asset.paraengine.com/", then we will fetch the asset via "http://asset.paraengine.com/model/test.dds"
		*  if the asset path is "", asset server will be disabled. 
		*/
		static const char* GetAssetServerUrl();

		/** when an asset is not found, we will try to find it via this url.  
		* e.g. if asset is "model/test.dds", and asset url is "http://asset.paraengine.com/", then we will fetch the asset via "http://asset.paraengine.com/model/test.dds"
		*  if the asset path is "", asset server will be disabled. 
		* @param path:if the asset path is "", asset server will be disabled. 
		*/
		static void SetAssetServerUrl(const char* path);

		/** Garbage Collect all assets according to reference count. If the reference count is not maintained 
		* well by the user, this function is not effective as UnloadAll(). @see UnloadAll(). */
		static void GarbageCollect();
		/**
		* Unload an asset by its type name. once an unloaded asset is used again, its device resource will be reinitialized.
		* @param strAssetName: value and meaning are listed below
		- "*": Unload all assets. @see UnloadAll()
		- "ParaX": Unload all ParaX models
		- "StaticMesh": Unload all StaticMesh models
		- "Texture": Unload all Texture 
		- "Sound": Unload all Sound
		- "Font": Unload all Font
		// TODO: currently only "*" is supported.
		*/
		static void Unload(const char* assettype);
		/** unload all assets. once an unloaded asset is used again, its device resource will be reinitialized.
		* @see Unload() */
		static void UnloadAll();

		/** unload all databases. */
		static void UnloadDatabase();

		/**
		* initialize all objects that have not been created yet
		* NOTE: always call this function at least once when finished creating a batch of assets
		* assets created using any of the functions in this namespace can not be used until 
		* this function is called.
		*/
		static void Init();

		/**
		* Load effect file from a text or compiled HLSL file. It will return the old effect if effect is already loaded before. 
		*/
		static ParaAssetObject LoadEffectFile(const char* strAssetName, const char* strFilePath);

		/**
		* load an effect file by its name. it will return an invalid effect if the effect is not found. 
		*/
		static ParaAssetObject GetEffectFile(const char* strAssetName);

		static ParaAssetObject GetBlockMaterial(int32_t materialId);
		static ParaAssetObject CreateBlockMaterial();
		static ParaAssetObject CreateGetBlockMaterial(int32_t materialId);
		static void DeleteBlockMaterial(ParaAssetObject asset);
		/**
		* Load ParaX model asset, ParaX model file contains mesh, skeletal animation, etc.
		* Currently ParaX and mdx file format is supported, please refer to ParaX file document for
		* creating ParaX file based multianimation asset. 
		*/
		static ParaAssetObject LoadParaX(const char* strAssetName, const char* strFilePath);

		/**
		* Load Database asset. it must be sqlite database at the moment. 
		*/
		static ParaAssetObject LoadDatabase( const char* strAssetName, const char* strFilePath);

		/**
		* Load a X file based static mesh object. If any side of the mesh's bounding box is longer
		* than 50 meters(units) and that the triangle count is over 1500, 
		* Octree will be used to sort its triangle lists, otherwise no culling optimization will be
		* used when rendering the mesh. Static mesh is suitable for
		* rendering small repeated object such as stones, trees, or large object such as level mesh.
		* another usage is that it can be used as physical data to be fed to the physics engine, in which 
		* case simple and convex geometry should be used as much as possible.
		*/
		static ParaAssetObject LoadStaticMesh(const char* strAssetName, const char* strFilePath);

		/** get or load picking buffer */
		static ParaAssetObject LoadPickingBuffer(const char* strAssetName);

		/**
		* @note: we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
		* If one wants to ensure high resolution texture, use TGA format instead.
		* All dds textures are loaded with full mipmapping default. 
		* @param strAssetName 
		* @param strFilePath: if the file name ends with  _a{0-9}{0-9}{0-9}.xxx, it will be regarded as a texture sequence. 
		*  and the nSurfaceType will be ignored and forced to TextureSequence 
		* @param nSurfaceType 
		enum _SurfaceType
		{
		/// render target, call SetTextureInfo() to specify size. if SetTextureInfo()
		/// is never called, the back buffer size is used.
		RenderTarget = 0,		
		/// normal managed texture, with all mip-mapping level created
		StaticTexture = 1,
		/// a group of textures, such as xxx_a001.jpg, ..., xxx_a009.jpg
		TextureSequence = 2,
		/// texture in memory
		SysMemoryTexture,
		/// BLP textures
		BlpTexture,
		/// detailed terrain texture
		TerrainHighResTexture,
		/// cube texture for environment mapping, etc. 
		CubeTexture,
		}
		* @return 
		*/
		static ParaAssetObject LoadTexture(const char* strAssetName, const char* strFilePath, int nSurfaceType);
		static ParaAssetObject LoadRenderTarget(const char* strAssetName, int width, int height);
		/**
		* A sprite object must be created from Sprite frame and a texture. This is to create
		* the sprite frame, which tells the engine how the sprite is loaded in the texture.
		* @param nFrames: how many frames the sprite has
		* @param nRow: number of rows in the texture
		* @param nCol: number of columns in the texture
		*/
		static ParaAssetObject LoadSpriteFrame(const char* strAssetName, int nFrames, int nRow, int nCol);
		/**
		* load a system font, such as arial, times new man, etc. 
		* @param nFontSize: in pixels
		*/
		static ParaAssetObject LoadFont(const char* strAssetName, const char* FontName, int nFontSize);
		/**
		* load an image based font, not tested.
		*/
		static ParaAssetObject LoadImageFont(const char* strAssetName, const char* TextureName, int nSize, int nTxtColumns);
		/**
		* load a sound or music. The sound is not initialized until it is played for the first time.
		* @param bInit: whether to initialize the file
		*/
		static ParaAssetObject LoadSound(const char* strAssetName, const char* strFilePath, bool bInit);

		/**
		* give an alias name to a given font name. The most common use of this function is to replace the "System" font 
		* with a custom game font installed at "fonts/" folder. 
		* @param sLocalName: a local file name like "System", "Default"
		* @param sTypeFaceName: the real type face name to use when creating the font. 
		* please note that, the engine will search for the file "fonts/[sTypeFaceName].ttf", if this file exists, it will use that it, instead of the system installed font if any. 
		* Note: game custom font files under "fonts/" must be named by their true font name (i.e. type face name), otherwise they will not be loaded properly. 
		*/
		static void AddFontName(const string& sLocalName, const string& sTypeFaceName);

		//////////////////////////////////////////////////////////////////////////
		//
		// static functions: manager class functions.
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* Get provider id by name. Name is used when creating the provider instance. It is usually 
		* the same as the file path from which animation data is loaded. 
		* return -1 if not found
		*/
		static int GetBoneAnimProviderIDByName(const char* sName);

		/**
		* get the file name of a given animID. It may return NULL, if animID is invalid or is an internal animation id. 
		*/
		static const char* GetBoneAnimProviderFileNameByID(int nAnimID);

		/**
		* Create an animation provider from file. 
		* @param nAnimID: -1 if one wants it to be automatically assigned. otherwise one can manually specify one. 
		*  Please note, if there is already a provider with the same ID, the old one is automatically released and replaced with the new one. 
		* @param name: optional key. Allowing user to query by a user friendly name. This can be NULL.
		* @param filename: from which file the animation data is loaded. It can be a ParaX animation file or BVH file.
		* @param bOverwrite: whether to overwrite existing with the same nAnimID or name
		* @return: return the nAnimID, in case nAnim is set to -1. -1 is returned, if failed. 
		*/
		static int CreateBoneAnimProvider(int nAnimID, const char* name, const char* filename, bool bOverwrite);

		/** delete a provider by ID. 
		* @return: return true if succeed. 
		*/
		static bool DeleteBoneAnimProvider(int nAnimID);

		/** print all asset file to a given file. Each asset is on a single line, in the following format: 
		[AssetFileName]
		* @param strFileName: to which file the result is written to. if NULL or "", it is "temp/assets.txt"
		* @param dwSelection: bitwise on which assets to export, 1 is for texture, 2 is for Mesh, 4 is for ParaXEntity. Default to 0xffffffff
		* @return: the number of results printed are returned. 
		*/
		static int PrintToFile(const char* strFileName, DWORD dwSelection);

		/** refresh asset if it is already loaded. it will search for all refreshable asset type, such as textures and mesh, etc. 
		* if found, it will call the Refresh() method on the asset entity and return true, or return false. 
		*/
		static bool Refresh(const char* filename);
	};
}