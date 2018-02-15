// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "TerrainLattice.h"
#include "TextureEntity.h"
#include "TerrainFilters.h"

#include "WorldInfo.h"
#include "OceanManager.h"
#include "TTerrain.h"

#include "memdebug.h"
 
using namespace ParaEngine;

/** @def Set the height to the lowest terrain height in the inner radius */
#define USE_CENTER_HEIGHT
/** @def set how many terrain tile(including height fields, etc) are cached in memory. */
#define DEFAULT_TERRAIN_TILE_CACHE_SIZE	18

/** @def the smallest y scale. */
#define MIN_Y_SCALE		0.0001f

using namespace ParaTerrain;
using namespace std;

TerrainLattice::TerrainLattice(TerrainLatticeLoader * pLoader,bool useGeoMipmap)
	:m_bIsGlobalConfigModified(false)
{
	m_pLoader = pLoader;
	m_nMaxCacheSize = DEFAULT_TERRAIN_TILE_CACHE_SIZE;
	
	m_WidthActiveTerrains = m_HeightActiveTerrains = 1;
	m_TerrainWidth = pLoader->GetTerrainWidth();
	m_TerrainHeight = pLoader->GetTerrainHeight();

	m_useGeoMipmap = useGeoMipmap;

	//GeoMipmapCode
	m_camPosX = 0;
	m_camPosY = 0;
	m_Level0ChunkCount = 2;
	m_Level1ChunkCount = 2;

	if(m_useGeoMipmap)
	{
		//create shared index buffer
		CreateGeoMipmapIndexBuffer();
	}
}

TerrainLattice::~TerrainLattice()
{
	if (m_pIndices)
		m_pIndices.ReleaseBuffer();
	
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		delete ((*itCurCP).second.pTerrain);
	}
	m_pCachedTerrains.clear();
}

int TerrainLattice::GetTileIDFromXY(int x, int y)
{
	if(x>=0)
	{
		int r = x+abs(y)+1;
		return r*r*4+r+y;
	}
	else
	{
		int r = abs(x)-y+1;
		return r*r*4+3*r+y;
	}

	/*if(x<=y)
	{
		return y*y+x;
	}
	else
	{
		return x*x + 2*x-y;
	}*/
}
bool TerrainLattice::GetXYFromTileID(int nTileID, int* X, int* Y)
{
	int x,y;
	if(nTileID<0)
		return false;
	int nBase = (int)sqrt((float)nTileID); // 2*r
	int nLeft = nTileID - nBase*nBase;
	int r = nBase/2;
	if(nLeft<nBase)
	{
		y = nLeft - r;
		x = r-abs(y)-1;
	}
	else
	{
		y = nLeft - 3*r;
		x = -(r-abs(y)-1);
	}
	(*X) = x;
	(*Y) = y;
	return true;
	/*
	if(nLeft<=nBase)
	{
		(*X) = nLeft;
		(*Y) = nBase;
	}
	else
	{
		(*X) = nBase;
		(*Y) = nBase*2-nLeft;
	}
	return true;*/
}
/**
* obsoleted function
*/
Terrain * TerrainLattice::CreateTerrainTile(int positionX, int positionY)
{
	int nTileID = GetTileIDFromXY(positionX, positionY);
	pair<TerrainTileCacheMap_type::iterator, bool> res = m_pCachedTerrains.insert(pair<int, TerrainTileCacheItem>(nTileID, TerrainTileCacheItem()));
	if(res.second ==false)
	{
		/// just return the object if it already contains the terrain tile
	}
	else
	{
		/// create a new terrain tile if it has been created before
		Terrain * pTerrain = new Terrain();
		(*(res.first)).second.pTerrain = pTerrain;
		pTerrain->SetLatticePosition(positionX, positionY);
		pTerrain->SetOffset(positionX * m_TerrainWidth, positionY * m_TerrainHeight);
	}
	return (*(res.first)).second.pTerrain;
}

Terrain *TerrainLattice::GetTerrain(int positionX, int positionY)
{
	int nTileID = GetTileIDFromXY(positionX, positionY);
	TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(nTileID);
	if(iter == m_pCachedTerrains.end())
	{
		return LoadTerrain(nTileID);
	}
	else
	{
		return (*iter).second.pTerrain;
	}
}

Terrain *TerrainLattice::GetTerrainRelative(Terrain * pTerrain, ParaTerrain::DIRECTION direction)
{
	int offsetX, offsetY;
	switch (direction)
	{
	case ParaTerrain::DIR_NORTH:
		offsetX = 0;
		offsetY = 1;
		break;
	case ParaTerrain::DIR_NORTHEAST:
		offsetX = 1;
		offsetY = 1;
		break;
	case ParaTerrain::DIR_EAST:
		offsetX = 1;
		offsetY = 0;
		break;
	case ParaTerrain::DIR_SOUTHEAST:
		offsetX = 1;
		offsetY = -1;
		break;
	case ParaTerrain::DIR_SOUTH:
		offsetX = 0;
		offsetY = -1;
		break;
	case ParaTerrain::DIR_SOUTHWEST:
		offsetX = -1;
		offsetY = -1;
		break;
	case ParaTerrain::DIR_WEST:
		offsetX = -1;
		offsetY = 0;
		break;
	case ParaTerrain::DIR_NORTHWEST:
		offsetX = -1;
		offsetY = 1;
		break;
	case ParaTerrain::DIR_CENTER:
	case ParaTerrain::DIR_INVALID:
		break;
	}
	return GetTerrainRelative(pTerrain, offsetX, offsetY);
}


Terrain *TerrainLattice::GetTerrainRelative(Terrain * pTerrain, int positionX, int positionY)
{
	Terrain *pRequestedTerrain = NULL;
	if (-1 <= positionX && positionX <= 1 && -1 <= positionY && positionY <= 1)
	{
		int x, y;
		pTerrain->GetLatticePosition(x, y);
		x += positionX;
		y += positionY;

		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[ParaTerrain::DIR_CENTER]);
		if(iter == m_pCachedTerrains.end())
		{
			return NULL;
		}
		Terrain *pTerrainCenter = (*iter).second.pTerrain;

		int centerX, centerY;
		pTerrainCenter->GetLatticePosition(centerX, centerY);
		if (abs(x - centerX) <= m_WidthActiveTerrains && abs(y - centerY) <= m_HeightActiveTerrains)
		{
			if (0 <= x && 0 <= y)
				pRequestedTerrain = GetTerrain(x, y);
		}
	}
	return pRequestedTerrain;
}

