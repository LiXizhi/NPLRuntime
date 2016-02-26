//-----------------------------------------------------------------------------
// Class:	CDetailTextureFactory
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "TerrainCommon.h"
#include "TextureCell.h"
#include "Terrain.h"
#include "Texture.h"
#include "DetailTextureFactory.h"

#include "memdebug.h"

using namespace ParaEngine;

using namespace ParaTerrain;
using namespace std;


CDetailTextureFactory::CDetailTextureFactory(void)
{
	m_TexturesMatrix = NULL;
	m_CachedTextureItems.clear();
	m_listTextures.clear();
	m_nNumTextureCache = 40;
}

CDetailTextureFactory::~CDetailTextureFactory(void)
{
	cleanup();
}

void CDetailTextureFactory::DeleteOldestCacheItem()
{
	list<TextureCacheItem>::iterator itCurCP, oldestCP, itEndCP = m_CachedTextureItems.end();
	oldestCP = m_CachedTextureItems.end();

	///  use 0x3fffffff, instead of 0xffffffff prevents the current frame used texture to be deleted.
	DWORD nHit = 0x3fffffff;
	for( itCurCP = m_CachedTextureItems.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP).nHitCount < nHit)
		{
			oldestCP = itCurCP;
			nHit = (*itCurCP).nHitCount;
		}
	}
	if(oldestCP != itEndCP)
	{
		// <=2 is tricky, since the this object and asset manager keep two reference copies
		if((*oldestCP).pTexture->GetRefCount() <= 2)
			(*oldestCP).pTexture->UnloadAsset();
		m_CachedTextureItems.erase(oldestCP);
	}
}

void CDetailTextureFactory::DeleteAllTextures()
{
	list<TextureCacheItem>::iterator itCurCP, itEndCP = m_CachedTextureItems.end();
	for( itCurCP = m_CachedTextureItems.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		// <=2 is tricky, since the this object and asset manager keep two reference copies
		if((*itCurCP).pTexture->GetRefCount() <= 2)
			(*itCurCP).pTexture->UnloadAsset();
	}
	m_CachedTextureItems.clear();
}

void CDetailTextureFactory::cleanup()
{
	if(m_TexturesMatrix)
	{
		delete [] m_TexturesMatrix;
		m_TexturesMatrix = NULL;
	}

	DeleteAllTextures();

	m_listTextures.clear();
}

void CDetailTextureFactory::Init(int numTiles, float fTileSize)
{
	cleanup();
	m_nTileNum = numTiles;
	m_fTileSize = fTileSize;
	int nNumTex = m_nTileNum*m_nTileNum;
	m_TexturesMatrix = new HighResTextureInfo[nNumTex];
	for(int i=0;i<nNumTex;i++)
	{
		for(int j=0;j<MAX_NUM_TEXTURE_LAYERS;j++)
			m_TexturesMatrix[i].layers[j] = -1;
	}
	
	/*
	* It will research in the directory of the sFilename for files in the following format
	* "{file name} [x] [y].[jpg|bmp|...]"where x, y is the tile coordinates of the file in the terrain surface.
	* And build the m_TexturesMatrix matrix accordingly. If no file is found at (x,y) m_TexturesMatrix(x,y) will
	* be set to -1. In the future, I may add a terrain texture configuration files to allow more flexible texture 
	* file assignment.
	*/
	//m_sFileName = szFullFilename;

	//char sFilename[MAX_FILENAME_LENGTH];
	//strcpy(sFilename, szFullFilename);
	//for(int i =0; sFilename[i]!='\0';i++)
	//{
	//	if(sFilename[i] == '\\')
	//		sFilename[i] = '/';
	//}
	//m_TexturesMatrix = new int[m_nTileNum*m_nTileNum];
	//memset(m_TexturesMatrix, -1, sizeof(int)*m_nTileNum*m_nTileNum);

	////TODO: build texture matrix and texture list
	//fs::path full_path(sFilename);
	//string rootTerrainName = full_path.leaf();
	//int nNameLength;
	//for (nNameLength=0; nNameLength<(int)rootTerrainName.size();nNameLength++)
	//{
	//	if(rootTerrainName[nNameLength] == '.')
	//		break;
	//}
	//fs::path dir_path = full_path.branch_path();

	//if ( !fs::exists( dir_path ) )
	//	return;
	//fs::directory_iterator end_itr; // default construction yields past-the-end
	//for ( fs::directory_iterator itr( dir_path );
	//	itr != end_itr;
	//	++itr )
	//{
	//	if ( fs::is_directory( *itr ) )
	//	{
	//		// without searching sub directories
	//	}
	//	else
	//	{
	//		string fileName = itr->leaf();
	//		char filename[100];
	//		int x,y;
	//		if(sscanf(fileName.c_str(), "%s %d %d", filename, &x,&y) == 3)
	//		{
	//			bool bFound = true;
	//			for (int i=0;i<nNameLength;i++)
	//			{
	//				if(fileName[i] != rootTerrainName[i])
	//				{
	//					bFound= false;
	//					break;
	//				}
	//			}
	//			if(bFound)
	//			{
	//				m_listTextures.push_back(itr->string());
	//				m_TexturesMatrix[x*m_nTileNum+y] = (int)m_listTextures.size()-1;
	//			}
	//		}
	//	}
	//}

}


