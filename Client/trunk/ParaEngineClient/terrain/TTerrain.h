#pragma once

namespace ParaTerrain
{
	class Terrain;
}

namespace ParaEngine
{
	using namespace ParaTerrain;
	/**
	* temp height field terrain data used by terrain filters. 
	* the Input and Output of terrain filters will be passed in this structure. 
	* terrain data may be saved in normalized space.
	*/
	class TTerrain
	{
	private:
		/** lowest and highest point in the terrain data.*/
		float			fMin,fMax;
		/** y scale. this is the same as (fMax - fMin) */
		float           y_scale_factor;
		/** whether height field data is normalized */
		bool bIsNormalized;

	public:
		/** size of the terrain. Usually Width equals to Height */
		int             width, height;
		float			fVertexSpacing;
		float*			heightfield;
		float*			selection;

		/** sea level */
		float           sealevel;
	public:
		inline float* GetHeightmap(){return heightfield;};
		/** it is good practice to normalize terrain height field before computation, 
		* this will increase floating point accuracy, especially for those which have large average height value.
		* it normalized terrain and unnormalized terrain are the same. 
		* the terrain height data [fMin, fMax] is normalized to [0,1]
		* i.e. Normalize(h) = (h-fMin)/y_scale_factor;
		* fMin,fMax, y_scale_factor will be recomputed.
		*/
		void Normalize();
		/** normalize a height using current setting */
		void NormalizeHeight(float& height);
		/** recover normalized data back: 
		* i.e. Recover(h) = (h*y_scale_factor+fMin) */
		void Recover();
		/** create an unnormalized plane containing gridSize*gridSize number of vertices */
		bool CreatePlane(int gridSize, float fHeight, float fVertexSpacing_=1 );
		/**
		* load height field from file
		* @param filename : the raw elevation or gray scale image file that contains the height field. 
		*/
		bool CreatePlane(const char* filename,float fVertexSpacing = 1);

		/** set the height at the specified gird position. No boundary check is performed. */
		void SetHeight(int x, int y, float fValue);
		/** Get the height at the specified gird position. No boundary check is performed. 
		* the height value returned is the same as what is stored in the height field. It may be normalized or unnormalized.
		*/
		float GetHeight(int x, int y);
		/** get the unnormalized (final) height at the specified gird position. No boundary check is performed. */
		float GetHeightFinal(int x, int y);

		float GetVertexSpacing();
		void SetVertexSpacing(float fSpacing);

		float GetMinHeight();
		void SetMinHeight(float fMinHeight);

		float GetMaxHeight();
		/** Note: if 0 is returned, it means that there is no scale. */
		float GetYScale();
		int GetGridSize();

		/** set whether the heightfield data is normalized.*/
		void SetNormalized(bool bSet);
		/** whether height field data is normalized */
		bool IsNormalized();

		/** get the grid size*/
		int GetGridWidth(){return width;}
		int GetGridHeight(){return height;}
	public:
		TTerrain();
		~TTerrain();
		/** delete terrain data */
		void Cleanup();

		friend class CTerrainFilters;
	};

	/**
	* a cached Terrain tile item
	*/
	struct TerrainTileCacheItem
	{
		/// the texture object
		union
		{
			Terrain * pTerrain;
			TTerrain* pHeightmap;
		};

		/// a bits mask of number of times that this tile is hit in the last 32 render frames.
		/// For each render frame nHitCount = nHitCount>>1;
		/// for each get texture call: nHitCount |= 1<<31;
		DWORD nHitCount;
		TerrainTileCacheItem(Terrain * pTerrain)
		{
			this->pTerrain = pTerrain;
			this->nHitCount = 1<<31;
		}
		TerrainTileCacheItem(TTerrain * pHeightmap)
		{
			this->pHeightmap = pHeightmap;
			this->nHitCount = 1<<31;
		}
		TerrainTileCacheItem()
		{
			this->pTerrain = NULL;
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
}