Terrain * TerrainLattice::GetTerrainAtPoint(float x, float y)
{
	int indexX = (int)(x / m_TerrainWidth);
	int indexY = (int)(y / m_TerrainHeight);
	return GetTerrain(indexX, indexY);
}

Terrain * TerrainLattice::LoadTerrain(int index)
{
	int indexX, indexY;
	Terrain *pTerrain = NULL;
	if(GetXYFromTileID(index, &indexX, &indexY))
	{
		pTerrain = m_pLoader->LoadTerrainAt( NULL, indexX, indexY, m_useGeoMipmap);

		pair<TerrainTileCacheMap_type::iterator, bool> res = m_pCachedTerrains.insert(pair<int, TerrainTileCacheItem>(index, TerrainTileCacheItem(pTerrain)));
		if(res.second ==false)
		{
			/// replace the old object, if the terrain already exist
			OUTPUT_LOG("replacing old tile, this is really unexpected\n");
			SAFE_DELETE((*(res.first)).second.pTerrain);
			(*(res.first)).second.pTerrain = pTerrain;
		}
		if(pTerrain)
		{
			pTerrain->OnLoad();

			//GeoMipmapCode
			if(m_useGeoMipmap)
			{
				pTerrain->SetSharedIB(m_pIndices);
				pTerrain->SetSharedIndexInfoGroup(&m_geoMipmapIndices);
			}
		}
	}
	return pTerrain;
}

DWORD TerrainLattice::GetRegionValue(const string& sLayerName, float x, float y)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain != NULL)
		return pTerrain->GetRegionValueW(sLayerName, x, y);
	else
		return 0;
}

float TerrainLattice::GetElevation(float x, float y)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain != NULL)
		return pTerrain->GetElevationW(x, y);
	else
		return 0.0f;
}

void TerrainLattice::GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain != NULL)
		return pTerrain->GetNormalW(x, y,normalX,normalY,normalZ );
	else
	{
		normalX = 0;
		normalY = 0;
		normalZ = 1.0f;
	}
}

ParaTerrain::DIRECTION TerrainLattice::GetOppositeDirection(ParaTerrain::DIRECTION direction)
{
	ParaTerrain::DIRECTION oppositeDirection;
	switch (direction)
	{
	case ParaTerrain::DIR_NORTH:
		oppositeDirection = ParaTerrain::DIR_SOUTH;
		break;
	case ParaTerrain::DIR_NORTHEAST:
		oppositeDirection = ParaTerrain::DIR_SOUTHWEST;
		break;
	case ParaTerrain::DIR_EAST:
		oppositeDirection = ParaTerrain::DIR_WEST;
		break;
	case ParaTerrain::DIR_SOUTHEAST:
		oppositeDirection = ParaTerrain::DIR_NORTHWEST;
		break;
	case ParaTerrain::DIR_SOUTH:
		oppositeDirection = ParaTerrain::DIR_NORTH;
		break;
	case ParaTerrain::DIR_SOUTHWEST:
		oppositeDirection = ParaTerrain::DIR_NORTHEAST;
		break;
	case ParaTerrain::DIR_WEST:
		oppositeDirection = ParaTerrain::DIR_EAST;
		break;
	case ParaTerrain::DIR_NORTHWEST:
		oppositeDirection = ParaTerrain::DIR_SOUTHEAST;
		break;
	case ParaTerrain::DIR_CENTER:
		oppositeDirection = ParaTerrain::DIR_CENTER;
		break;
	case ParaTerrain::DIR_INVALID:
		break;
	}
	return oppositeDirection;
}


void TerrainLattice::SetCameraPosition(float x, float y, float z)
{
	int indexX = (int)x / (int)m_TerrainWidth;
	int indexY = (int)y / (int)m_TerrainHeight;
	m_CurrentTerrainIndex[ParaTerrain::DIR_CENTER] = GetTileIDFromXY(indexX, indexY);
	m_CurrentTerrainIndex[ParaTerrain::DIR_SOUTH] = GetTileIDFromXY(indexX, indexY-1);
	m_CurrentTerrainIndex[ParaTerrain::DIR_SOUTHEAST] = GetTileIDFromXY(indexX+1, indexY-1);
	m_CurrentTerrainIndex[ParaTerrain::DIR_SOUTHWEST] = GetTileIDFromXY(indexX-1, indexY-1);
	m_CurrentTerrainIndex[ParaTerrain::DIR_EAST] = GetTileIDFromXY(indexX+1, indexY);
	m_CurrentTerrainIndex[ParaTerrain::DIR_WEST] = GetTileIDFromXY(indexX-1, indexY);
	m_CurrentTerrainIndex[ParaTerrain::DIR_NORTH] = GetTileIDFromXY(indexX, indexY+1);
	m_CurrentTerrainIndex[ParaTerrain::DIR_NORTHEAST] = GetTileIDFromXY(indexX+1, indexY+1);
	m_CurrentTerrainIndex[ParaTerrain::DIR_NORTHWEST] = GetTileIDFromXY(indexX-1, indexY+1);

	/** ensure that the 9 tiles near the eye position are always active.And increase their hit count */
	for (int dir = 0; dir < 9 ; dir++)
	{
		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
		if(iter == m_pCachedTerrains.end())
		{
			LoadTerrain(m_CurrentTerrainIndex[dir]);
		}
		else
		{
			(*iter).second.OnHit();
		}
	}
	/** garbage collect other terrain tiles; while frame move all active tiles */
	if((int)m_pCachedTerrains.size() > m_nMaxCacheSize)
	{
		TerrainTileCacheMap_type::iterator itCurCP, oldestCP, itEndCP = m_pCachedTerrains.end();
		oldestCP = itEndCP;
		///  use 0x3fffffff, instead of 0xffffffff prevents the current frame used terrain to be deleted.
		DWORD nHit = 0x3fffffff;
		for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			if((*itCurCP).second.nHitCount < nHit)
			{
				oldestCP = itCurCP;
				nHit = (*itCurCP).second.nHitCount;
			}
			(*itCurCP).second.FrameMove();
		}	
		if(oldestCP != itEndCP)
		{
			delete ((*oldestCP).second.pTerrain);
			m_pCachedTerrains.erase(oldestCP);
		}
	}

	m_camPosX = x;
	m_camPosY = y;
}