int CDetailTextureFactory::GetTextureIDAt(float originX, float originY, short nLayer)
{
	float nX = (float)Math::Round((originX / m_fTileSize));
	float nY = (float)Math::Round((originY / m_fTileSize));
	if((nX>=0) && (nX<m_nTileNum) && 
		(nY>=0) && (nY<m_nTileNum))
	{
		return m_TexturesMatrix[int(nX*m_nTileNum + nY)].layers[nLayer];
	}
	return -1;
}

void CDetailTextureFactory::AdvanceFrame()
{
	list<TextureCacheItem>::iterator itCurCP, itEndCP = m_CachedTextureItems.end();
	for( itCurCP = m_CachedTextureItems.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		(*itCurCP).FrameMove();
	}
}

TextureEntity * CDetailTextureFactory::GetTexture(int nIndex)
{
	if((nIndex < 0) || (nIndex >= (int)m_listTextures.size()) )
		return NULL;

	/// first search in the cache
	TextureEntity * pTexture = NULL;
	list<TextureCacheItem>::iterator itCurCP, itEndCP = m_CachedTextureItems.end();
	for( itCurCP = m_CachedTextureItems.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP).nIndex == nIndex)
		{
			pTexture = (*itCurCP).pTexture.get();
			// flag the hit count bit if the texture is found in the cache
			(*itCurCP).OnHit();
			break;
		}
	}
	/// load into the cache if it is not in it.
	if(pTexture == NULL)
	{
		if((int)m_CachedTextureItems.size() >= m_nNumTextureCache)
			DeleteOldestCacheItem();
		pTexture = CGlobals::GetAssetManager()->LoadTexture("", m_listTextures[nIndex].c_str(), TextureEntity::TerrainHighResTexture);
		m_CachedTextureItems.push_back(TextureCacheItem(nIndex, pTexture));
	}
	return pTexture;

}

void CDetailTextureFactory::UnloadTexture(int index)
{
	list<TextureCacheItem>::iterator itCurCP, itEndCP = m_CachedTextureItems.end();
	for( itCurCP = m_CachedTextureItems.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP).nIndex == index)
		{
			TextureEntity* pEntity = (*itCurCP).pTexture.get();
			(*itCurCP).pTexture.reset();
			m_CachedTextureItems.erase(itCurCP);
			CGlobals::GetAssetManager()->GetTextureManager().DeleteEntity(pEntity);
			return;
		}
	}
}


