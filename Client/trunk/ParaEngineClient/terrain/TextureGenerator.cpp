// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.
#include "ParaEngine.h"
#include "TextureGenerator.h"

using namespace ParaTerrain;

void TextureGenerator::Init(Terrain * pTerrain, int textureWidth, int textureHeight)
{
	m_pTerrain = pTerrain;
	m_TextureWidth = textureWidth;
	m_TextureHeight = textureHeight;
}

void TextureGenerator::GetPosition(int texelX, int texelY, float &worldX, float &worldY)
{
	worldX = ((float)texelX / (float)m_TextureWidth) * m_pTerrain->GetWidth();
	worldY = ((float)texelY / (float)m_TextureHeight) * m_pTerrain->GetHeight();
}