int TerrainLattice::GetMaxTileCacheSize()
{
	return m_nMaxCacheSize;
}

void TerrainLattice::SetMaxTileCacheSize(int nNum)
{
	if(nNum>=18 || nNum<=4000)
		m_nMaxCacheSize = nNum;
}

void TerrainLattice::ModelViewMatrixChanged()
{
	if(!m_useGeoMipmap)
	{
		SetLowestVisibleHeight(FLOAT_POS_INFINITY);

		m_BoundingBox.SetEmpty();
		/// tessellation according to the new view matrix
		for (int dir = 0; dir < 9 ; dir++)
		{
			TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
			if(iter != m_pCachedTerrains.end())
			{
				//PERF_BEGIN("tesse");
				if( ((*iter).second.pTerrain->Tessellate()) > 0 )
				{
					CShapeBox box = (*iter).second.pTerrain->GetBoundingBoxW();
					if(box.IsValid())
						m_BoundingBox.Extend(&box,1);
				}
				//PERF_END("tesse");
				float fHeight = (*iter).second.pTerrain->GetLowestVisibleHeight();
				if(fHeight < GetLowestVisibleHeight())
					SetLowestVisibleHeight(fHeight);
			}
		}
	
		/// Update neighbor, in case the shorelines are not uniformly tessellated.
		// TODO: no need to update so many neighbors 
		for (int dir = 0; dir < 9 ; dir++)
		{
			TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
			if(iter != m_pCachedTerrains.end())
			{
				for (int direction = 0; direction < 9; direction++)
				{
					if (direction != ParaTerrain::DIR_CENTER)
					{
						Terrain *pTerrain = GetTerrainRelative((*iter).second.pTerrain, (ParaTerrain::DIRECTION) direction);
						if (pTerrain != NULL)
						{
							(*iter).second.pTerrain->UpdateNeighbor(pTerrain, (ParaTerrain::DIRECTION) direction);
							pTerrain->UpdateNeighbor((*iter).second.pTerrain, GetOppositeDirection((ParaTerrain::DIRECTION) direction));
						}
					}
				}
			}
		}
		/// repair cracks and rebuild render buffer
		for (int dir = 0; dir < 9 ; dir++)
		{
			TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
			if(iter != m_pCachedTerrains.end())
			{
				Terrain* pTerrain = (*iter).second.pTerrain;
				//PERF_BEGIN("repair");
				pTerrain->RepairCracks();
				//PERF_END("repair");
				//PERF_BEGIN("Buff");
				pTerrain->RebuildRenderBuffer();
				//PERF_END("Buff");
			}
		}
	}
	else
	{
		SetLowestVisibleHeight(FLOAT_POS_INFINITY);

		m_BoundingBox.SetEmpty();
		/// tessellation according to the new view matrix
		for (int dir = 0; dir < 9 ; dir++)
		{
			TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
			if(iter != m_pCachedTerrains.end())
			{
				//PERF_BEGIN("tesse");
				if( ((*iter).second.pTerrain->Tessellate()) > 0 )
				{
					CShapeBox box = (*iter).second.pTerrain->GetBoundingBoxW();
					if(box.IsValid())
						m_BoundingBox.Extend(&box,1);
				}
				//PERF_END("tesse");
				float fHeight = (*iter).second.pTerrain->GetLowestVisibleHeight();
				if(fHeight < GetLowestVisibleHeight())
					SetLowestVisibleHeight(fHeight);
			}
		}

		CalcGeoMipmapLod(m_camPosX,m_camPosY);
	}
}


float TerrainLattice::IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance)
{
	float hitX, hitY,hitZ;
	float fDistance = fMaxDistance;
	for (int dir = 0; dir < 9 ; dir++)
	{
		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
		if(iter != m_pCachedTerrains.end())
		{
			float fDist = (*iter).second.pTerrain->IntersectRayW(startX, startY, startZ, dirX, dirY, dirZ, hitX, hitY,hitZ, fDistance);
			if(fDist>=0 && fDist<fDistance)
			{
				intersectX = hitX;
				intersectY = hitY;
				intersectZ = hitZ;
				fDistance = fDist;
			}
		}
	}
	return (fDistance >= fMaxDistance) ? -1.f : fDistance;
}

void TerrainLattice::Render()
{
	for (int dir = 0; dir < 9 ; dir++)
	{
		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
		if(iter != m_pCachedTerrains.end())
		{
			(*iter).second.pTerrain->Render();
		}
	}
	/// generate terrain rendering report to console
	if(CGlobals::WillGenReport())
	{
		string sReport = "";
		char tmp[300];
		sprintf(tmp, "%d cached tiles:", (int)m_pCachedTerrains.size());
		sReport.append(tmp);

		for (int dir = 0; dir < 9 ; dir++)
		{
			TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
			if(iter != m_pCachedTerrains.end())
			{
				sReport.append((*iter).second.pTerrain->GetRenderReport());
			}
		}
		CGlobals::GetReport()->SetString("Terra",sReport.c_str());
	}
}

void TerrainLattice::InitDeviceObjects()
{
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		(*itCurCP).second.pTerrain->InitDeviceObjects();
	}
}
void TerrainLattice::DeleteDeviceObjects()
{
	m_pIndices.ReleaseBuffer();

	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		(*itCurCP).second.pTerrain->DeleteDeviceObjects();
	}
}
void TerrainLattice::InvalidateDeviceObjects()
{
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		(*itCurCP).second.pTerrain->InvalidateDeviceObjects();
	}
}

void TerrainLattice::SetTileSize(float fTileSize)
{
	m_TerrainWidth = fTileSize;
	m_TerrainHeight = fTileSize;
}

void  TerrainLattice::SetLowestVisibleHeight(float fHeight)
{
	m_fLowestVisibleHeight = fHeight;
}

float TerrainLattice::GetLowestVisibleHeight()
{
	return m_fLowestVisibleHeight;
}


