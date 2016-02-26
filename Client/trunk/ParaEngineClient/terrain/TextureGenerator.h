#pragma once
#include "Terrain.h"

namespace ParaTerrain
{

	class TextureGenerator
	{
	      public:
		virtual void Init(Terrain * pTerrain, int textureWidth, int textureHeight);
		virtual void Generate() = 0;
	      protected:
		void GetPosition(int texelX, int texelY, float &worldX, float &worldY);
		Terrain *m_pTerrain;
		int m_TextureWidth, m_TextureHeight;
	};
}
