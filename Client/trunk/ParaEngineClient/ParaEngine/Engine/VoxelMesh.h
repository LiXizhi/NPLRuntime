#pragma once
#include "TileObject.h"
#include "VoxelMesh/MetaWorldFragment.h"

namespace ParaEngine
{
	/** A voxel based mesh in the 3d scene. */
	class CVoxelMesh : 	public CTileObject
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::VoxelMesh;};
		CVoxelMesh(void);
		virtual ~CVoxelMesh(void);

		/** reset the object to its default settings.*/
		virtual void Reset();

		/** set the scale of the object. This function takes effects on both character object and mesh object. 
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);

		/**get scaling */
		virtual float GetScaling();

		/** init the object 
		@param pVoxelFile: the text file contains the grid data in the following format. 
		-- how many sells in the grid in x,y,z direction
		cellx = 32
		celly = 32
		cellz = 32
		-- index and value pairs. If an index is not defined, it is empty. 
		-- number of value pairs followed by index, value pairs on each line. 
		ValueCount = 32
		index,value;
		...
		index,value;
		
		@param vPos: the world position of the voxel mesh object. All voxel mesh are assumed to have the same predefined size, 
			which is usually a 32*32*32 grid matching the current global height map terrain tile grid. 
		@param ppTexture: the base texture entity to be replaced with the one in the voxel file. 
		*/
		HRESULT InitObject(const char* sVoxelFile, const DVector3& vPos, TextureEntity* pTexture = NULL);
		
		// -- for rendering
		virtual HRESULT Draw( SceneState * sceneState);

		virtual AssetEntity* GetPrimaryAsset(){return (AssetEntity*)m_pBaseTexture.get();};


		/** get the meta world fragment that this object is bound to. it will be created if not exists. */
		virtual MetaWorldFragment* GetMetaWorldFragment();

		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID. 
		* but it does not mean that all ID contains valid replaceable textures.  
		* This function can be used to quickly decide whether the model contains replaceable textures. 
		* Generally we allow 32 replaceable textures per model.
		* @note: This function will cause the mesh entity to be initialized.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		virtual int GetNumReplaceableTextures();

		/**
		* get the current replaceable texture by its ID.
		* if no replaceable textures is set before, this will return the same result as GetNumReplaceableTextures(). 
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID); 

		/**
		* set the replaceable texture at the given index with a new texture. 
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time. 
		* because default instance of the mesh may use different replaceable texture set. 
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one. 
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity); 


	private:
		/// file of the voxel file, we use lazy loading, only load from file on first draw call. 
		std::string m_sVoxelFile;
		
		/// the base texture entity to be replaced with the one in the voxel file. 
		asset_ptr<TextureEntity> m_pBaseTexture;

		/// the meta world fragments that contains a collection of meta objects all of which are rendered with an IsoSurface. 
		MetaWorldFragmentPtr m_pMetaWorldFragment;
	};
}