void ParaTerrain::TerrainLattice::UpdateHoles(float x, float y)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain!=NULL)
	{
		pTerrain->UpdateHoles();
	}
}

bool ParaTerrain::TerrainLattice::IsHole( float x, float y )
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain!=NULL)
	{
		return pTerrain->IsHoleW(x,y);
	}
	return false;
}

void ParaTerrain::TerrainLattice::SetHole( float x, float y, bool bIsHold )
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain!=NULL)
	{
		pTerrain->SetHole(x,y,bIsHold);
	}
}


float TerrainLattice::GetVertexElevation(float x, float y)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain!=NULL)
	{
		return pTerrain->GetVertexElevation(pTerrain->GetVertexW(x,y));
	}
	else
		return 0.f;
}


void TerrainLattice::SetVertexElevation(float x, float y, float newElevation, bool recalculate_geometry)
{
/* some vertex on border needs to be duplicated. This macro set the vertex on the given terrain tile.
* it is just to make the code easy to understand
*/
#define SETELEVATION(indexX, indexY) \
pTerrain = GetTerrain((indexX), (indexY));\
if(pTerrain && (!pTerrain->IsEmpty())){\
	int nIndex = pTerrain->GetVertexW(x,y);\
	pTerrain->SetVertexElevation(nIndex, newElevation,recalculate_geometry);\
	pTerrain->SetModified(true, MODIFIED_HEIGHTMAP);\
}

	Terrain *pTerrain = NULL;
	if(x<0 || y<0)
		return;
	int indexX = (int)(x / m_TerrainWidth);
	int indexY = (int)(y / m_TerrainHeight);
	
	SETELEVATION(indexX, indexY);
	
	float dX = x-indexX*m_TerrainWidth;
	float dY = y-indexY*m_TerrainHeight;
	float fRadius = pTerrain->GetVertexSpacing()/2;
	if(dX<fRadius)
	{
		SETELEVATION(indexX-1, indexY);
		if(dY<fRadius)
		{
			SETELEVATION(indexX-1, indexY-1);
		}
		else if(dY>m_TerrainHeight-fRadius)
		{
			SETELEVATION(indexX-1, indexY+1);
		}
	}
	else if(dX>m_TerrainWidth-fRadius)
	{
		SETELEVATION(indexX+1, indexY);
		if(dY<fRadius)
		{
			SETELEVATION(indexX+1, indexY-1);
		}
		else if(dY>m_TerrainHeight-fRadius)
		{
			SETELEVATION(indexX+1, indexY+1);
		}
	}
	if(dY<fRadius)
	{
		SETELEVATION(indexX, indexY-1);
	}
	else if(dY>m_TerrainHeight-fRadius)
	{
		SETELEVATION(indexX, indexY+1);
	}
}

void TerrainLattice::Paint(ParaEngine::TextureEntity* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
{
	Terrain * AffectedTerrainTiles[4];
	Terrain * pCenterTerrain = GetTerrainAtPoint(x, y);
	if(pCenterTerrain==NULL || pCenterTerrain->IsEmpty())
		return;

	float fLogicalBrushRadius = brushRadius;
	AffectedTerrainTiles[0] = GetTerrainAtPoint(x+fLogicalBrushRadius, y+fLogicalBrushRadius);
	AffectedTerrainTiles[1] = GetTerrainAtPoint(x+fLogicalBrushRadius, y-fLogicalBrushRadius);
	AffectedTerrainTiles[2] = GetTerrainAtPoint(x-fLogicalBrushRadius, y+fLogicalBrushRadius);
	AffectedTerrainTiles[3] = GetTerrainAtPoint(x-fLogicalBrushRadius, y-fLogicalBrushRadius);

	for(int i=0;i<4; i++)
	{
		Terrain * pTerrain = AffectedTerrainTiles[i];
		bool bDraw = true;
		for(int k=i-1;k>=0; --k)
		{
			if(AffectedTerrainTiles[k] == pTerrain)
			{
				bDraw = false;
				break;
			}
		}
		if(bDraw && pTerrain != NULL && (! pTerrain->IsEmpty()))
		{
			pTerrain->SetModified(true, MODIFIED_TEXTURE);
			pTerrain->Paint(detailTexture, brushRadius, brushIntensity, maxIntensity, erase, x,y);
		}
	}
}

bool TerrainLattice::SnapPointToVertexGrid(float& x, float& y)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain==NULL || pTerrain->IsEmpty())
		return false;
	float fVertexSpacing = pTerrain->GetVertexSpacing();
	x = (Math::Round(x / fVertexSpacing))*fVertexSpacing;
	y = (Math::Round(y / fVertexSpacing))*fVertexSpacing;
	return true;
}

bool  TerrainLattice::Copy_Rect_to_HeightField( TTerrain* pHeightField, float x, float y, float fRadius, bool bNormalize )
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain==NULL || pTerrain->IsEmpty())
		return false;
	float fVertexSpacing = pTerrain->GetVertexSpacing();
	
	int nCount = Math::Round(fRadius/fVertexSpacing)*2+1;
	float fMinX = x-nCount/2*fVertexSpacing;
	float fMinY = y-nCount/2*fVertexSpacing;

	pHeightField->CreatePlane(nCount, 0,fVertexSpacing);
	for(int i=0;i<nCount;i++)
	{
		for(int j=0;j<nCount;j++)
		{
			float fVertexX = fMinX + i*fVertexSpacing;
			float fVertexY = fMinY + j*fVertexSpacing;
			float fElev = GetVertexElevation(fVertexX, fVertexY);
			pHeightField->SetHeight(i,j, fElev);
		}
	}
	if(bNormalize)
		pHeightField->Normalize();
	return true;
}

