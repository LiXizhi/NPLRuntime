#pragma once
#include "TerrainLattice.h"

#include <map>
#include <string>
using namespace std;

namespace ParaTerrain
{
	class CDynamicTerrainLoader :
		public TerrainLatticeLoader
	{
	public:
		CDynamicTerrainLoader(const char* sConfigFile);
		~CDynamicTerrainLoader(void);
	private:
		struct TerrainTileInfo
		{
			/** name of the configuration file */
			string	m_sConfigFileName;
			/** whether the configuration file is valid */
			bool	m_bIsValid;
			TerrainTileInfo(const char* sConfigFileName){
				m_sConfigFileName = sConfigFileName;
				m_bIsValid = true;
			}
			TerrainTileInfo():m_bIsValid(false){};
		};
		map <int, TerrainTileInfo> m_TerrainTiles;
		float m_fTileSize;
		string m_sConfigFilePath;

		void Cleanup();
		void LoadFromFile(const char* sConfigFile);
	public:
		/** \brief Called by the TerrainLattice when a Terrain object in the lattice has entered the visible region and, therefore, needs to be loaded into RAM.
		* this function will always return a valid terrain. If there is no terrain data, a default terrain is returned.
		* @param pTerrain: the terrain object to be filled with data. If it is NULL, a new terrain object will be created.
		* @return: the loaded terrain object is returned.
		*/
		virtual Terrain * LoadTerrainAt(Terrain *pTerrain, int latticeX, int latticeY ,bool useGeoMipmap = false);
		/** \brief Called by the TerrainLattice when a Terrain object in the lattice is no longer within the visible region and can, therefore, be disposed of, freeing RAM for other visible Terrain objects.
		* @param pTerrain: the terrain object will be reduced a blank terrain after calling this function.
		*/
		virtual void UnloadTerrain(int latticeX, int latticeY, Terrain * pTerrain);
		/// \brief Returns the width in world units of each of the individual Terrain objects in the lattice (they must all be the same width.)
		virtual float GetTerrainWidth(){return m_fTileSize;};
		/// \brief Returns the height in world units of each of the individual Terrain objects in the lattice (they must all be the same height.)
		virtual float GetTerrainHeight(){return m_fTileSize;};

		/** update the tile config file for the terrain terrain tile x,y. return true if succeeded. 
		One needs to SaveWorldConfigFile() in order for the changes to be written to disk. 
		@param sTileConfigFile: if "", it will erase the tile. otherwise it will change the config file. 
		*/
		virtual bool UpdateTileConfigFile(int x, int y, const string& sTileConfigFile);

		/** save world config file to disk. */
		virtual bool SaveWorldConfigFile();
	};
}
