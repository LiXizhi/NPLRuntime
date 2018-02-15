#pragma once
#include "Terrain.h"


namespace ParaTerrain
{
	class TerrainLoadListener
	{
	      public:
		virtual void TerrainLoaded(Terrain * pTerrain) = 0;
		virtual void TerrainUnloading(Terrain * pTerrain) = 0;
	};
}