bool  TerrainLattice::Apply_HeightField_to_Rect( TTerrain* pHeightField, float x, float y, bool bRecalculateGeometry, bool bRecoverHeightData)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain==NULL || pTerrain->IsEmpty())
		return false;
	float fVertexSpacing = pTerrain->GetVertexSpacing();

	int nCount = pHeightField->GetGridSize();
	float fMinX = x-nCount/2*fVertexSpacing;
	float fMinY = y-nCount/2*fVertexSpacing;

	if(bRecoverHeightData)
		pHeightField->Recover();

	for(int i=0;i<nCount;i++)
	{
		for(int j=0;j<nCount;j++)
		{
			float fVertexX = fMinX + i*fVertexSpacing;
			float fVertexY = fMinY + j*fVertexSpacing;
			SetVertexElevation(fVertexX, fVertexY, pHeightField->GetHeight(i,j), false);
		}
	}
	if(bRecalculateGeometry == true)
	{
		/** make the region slightly larger. the exact region radius is (nCount-1)*fVertexSpacing/2 */
		RecalculateTerrainGeometry(x,y,nCount*fVertexSpacing/2);
	}
	return true;
}
void  TerrainLattice::RecalculateTerrainGeometry(float x, float y, float fAffectedRadius)
{
	Terrain * AffectedTerrainTiles[4];
	AffectedTerrainTiles[0] = GetTerrainAtPoint(x+fAffectedRadius, y+fAffectedRadius);
	AffectedTerrainTiles[1] = GetTerrainAtPoint(x+fAffectedRadius, y-fAffectedRadius);
	AffectedTerrainTiles[2] = GetTerrainAtPoint(x-fAffectedRadius, y+fAffectedRadius);
	AffectedTerrainTiles[3] = GetTerrainAtPoint(x-fAffectedRadius, y-fAffectedRadius);

	for(int i=0;i<4; i++)
	{
		Terrain * pTerrain = AffectedTerrainTiles[i];
		bool bDraw = true;
		for(int k=i-1;k>=0; --k)
		{
			if(AffectedTerrainTiles[k] == pTerrain)
			{
				bDraw = false;
				break;
			}
		}
		if(bDraw && pTerrain != NULL && (!pTerrain->IsEmpty()))
		{
			pTerrain->RecalcGeometry();

			if(m_useGeoMipmap)
				pTerrain->BuildGeoMipmapBuffer();
		}
	}
	CGlobals::GetOceanManager()->CleanupTerrainCache();
}

void TerrainLattice::GaussianHill(float x,float y,float radius,float fHillHeight,float standard_deviation,float smooth_factor)
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();


	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;

	// apply the filter
	if(pHeightField->GetYScale() > 0)
		fHillHeight /= pHeightField->GetYScale();
	terrafilter.GaussianHill(0.5f,0.5f, 0.5f, fHillHeight, standard_deviation, smooth_factor);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void TerrainLattice::Roughen_Smooth(float x, float y, float radius, bool roughen, bool big_grid,float factor)
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;

	// apply the filter
	terrafilter.Roughen_Smooth(roughen, big_grid, factor);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void ParaTerrain::TerrainLattice::Ramp( float x1, float y1, float x2, float y2, float ramp_radius, float borderpercentage/*=0.5f*/, float factor/*=1.0f*/ )
{
	if((fabs(x1-x2)+fabs(y1-y2))<GetVertexSpacing())
		return;
	float x,y,radius;
	x = (x1+x2)/2;
	y = (y1+y2)/2;
	radius = ramp_radius + sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1)) + GetVertexSpacing();
	float height1 = GetElevation(x1,y1);
	float height2 = GetElevation(x2,y2);

	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;

	int nCount = pHeightField->GetGridWidth();
	float fSize = (nCount-1)*pHeightField->GetVertexSpacing();
	float fMinX = x-(nCount-1)/2*pHeightField->GetVertexSpacing();
	float fMinY = y-(nCount-1)/2*pHeightField->GetVertexSpacing();
	x1 = (x1-fMinX)/fSize;
	y1 = (y1-fMinY)/fSize;
	x2 = (x2-fMinX)/fSize;
	y2 = (y2-fMinY)/fSize;
	pHeightField->NormalizeHeight(height1);
	pHeightField->NormalizeHeight(height2);
	ramp_radius = ramp_radius/fSize;

	// apply the filter
	terrafilter.Ramp(x1,y1,height1,x2,y2,height2,ramp_radius,borderpercentage, factor);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void TerrainLattice::RadialScale(float x, float y, float scale_factor, float min_dist,float max_dist, float smooth_factor, int frequency)
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, max_dist,true) == false)
		return;

	// apply the filter
	if(pHeightField->GetYScale() > 0)
		scale_factor /= pHeightField->GetYScale();
	terrafilter.RadialScale(0.5f,0.5f, scale_factor, min_dist/max_dist*0.5f, 0.5f, smooth_factor, frequency);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void ParaTerrain::TerrainLattice::Flatten( float x,float y,float radius, int flatten_op, float elevation,float factor )
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;

	float fElev = elevation;
	float fHeightDiff = fabs(pHeightField->GetMaxHeight()-pHeightField->GetMinHeight());

	if( fHeightDiff> MIN_Y_SCALE)
		fElev = (fElev -pHeightField->GetMinHeight())/(fHeightDiff);
	else
		fElev = fElev - pHeightField->GetMinHeight();
	// apply the filter
	terrafilter.Flatten((CTerrainFilters::FlattenOperation)flatten_op, fElev, factor, 0.5f, 0.5f, 0.5f);
	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void TerrainLattice::DigCircleFlat(float x, float y, float radius, float fFlatPercentage,float factor)
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;

	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;
	
	// Set the height to the lowest terrain height in the inner radius
	float fElev = 0.f;

#ifdef USE_CENTER_HEIGHT
	// set the flatten height to the height of the center of the circle.
	fElev = GetElevation(x,y);
	float fHeightDiff = fabs(pHeightField->GetMaxHeight()-pHeightField->GetMinHeight());
	if( fHeightDiff> 0.001f)
	{
		fElev = (fElev -pHeightField->GetMinHeight())/(fHeightDiff);
		if(fElev<0.f)
			fElev = 0.f;
		else if(fElev>1.f)
			fElev = 1.f;
	}
	else
		fElev = 0.f;
#endif
	terrafilter.Flatten(CTerrainFilters::Flatten_Op, fElev, factor, 0.5f,0.5f, fFlatPercentage);
	
	terrafilter.Roughen_Smooth(false, false, 0.5f);
	terrafilter.Roughen_Smooth(false, false, 0.5f);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void ParaTerrain::TerrainLattice::Spherical( float x,float y,float radius, float offset )
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;
	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	if(Copy_Rect_to_HeightField(pHeightField, x,y, radius,true) == false)
		return;

	// apply the filter
	terrafilter.Spherical(offset);

	if(Apply_HeightField_to_Rect(pHeightField, x,y,true,true) == false)
		return;
}

