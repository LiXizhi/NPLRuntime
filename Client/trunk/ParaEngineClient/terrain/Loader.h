#pragma once
#include "Terrain.h"


namespace ParaTerrain
{
	/// \brief The Loader is a Singleton object that provides access to ElevationLoaders and TextureLoaders.

	/// Loader dynamically loads DLL's (or shared objects) when their services are requested. It forwards along
	/// the loading parameters passed by the application to the dynamically loaded ElevationLoader or TextureLoader
	/// library. Since ElevationLoader and TextureLoader libraries are loaded dynamically, it is your responsibility
	/// to ensure that your application provides the required .DLL or .so files along with your install.
	/// See the documentation at terrainengine.com for a discussion of this topic.
	class Loader
	{
	      public:
		/// \brief Returns the Singleton instance of the Loader class.
		static Loader *GetInstance();
		/**
		*
		* @param pTerrain [out] output terrain
		* @param szFilename : file name to extract the data. it can either be a gray scale image or 
		*   a raw elevation file containing just float value arrays. 
		* @param fTerrainSize size of the terrain
		* @param elevationScale 
		* @param swapVertical whether swap vertically of the loaded height map
		* @return S_OK if succeeded.
		*/
		HRESULT LoadElevations( Terrain * pTerrain, const char * szFilename, float fTerrainSize, float elevationScale, bool swapVertical=TRUE );

		/**
		* load elevation to a buffer. 
		* @param ppImageData [out]: the out height field buffer. the caller is responsible for deleting the buffer using delete [] function. 
		* @param nSize [out]: the output size of the terrain.
		* @param szFilename : file name to extract the data. it can either be a gray scale image or a raw elevation file containing just float value arrays. 
		* @param swapVertical whether swap vertically of the loaded height map
		* @return S_OK if succeeded.
		*/
		HRESULT LoadElevations( float **ppImageData, int* nSize, const char * szFilename, bool swapVertical=TRUE);

		/// [absoleted: holes are specified in terrain config file]
		/// Load the terrain hole file. Terrain hole file is currently encoded the same with RAW elevation file. 
		/// with the distinction that it is an X*X array of BYTE values. If the BYTE value is 1, it is interpreted as a hole, otherwise it is false. 
		HRESULT LoadHoleFile(Terrain * pTerrain, const char * szFilename, short nHoleScale);

		/// The texture will automatically be chopped into TextureCell objects and a grid of textures will be arranged
		/// across the Terrain's surface.
		void LoadTerrainTexture(Terrain * pTerrain, const char * fileName);
		/// \brief Uses the specified TextureLoader library to apply a repeating common texture to the specified Terrain object.

		/// See Terrain::SetCommonTexture for details on what a common texture is.
		void LoadCommonTerrainTexture(Terrain * pTerrain, const char * fileName);


		HRESULT LoadTerrainInfo(Terrain *pTerrain,const char* szFilename);
	private:
		  Loader();
		 ~Loader();
		void ApplyTexture(Terrain * pTerrain, const char *szFilename, bool isBaseTexture);
	};
}
