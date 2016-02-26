#pragma once
#include "TerrainCommon.h"
#include "Terrain.h"
#include "Texture.h"

#include <stdlib.h>
#include <vector>

namespace ParaEngine
{
	struct TextureEntity;
	class CParaFile;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	//class Terrain;
	//class vector;
	//class Texture;

	/// \brief A TextureSet is a pool of Texture objects that are used by a Terrain to do detail texture rendering.

	/// You use TextureSet when your application needs to do detail painting (or "splats.")
	/// You can get a Terrain's TextureSet by calling Terrain::GetTextureSet(). Then you may add Texture objects to
	/// it by calling TextureSet::AddTexture() as many times as needed to fill it with all of the Texture objects you
	/// want to do detail painting with. Each of these Texture objects now has a unique index that you can get by
	/// calling Texture::GetSharedIndex(). Use this index to call the Terrain::Paint() method or any method that requires
	/// a shared index of a Texture.
	class TextureSet
	{
	public:
		/// \brief Constructs a new TextureSet.

		/// Since Terrain automatically creates one of these for you, you should rarely need to
		/// create new instances of this class manually. You might do so if you are making several
		/// instances of Terrain share a single TextureSet.
		TextureSet();
		~TextureSet();
		/// \brief Adds a Texture to the TextureSet.

		/// This gives the Texture a shared index which can be used to identify the Texture as a
		/// part of the Terrain's pool of shared Textures.
		int AddTexture(Texture * pTexture);
		
		/// \brief Gets a Texture by its shared index.
		/// \param index The index of the Texture to fetch from the pool. This is the value returned by Texture::GetSharedIndex().
		Texture *GetTexture(int index);
		/**
		* replace the texture at the given index.
		* @param nIndex: if there is no texture at the given index, the new texture will be appended. 
		* @param TextureAsset: the texture the replace with
		*/
		bool ReplaceTexture(int nIndex, ParaEngine::TextureEntity* TextureAsset);
		
		/** find the index of the given texture entity in the set, if not found returned -1*/
		int FindTexture(ParaEngine::TextureEntity* pTexture);
		
		/** unbind and delete all device texture objects. */
		int UnbindAllTextures();
		/// \brief Indicates how many Texture objects are in the TextureSet.
		int GetNumTextures();
				
		void WriteMask(CParaFile& file, Terrain * pTerrain);
		void ReadMask(CParaFile& file, Terrain * pTerrain);

		/** remove unused textures*/
		void GarbageCollect(Terrain * pTerrain);

		/** remove the given texture
		* @param bNormalizeOthers: if true(default), all other texture layers of pTerrain will be normalized to one. 
		*/
		void RemoveTexture(int nIndex, Terrain * pTerrain, bool bNormalizeOthers=true);
		
    private:
		  std::vector < Texture * >m_Textures;
		  
	};
}