void TerrainLattice::MergeHeightField( float x, float y, const char* filename, int mergeOperation/*=0*/, float weight1/*=1.0*/, float weight2/*=1.0*/, int nSmoothPixels/*=7*/ )
{
	// snap to terrain grid.
	if(SnapPointToVertexGrid(x,y) == false)
		return;

	CTerrainFilters terrafilter;
	TTerrain* pHeightField = terrafilter.GetTerrainData();

	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if(pTerrain==NULL || pTerrain->IsEmpty())
		return;
	float fVertexSpacing = pTerrain->GetVertexSpacing();

	if(pHeightField->CreatePlane(filename, fVertexSpacing))
	{
		// Set the height to the lowest terrain height in the inner radius
		terrafilter.SetConstEdgeHeight(0, nSmoothPixels);

		CTerrainFilters terrafilter2;
		TTerrain* pHeightFieldCurrent = terrafilter2.GetTerrainData();
		if(Copy_Rect_to_HeightField(pHeightFieldCurrent, x,y, (pHeightField->GetGridSize()-1)*fVertexSpacing/2,false) == false)
			return;
		terrafilter2.Merge( pHeightFieldCurrent, pHeightField, weight1, weight2, (CTerrainFilters::MergeOperation)mergeOperation);

		if(Apply_HeightField_to_Rect(pHeightFieldCurrent, x,y,true,true) == false)
			return;
	}
}

void TerrainLattice::AddHeightField(float x, float y, const char* filename, int nSmoothPixels)
{
	MergeHeightField(x,y, filename, CTerrainFilters::Addition, 1.f, 1.f, nSmoothPixels);
}

void TerrainLattice::SetAllLoadedModified(bool bIsModified, DWORD dwModifiedBits)
{
	bool bSaveConfigFile = false;
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		Terrain* pTerrain = (*itCurCP).second.pTerrain;
		pTerrain->SetModified(bIsModified, dwModifiedBits);
	}
}


void ParaTerrain::TerrainLattice::ResizeTextureMaskWidth( int nWidth )
{
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		Terrain* pTerrain = (*itCurCP).second.pTerrain;
		if(pTerrain)
		{
			pTerrain->ResizeTextureMaskWidth(nWidth);
		}
	}
}

void ParaTerrain::TerrainLattice::SetGlobalConfigModified( bool bEnable /*= true*/ )
{
	m_bIsGlobalConfigModified = bEnable;
}

IAttributeFields* ParaTerrain::TerrainLattice::GetChildAttributeObject(const std::string& sName)
{
	return NULL;
}

IAttributeFields* ParaTerrain::TerrainLattice::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		if (nRowIndex < (int)m_pCachedTerrains.size())
		{
			auto iter = m_pCachedTerrains.begin();
			std::advance(iter, nRowIndex);
			TerrainTileCacheItem& item = iter->second;
			return item.pTerrain;
		}
	}
	return NULL;
}

int ParaTerrain::TerrainLattice::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return (int)m_pCachedTerrains.size();
	else
		return 0;
}

int ParaTerrain::TerrainLattice::GetChildAttributeColumnCount()
{
	return 1;
}

void TerrainLattice::SaveTerrain(bool bHeightMap, bool bTextures)
{
	bool bSaveConfigFile = false;
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		Terrain* pTerrain = (*itCurCP).second.pTerrain;
		if(pTerrain->IsModified())
		{
			if(pTerrain->IsEmpty()){
				pTerrain->SetModified(false, MODIFIED_ALL);
			}
			else
			{
				int x,y;
				pTerrain->GetLatticePosition(x,y);
				string sConfig = CGlobals::GetWorldInfo()->GetTerrainConfigFile(x,y);
				if(pTerrain->m_sConfigFile != sConfig)
				{
					pTerrain->m_sConfigFile = sConfig;
					pTerrain->SetModified(true, MODIFIED_CONFIGURATION | MODIFIED_TEXTURE);
					if (m_pLoader)
					{
						m_pLoader->UpdateTileConfigFile(x,y, sConfig);
					}
					bSaveConfigFile = true;
				}
				pTerrain->SaveToFile();
				pTerrain->SetModified(false, MODIFIED_ALL);
			}
		}
	}
	// save global world config file if any terrain tile is newly created. 
	if(m_bIsGlobalConfigModified || bSaveConfigFile)
	{
		if(m_pLoader){
			m_pLoader->SaveWorldConfigFile();
		}
	}
	m_bIsGlobalConfigModified = false;
}

bool TerrainLattice::IsModified()
{
	TerrainTileCacheMap_type::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		bool bIsModified = (*itCurCP).second.pTerrain->IsModified();
		if(bIsModified)
			return true;
	}	
	return false;
}

const CShapeBox& TerrainLattice::GetBoundingBox() const
{
	return m_BoundingBox;
}

float TerrainLattice::GetVertexSpacing() const
{
	// use the first cached terrain tile's spacing
	TerrainTileCacheMap_type::const_iterator itCurCP, itEndCP = m_pCachedTerrains.end();
	for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		return (*itCurCP).second.pTerrain->GetVertexSpacing();
	}
	// if there is no terrain tile, we will assume 128 vertices in width, and compute the vertex spacing.
	return m_TerrainWidth/128.f;
}

void TerrainLattice::GetBoundingBoxes(vector<CShapeAABB>& boxes, const Matrix4* modelView, const CShapeFrustum& frustum, int nMaxBoxesNum/*=1024*/, int nSmallestBoxStride/*=4*/)
{
	PERF1("TerrainLattice::GetBoundingBoxes"); 
	for (int dir = 0; dir < 9 ; dir++)
	{
		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
		if(iter != m_pCachedTerrains.end())
		{
			(*iter).second.pTerrain->GetBoundingBoxes(boxes, modelView, frustum, nMaxBoxesNum, nSmallestBoxStride);;
		}
	}
}

