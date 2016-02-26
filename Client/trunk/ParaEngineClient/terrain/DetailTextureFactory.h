#pragma once
#include "TextureFactory.h"
#include "TextureEntity.h"
#include <list>
#include <vector>


namespace ParaTerrain
{
#define MAX_NUM_TEXTURE_LAYERS	4
class TextureFactory;
using namespace std;
using namespace ParaEngine;

/**
* this class implements a texture factory class for detailed tile based terrain surface texture.
* This high-resolution texture is has  16 or 32 times the resolution of the low-res surface which 
* is managed by the terrain class internally.
* Hence, This high-resolution texture can not be stored in one file, nor can it be persistent in the memory.
* it is always divided in square tiles. The naming conventions of these files are given below:
* "{file name} [x] [y].[jpg|bmp|...]", where x, y is the tile coordinates of the file in the terrain surface.
* e.g. "World 0 8.bmp" will be the detailed texture for the tile (0,8). All {file name} must be the same as 
* the low-res terrain texture file specified in the main terrain class. e.g. the low-res texture, in the above
* case, should be called "World.bmp".
* This factory will cache a total tile textures of m_nMaxTextureCache in memory.
*/
class CDetailTextureFactory
{
public:
	CDetailTextureFactory(void);
	~CDetailTextureFactory(void);
public:
	/** the low-res texture file path */
	string m_sFileName;
	/** the number of tiles in the x or y direction.They must be equal.*/
	int m_nTileNum;
	/** the size of each tile in world unit.*/
	float m_fTileSize;

	/**
	* index to he texture list for each layer
	*/
	struct HighResTextureInfo
	{
		/*
		* if the index is negative at m_TexturesMatrix[x,y], it means that there is no 
		* detailed texture at the tile [x,y] and that the low-res texture should be used instead */
		short layers[MAX_NUM_TEXTURE_LAYERS];
	};
	/** a m_nTileNumX*m_nTileNumY matrix, with the cell storing the texture index. 
	* if the index is negative at m_TexturesMatrix[x,y], it means that there is no 
	* detailed texture at the tile [x,y] and that the low-res texture should be used instead */
	HighResTextureInfo* m_TexturesMatrix;

	/** maximum number of tile textures to cache in memory*/
	int m_nNumTextureCache;
	/**
	* a cached texture item
	*/
	struct TextureCacheItem
	{
		/// index of the item, into the m_listTextures
		int nIndex;
		/// the texture object
		asset_ptr<TextureEntity> pTexture;
		/// a bits mask of number of times that this texture is hit in the last 32 render frames.
		/// For each render frame nHitCount = nHitCount>>1;
		/// for each get texture call: nHitCount |= 1<<31;
		DWORD nHitCount;
		TextureCacheItem(int nIndex, TextureEntity * pTexture)
		{
			this->nIndex = nIndex;
			this->pTexture = pTexture;
			this->nHitCount = 1<<31;
		}
		/** on hit */
		void OnHit(){
			this->nHitCount |= 1<<31;
		}
		/** this is called every rendering frame. the last 16 bits are count downs, while the first 16 bits are time relavent */
		void FrameMove(){
			if(this->nHitCount<0x0000ffff)
				this->nHitCount --;
			else
				this->nHitCount = this->nHitCount>>1;
		}
	};
	/** all cached items */
	list <TextureCacheItem> m_CachedTextureItems;

	/** all texture file path lists. */
	vector <string> m_listTextures;

private:
	/** delete the oldest cached texture item, if any.
	* i.e. the cache item with the smallest nHitCount value is deleted. */
	void DeleteOldestCacheItem();

public:
	/** Initialize the texture factory.
	* @params numTiles:	the number of tiles in both x and y direction.
	* @params fTileSize: the size of each tile in world unit
	*	usually they should be a power of 2. such as 16, 32
	*/
	void Init(int numTiles, float fTileSize);

	/** clear this class to uninitialized state */
	void cleanup();

	/** delete all textures in the cache */
	void DeleteAllTextures();

	/** check whether there is detailed texture at the specified world coordinates
	* if so its index is returned. If not,a negative number (-1) is returned.
	* @param nLayer: the layer number, up to 4. The default is the 0th layer. */
	int GetTextureIDAt(float originX, float originY, short nLayer=0);

	/** advance one frame, this is only used for managing texture cache hit counts. 
	* this function should be called once per frame. Here frame is means whenever the 
	* the terrain view changed.*/
	void AdvanceFrame();

public:
	TextureEntity * GetTexture(int index);

	void UnloadTexture(int index);
};
}