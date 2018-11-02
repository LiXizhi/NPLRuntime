#pragma once
#include "TerrainCommon.h"

namespace ParaTerrain
{
	class TerrainVertex
	{
	      public:
		TerrainVertex(int index, float x, float y, float z);
		 ~TerrainVertex();
		int m_Index;
		float m_X, m_Y, m_Z;
	};
}