/////////////////////////////////////////////////////////
//GeoMipmapCode
void TerrainLattice::CreateGeoMipmapIndexBuffer()
{
	int vertexWidth = 129;
	m_geoMipmapIndices.CreateIndex(0,129,1,3);
	
	D3DFORMAT fmt;
	int bufferSize;

	int maxVertexIdx = vertexWidth * vertexWidth;
	fmt = D3DFMT_INDEX16;
	bufferSize = m_geoMipmapIndices.GetIndexCount() * 2;

	if (m_pIndices.CreateIndexBuffer(bufferSize, fmt, 0))
	{
		void *pBufferData;
		if (m_pIndices.Lock((void**)&pBufferData, 0, 0))
		{
			short* tempBuffer = m_geoMipmapIndices.GetIndexData();
			memcpy(pBufferData, tempBuffer, bufferSize);
			m_pIndices.Unlock();
		}
	}

	m_geoMipmapIndices.ReleaseInternalIndices();
}

void TerrainLattice::CalcGeoMipmapLod(float x,float y)
{
	Terrain* pTerrain = m_pCachedTerrains[m_CurrentTerrainIndex[0]].pTerrain;
	int blockCountX = (pTerrain->GetWidthVertices()-1) / pTerrain->GetMaximumVisibleBlockSize();

	float blockSize = m_TerrainWidth/blockCountX;
	int terrainCountX = (int)x / (int)m_TerrainWidth;
	int terrainCountY = (int)y / (int)m_TerrainHeight;
	x = x - terrainCountX * (int)m_TerrainWidth;
	y = y - terrainCountY * (int)m_TerrainHeight;

	int centerChunkIdxX = (int)(x / blockSize) + blockCountX;
	int centerChunkIdxY = (int)(y / blockSize) + blockCountX;

	int lodBounds[8];
	int halfWidth = m_Level0ChunkCount / 2;
	lodBounds[0] = centerChunkIdxX - halfWidth;
	lodBounds[1] = centerChunkIdxX + halfWidth;
	lodBounds[2] = centerChunkIdxY - halfWidth;
	lodBounds[3] = centerChunkIdxY + halfWidth;
	halfWidth = m_Level1ChunkCount / 2;
	lodBounds[4] = lodBounds[0] - halfWidth;
	lodBounds[5] = lodBounds[1] +	halfWidth;
	lodBounds[6] = lodBounds[2] -	halfWidth;
	lodBounds[7] = lodBounds[3] + halfWidth;

	const int leftFlag = 0x01;
	const int rightFlag = 0x02;
	const int topFlag = 0x04;
	const int bottomFlag = 0x08;

	//calc lod for each block in each terrain
	for (int dir = 0; dir < 9 ; dir++)
	{
		TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.find(m_CurrentTerrainIndex[dir]);
		if(iter != m_pCachedTerrains.end())
		{
			int startBlockIdxX = 0;
			int startBlockIdxY = 0;
			Terrain* pTerrain = (*iter).second.pTerrain;

			if(dir == DIR_CENTER)
			{
				startBlockIdxX = blockCountX;
				startBlockIdxY = blockCountX;
			}
			else if(dir == DIR_NORTH)
			{
				startBlockIdxX = blockCountX;
				startBlockIdxY = blockCountX * 2;
			}
			else if(dir == DIR_NORTHEAST)
			{
				startBlockIdxX = blockCountX * 2;
				startBlockIdxY = blockCountX * 2;
			}
			else if(dir == DIR_EAST)
			{
				startBlockIdxX = blockCountX * 2;
				startBlockIdxY = blockCountX;
			}
			else if(dir == DIR_SOUTHEAST)
			{
				startBlockIdxX = blockCountX * 2;
				startBlockIdxY = 0;
			}
			else if(dir == DIR_SOUTH)
			{
				startBlockIdxX = blockCountX;
				startBlockIdxY = 0;
			}
			else if(dir == DIR_SOUTHWEST)
			{
				startBlockIdxX = 0;
				startBlockIdxY = 0;
			}
			else if(dir == DIR_WEST)
			{
				startBlockIdxX = 0;
				startBlockIdxY = blockCountX;
			}
			else if(dir == DIR_NORTHWEST)
			{
				startBlockIdxX = 0;
				startBlockIdxY = blockCountX * 2;
			}

			for(int i=0;i<blockCountX;i++)
			{
				for(int j=0;j<blockCountX;j++)
				{
					int blockX = startBlockIdxX + j;
					int blockY = startBlockIdxY + i;

					int level = CalcBlockLod(lodBounds,blockX,blockY,3);
					int blockType = 0;

					int leftBlock = CalcBlockLod(lodBounds,blockX-1,blockY,3);
					if(leftBlock > level)
						blockType |= leftFlag;

					int rightBlock = CalcBlockLod(lodBounds,blockX+1,blockY,3);
					if(rightBlock > level)
						blockType |= rightFlag;

					int topBlock = CalcBlockLod(lodBounds,blockX,blockY-1,3);
					if(topBlock > level)
						blockType |= topFlag;

					int bottomBlock = CalcBlockLod(lodBounds,blockX,blockY+1,3);
					if(bottomBlock > level)
						blockType |= bottomFlag;

					GeoMipmapChunkType chunkType;
					switch (blockType)
					{
					case leftFlag|rightFlag|topFlag|bottomFlag:
						chunkType = gmc_surrounded;
						break;
					case topFlag | leftFlag:
						chunkType = gmc_topLeft;
						break;
					case leftFlag:
						chunkType = gmc_left;
						break;
					case bottomFlag | leftFlag:
						chunkType = gmc_bottomLeft;
						break;
					case bottomFlag:
						chunkType = gmc_bottom;
						break;
					case bottomFlag | rightFlag:
						chunkType = gmc_bottomRight;
						break;
					case rightFlag:
						chunkType = gmc_right;
						break;
					case topFlag | rightFlag:
						chunkType = gmc_topRight;
						break;
					case topFlag:
						chunkType = gmc_top;
						break;
					default:
						chunkType = gmc_normal;
						break;
					}					
					pTerrain->SetBlockLod(i,j,level,chunkType);									
				}
			}
		}
	}
}

int TerrainLattice::CalcBlockLod(int* lodBounds,int blockIndexX,int blockIndexY,int maxLod)
{
	for(int i=0,j=0;i<maxLod;i++)
	{
		if(blockIndexX >= lodBounds[j] && blockIndexX<= lodBounds[j+1]
		&& blockIndexY >= lodBounds[j+2] && blockIndexY<= lodBounds[j+3])
		{
			return i;
		}
		j += 4;
	}
	return maxLod-1;
}

void TerrainLattice::SwitchLodStyle(bool useGeoMipmap)
{
	if(m_useGeoMipmap == useGeoMipmap)
		return;
#ifdef USE_OPENGL_RENDERER
	if (useGeoMipmap)
	{
		m_useGeoMipmap = false;
		OUTPUT_LOG("warning: GeoMipmap rendering is disabled for opengl\n");
		return;
	}
#endif
	if(m_useGeoMipmap)
		m_pIndices.ReleaseBuffer();

	m_useGeoMipmap = useGeoMipmap;
	if(m_useGeoMipmap)
		CreateGeoMipmapIndexBuffer();

	
	for(TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.begin();iter!= m_pCachedTerrains.end();iter++)
	{
		Terrain* pTerrain = (*iter).second.pTerrain;
		pTerrain->SwitchLodStyle(m_useGeoMipmap,m_pIndices,&m_geoMipmapIndices);
	}
	ModelViewMatrixChanged();

}


void TerrainLattice::SetTerrainInfo(float x,float y,float radius,uint32 data,uint32 bitMask,uint32 bitOffset)
{
	if(SnapPointToVertexGrid(x,y) == false)
		return;

	Terrain* pTerrain = GetTerrainAtPoint(x,y);
	if(pTerrain == NULL || pTerrain->IsEmpty())
		return;

	float vertexSpacing = pTerrain->GetVertexSpacing();
	float minX = x - radius;
	float minY = y - radius;

	int count = (int)(radius * 2.f / vertexSpacing);
	if(count < 1)
		count = 1;
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<count;j++)
		{
			float curX = minX + i * vertexSpacing;
			float curY = minY + j* vertexSpacing;
			SetVertexInfo(curX,curY,data,bitMask,bitOffset);
		}
	}

	Terrain* affectedTerrainTiles[4];
	affectedTerrainTiles[0] = GetTerrainAtPoint(x+radius, y+radius);
	affectedTerrainTiles[1] = GetTerrainAtPoint(x+radius, y-radius);
	affectedTerrainTiles[2] = GetTerrainAtPoint(x-radius, y+radius);
	affectedTerrainTiles[3] = GetTerrainAtPoint(x-radius, y-radius);

	for(int i=0;i<4; i++)
	{
		Terrain * pTerrain = affectedTerrainTiles[i];
		bool bDraw = true;
		for(int k=i-1;k>=0; --k)
		{
			if(affectedTerrainTiles[k] == pTerrain)
			{
				bDraw = false;
				break;
			}
		}
	
		if(bDraw && pTerrain != NULL && (!pTerrain->IsEmpty()))
		{
			pTerrain->RefreshTerrainInfo();
		}
	
	}
}

uint32 TerrainLattice::GetTerrainInfo(float x,float y,uint32 bitMask,uint32 bitOffset)
{
	if(SnapPointToVertexGrid(x,y) == false)
		return 0;

	if(x<0 || y<0)
		return 0;

	int idxX = (int)(x/m_TerrainWidth);
	int idxY = (int)(y/m_TerrainHeight);
	Terrain* pTerrain = NULL;
	pTerrain = GetTerrain(idxX,idxY);
	if(pTerrain && !(pTerrain->IsEmpty()))
	{
		int idx = pTerrain->GetVertexW(x,y);
		return pTerrain->GetVertexInfo(idx,bitMask, (uint8)bitOffset);
	}
	return 0;
}


void TerrainLattice::SetVertexInfo(float x,float y,uint32 data,uint32 bitMask,uint32 bitOffset)
{
#define SETDATA(idxX,idxY)\
	pTerrain = GetTerrain((idxX),(idxY));\
	if(pTerrain && (!pTerrain->IsEmpty())){\
		int idx = pTerrain->GetVertexW(x,y);\
		pTerrain->SetVertexInfo(idx,data,bitMask,bitOffset);\
	}

	Terrain* pTerrain = NULL;
	if(x<0 || y<0)
		return;

	int idxX = (int)(x / m_TerrainWidth);
	int idxY = (int)(y / m_TerrainHeight);
	SETDATA(idxX,idxY);

	float dx = x - idxX * m_TerrainWidth;
	float dy = y - idxY * m_TerrainHeight;
	float radius = pTerrain->GetVertexSpacing() / 2;
	if(dx < radius)
	{
		SETDATA(idxX - 1,idxY);
		if(dy < radius)
		{
			SETDATA(idxX-1,idxY-1);
		}
		else if(dy > m_TerrainHeight - radius)
		{
			SETDATA(idxX-1,idxY+1);
		}
	}
	else if(dx > m_TerrainWidth - radius)
	{
		SETDATA(idxX+1, idxY);
		if(dy<radius)
		{
			SETDATA(idxX+1, idxY-1);
		}
		else if(dy>m_TerrainHeight-radius)
		{
			SETDATA(idxX+1, idxY+1);
		}
	}

	if(dy<radius)
	{
		SETDATA(idxX, idxY-1);
	}
	else if(dy>m_TerrainHeight-radius)
	{
		SETDATA(idxX, idxY+1);
	}
}


bool TerrainLattice::IsWalkable(float x ,float y,Vector3& oNormal)
{
	Terrain *pTerrain = GetTerrainAtPoint(x, y);
	if (pTerrain != NULL)
		return pTerrain->IsWalkable(x, y, oNormal);
	else
	{
		oNormal.x = 0;
		oNormal.y = 1;
		oNormal.z = 0;
		return true;
	}
}

void TerrainLattice::SetEditorMode(bool enable)
{
	for(TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.begin();iter!= m_pCachedTerrains.end();iter++)
	{
		Terrain* pTerrain = (*iter).second.pTerrain;
		pTerrain->SetEditorMode(enable);
	}
}

void TerrainLattice::SetVisibleDataMask(uint32 mask,uint8 bitOffset)
{
	for(TerrainTileCacheMap_type::iterator iter = m_pCachedTerrains.begin();iter!= m_pCachedTerrains.end();iter++)
	{
		Terrain* pTerrain = (*iter).second.pTerrain;
		pTerrain->SetVisibleDataMask(mask,bitOffset);
	}
}