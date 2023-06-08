//-----------------------------------------------------------------------------
// Class:	Terrain
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.1.8
// Revised: 2009.6.23
// The terrain engine is based on Demeter Terrain Visualization Library by Clay Fowler
// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "AutoCamera.h"
#include "GlobalTerrain.h"
#include "SceneObject.h"
#include "AISimulator.h"
#include "NPLRuntime.h"
#include "SceneWriter.h"
#include "OceanManager.h"
#ifdef USE_DIRECTX_RENDERER
#include "ShadowMap.h"
#include "RenderDeviceD3D9.h"
#endif
#include "Terrain.h"
#include "Brush.h"
#include "Loader.h"
#include "WorldInfo.h"
#include "TerrainRegions.h"
#include "AsyncLoader.h"
#include "AssetManifest.h"
#include "util/regularexpression.h"

#include "memdebug.h"


using namespace ParaEngine;

using namespace ParaTerrain;
using namespace std;

/** @def whether the alpha mask is used for the base layer. This will solve the problem when texture is too bright in HLSL. currently it has no effects in the fixed function pipeline. */
#define USE_BASE_LAYER_MASK
/** how many simultaneous textures the terrain shader support. This can be 12 if future GPU support it. */
#define SIMULTANEOUS_TEXTURE_NUM	8
/** maximum number of texture layers */
#define MAX_TEXTURE_LAYERS	16

/** default terrain height when height is not loaded. */
#define DEFAULT_TERRAIN_HEIGHT -1000.f

Vector2* Terrain::s_LowResTexCords = NULL;
Vector2* Terrain::s_HighResTexCords = NULL;

//#undef MAX_NUM_VISIBLE_TERRAIN_TRIANGLES
//#define		MAX_NUM_VISIBLE_TERRAIN_TRIANGLES	100

float ParaTerrain::Terrain::m_DetailTextureRepeats = 16.0f;

namespace ParaTerrain
{
	extern Matrix4		g_matTerrainViewProj;
}

/* define WIRED_TERRAIN to render terrain as wires.*/
// #define WIRED_TERRAIN

float numBlocks = 0.0f;
float numLevels = 0.0f;
float hashDelta = 0.0f;

map < string, TextureGenerator * >Terrain::m_TextureGenerators;

Brush brush(20);


bool IsPowerOf2(double number)
{
	const int MAX_POWER = 1024;
	bool isPowerOf2 = false;

	for (int i = 0; i < MAX_POWER && !isPowerOf2; i++)
	{
		if (pow(2.0, i) == number)
			isPowerOf2 = true;
	}

	return isPowerOf2;
}

bool IsPowerOf2Plus1(double number)
{
	const int MAX_POWER = 1024;
	bool isPowerOf2Plus1 = false;

	for (int i = 0; i < MAX_POWER && !isPowerOf2Plus1; i++)
	{
		if (pow(2.0, i) + 1 == number)
			isPowerOf2Plus1 = true;
	}

	return isPowerOf2Plus1;
}

Terrain::Terrain()
{
	Init(MAX_NUM_VISIBLE_TERRAIN_TRIANGLES,0,0);
}
 
Terrain::Terrain(int maxNumTriangles, float offsetX, float offsetY)
{
	Init(maxNumTriangles, offsetX, offsetY);
}

Terrain::Terrain(int widthVertices, int heightVertices, float vertexSpacing, int maxNumTriangles)
{
	Init(maxNumTriangles, 0.0, 0.0);
	BuildVertices(widthVertices, heightVertices, vertexSpacing);
}

Terrain::Terrain(const float *pElevations, int elevWidth, int elevHeight, const uint8 * pTextureImage, int textureWidth, int textureHeight, const uint8 * pDetailTextureImage, int detailWidth, int detailHeight, float vertexSpacing, float elevationScale, int maxNumTriangles, float offsetX, float offsetY)
{
	Init(maxNumTriangles, offsetX, offsetY);
	SetAllElevations(pElevations, elevWidth, elevHeight, vertexSpacing, elevationScale);
	Init(pTextureImage, textureWidth, textureHeight, pDetailTextureImage, detailWidth, detailHeight, offsetX, offsetY);
}

Terrain::~Terrain()
{
	static DestroyStaticObject_ staticDestroy_;
	Cleanup();
}
bool Terrain::IsEmpty()
{
	return (m_pRootBlock==NULL);
}
void Terrain::Cleanup()
{
	m_TerrainBuffer.DeleteDeviceObjects();
	m_pCollisionBuffer.ReleaseBuffer();
	m_pEditorMeshVB.ReleaseBuffer();

	SAFE_DELETE(m_pTerrainInfoData);

	SAFE_DELETE(m_pMaskFileCallbackData);

	for (uint32 i = 0; i < m_TextureCells.size(); i++)
	{
		SAFE_DELETE(m_TextureCells[i]);
	}
	m_TextureCells.clear();

	SAFE_DELETE(m_pDefaultBaseLayerMask);

	SAFE_DELETE(m_pCommonTexture)
	
	m_pTriangleStrips.clear();
	m_pTriangleFans.clear();

	SAFE_DELETE_ARRAY(m_pVertices);

	SAFE_DELETE(m_pVertexStatus)
	SAFE_DELETE(m_pRootBlock)
	SAFE_DELETE(m_pTextureMain)
	SAFE_DELETE(m_pTextureDetail)
	SAFE_DELETE_ARRAY(m_pNormals)
	SAFE_DELETE(m_pTextureSet)
	SAFE_DELETE_ARRAY(m_pHolemap)
	SAFE_DELETE(m_pDetailedTextureFactory);

	SAFE_DELETE(m_pRegions);
}
  
void Terrain::Init(int maxNumTriangles, float offsetX, float offsetY)
{
	m_dwModified = MODIFIED_NONE;
	m_NumberOfTextureTilesWidth = m_NumberOfTextureTilesHeight = m_NumberOfTextureTiles = 0;
	m_WidthVertices = m_HeightVertices = 0;
	m_refCount = 0;
	m_NumberOfVertices = 0;
	m_pCommonTexture = NULL;
	m_pVertices = NULL;
	m_pVertexStatus = NULL;
	m_pRootBlock = NULL;
	m_pTextureMain = NULL;
	m_pTextureDetail = NULL;
	m_pNormals = NULL;
	m_pTextureSet = NULL;
	m_pDefaultBaseLayerMask = NULL;
	m_MaxNumberOfPrimitives = maxNumTriangles / 4;
	m_MaximumVisibleBlockSize = 16;
	m_OffsetX = offsetX;
	m_OffsetY = offsetY;

	m_CountFans = 0;
	m_CountStrips = 0;

	m_pHolemap = NULL;
	m_nHoleScale = 2;

	m_pRegions = NULL;
	m_pMaskFileCallbackData = NULL;
	m_bMaskFileInited = true;

	m_useGeoMipmap = false;

	m_pTerrainInfoData = NULL;
	m_visibleDataMask = 0xffffffff;
	m_visibleDataBitOffset = 0;
	m_isEditorMode = false;
	m_bChopBaseTexture = true;
	SetHighResTextureRadius(Settings::GetInstance()->GetHighResTextureRadius());
	Init(NULL, 0, 0, NULL, 0, 0, 0, 0);
}

void Terrain::Init(const uint8 * pTextureImage, int textureWidth, int textureHeight, const uint8 * pDetailTextureImage, int detailWidth, int detailHeight, float offsetX, float offsetY)
{
	m_dwModified = MODIFIED_NONE;
	m_pTextureSet = new TextureSet;
	m_pDefaultBaseLayerMask = NULL;
	m_DetailThreshold = 10.0f;
	m_TerrainBuffer.m_pTerrain = this;
	m_pDetailedTextureFactory = new CDetailTextureFactory();
	m_WidthVertices = m_HeightVertices = 0;
	
	SetRenderOffset(Vector3(0,0,0));
	
	SetLowestVisibleHeight(FLOAT_POS_INFINITY);
	m_pHolemap = NULL;
	m_nHoleScale = 2;
	m_CountFans = 0;
	m_CountStrips = 0;

	m_pRegions = NULL;
	m_pMaskFileCallbackData = NULL;
	m_bChopBaseTexture = true;

	if (pTextureImage != NULL)
		SetTexture(pTextureImage, textureWidth, textureHeight);
	if (pDetailTextureImage != NULL)
		SetCommonTexture(pDetailTextureImage, detailWidth, detailHeight);
}

void Terrain::InitDeviceObjects()
{
	if(!(Settings::GetInstance()->IsEditor() || m_bMaskFileInited))
	{
		if(!m_sBaseTextureFile.empty())
		{
			SetBaseTexture(m_sBaseTextureFile.c_str());
		}
		LoadMaskFromDisk();
	}
	m_bMaskFileInited = true;
}

void Terrain::RendererRecreated()
{
	m_TerrainBuffer.RendererRecreated();
	m_pCollisionBuffer.RendererRecreated();
	m_pEditorMeshVB.RendererRecreated();
	DeleteDeviceObjects();

	if (m_pTextureSet)
	{
		delete m_pTextureSet;
		m_pTextureSet = new TextureSet();
	}

	InitDeviceObjects();
}

void Terrain::InvalidateDeviceObjects()
{
	if(m_useGeoMipmap)
		return;

	// delete terrain buffer which is in DEFAULT POOL.
	m_TerrainBuffer.DeleteDeviceObjects();
}

/// must call this function when device is lost.
void Terrain::DeleteDeviceObjects()
{
	m_bMaskFileInited = false;
	m_TerrainBuffer.DeleteDeviceObjects();
	m_pCollisionBuffer.ReleaseBuffer();
	m_pEditorMeshVB.ReleaseBuffer();

	// delete all DirectX managed alpha maps. 
	for (uint32 i = 0; i < m_TextureCells.size(); i++)
	{
		if(m_TextureCells[i])
			m_TextureCells[i]->UnbindAll();
	}

	if(m_pDefaultBaseLayerMask)
		m_pDefaultBaseLayerMask->Unbind();


	// there is no need to delete these textures, since they are all ParaEngine's Managed Textures.
#ifdef DONOT_USE_PARAENGINE_TEXTURE
	if (m_pTextureSet)
		m_pTextureSet->UnbindAllTextures();
	if(m_pCommonTexture)
		m_pCommonTexture->UnloadTexture();
	if(m_pDetailedTextureFactory)
		m_pDetailedTextureFactory->DeleteAllTextures();
#endif
}

void Terrain::StaticInitUV(int nWidthVertices, int nLowResTileCount, int nHighResTileCount)
{
	if(s_LowResTexCords == NULL)
	{
		int nSize = nWidthVertices * nWidthVertices;
		s_LowResTexCords = new Vector2[nSize];
		for (int nIndex=0; nIndex < nSize; ++nIndex)
		{
			int Y = int(nIndex / (nWidthVertices));
			int X = (nIndex % nWidthVertices);
			s_LowResTexCords[nIndex].x = (float)(X*nLowResTileCount)/(float)(nWidthVertices-1);
			s_LowResTexCords[nIndex].y = (float)(Y*nLowResTileCount)/(float)(nWidthVertices-1);
		}
	}
	if(s_HighResTexCords == NULL)
	{
		int nSize = nWidthVertices * nWidthVertices;
		s_HighResTexCords = new Vector2[nSize];
		for (int nIndex=0; nIndex < nSize; ++nIndex)
		{
			int Y = int(nIndex / (nWidthVertices));
			int X = (nIndex % nWidthVertices);
			s_HighResTexCords[nIndex].x = (float)(X*nHighResTileCount)/(float)(nWidthVertices-1);
			s_HighResTexCords[nIndex].y = (float)(Y*nHighResTileCount)/(float)(nWidthVertices-1);
		}
	}
}

CTerrainRegions* Terrain::GetRegions()
{
	return m_pRegions;
}

CTerrainRegions* Terrain::CreateGetRegions()
{
	if(m_pRegions!=0)
		return m_pRegions;
	else
	{
		m_pRegions = new CTerrainRegions();
		return m_pRegions;
	}
}

int Terrain::GetNumOfRegions()
{
	if(GetRegions() == 0)
		return 0;
	else
		return GetRegions()->GetNumOfRegions();
}

void Terrain::SetCurrentRegionIndex(int nRegion)
{
	if(CreateGetRegions() != 0)
		GetRegions()->SetCurrentRegionIndex(nRegion);
}
int Terrain::GetCurrentRegionIndex()
{
	if(GetRegions() == 0)
		return -1;
	else
		return GetRegions()->GetCurrentRegionIndex();
}

void Terrain::SetCurrentRegionName(const string& name)
{
	if(CreateGetRegions() != 0)
		GetRegions()->SetCurrentRegionName(name);
}

const string& Terrain::GetCurrentRegionName()
{
	if(GetRegions() == 0)
		return CGlobals::GetString(0);
	else
		return GetRegions()->GetCurrentRegionName();
}

void Terrain::SetCurrentRegionFilepath(const string& filename)
{
	if(CreateGetRegions() != 0)
		GetRegions()->SetCurrentRegionFilepath(filename);
}

const string& Terrain::GetCurrentRegionFilepath()
{
	if(GetRegions() == 0)
		return CGlobals::GetString(0);
	else
		return GetRegions()->GetCurrentRegionFilepath();
}

void Terrain::SetAllElevations(const float *pElevations, int elevWidth, int elevHeight, float fTerrainSize, float elevationScale)
{
	// It should be safe to delete[] on NULL pointers, but MSVC used to choke, so we still check...
	if (m_pVertices)
		delete[]m_pVertices;
	if (m_pVertexStatus)
		delete m_pVertexStatus;
	if (m_pRootBlock)
		delete m_pRootBlock;
	if (m_pNormals)
		delete[]m_pNormals;

	bool isQuadSized = IsPowerOf2Plus1(elevWidth) && IsPowerOf2Plus1(elevHeight);
	
	if (!isQuadSized && (!IsPowerOf2(elevWidth) || !IsPowerOf2(elevHeight)))
	{
		string msg("The elevation data is NOT a power of 2 in both width and height. Elevation data must be a power of 2 in both width and height.");
		throw new TerrainException(msg);
		m_pVertices = NULL;
	}

	m_fTerrainSize = fTerrainSize;
	m_WidthVertices = elevWidth;
	m_HeightVertices = elevHeight;

	if (!isQuadSized)
	{
		m_WidthVertices++;	// Add 1 dummy pixel line to edges for block strides
		m_HeightVertices++;
	}
	m_VertexSpacing = m_fTerrainSize / (m_WidthVertices-1);

	m_NumberOfVertices = m_WidthVertices * m_HeightVertices;
	m_pVertices = new Vector3[m_NumberOfVertices];


	int i, j;
	float x, y;
	y = 0;
	m_MaxElevation = -10000.f;
	int end = elevWidth * elevHeight;
	for (i = 0, j = 0; i < end; i += elevWidth, y += m_VertexSpacing)
	{
		const float *pImageRow = pElevations + i;
		x = 0;
		for (const float *pImagePixel = pImageRow; pImagePixel < pImageRow + elevWidth; pImagePixel++, j++, x += m_VertexSpacing)
		{
			m_pVertices[j].x = x;
			m_pVertices[j].y = y;
			//m_pVertices[j].z = *pImagePixel * elevationScale;
			
			//extract walkable info and elevation   --clayman 2012.1.16;
			float packedData = *pImagePixel;
			float elevation = ((int)(packedData*100)) / 100.0f;
			m_pVertices[j].z = elevation * elevationScale;
			if (m_MaxElevation < m_pVertices[j].z)
				m_MaxElevation = m_pVertices[j].z;

			bool walkable = true;
			if(elevation >= 0)
			{
				walkable = (packedData-elevation)>0.004f;
			}
			else
			{
				walkable = (elevation - packedData) > 0.004f;
			}
		}
		if (!isQuadSized)
		{
			// Account for dummy column on right edge
			m_pVertices[j].x = float((m_WidthVertices - 1) * m_VertexSpacing);
			m_pVertices[j].y = y;
			m_pVertices[j].z = m_pVertices[j - 1].z;
			if (m_MaxElevation < m_pVertices[j].z)
				m_MaxElevation = m_pVertices[j].z;
			j++;
		}
		x += m_VertexSpacing;
	}
	
	if (!isQuadSized)
	{
		x = 0;
		for (i = m_NumberOfVertices - m_WidthVertices; i < m_NumberOfVertices; i++, x += m_VertexSpacing)
		{
			m_pVertices[i].x = x;
			m_pVertices[i].y = float((m_HeightVertices - 1) * m_VertexSpacing );
			m_pVertices[i].z = m_pVertices[i - m_WidthVertices].z;
		}
	}

	/// this will repair imprecisions due to the floating calculations 
	{
		//float fBottom =  GetHeight();
		float fBottom = (m_LatticePositionY+1)*GetHeight()-m_OffsetY; 
		for (i = m_NumberOfVertices - m_WidthVertices; i < m_NumberOfVertices; i++)
		{
			m_pVertices[i].y = fBottom;
		}
		//float fRight = GetWidth();
		float fRight = (m_LatticePositionX+1)*GetWidth()-m_OffsetX;
		for (i = m_WidthVertices-1; i < m_NumberOfVertices; i+=m_WidthVertices)
		{
			m_pVertices[i].x = fRight;
		}
	}

	// Build the terrain blocks
	BuildBlocks();

	// Generate vertex normal arrays
	if (Settings::GetInstance()->UseNormals())
	{
		GenerateTerrainNormal(false);
	}
}

bool Terrain::GenerateTerrainNormal(bool bForceRegenerate)
{
	if(m_pNormals==NULL)
	{
		m_pNormals = new Vector3[m_NumberOfVertices];
		bForceRegenerate = true;
		PE_ASSERT(m_pNormals!=0);
	}
	if(bForceRegenerate)
	{
		for (int i = 0; i < m_NumberOfVertices; i++)
			RecalcNormal(i);
	}
	return m_pNormals!=0;
}

inline void RotateZ(Vector3& v, float theta)
{
	float newx, newy;

	newx = v.x * cosf(theta) - v.y * sinf(theta);
	newy = v.x * sinf(theta) + v.y * cosf(theta);
	v.x = newx;
	v.y = newy;
}

void Terrain::RecalcNormal(int vertexIndex)
{
	const float delta = (MATH_PI * 2.0f) / 8.0f;
	int indexX, indexY;
	indexX = vertexIndex % m_WidthVertices;
	indexY = vertexIndex / m_WidthVertices;
	float vertexX, vertexY;
	vertexX = indexX * m_VertexSpacing;
	vertexY = indexY * m_VertexSpacing;
	Vector3 avgNormal;
	avgNormal.x = avgNormal.y = avgNormal.z = 0.0f;
	for (float theta = -0.5f * delta; theta < (MATH_PI * 2.0f); theta += delta)/*modified by LiXizhi:old is theta ++;)*/
	{
		Vector3 v;
		v.x = 1.0f;
		v.y = 0.0f;
		v.z = 0.0f;
		RotateZ(v,theta);
		v.x += vertexX;
		v.y += vertexY;
		float nx, ny, nz;
		GetNormal(v.x, v.y, nx, ny, nz);
		avgNormal.x += nx;
		avgNormal.y += ny;
		avgNormal.z += nz;
	}
	m_pNormals[vertexIndex].x = avgNormal.x / 8.0f;
	m_pNormals[vertexIndex].y = avgNormal.y / 8.0f;
	m_pNormals[vertexIndex].z = avgNormal.z / 8.0f;
}

void Terrain::RecalcNormal(float x, float y)
{
	int index = GetVertex(x, y);
	RecalcNormal(index);
}

void Terrain::BuildVertices(int widthVertices, int heightVertices, float vertexSpacing)
{
	float *pElevations = new float[widthVertices * heightVertices];
	for (int i = 0; i < widthVertices * heightVertices; i++)
		pElevations[i] = 0.0f;

	SetAllElevations(pElevations, widthVertices, heightVertices, vertexSpacing);

	delete[]pElevations;
}

void Terrain::SetOffset(float x, float y)
{
	m_OffsetX = x;
	m_OffsetY = y;
}

float Terrain::GetOffsetX() const
{
	return m_OffsetX;
}

float Terrain::GetOffsetY() const
{
	return m_OffsetY;
}

bool Terrain::IsMultiTextureSupported() const
{
	return true;		// Legacy
}

int Terrain::GetNumberOfVertices() const
{
	return m_NumberOfVertices;
}

void Terrain::UpdateNeighbor(Terrain * pTerrain, ParaTerrain::DIRECTION direction)
{
	if(m_pRootBlock == NULL)
		return;

	int thisVertex, otherVertex;
	if (direction == ParaTerrain::DIR_SOUTH)
	{
		for (thisVertex = 0, otherVertex = m_NumberOfVertices - m_WidthVertices; thisVertex < m_WidthVertices; thisVertex++, otherVertex++)
		{
			if (GetVertexStatus(thisVertex)==1)
				pTerrain->SetVertexStatus(otherVertex, 1);
		}
	}
	else if (direction == ParaTerrain::DIR_NORTH)
	{
		for (thisVertex = m_NumberOfVertices - m_WidthVertices, otherVertex = 0; thisVertex < m_NumberOfVertices; thisVertex++, otherVertex++)
		{
			if (GetVertexStatus(thisVertex)==1)
				pTerrain->SetVertexStatus(otherVertex, 1);
		}
	}
	else if (direction == ParaTerrain::DIR_WEST)
	{
		for (thisVertex = 0, otherVertex = m_WidthVertices - 1; thisVertex < m_NumberOfVertices; thisVertex += m_WidthVertices, otherVertex += m_WidthVertices)
		{
			if (GetVertexStatus(thisVertex)==1)
				pTerrain->SetVertexStatus(otherVertex, 1);
		}
	}
	else if (direction == ParaTerrain::DIR_EAST)
	{
		for (thisVertex = m_WidthVertices - 1, otherVertex = 0; thisVertex < m_NumberOfVertices; thisVertex += m_WidthVertices, otherVertex += m_WidthVertices)
		{
			if (GetVertexStatus(thisVertex)==1)
				pTerrain->SetVertexStatus(otherVertex, 1);
		}
	}
	else if (direction == ParaTerrain::DIR_NORTHWEST)
	{
		if (GetVertexStatus(m_NumberOfVertices - m_WidthVertices))
			pTerrain->SetVertexStatus(m_WidthVertices - 1, 1);
	}
	else if (direction == ParaTerrain::DIR_NORTHEAST)
	{
		if (GetVertexStatus(m_NumberOfVertices - 1)==1)
			pTerrain->SetVertexStatus(0, 1);
	}
	else if (direction == ParaTerrain::DIR_SOUTHEAST)
	{
		if (GetVertexStatus(m_WidthVertices - 1)==1)
			pTerrain->SetVertexStatus(m_NumberOfVertices - m_WidthVertices, 1);
	}
	else if (direction == ParaTerrain::DIR_SOUTHWEST)
	{
		if (GetVertexStatus(0)==1)
			pTerrain->SetVertexStatus(m_NumberOfVertices - 1, 1);
	}
}

void Terrain::CreateHoleMap(BYTE* pHoleData, int nLength)
{
	if(pHoleData == NULL)
	{
		SAFE_DELETE_ARRAY(m_pHolemap);
		m_nNumOfHoleVertices = m_NumberOfVertices/(m_nHoleScale*m_nHoleScale);
		m_pHolemap = new bool[m_nNumOfHoleVertices];
		memset(m_pHolemap, 0, sizeof(bool)*m_nNumOfHoleVertices);
	}
	else
	{
		// NOTE: creating from Hole map is not used. 
		SAFE_DELETE_ARRAY(m_pHolemap);
		m_nNumOfHoleVertices = m_NumberOfVertices/(m_nHoleScale*m_nHoleScale);
		m_pHolemap = new bool[m_nNumOfHoleVertices];
		int nSize = (m_nNumOfHoleVertices>nLength) ? nLength : m_nNumOfHoleVertices;
		for(int i=0; i< nLength;i++)
		{
			m_pHolemap[i] = (pHoleData[i]==1)? true :false;
		}
	}
}

void Terrain::UpdateHoles()
{
	if(m_pRootBlock)
		m_pRootBlock->UpdateHoles(this);
}

void Terrain::SetHoleLocal(float x, float y, bool bIsHold)
{
	if(m_pHolemap == NULL)
	{
		/// create a concrete hole map.
		CreateHoleMap(NULL);
	}
	int nRow = (int)(y / (m_VertexSpacing * m_nHoleScale));
	int nCol = (int)(x / (m_VertexSpacing * m_nHoleScale));
	int vertexID = (nRow) * (m_WidthVertices/m_nHoleScale) + (nCol);

	if(vertexID<m_nNumOfHoleVertices && vertexID >=0)
	{
		if(m_pHolemap[vertexID]!=bIsHold)
		{
			m_pHolemap[vertexID] = bIsHold;
			CGlobals::GetOceanManager()->CleanupTerrainCache();
		}
	}
}

void Terrain::SetHole(float x, float y, bool bIsHold)
{
	SetHoleLocal(x- m_OffsetX,y- m_OffsetY,bIsHold);
}

void Terrain::SetHoleScale(int nHoleScale)
{
	m_nHoleScale = nHoleScale;
}

bool Terrain::IsHoleW(float x, float y)
{
	return IsHole(x-m_OffsetX,y-m_OffsetY);
}

bool Terrain::IsHole(float x, float y)
{
	if(CGlobals::GetGlobalTerrain()->TerrainRenderingEnabled())
	{
		if(m_pHolemap == NULL)
			return false;

		bool bIsHole = false;
	
		int nRow = (int)(y / (m_VertexSpacing * m_nHoleScale));
		int nCol = (int)(x / (m_VertexSpacing * m_nHoleScale));
		int vertexID = (nRow) * (m_WidthVertices/m_nHoleScale) + (nCol);

		if(vertexID<m_nNumOfHoleVertices && vertexID >=0)
		{
			bIsHole = m_pHolemap[vertexID];
		}
		return bIsHole;
	}
	else
		return true;
}

float Terrain::GetElevation(int index) const
{
	if(m_pVertices == NULL)
		return DEFAULT_TERRAIN_HEIGHT;

	return m_pVertices[index].z;
}
float Terrain::GetElevationW(float x, float y) const
{
	return GetElevation(x- m_OffsetX,y- m_OffsetY);
}
float Terrain::GetElevation(float x, float y) const
{
	if(m_pVertices == NULL)
	{
		return DEFAULT_TERRAIN_HEIGHT;
	}

	Plane plane;
	int vertexID;
	float elevation;

	if (x < 0.0f || y < 0.0f || GetWidth() < x || GetHeight() < y)
	{
		elevation = 0.0f;
	}
	else
	{
		int nRow = (int)(y / m_VertexSpacing);
		int nCol = (int)(x / m_VertexSpacing);

		// check if the point is inside a hole. 
		bool bIsHole = false;
		if(CGlobals::GetGlobalTerrain()->TerrainRenderingEnabled())
		{
			if(m_pHolemap != NULL)
			{
				vertexID = (nRow/m_nHoleScale) * (m_WidthVertices/m_nHoleScale) + (nCol/m_nHoleScale);
				if(vertexID<m_nNumOfHoleVertices)
				{
					bIsHole = m_pHolemap[vertexID];
				}
			}
		}
		else
		{
			bIsHole = true;
		}
		

		if(bIsHole == true)
		{
			// if it is inside a hole, return -FLOAT_POS_INFINITY
			// elevation =  -FLOAT_POS_INFINITY;
			elevation =  DEFAULT_TERRAIN_HEIGHT;
		}
		else
		{
			// if the point is not inside a hole, return z value using the height map.
			if(nRow >= (m_WidthVertices-1) )
				nRow = (m_WidthVertices-2);
			if(nCol >= (m_HeightVertices-1) )
				nCol = (m_HeightVertices-2);
			vertexID = (nRow) * m_WidthVertices + (nCol);

			if(m_useGeoMipmap)
			{
				if ((fmod(y, m_VertexSpacing) + (m_VertexSpacing - fmod(x, m_VertexSpacing))) <= m_VertexSpacing)
					plane.redefine(m_pVertices[vertexID + m_WidthVertices + 1],m_pVertices[vertexID + 1],m_pVertices[vertexID]);
				else
					plane.redefine(m_pVertices[vertexID + m_WidthVertices], m_pVertices[vertexID + m_WidthVertices + 1], m_pVertices[vertexID]);
			}
			else
			{
				if ((fmod(y, m_VertexSpacing) + fmod(x, m_VertexSpacing)) <= m_VertexSpacing)
					plane.redefine(m_pVertices[vertexID], m_pVertices[vertexID + m_WidthVertices], m_pVertices[vertexID + 1]);
				else
					plane.redefine(m_pVertices[vertexID + 1], m_pVertices[vertexID + 1 + m_WidthVertices], m_pVertices[vertexID + m_WidthVertices]);
			}
				
			elevation = -1.0f * ((plane.a() * (x)+plane.b() * (y)+plane.d) / plane.c());
		}
	}

	return elevation;
}

void Terrain::GetNormalW(float x, float y, float &normalX, float &normalY, float &normalZ) const
{
	return GetNormal(x- m_OffsetX,y- m_OffsetY, normalX, normalY, normalZ);
}

void Terrain::GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ) const
{
	if(m_pVertices == NULL)
	{
		normalX = normalY = 0.0f;
		normalZ = 1.0f;
		return;
	}

	Plane plane;
	int vertexID;

	if (x < 0.0f || y < 0.0f || GetWidth() < x || GetHeight() < y)
	{
		normalX = normalY = 0.0f;
		normalZ = 1.0f;
	}
	else
	{
		vertexID = ((int)(y / m_VertexSpacing)) * m_WidthVertices + ((int)(x / m_VertexSpacing));

		if ((fmod(y, m_VertexSpacing) + fmod(x, m_VertexSpacing)) <= m_VertexSpacing)
			plane.redefine(m_pVertices[vertexID], m_pVertices[vertexID + 1], m_pVertices[vertexID + m_WidthVertices]);
		else
			plane.redefine(m_pVertices[vertexID + 1], m_pVertices[vertexID + 1 + m_WidthVertices], m_pVertices[vertexID + m_WidthVertices]);

		normalX = plane.a();
		normalY = plane.b();
		normalZ = plane.c();
	}
}

void Terrain::SetDetailThreshold(float threshold)
{
	m_DetailThreshold = threshold;

	if(m_DetailThreshold > MIN_DETAILTHRESHOLD)
		m_DetailThreshold = MIN_DETAILTHRESHOLD;
}

float Terrain::GetDetailThreshold() const
{
	return m_DetailThreshold;
}

void Terrain::SetHighResTextureRadius(float radius)
{
	m_fHighResTextureRadius = radius;
}

float Terrain::GetHighResTextureRadius()
{
	return m_fHighResTextureRadius;
}

int Terrain::GetWidthVertices() const
{
	return m_WidthVertices;
}

int Terrain::GetHeightVertices() const
{
	return m_HeightVertices;
}

float Terrain::GetWidth() const
{
	return m_fTerrainSize;//(float)(m_WidthVertices - 1) * m_VertexSpacing;
}

float Terrain::GetHeight() const
{
	return m_fTerrainSize;//(float)(m_HeightVertices - 1) * m_VertexSpacing;
}

int Terrain::GetBaseTextureWidth()
{
	return m_BaseTextureWidth;
}

int Terrain::GetBaseTextureHeight()
{
	return m_BaseTextureHeight;
}

float Terrain::GetMaxElevation() const
{
	return m_MaxElevation;
}

void Terrain::SetTextureFactory(TextureFactory * pFactory)
{
	SAFE_DELETE(m_pDetailedTextureFactory);
	m_pDetailedTextureFactory = (CDetailTextureFactory*) pFactory;

}

float Terrain::GetVertexElevation(int index) const
{
	if (index < 0 || m_NumberOfVertices <= index)
		return 0.0f;
	else
		return m_pVertices[index].z;
}
int Terrain::GetVertexW(float x, float y) const
{
	return GetVertex(x- m_OffsetX,y- m_OffsetY);
}
int Terrain::GetVertex(float x, float y) const
{
	return Math::Round(y / m_VertexSpacing) * m_WidthVertices + Math::Round(x / m_VertexSpacing);
/*
        if ((fmod(y,m_VertexSpacing) + fmod(x,m_VertexSpacing)) <= m_VertexSpacing)
            plane.redefine(m_pVertices[vertexID],m_pVertices[vertexID + m_WidthVertices],m_pVertices[vertexID + 1]);
        else
            plane.redefine(m_pVertices[vertexID + 1],m_pVertices[vertexID + 1 + m_WidthVertices],m_pVertices[vertexID + m_WidthVertices]);

    float vx,vy;

    vx = fmod(x,m_VertexSpacing);
    vy = y / m_VertexSpacing;

    int index;

    if ((fmod(y,m_VertexSpacing) + fmod(x,m_VertexSpacing)) <= m_VertexSpacing)
        index = (int)vy * m_WidthVertices + (int)vx;
    else
        index = ((int)vy + 1) * m_WidthVertices + ((int)vx + 1);

    return index;*/
}

void Terrain::SetVertexElevation(int index, float newElevation, bool recalculate_geometry)
{
	if(m_pRootBlock==NULL)
		return;
	if (0 <= index && index < m_NumberOfVertices)
		m_pVertices[index].z = newElevation;
	if (recalculate_geometry)
		m_pRootBlock->VertexChanged(this, index);
}

void Terrain::RecalcGeometry()
{
	if(m_pRootBlock==NULL)
		return;
	m_pRootBlock->VertexChanged(this);

	// Regenerate vertex normal arrays
	if (m_pNormals!=NULL)
	{
		GenerateTerrainNormal(true);
	}
}

void Terrain::RecalcGeometry(int index1, int index2)
{
	if(m_pRootBlock==NULL)
		return;
	m_pRootBlock->VertexChanged(this, index1, index2);
}

void Terrain::SetVertexElevation(float x, float y, float newElevation)
{
	if(m_pRootBlock==NULL)
		return;
	SetVertexElevation(GetVertex(x, y), newElevation);
}

float Terrain::GetVertexSpacing() const
{
	return m_VertexSpacing;
}

void Terrain::BuildBlocks()
{
#if _USE_RAYTRACING_SUPPORT_ == 0
	if (Settings::GetInstance()->IsHeadless())
		return;
#endif
	numLevels = 0.0f;
	numBlocks = 0.0f;
	for (int i = m_WidthVertices - 1; 2 <= i; i /= 2)
		numLevels += 1.0f;
	for (double j = 0.0f; j < numLevels; j += 1.0f)
		numBlocks += (float)pow(4.0, j);
	if (Settings::GetInstance()->IsVerbose())
	{
		cout << "TERRAIN: Building " << numBlocks << " blocks; please wait..." << endl;
#if _USE_RAYTRACING_SUPPORT_ > 0
		cout << "TERRAIN: Memory required at runtime for blocks = " << numBlocks * (sizeof(TerrainBlock) + 8 * sizeof(Triangle)) << " bytes" << endl;
#else
		cout << "TERRAIN: Memory required at runtime for blocks = " << numBlocks * sizeof(TerrainBlock) << " bytes" << endl;
#endif
		cout << ".............................." << endl;
		hashDelta = (float)numBlocks / 30.0f;
		cout << "#" << flush;
	}
	m_pVertexStatus = new unsigned char[m_WidthVertices * m_HeightVertices];
	
	// We assume that the terrain's width is always a power of 2 + 1!

	//GeoMipmapCode
	m_pRootBlock = new TerrainBlock(0,m_WidthVertices - 1,this,NULL,m_useGeoMipmap);
	m_pRootBlock->CalculateGeometry(this);

	if (Settings::GetInstance()->IsVerbose())
		cout << endl;
}

int Terrain::Tessellate()
{
	if(m_pRootBlock == NULL)
		return 0;

	/** set the render offset for this terrain.*/

	Vector3 vRenderOffset(m_OffsetX, 0, m_OffsetY);
	vRenderOffset -=  CGlobals::GetScene()->GetGlobalTerrain()->GetTerrainRenderOffset();
	SetRenderOffset(vRenderOffset);

	SetEyePosition(CGlobals::GetScene()->GetGlobalTerrain()->GetMatTerrainEye());

	m_bFogEnabled = CGlobals::GetScene()->IsFogEnabled();

	if(m_pVertexStatus)
	{
		ZeroMemory(m_pVertexStatus, sizeof(m_WidthVertices * m_HeightVertices));
	}
	
	m_CountStrips = m_CountFans = 0;
	SetLowestVisibleHeight(FLOAT_POS_INFINITY);
	m_BoundingBox.SetEmpty();
	m_pRootBlock->Tessellate(&m_CountStrips, this);
	return m_CountStrips * 2 + m_CountFans * 6;
}

void Terrain::GetBoundingBoxes( vector<CShapeAABB>& boxes, const Matrix4* modelView, const CShapeFrustum& frustum, int nMaxBoxesNum, int nSmallestBoxStride/*=4*/ )
{
	if(m_pRootBlock == NULL)
		return;
	queue_TerrainBlockPtr_Type queueBlocks;
	queueBlocks.push((TerrainBlock*)m_pRootBlock);

	/// breadth first transversing the quad tree 
	while(!queueBlocks.empty())
	{
		TerrainBlock* pBlock = queueBlocks.front();
		queueBlocks.pop();
		bool bProcessChild = false;
		if(!pBlock->IsHole())
		{
			CShapeAABB boundingBox;
			float halfwidth = pBlock->GetStride() * GetVertexSpacing()/2;
			boundingBox.SetCenter(Vector3(m_pVertices[pBlock->GetHomeIndex()].x+halfwidth, (pBlock->GetMaxElevation()+pBlock->GetMinElevation())/2, m_pVertices[pBlock->GetHomeIndex()].y+halfwidth)+m_vRenderOffset);
			boundingBox.SetExtents(Vector3(halfwidth, (pBlock->GetMaxElevation()-pBlock->GetMinElevation())/2, halfwidth));

			if(modelView!=NULL)
			{
				boundingBox.Rotate(*modelView, boundingBox);
			}
			int nResult = frustum.TestBox(&boundingBox);
			if(nResult>0)
			{
				if((int)boxes.size()<nMaxBoxesNum)
				{
					if(pBlock->GetStride()>nSmallestBoxStride)
					{
						if(nResult==1)
						{
							boxes.push_back(boundingBox);
						}
						else
						{
							// more processing
							bProcessChild = true;
						}
					}
					else
					{
						PE_ASSERT(pBlock->GetStride()==nSmallestBoxStride);
						boxes.push_back(boundingBox);
					}
				}
				else
				{
					boxes.push_back(boundingBox);
				}
			}
		}
		if(bProcessChild && pBlock->m_pChildren)
		{
			queueBlocks.push(pBlock->m_pChildren[0]);
			queueBlocks.push(pBlock->m_pChildren[1]);
			queueBlocks.push(pBlock->m_pChildren[2]);
			queueBlocks.push(pBlock->m_pChildren[3]);
		}
	}
}

CShapeBox Terrain::GetBoundingBoxW()
{
	if(m_BoundingBox.IsValid())
	{
		CShapeBox box;
		box.m_Max.x = m_BoundingBox.m_Max.x + m_OffsetX;
		box.m_Max.y = m_BoundingBox.m_Max.z;
		box.m_Max.z = m_BoundingBox.m_Max.y + m_OffsetY;

		box.m_Min.x = m_BoundingBox.m_Min.x + m_OffsetX;
		box.m_Min.y = m_BoundingBox.m_Min.z;
		box.m_Min.z = m_BoundingBox.m_Min.y + m_OffsetY;
		return box;
	}
	else
		return m_BoundingBox;
}

void Terrain::SetEyePosition(const Vector3& vEye)
{
	m_vEye = vEye;
	m_vEye.x -= m_OffsetX;
	m_vEye.z -= m_OffsetY;
}

void Terrain::RepairCracks()
{
	if(m_pRootBlock)
		m_pRootBlock->RepairCracks(this, &m_CountFans);
}

void Terrain::RebuildRenderBuffer()
{
	//GeoMipmapCode
	if(m_useGeoMipmap)
		return;
	
	if(m_pVertices)
		m_TerrainBuffer.RebuildBuffer();
	else
	{
		// TODO: render a default plane maybe.
	}
}

Texture *Terrain::GetCommonTexture() const
{
	return m_pCommonTexture;
}

void Terrain::SetCommonTexture(const char* fileName)
{
	if(fileName==0)
		return;
	
	if (!CParaFile::DoesAssetFileExist2(fileName, true))
	{
		OUTPUT_LOG("error: terrain common texture file %s is not found. One needs to fix %s\n", fileName, m_sConfigFile.c_str());
		fileName = "Texture/whitedot.png";
		OUTPUT_LOG("default terrain common texture file %s is used instead\n", fileName);
	}

	TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", fileName, TextureEntity::StaticTexture);
	m_sCommonTextureFile = fileName;
	if(pTex)
	{
		Texture* pCommonTex = new Texture(pTex);
		if(pCommonTex)
			SetCommonTexture(pCommonTex);
	}
}

void Terrain::SetCommonTexture(Texture * pTexture)
{
	SAFE_DELETE(m_pCommonTexture);
	m_pCommonTexture = pTexture;
}

bool Terrain::SetCommonTexture(const uint8 * pBuffer, int width, int height)
{
	bool bSuccess = false;
	// Test to see if the image is a power of 2 in both width and height.
	if (!IsPowerOf2(width) || !IsPowerOf2(height))
	{
		string msg("The detail texture image file is NOT a power of 2 in both width and height.\nTexture files must be a power of 2 in both width and height.");
		throw new TerrainException(msg);
	}
	m_pCommonTexture = new Texture(pBuffer, width, height, width, 0, false, Settings::GetInstance()->IsTextureCompression());
	bSuccess = true;
	if (Settings::GetInstance()->IsVerbose())
		cout << "TERRAIN: Common texture set successfully" << endl;
	return bSuccess;
}

bool Terrain::SetBaseTexture(const string& filename, int numTextureCellsX, int numTextureCellsY)
{
	// Reallocate texture cells according to settings.
	AllocateTextureCells(numTextureCellsX, numTextureCellsY);

	m_sBaseTextureFile = filename;
	const char* sBaseTextureFileName = filename.c_str();

	// load from image
	CParaFile cFile;
	cFile.OpenAssetFile(filename.c_str(), true, ParaTerrain::Settings::GetInstance()->GetMediaPath());
	if(cFile.isEof())
	{
		OUTPUT_LOG("error: terrain base texture file %s is not found. One needs to fix %s\n", filename.c_str(), m_sConfigFile.c_str());
		sBaseTextureFileName = "Texture/tileset/generic/MainTexture.dds";
		cFile.OpenAssetFile(sBaseTextureFileName);
		if (cFile.isEof())
		{
			return false;
		}
		else
		{
			OUTPUT_LOG("default terrain base texture file %s is used instead\n", sBaseTextureFileName);
		}
	}
#ifdef USE_OPENGL_RENDERER
	/** in opengl, we will not chop texture. */
	m_bChopBaseTexture = false;
#endif
	if (m_bChopBaseTexture)
	{
		// Load the texture data.
		int texWidth, texHeight;
		uint8 *pTextureImage = NULL;
		TextureEntity::LoadImage(cFile.getBuffer(), (int)cFile.getSize(), texWidth, texHeight, &pTextureImage, false);
		if(pTextureImage == NULL)
			return false;
		
		bool bFlipImages = true;
		if(bFlipImages)
		{
			uint8 *pBufferNew = new uint8[texWidth * texHeight * 3];
			for (int y = 0; y < texWidth; y++)
			{
				for (int x = 0; x < texHeight; x++)
				{
					int sourceIndex = y * texWidth * 3 + x * 3;
					int destIndex = ((texHeight - y) - 1) * texWidth * 3 + x * 3;
					for (int i = 0; i < 3; i++)
						pBufferNew[destIndex + i] = pTextureImage[sourceIndex + i];
				}
			}
			SAFE_DELETE_ARRAY(pTextureImage);
			pTextureImage = pBufferNew;
		}

		if (!Settings::GetInstance()->UseBorders())
		{
			// Test to see if the image is a power of 2 in both width and height.
			if (!IsPowerOf2(texWidth) || !IsPowerOf2(texHeight))
			{
				OUTPUT_LOG("terrain base texture should be power of 2 in both width and height: %s\n", filename.c_str());
				delete[]pTextureImage;
				return false;
			}
			m_BaseTextureWidth = texWidth;
			m_BaseTextureHeight = texHeight;

			int tileSize = texWidth / m_NumberOfTextureTilesWidth;
			m_TileSize = tileSize;
			int cellIndex = 0;
			// Create texture tiles by roaming across the image.
			for (int i = 0; i < texHeight; i += tileSize)
			{
				for (int j = 0; j < texWidth; j += tileSize)
				{
					TextureCell *pCell = m_TextureCells[cellIndex];
					if (pCell)
					{
						const uint8 *pTile = pTextureImage + i * texWidth * 3 + j * 3;
						Texture *pTexture = new Texture(pTile, tileSize, tileSize, texWidth, 0, true, Settings::GetInstance()->IsTextureCompression());
						// pTexture->SetBufferPersistent(true);
						pCell->SetTexture(pTexture);
					}
					++cellIndex;
				}
			}
		}
		else
		{
			// TODO: 
		}
		delete[]pTextureImage;
	}
	else
	{
		// for non-chopped textures, the texture coordinate of base later 0, should be 8 times bigger. 
		TextureEntity* pTexture = CGlobals::GetAssetManager()->LoadTexture(sBaseTextureFileName, sBaseTextureFileName);
		if (pTexture)
		{
			for (uint32 i = 0; i < (int)m_TextureCells.size(); ++i)
			{
				TextureCell *pCell = m_TextureCells[i];
				if (pCell)
				{
					Texture *pTex = new Texture(pTexture);
					pCell->SetTexture(pTex);
				}
			}
		}
	}
	return true;
}
bool Terrain::SetTexture(const uint8 * pBuffer, int width, int height)
{
	if (width <= 256 || height <= 256)
		throw new TerrainException("The overall terrain texture must be > 256 in both width and height");
	for (uint32 i = 0; i < m_TextureCells.size(); i++)
	{
		SAFE_DELETE(m_TextureCells[i]);
	}
	m_TextureCells.clear();

	m_BaseTextureWidth = width;
	m_BaseTextureHeight = height;
	if (Settings::GetInstance()->IsHeadless())
		return true;
	bool bSuccess = false;
	if (!Settings::GetInstance()->UseBorders())
	{
		// Test to see if the image is a power of 2 in both width and height.
		if (!IsPowerOf2(width) || !IsPowerOf2(height))
		{
			string msg("The texture is NOT a power of 2 in both width and height.\nTextures must be a power of 2 in both width and height.");
			throw new TerrainException(msg);
		}
	}
	ChopTexture(pBuffer, width, height, 256);
	bSuccess = true;
	if (Settings::GetInstance()->IsVerbose())
		cout << "TERRAIN: Texture set successfully" << endl;
	if (Settings::GetInstance()->GetPreloadTextures())
		PreloadTextures();
	return bSuccess;
}

void Terrain::ChopTexture(const uint8 * pImage, int width, int height, int tileSize)
{
	// It is assumed that the image is in a 3-unsigned char per pixel, RGB format, with no padding on the pixel rows
	if (Settings::GetInstance()->UseBorders())
	{
		m_NumberOfTextureTilesWidth = (width - 1) / (tileSize + 1);
		m_NumberOfTextureTilesHeight = (height - 1) / (tileSize + 1);
	}
	else
	{
		m_NumberOfTextureTilesWidth = width / tileSize;
		m_NumberOfTextureTilesHeight = height / tileSize;
	}
	m_NumberOfTextureTiles = m_NumberOfTextureTilesWidth * m_NumberOfTextureTilesHeight;
	float verticesPerTileWidth = (float)(m_WidthVertices + m_NumberOfTextureTilesWidth - 1) / (float)m_NumberOfTextureTilesWidth;
	float verticesPerTileHeight = (float)(m_HeightVertices + m_NumberOfTextureTilesHeight - 1) / (float)m_NumberOfTextureTilesHeight;
	if(verticesPerTileWidth<m_MaximumVisibleBlockSize)
		m_MaximumVisibleBlockSize = (int)verticesPerTileWidth-1;
	m_TextureTileWidth = (verticesPerTileWidth - 1.0f) * m_VertexSpacing;
	m_TextureTileHeight = (verticesPerTileHeight - 1.0f) * m_VertexSpacing;
	m_TileSize = tileSize;

	int cellIndex = 0;

	if (Settings::GetInstance()->UseBorders())
	{
		int i;
		// Create texture tiles by roaming across the bordered image.
		for (i = 0; i < height - 1; i += m_TileSize + 1)
		{
			for (int j = 0; j < width - 1; j += m_TileSize + 1)
			{
				const uint8 *pTile = pImage + i * width * 3 + j * 3;
				Texture *pTexture = new Texture(pTile, m_TileSize + 2, m_TileSize + 2, width, 1, true, Settings::GetInstance()->IsTextureCompression());
				TextureCell *pCell = new TextureCell(cellIndex++);
				pCell->SetTexture(pTexture);
				m_TextureCells.push_back(pCell);
			}
		}
	}
	else
	{
		// Create texture tiles by roaming across the image.
		for (int i = 0; i < height; i += tileSize)
		{
			for (int j = 0; j < width; j += tileSize)
			{
				const uint8 *pTile = pImage + i * width * 3 + j * 3;
				Texture *pTexture = new Texture(pTile, tileSize, tileSize, width, 0, true, Settings::GetInstance()->IsTextureCompression());
				TextureCell *pCell = new TextureCell(cellIndex++);
				pCell->SetTexture(pTexture);
				m_TextureCells.push_back(pCell);
			}
		}
	}
}

void Terrain::GenerateTexture(int widthTexels, int heightTexels, const char *szGeneratorName)
{
	string name(szGeneratorName);
	TextureGenerator *pGenerator = m_TextureGenerators[name];
	if (pGenerator == NULL)
		throw new TerrainException("The specified texture generator does not exist");
	GenerateTexture(widthTexels, heightTexels, pGenerator);
}

void Terrain::GenerateTexture(int widthTexels, int heightTexels, TextureGenerator * pGenerator)
{
	pGenerator->Init(this, widthTexels, heightTexels);
	pGenerator->Generate();
}

void Terrain::RegisterTextureGenerator(string name, TextureGenerator * pGenerator)
{
	Terrain::m_TextureGenerators[name] = pGenerator;
}

void Terrain::RegisterDefaultGenerators()
{
//      Terrain::RegisterTextureGenerator("White",new WhiteTextureGenerator);
//      Terrain::RegisterTextureGenerator("GrassyFlats",new GrassyFlatsTextureGenerator);
}

void Terrain::SetMaximumVisibleBlockSize(int stride)
{
	m_MaximumVisibleBlockSize = stride;
}

int Terrain::GetMaximumVisibleBlockSize()
{
	return m_MaximumVisibleBlockSize;
}

int Terrain::ModelViewMatrixChanged()
{
	int count = Tessellate();
	RepairCracks();
	RebuildRenderBuffer();
	return count;
}
void Terrain::Paint(TextureEntity* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
{
	if(m_pRootBlock == NULL)
		return;
	TextureSet* pTexSet =  GetTextureSet();
	if(pTexSet)
	{
		if(detailTexture)
		{
			int detailTextureIndex = pTexSet->FindTexture(detailTexture);
			if(detailTextureIndex<0)
			{
				detailTextureIndex = pTexSet->AddTexture(new Texture(detailTexture));
			}
			if(detailTextureIndex >= 0)
			{
				Paint(detailTextureIndex, brushRadius, brushIntensity, maxIntensity, erase, x,y);
			}
		}
		else
		{
			Paint(-1, brushRadius, brushIntensity, maxIntensity, erase, x,y);
		}
	}
}
void Terrain::Paint(int detailTextureIndex, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y)
{
	if(m_pRootBlock == NULL)
		return;
	int nRadius = (int)(brushRadius*(float)Settings::GetInstance()->GetTextureMaskWidth()*2.f/GetTextureTileWidth());
	
	brush.SetWidth(nRadius*2+1);
	brush.SetIntensity(brushIntensity);
	brush.SetMaxIntensity(maxIntensity);
	brush.SetErase(erase);
	brush.Paint(this, detailTextureIndex, x- m_OffsetX,y- m_OffsetY);
}

uint8 *Terrain::GetMaskBits(int textureCellX, int textureCellY, int detailIndex, int &maskWidth, int &maskHeight)
{
	if (0 <= textureCellX && (uint32)textureCellX < m_NumberOfTextureTilesWidth && 0 <= textureCellY && (uint32)textureCellY < m_NumberOfTextureTilesHeight)
	{
		TextureCell *pCell = GetTextureCell(textureCellX, textureCellY);
		DetailTexture *pDet = pCell->GetDetail(GetTextureSet()->GetTexture(detailIndex));
		if (pDet == NULL)
		{
			pDet = new DetailTexture(GetTextureSet()->GetTexture(detailIndex));
			pCell->AddDetail(pDet);
		}
		Texture *pMask = pDet->GetMask();
		if(pMask==0)
		{
			pMask = pDet->RegenerateMask(detailIndex>=0 ? 0 : 0xff);
		}
		// Mask sizes are actually fixed right now, but this will allow flexibility in the future.
		maskWidth = pCell->GetDetailMaskImageWidth(detailIndex);
		maskHeight = pCell->GetDetailMaskImageHeight(detailIndex);

		return pMask ? pMask->GetBuffer(): NULL;
	}
	return NULL;
}

void Terrain::ReloadMask(int textureCellX, int textureCellY, int detailIndex)
{
	if (0 <= textureCellX && (uint32)textureCellX < m_NumberOfTextureTilesWidth && 0 <= textureCellY && (uint32)textureCellY < m_NumberOfTextureTilesHeight)
	{
		TextureCell *pCell = GetTextureCell(textureCellX, textureCellY);
		DetailTexture *pDet = pCell->GetDetail(GetTextureSet()->GetTexture(detailIndex));
		if (pDet != NULL)
		{
			Texture *pMask = pDet->GetMask();
			if(pMask)
			{
				pMask->UnloadTexture();
				//pMask->UploadTexture(); // lazy loading instead.
			}
		}
	}
}

int Terrain::NormalizeMask( int textureCellX, int textureCellY, int detailIndex, bool bReload)
{
	if (0 <= textureCellX && (uint32)textureCellX < m_NumberOfTextureTilesWidth && 0 <= textureCellY && (uint32)textureCellY < m_NumberOfTextureTilesHeight)
	{
		TextureCell *pCell = GetTextureCell(textureCellX, textureCellY);
		if(pCell)
		{
			int index = pCell->GetDetailIndex(GetTextureSet()->GetTexture(detailIndex));
			if(index >= -1)
			{
				return pCell->NormalizeMask(index);
			}
		}
	}
	return 0;
}

void Terrain::ResizeTextureMaskWidth( int nWidth )
{
	/// export all alpha mask data to disk with the name:
	int nCellCount = (int)m_TextureCells.size();
	
	// number of cells, usually 8x8=64
	for (int i = 0; i < nCellCount; ++i)
	{
		if(m_TextureCells[i])
		{
			m_TextureCells[i]->ResizeTextureMaskWidth(nWidth);
		}
	}
}

//int*	g_TriangleGroups = NULL;
//void Terrain::RenderTextureGroup(int i)
//{
//	int numVertice = g_TriangleGroups[i+1]-g_TriangleGroups[i];
//	if(numVertice >= 3)
//		DirectXPerf::DrawPrimitive( CGlobals::GetRenderDevice(), DirectXPerf::DRAW_PERF_TRIANGLES_MESH, EPrimitiveType::TRIANGLELIST, g_TriangleGroups[i], (numVertice)/3 );
//}

void Terrain::Render()
{
	if(m_pRootBlock == NULL)
		return;

	if(m_useGeoMipmap)
	{
		RenderGeoMipmap();
		return;
	}

	/** draw nothing if there is nothing to draw */
	if(m_TerrainBuffer.HasData() == false)
	{
		m_TerrainBuffer.RebuildBuffer();
		if(m_TerrainBuffer.HasData() == false)
		{
			return;
		}
	}

	// just for hit count
	m_pDetailedTextureFactory->AdvanceFrame();
	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();

	bool bUseNormal = Settings::GetInstance()->UseNormals();

	if(!bUseNormal)
		CGlobals::GetRenderDevice()->SetStreamSource(0, m_TerrainBuffer.GetVertexBufferDevicePtr(), 0, sizeof(terrain_vertex));
	else
		CGlobals::GetRenderDevice()->SetStreamSource(0, m_TerrainBuffer.GetVertexBufferDevicePtr(), 0, sizeof(terrain_vertex_normal));

	TextureGroups_Type&  textureGroups = m_TerrainBuffer.m_textureGroups;

	CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if ( pEffectFile == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		//////////////////////////////////////////////////////////////////////////
		// render using fixed function pipeline
		/** 
		* render the terrain by texture groups
		*/
		int nTextureGroupCount = (int)(textureGroups.size()) - 1;
		for(int nTextureID =0;nTextureID<nTextureGroupCount;++nTextureID)
		{
			/** 
			* for each texture group, we will use multi-texture stage to render blended textures at the same time 
			*/
			TextureGroup& texGroup = textureGroups[nTextureID];
			if(texGroup.nNumTriangles>0)
			{
				/**
				* Pass 1: base texture:
				* 		texture stage0 - main low-res texture
				* 		texture stage1 - overall detail (dirt) texture
				*/
				TextureCell *pCell = m_TextureCells[nTextureID]; // TODO: deal with i th group

				if(pCell == NULL)
					continue;

				if (Settings::GetInstance()->IsBaseTextureEnabled())
				{		
					DeviceTexturePtr_type texId = pCell->BindTexture();
					GETD3D(CGlobals::GetRenderDevice())->SetTexture(0, texId );

					if (m_pCommonTexture != NULL)
					{
						if (Settings::GetInstance()->IsBaseTextureEnabled())
						{
							DeviceTexturePtr_type cTexId = m_pCommonTexture->UploadTexture();
							GETD3D(CGlobals::GetRenderDevice())->SetTexture(1, cTexId );
						}
					}
					// render triangles for the current texture group 
					CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
				}

				/**
				* Pass 2: render detail textures with alpha blending:
				*	for each detailed texture layer
				* 		texture stage0 - alpha mask 
				* 		texture stage1 - detail texture
				*/
				if (pCell->GetNumberOfDetails() > 0)
				{
					pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, FALSE );
					pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);

					for (int k = 0; k < pCell->GetNumberOfDetails(); k++)
					{
						GETD3D(CGlobals::GetRenderDevice())->SetTexture(0, pCell->BindMask(k) );
						GETD3D(CGlobals::GetRenderDevice())->SetTexture(1, pCell->BindDetail(k) );

						// render triangles for the current texture group 
						CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
					}
					// restore states
					pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
					pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
				}
			}
		}//for( itCurCP = 

		TextureGroup& texGroup = textureGroups[textureGroups.size() - 1];
		if(texGroup.nNumTriangles>0)
		{
			// render all terrain in the distance fog using the simple fog color.
			// TODO: do this with fixed function pipeline. 
			// DirectXPerf::DrawPrimitive( pRenderDevice, DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,  EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
		}
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// render using programmable pipeline
		int nMaxNumGPUTextures = pRenderDevice->GetMaxSimultaneousTextures();
		// my shader only support 8, so ...
		if(nMaxNumGPUTextures >SIMULTANEOUS_TEXTURE_NUM)
			nMaxNumGPUTextures = SIMULTANEOUS_TEXTURE_NUM;
		if(pEffectFile->GetCurrentTechniqueDesc()->nCategory == CEffectFile::TechCategory_GenShadowMap)
		{
			//////////////////////////////////////////////////////////////////////////
			// render shadow map
			if(pEffectFile->begin(true))
			{
				pEffectFile->setParameter(CEffectFile::k_posScaleOffset, Vector3(0,0,0).ptr());
				if(pEffectFile->BeginPass(0))
				{
					/** 
					* render the terrain by texture groups
					*/
					int nTextureGroupCount = (int)(textureGroups.size()) - 1;
					for(int nTextureID =0;nTextureID<nTextureGroupCount;++nTextureID)
					{
						/** 
						* for each texture group, we will use multi-texture stage to render blended textures at the same time 
						*/
						TextureGroup& texGroup = textureGroups[nTextureID];
						if(texGroup.nNumTriangles>0)
						{
							// render triangles for the current texture group 
							pEffectFile->CommitChanges();
							CGlobals::GetRenderDevice()->DrawPrimitive( EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
						}
					}//for( itCurCP = 
					pEffectFile->EndPass();
				}
				pEffectFile->end();
			}
		}
		else
		{
			const int LAYER_BASE_INDEX = 10;
			//////////////////////////////////////////////////////////////////////////
			// render terrain 
			if(pEffectFile->begin(true))
			{
				pEffectFile->setParameter(CEffectFile::k_posScaleOffset, Vector3(0,0,0).ptr());
				pEffectFile->setParameter(CEffectFile::k_texCoordOffset, Vector3(m_vRenderOffset.x, m_vRenderOffset.z, GetWidth()).ptr());
				
				if(pEffectFile->BeginPass(bUseNormal ? 0 : 2))
				{
#ifdef USE_DIRECTX_RENDERER
					// bind shadow map on tex2
					if(CGlobals::GetScene()->IsShadowMapEnabled())
					{
						CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
						if (pShadowMap !=NULL)
						{
							CGlobals::GetEffectManager()->EnableUsingShadowMap(true);
							if(pShadowMap->UsingBlurredShadowMap())
							{
								pEffectFile->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
								pShadowMap->SetShadowTexture(*pEffectFile, 2, 1);
							}
							else
							{
								pEffectFile->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
								pShadowMap->SetShadowTexture(*pEffectFile, 2);
							}
						}
					}
					else
					{
						pEffectFile->EnableShadowmap(0);
					}
#endif
					
					bool bBaseEnabled = Settings::GetInstance()->IsBaseTextureEnabled();
					

					/** 
					* render the terrain by texture groups
					*/
					int nTextureGroupCount = (int)(textureGroups.size())-1;
					for(int nTextureID =0;nTextureID<nTextureGroupCount;++nTextureID)
					{
						/** 
						* for each texture group, we will use multi-texture stage to render blended textures at the same time 
						*/
						TextureGroup& texGroup = textureGroups[nTextureID];
						if(texGroup.nNumTriangles>0)
						{
							/** three groups at a time. 
							* ( alpha0 + detail1 ) + ( alpha3 + detail4 ) + ( alpha5 + detail6 )
							*/
							TextureCell *pCell;
							pCell = m_TextureCells[nTextureID]; // TODO: deal with i th group

							if(pCell == NULL)
								continue;

							//////////////////////////////////////////////////////////////////////////
							// this is for GPU with nMaxNumGPUTextures(8) textures
							// we will only draw the first MAX_TEXTURE_LAYERS layers.
							int nLayers = min(pCell->GetNumberOfDetails()+1, MAX_TEXTURE_LAYERS);

							int nPass = 0;
							int nFinishedLayers = 0;
							int nCurrentLayers = 0;
							int nCurrentTexIndex = 0;
						
							if (bBaseEnabled)
							{	
								// the seventh texture is for non-repeatable base layer
								int nBaseIndex = 7;
#ifdef USE_OPENGL_RENDERER
								nBaseIndex = 2;
#endif
								auto pTexture = pCell->GetDetailBase()->BindTexture();
								if (pTexture == 0){
									pTexture = CGlobals::GetAssetManager()->GetDefaultTexture(0)->GetTexture();
								}
								pEffectFile->setTexture(nBaseIndex, pTexture);
							}
							// first layer (common repeatable texture) is a little special, we will skip it if m_pCommonTexture is not provided
							// TODO: we shall also skip it if alpha is 0
							if(m_pCommonTexture != NULL)
							{
								// bind alpha for first layer
								if(pCell->GetDetailBase()->GetMask() != 0)
									pEffectFile->setTexture(0, pCell->GetDetailBase()->BindMask());
								else
								{
									// bind the default white mask, if the mask does not exist in the base layer.  
									DetailTexture* pWhiteMask = GetDefaultBaseLayerMask();
									if(pWhiteMask)
										pEffectFile->setTexture(0, pWhiteMask->BindMask() );
								}
								// bind detail for first layer
								pEffectFile->setTexture(1, m_pCommonTexture->UploadTexture() );
								// skip the shadow map here
								nCurrentTexIndex = 3; 
							}
							nCurrentLayers = 1;

							for (int i =nCurrentLayers;i<nLayers;++i)
							{
								if((nCurrentTexIndex+3)>nMaxNumGPUTextures)
								{
									//////////////////////////////////////////////////////////////////////////
									// there are no available GPU texture registers, we have to end this pass,
									// and render the following layers in another pass.
									pEffectFile->SetBoolean(LAYER_BASE_INDEX, false);
									CGlobals::GetEffectManager()->EnableD3DAlphaBlending(nPass > 0);
									int nTotalLayers = nLayers-nFinishedLayers;
									for (int layer=1; layer<=2;++layer)
									{
										pEffectFile->SetBoolean(LAYER_BASE_INDEX+layer, layer<nTotalLayers);
									}

									pEffectFile->CommitChanges();
									// render triangles for the current texture group 
									CGlobals::GetRenderDevice()->DrawPrimitive( EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
									nFinishedLayers = i;
									++nPass;
									nCurrentTexIndex = 0; // the next pass begin with index 0;
								}

								//////////////////////////////////////////////////////////////////////////
								// add a new layer
								int nCurrentAlphaLayer = nCurrentLayers-1;
								if(nCurrentTexIndex == 0)
								{ // tex 0,1
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindMask(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindDetail(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									nCurrentTexIndex = 3; // side step shadow map
								}
								else if(nCurrentTexIndex >=3)
								{ // tex (3,4), (5,6),
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindMask(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindDetail(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
								}
								++nCurrentLayers;
							}
							//////////////////////////////////////////////////////////////////////////
							// End the final pass.
							pEffectFile->SetBoolean(LAYER_BASE_INDEX, true);
							CGlobals::GetEffectManager()->EnableD3DAlphaBlending(nPass > 0);
							int nTotalLayers = nLayers-nFinishedLayers;
							for (int layer=1; layer<=2;++layer)
							{
								pEffectFile->SetBoolean(LAYER_BASE_INDEX+layer, layer<nTotalLayers);
							}
							pEffectFile->CommitChanges();
							// render triangles for the current texture group 
							CGlobals::GetRenderDevice()->DrawPrimitive(    EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
							nFinishedLayers = nLayers;
							++nPass;
							nCurrentTexIndex = 0; // the next pass begin with index 0;
						}
					}//for( itCurCP = 
					pEffectFile->EndPass();
				}

				TextureGroup& texGroup = textureGroups[textureGroups.size() - 1];
				if(texGroup.nNumTriangles>0)
				{
					// render all terrain in the distance fog using the simple fog color.
					if(pEffectFile->BeginPass(1))
					{
						CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
						pEffectFile->CommitChanges();
						// render triangles for the current texture group 
						CGlobals::GetRenderDevice()->DrawPrimitive(    EPrimitiveType::TRIANGLELIST, texGroup.nStartIndex*3, texGroup.nNumTriangles);
						pEffectFile->EndPass(1);
					}
				}
				pEffectFile->end();
			}
		}
	}
}


////////////////////////////////////////////////////////////
//geoMipmapCode
///////////////////////////////////////////////////////////
void Terrain::BuildGeoMipmapBuffer()
{
	if(m_pVertices != NULL)
	{
		m_TerrainBuffer.BuildGeoMipmapBuffer();
	}

	if(m_isEditorMode && (m_visibleDataMask!=0xffffffff))
	{
		if(m_pTerrainInfoData == NULL)
			CreateTerrainInfoData();

		BuildHelperMeshBuffer();
	}
}

void Terrain::RenderGeoMipmap()
{
	if(m_pVertices == NULL)
		return;
#ifdef USE_DIRECTX_RENDERER
	// just for hit count
	m_pDetailedTextureFactory->AdvanceFrame();
	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
	bool bUseNormal = Settings::GetInstance()->UseNormals();

	if(!bUseNormal)
		GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(0, m_TerrainBuffer.GetVertexBufferDevicePtr(), 0, sizeof(terrain_vertex));
	else
		GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(0, m_TerrainBuffer.GetVertexBufferDevicePtr(), 0, sizeof(terrain_vertex_normal));

	GETD3D(CGlobals::GetRenderDevice())->SetIndices(m_pIndexBuffer.GetDevicePointer());

	//The order of triangle index is different from the order of vertex,
	//so we have to change cull mode here -.-        --clayman 2012.2.21
	CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CW);

	CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();

	if ( pEffectFile == 0)
	{
		//too bad, maybe we're running on some very old intel mobile chipset
		//fallback to ROAM style terrain

		CGlobals::GetGlobalTerrain()->SetGeoMipmapTerrain(false);
		
		GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(0,NULL,0,0);
		GETD3D(CGlobals::GetRenderDevice())->SetIndices(NULL);
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);

		return;
		/*
		int blockCount = m_pRootBlock->GetChildChunkCount();
		for(int i=0;i<blockCount;i++)
		{
			TerrainBlock* block = m_pRootBlock->GetChildBlock(i);
			if(block->GetFrustumState() > 0)
			{
				TextureCell* pCell = m_TextureCells[block->GetTextureGroupId()];
				if(pCell == NULL)
					continue;
				
				if (Settings::GetInstance()->IsBaseTextureEnabled())
				{		
					DeviceTexturePtr_type texId = pCell->BindTexture();
					pRenderDevice->SetTexture(0, texId );

					if (m_pCommonTexture != NULL)
					{
						if (Settings::GetInstance()->IsBaseTextureEnabled())
						{
							DeviceTexturePtr_type cTexId = m_pCommonTexture->UploadTexture();
							pRenderDevice->SetTexture(1, cTexId );
						}
					}
					
					IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());

					DirectXPerf::DrawIndexedPrimitive(pRenderDevice,DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,EPrimitiveType::TRIANGLELIST,
						block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
						idxInfo->GetIndexCount()/3);
				}
				*/
				/**
				* Pass 2: render detail textures with alpha blending:
				*	for each detailed texture layer
				* 		texture stage0 - alpha mask 
				* 		texture stage1 - detail texture
				*/
		/*
				if (pCell->GetNumberOfDetails() > 0)
				{
					pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, FALSE );
					pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
					pRenderDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

					for (int k = 0; k < pCell->GetNumberOfDetails(); k++)
					{
						pRenderDevice->SetTexture(0, pCell->BindMask(k) );
						pRenderDevice->SetTexture(1, pCell->BindDetail(k) );

						// render triangles for the current texture group 
						//DirectXPerf::DrawPrimitive( pRenderDevice, DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,  EPrimitiveType::TRIANGLELIST,0,0);
						IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());
						DirectXPerf::DrawIndexedPrimitive(pRenderDevice,DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,EPrimitiveType::TRIANGLELIST,
							block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
							idxInfo->GetIndexCount()/3);
					}
					// restore states
					pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
					pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
					pRenderDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE   );
				}
			}
		}
		*/
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// render using programmable pipeline
		int nMaxNumGPUTextures = pRenderDevice->GetMaxSimultaneousTextures(); // usually 4, 8 or above
		// my shader only support 8, so ...
		if(nMaxNumGPUTextures >SIMULTANEOUS_TEXTURE_NUM)
			nMaxNumGPUTextures = SIMULTANEOUS_TEXTURE_NUM;
		if(pEffectFile->GetCurrentTechniqueDesc()->nCategory == CEffectFile::TechCategory_GenShadowMap)
		{
			//////////////////////////////////////////////////////////////////////////
			// render shadow map
			if(pEffectFile->begin(true))
			{
				pEffectFile->setParameter(CEffectFile::k_posScaleOffset, m_vRenderOffset.ptr());
				if(pEffectFile->BeginPass(0))
				{

					int blockCount = m_pRootBlock->GetChildChunkCount();
					for(int i=0;i<blockCount;i++)
					{
						TerrainBlock* block = m_pRootBlock->GetChildBlock(i);
						if(block->GetFrustumState() > 0)
						{
							IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());
							CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
								block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
								idxInfo->GetIndexCount()/3);
						}
					}
					pEffectFile->EndPass();
				}
				pEffectFile->end();
			}
		}
		else
		{
			const int LAYER_BASE_INDEX = 10;
			//////////////////////////////////////////////////////////////////////////
			// render terrain 
			if(pEffectFile->begin(true))
			{
				pEffectFile->setParameter(CEffectFile::k_posScaleOffset, m_vRenderOffset.ptr());

				if(pEffectFile->BeginPass(bUseNormal ? 0 : 2))
				{
					// bind shadow map on tex2
					if(CGlobals::GetScene()->IsShadowMapEnabled())
					{
						CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
						if (pShadowMap !=NULL)
						{
							CGlobals::GetEffectManager()->EnableUsingShadowMap(true);
							if(pShadowMap->UsingBlurredShadowMap())
							{
								pEffectFile->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
								pShadowMap->SetShadowTexture(*pEffectFile, 2, 1);
							}
							else
							{
								pEffectFile->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
								pShadowMap->SetShadowTexture(*pEffectFile, 2);
							}
						}
					}
					else
					{
						pEffectFile->EnableShadowmap(0);
					}

					bool bBaseEnabled = Settings::GetInstance()->IsBaseTextureEnabled();
					
					int blockCount = m_pRootBlock->GetChildChunkCount();
					for(int i=0;i<blockCount;i++)
					{
						TerrainBlock* block = m_pRootBlock->GetChildBlock(i);
						if(block->GetFrustumState() > 0 && !block->IsInFog()) 
						{
							/** three groups at a time. 
							* ( alpha0 + detail1 ) + ( alpha3 + detail4 ) + ( alpha5 + detail6 )
							*/
							TextureCell *pCell;
							pCell = m_TextureCells[block->GetTextureGroupId()]; 

							if(pCell == NULL)
								continue;

							//////////////////////////////////////////////////////////////////////////
							// this is for GPU with nMaxNumGPUTextures(8) textures
							// we will only draw the first MAX_TEXTURE_LAYERS layers.
							int nLayers = min(pCell->GetNumberOfDetails()+1, MAX_TEXTURE_LAYERS);
							int nPass = 0;
							int nFinishedLayers = 0;
							int nCurrentLayers = 0;
							int nCurrentTexIndex = 0;
						
							if (bBaseEnabled)
							{	
								// the seventh texture is for non-repeatable base layer
								pEffectFile->setTexture(7, pCell->GetDetailBase()->BindTexture() );
							}
							// first layer (common repeatable texture) is a little special, we will skip it if m_pCommonTexture is not provided
							// TODO: we shall also skip it if alpha is 0
							if(m_pCommonTexture != NULL)
							{
								// bind alpha for first layer
								if(pCell->GetDetailBase()->GetMask() != 0)
									pEffectFile->setTexture(0, pCell->GetDetailBase()->BindMask() );
								else
								{
									// bind the default white mask, if the mask does not exist in the base layer.  
									DetailTexture* pWhiteMask = GetDefaultBaseLayerMask();
									if(pWhiteMask)
										pEffectFile->setTexture(0, pWhiteMask->BindMask() );
								}
								// bind detail for first layer
								pEffectFile->setTexture(1, m_pCommonTexture->UploadTexture() );
								// skip the shadow map here
								nCurrentTexIndex = 3; 
							}
							nCurrentLayers = 1;

							for (int i =nCurrentLayers;i<nLayers;++i)
							{
								if((nCurrentTexIndex+3)>nMaxNumGPUTextures)
								{
									//////////////////////////////////////////////////////////////////////////
									// there are no available GPU texture registers, we have to end this pass,
									// and render the following layers in another pass.
									pEffectFile->SetBoolean(LAYER_BASE_INDEX, false);
									CGlobals::GetEffectManager()->EnableD3DAlphaBlending(nPass > 0);
									int nTotalLayers = nLayers-nFinishedLayers;
									for (int layer=1; layer<=2;++layer)
									{
										pEffectFile->SetBoolean(LAYER_BASE_INDEX+layer, layer<nTotalLayers);
									}
									pEffectFile->CommitChanges();
									IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());
									CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
										block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
										idxInfo->GetIndexCount()/3);
									nFinishedLayers = i;
									++nPass;
									nCurrentTexIndex = 0; // the next pass begin with index 0;
								}

								//////////////////////////////////////////////////////////////////////////
								// add a new layer
								int nCurrentAlphaLayer = nCurrentLayers-1;
								if(nCurrentTexIndex == 0)
								{ // tex 0,1
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindMask(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindDetail(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									nCurrentTexIndex = 3; // side step shadow map
								}
								else if(nCurrentTexIndex >=3)
								{ // tex (3,4), (5,6),
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindMask(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
									pEffectFile->setTexture(nCurrentTexIndex, pCell->BindDetail(nCurrentAlphaLayer) );
									++nCurrentTexIndex;
								}
								++nCurrentLayers;
							}
							//////////////////////////////////////////////////////////////////////////
							// End the final pass.
							pEffectFile->SetBoolean(LAYER_BASE_INDEX, true);
							CGlobals::GetEffectManager()->EnableD3DAlphaBlending(nPass > 0);
							int nTotalLayers = nLayers-nFinishedLayers;
							for (int layer=1; layer<=2;++layer)
							{
								pEffectFile->SetBoolean(LAYER_BASE_INDEX+layer, layer<nTotalLayers);
							}
							pEffectFile->CommitChanges();
							
							IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());

							CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
								block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
								idxInfo->GetIndexCount()/3);
							nFinishedLayers = nLayers;
							++nPass;
							nCurrentTexIndex = 0; // the next pass begin with index 0;
						}
					}
					pEffectFile->EndPass();
				}

				//fog
				int blockCount = m_pRootBlock->GetChildChunkCount();
				for(int i=0;i<blockCount;i++)
				{
					TerrainBlock* block = m_pRootBlock->GetChildBlock(i);
					if(block->IsInFog() && block->GetFrustumState() > 0)
					{
						if(pEffectFile->BeginPass(1))
						{
							CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
							pEffectFile->CommitChanges();
						
							IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());
							CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
								block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
								idxInfo->GetIndexCount()/3);
							pEffectFile->EndPass(1);
						}
					}
				}


				//////////////////////////////////////
				//render helper mesh for terrain edit
				//////////////////////////////////////
				if(m_isEditorMode && m_visibleDataMask != 0xffffffff)
				{
					if(!m_pEditorMeshVB)
					{
						if(m_pTerrainInfoData == NULL)
							CreateTerrainInfoData();
						BuildHelperMeshBuffer();
					}

					DWORD defaultFillMode = pRenderDevice->GetRenderState(ERenderState::FILLMODE);
					
					pRenderDevice->SetRenderState(ERenderState::SRCBLEND,D3DBLEND_SRCALPHA);
					pRenderDevice->SetRenderState(ERenderState::DESTBLEND,D3DBLEND_INVSRCALPHA);
					GETD3D(CGlobals::GetRenderDevice())->SetFVF(LINEVERTEX::FVF);

					GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(0, m_pEditorMeshVB.GetDevicePointer(), 0, sizeof(LINEVERTEX));
					if(pEffectFile->BeginPass(3))
					{
						for(int i=0;i<blockCount;i++)
						{
							TerrainBlock* pBlock = m_pRootBlock->GetChildBlock(i);
							IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(pBlock->GetLodLevel(),pBlock->GetChunkType());
							if(pBlock->GetFrustumState() > 0 && !pBlock->IsInFog())
							{
								//transparent layer
								CGlobals::GetEffectManager()->EnableD3DAlphaBlending(true);
								CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
									pBlock->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
									idxInfo->GetIndexCount()/3);

								//wireframe layer
								CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
								//adjust depth bias to get stable line
								pRenderDevice->SetRenderState(ERenderState::FILLMODE,D3DFILL_WIREFRAME);
								float bias = -0.0003f;
								float slopeBias = 1.0f;
								float fzero = 0;
								pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS,*((DWORD*)&bias));
								pRenderDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS,*((DWORD*)&slopeBias));

								CGlobals::GetRenderDevice()->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST,
									pBlock->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
									idxInfo->GetIndexCount()/3);

								pRenderDevice->SetRenderState(ERenderState::FILLMODE,defaultFillMode);
								//Don't set to 0 directly, it may cause a render error on XP
								pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS,*((DWORD*)&fzero));
								pRenderDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS,*((DWORD*)&fzero));
							}
						}
						pEffectFile->EndPass();
					}

					pRenderDevice->SetRenderState(ERenderState::SRCBLEND,D3DBLEND_ONE);
					pRenderDevice->SetRenderState(ERenderState::DESTBLEND,D3DBLEND_ONE);
					if(!CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals())
						GETD3D(CGlobals::GetRenderDevice())->SetFVF(terrain_vertex::FVF);
					else
						GETD3D(CGlobals::GetRenderDevice())->SetFVF(terrain_vertex_normal::FVF);

					CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
				}
				/*
				if(m_isEditorMode && m_visibleEditorMeshFlag>0)
				{
					
					if(m_pCollisionBuffer == NULL || !m_pEditorMeshVB )
					{
						if(m_pTerrainInfoData == NULL)
							CreateTerrainInfoData();
						BuildHelperMeshBuffer();
					}

					DWORD defaultFillMode;
					pRenderDevice->GetRenderState(ERenderState::FILLMODE,&defaultFillMode);

					pRenderDevice->SetRenderState(ERenderState::SRCBLEND,D3DBLEND_SRCALPHA);
					pRenderDevice->SetRenderState(ERenderState::DESTBLEND,D3DBLEND_INVSRCALPHA);
					pRenderDevice->SetFVF(LINEVERTEX::FVF);

					if(pEffectFile->BeginPass(3))
					{
						for(int i=0;i<blockCount;i++)
						{
							bool drawEditorMesh = true;
							TerrainBlock* block = m_pRootBlock->GetChildBlock(i);
							IndexInfo* idxInfo = m_pGeoMipmapIndicesGroup->GetChunkIndexInfo(block->GetLodLevel(),block->GetChunkType());
							if(block->GetFrustumState() > 0 && !block->IsInFog()) 
							{
								for(int j=0;j<2;j++)
								{
									if(j==0)
									{
										int flag = 1<<g_slopeMeshVisibleBit;
										if( (m_visibleEditorMeshFlag & flag)>0)
										{
											pRenderDevice->SetStreamSource(0,m_pCollisionBuffer,0,sizeof(LINEVERTEX));
										}
										else
											continue;
									}
									else if(j==1)
									{
										int flag = 1<<g_waypointRegionVisibleBit;
										if( (m_visibleEditorMeshFlag & flag) > 0)
										{
											pRenderDevice->SetStreamSource(0,m_pEditorMeshVB.GetDevicePointer(),0,sizeof(LINEVERTEX));
										}
										else
											continue;
									}

									//transparent layer
									CGlobals::GetEffectManager()->EnableD3DAlphaBlending(true);
									DirectXPerf::DrawIndexedPrimitive(pRenderDevice,DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,EPrimitiveType::TRIANGLELIST,
										block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
										idxInfo->GetIndexCount()/3);
									
									//wireframe layer
									CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
									pRenderDevice->SetRenderState(ERenderState::FILLMODE,D3DFILL_WIREFRAME);
									//adjust depth bias to get stable wireframe
									float bias = -0.0003f;
									float slopeBias = 1.0f;
									float fzero = 0;
									pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS,*((DWORD*)&bias));
									pRenderDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS,*((DWORD*)&slopeBias));

									DirectXPerf::DrawIndexedPrimitive(pRenderDevice,DirectXPerf::DRAW_PERF_TRIANGLES_TERRAIN,EPrimitiveType::TRIANGLELIST,
										block->GetHomeIndex(),0,(m_MaximumVisibleBlockSize+1)*(m_MaximumVisibleBlockSize+1),idxInfo->GetStartIndexPos(),
										idxInfo->GetIndexCount()/3);

									pRenderDevice->SetRenderState(ERenderState::FILLMODE,defaultFillMode);
									//Don't set to 0 directly, it may cause a render error on XP
									pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS,*((DWORD*)&fzero));
									pRenderDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS,*((DWORD*)&fzero));
								}
							}
						}
						pEffectFile->EndPass();
					}

					//reset all state
					pRenderDevice->SetRenderState(ERenderState::SRCBLEND,D3DBLEND_ONE);
					pRenderDevice->SetRenderState(ERenderState::DESTBLEND,D3DBLEND_ONE);
					if(!CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals())
						pRenderDevice->SetFVF(terrain_vertex::FVF);
					else
						pRenderDevice->SetFVF(terrain_vertex_normal::FVF);

					CGlobals::GetEffectManager()->EnableD3DAlphaBlending(false);
				}
				*/
				pEffectFile->end();
			}
		}
	}
	CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
#endif
}

void Terrain::TessellateGeoMipmap()
{
	if(m_pRootBlock == NULL || m_pVertices == NULL)
		return;
	/** set the render offset for this terrain.*/
	Vector3 vRenderOffset(m_OffsetX, 0, m_OffsetY);
	vRenderOffset -= CGlobals::GetScene()->GetGlobalTerrain()->GetTerrainRenderOffset();
	SetRenderOffset(vRenderOffset);

	SetEyePosition(CGlobals::GetScene()->GetGlobalTerrain()->GetMatTerrainEye());

	m_bFogEnabled = CGlobals::GetScene()->IsFogEnabled();

	if(m_pVertexStatus)
	{
		ZeroMemory(m_pVertexStatus, sizeof(m_WidthVertices * m_HeightVertices));
	}

	m_CountStrips = m_CountFans = 0;
	SetLowestVisibleHeight(FLOAT_POS_INFINITY);
	m_BoundingBox.SetEmpty();

	m_pRootBlock->Tessellate(&m_CountStrips, this);
}

void Terrain::SetBlockLod(int indexX,int indexY,int level,GeoMipmapChunkType chunkType)
{
	int blockWidth = (m_WidthVertices - 1) / m_MaximumVisibleBlockSize;
	if(m_pRootBlock)
	{
		TerrainBlock* block = m_pRootBlock->GetChildBlock(indexY*blockWidth + indexX);
		if(block)
		{
			block->SetLod(level,chunkType);
		}
	}
}

void Terrain::SwitchLodStyle(bool useGeoMipmap,ParaIndexBuffer pSharedIndices, GeoMipmapIndicesGroup* geoMipmapIndicesGroup)
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
	//release old resource
	if(m_useGeoMipmap)
	{
		m_pGeoMipmapIndicesGroup = NULL;
	}
	else
	{
		m_pTriangleStrips.clear();
		m_pTriangleFans.clear();
	}
	
	SAFE_DELETE(m_pRootBlock);
	m_TerrainBuffer.DeleteDeviceObjects();
	m_pCollisionBuffer.ReleaseBuffer();
	m_pEditorMeshVB.ReleaseBuffer();

	m_useGeoMipmap = useGeoMipmap;

	if(m_pVertices != NULL)
	{
		m_pRootBlock = new TerrainBlock(0,m_WidthVertices - 1,this,NULL,m_useGeoMipmap);
		m_pRootBlock->CalculateGeometry(this);

		if(m_useGeoMipmap)
		{
			m_TerrainBuffer.BuildGeoMipmapBuffer();

			if(m_isEditorMode && m_useGeoMipmap)
			{
				BuildHelperMeshBuffer();
			}
		}
	}

	if(m_useGeoMipmap)
	{
		m_pIndexBuffer = pSharedIndices;
		m_pGeoMipmapIndicesGroup = geoMipmapIndicesGroup;
	}
}

void Terrain::BuildHelperMeshBuffer()
{
	if(m_pTerrainInfoData == NULL || !m_isEditorMode)
		return;
	int blockVertexWidth = m_MaximumVisibleBlockSize + 1;
	int blockCountX = (m_WidthVertices - 1) / m_MaximumVisibleBlockSize;
	int blockCount = blockCountX * blockCountX;
	int vertexCount = blockVertexWidth * blockVertexWidth * blockCount;
	int bufferSize = sizeof(LINEVERTEX) * vertexCount;

	HRESULT hr = E_FAIL;
	void *pBufferData = NULL;

	if(m_visibleDataMask != 0xffffffff)
	{
		if(!m_pEditorMeshVB)
		{
			if (m_pEditorMeshVB.CreateBuffer(bufferSize, 0, 0))
			{
				if (m_pEditorMeshVB.Lock((void**)&pBufferData, 0, 0))
					hr = S_OK;
			}
		}
		else
		{
			if (m_pEditorMeshVB.Lock((void**)&pBufferData, 0, 0))
				hr = S_OK;
		}
	}

	if(SUCCEEDED(hr))
	{
		LINEVERTEX* pVertexArr = (LINEVERTEX*)pBufferData;
		int idx = 0;

		uint32 clearMask = ~m_visibleDataMask;
		for(int i=0;i<blockCountX;i++)
		{
			for(int j=0;j<blockCountX;j++)
			{
				int blockStartIdx = i * m_WidthVertices * (blockVertexWidth-1) + j * (blockVertexWidth-1);

				for(int y = 0;y<blockVertexWidth;y++)
				{
					for(int x = 0;x<blockVertexWidth;x++)
					{
						int tempIdx = blockStartIdx + m_WidthVertices * y + x;
						LINEVERTEX vertex;
						Vector3& vec = m_pVertices[tempIdx];
						vertex.p.x = vec.x;
						vertex.p.y = vec.z;
						vertex.p.z = vec.y;

						uint32 value = m_pTerrainInfoData[tempIdx];
						value &= clearMask;
						value >>= m_visibleDataBitOffset;
						
						if(value == 0)
							vertex.color = (DWORD)(COLOR_ARGB(255,255,255,255));
						else
						{
							//choose a different color to display different color
							int v0 = ((value+36) * 74) % 211;
							int v1 = ((value+64)* 33) % 211;
							int v2 = ((value+27) * 121) % 211;
							
							vertex.color = (DWORD)(COLOR_ARGB(255,v0,v1,v2));
						}
						pVertexArr[idx++] = vertex;
					}
				}
			}
		}
		m_pEditorMeshVB.Unlock();
	}
}

void Terrain::SetVertexInfo(int idx,uint32 data,uint32 bitMask,uint32 bitOffset)
{
	if(m_pTerrainInfoData == NULL)
		CreateTerrainInfoData();

	if(idx >= 0 && idx < m_NumberOfVertices)
	{
		uint32 value = m_pTerrainInfoData[idx];
		value &= bitMask;
		data <<= bitOffset;
		value |= data;
		m_pTerrainInfoData[idx] = value;
	}
}

uint32 Terrain::GetVertexInfo(int idx,uint32 bitMask,uint8_t bitOffset)
{
	if(m_pTerrainInfoData != NULL)
	{
		uint32 value = m_pTerrainInfoData[idx];
		if(idx >= 0 && idx < m_NumberOfVertices)
		{
			uint32 value = m_pTerrainInfoData[idx];
			value &= (~bitMask);
			value >>= bitOffset;
			return value;
		}
	}
	return 0;
}

void Terrain::RefreshTerrainInfo()
{
	BuildHelperMeshBuffer();
}

void Terrain::SaveInfoData(const string& filePath)
{	
	if(m_pTerrainInfoData == NULL || m_NumberOfVertices < 0)
		return;

	string file;
	if(filePath == "")
		file = m_terrInfoFile;
	else
		file = filePath;

	CParaFile cFile;
	if(cFile.CreateNewFile(file.c_str(),true) == false)
		return;

	cFile.write(m_pTerrainInfoData,m_NumberOfVertices*4);
}

void Terrain::SetTerrainInfoData(const uint32* pInfoData, int width)
{
	SAFE_DELETE(m_pTerrainInfoData);
	if((width*width) == m_NumberOfVertices)
	{
		m_pTerrainInfoData = new uint32[m_NumberOfVertices];
		memcpy(m_pTerrainInfoData,pInfoData,m_NumberOfVertices*sizeof(uint32));
	}
}

void Terrain::CreateTerrainInfoData()
{
	m_pTerrainInfoData = new uint32[m_NumberOfVertices];
	memset(m_pTerrainInfoData, 0, m_NumberOfVertices * sizeof(uint32));
}

bool Terrain::IsWalkable(float x, float y,Vector3& oNormal) const
{
	if(m_pTerrainInfoData ==NULL)
	{
		oNormal.x = 0;
		oNormal.y = 1;
		oNormal.z = 0;
		return true;
	}

	bool result;
	x -= m_OffsetX;
	y -= m_OffsetY;

	if (x < 0.0f || y < 0.0f || GetWidth() < x || GetHeight() < y)
	{
		oNormal.x = 0;
		oNormal.y = 1;
		oNormal.z = 0;
		result = false;
	}
	else
	{
		int ny = (int)(y / m_VertexSpacing);
		int nx = (int)(x / m_VertexSpacing);

		if(ny >= (m_WidthVertices-1) )
			ny = (m_WidthVertices-2);
		if(nx >= (m_HeightVertices-1) )
			nx = (m_HeightVertices-2);

		int v1,v2,v3;
		v1 = (ny) * m_WidthVertices + (nx);

		if(m_useGeoMipmap)
		{
			if ((fmod(y, m_VertexSpacing) + (m_VertexSpacing - fmod(x, m_VertexSpacing))) <= m_VertexSpacing)
			{
				v2 = v1 + m_WidthVertices + 1;
				v3 = v1 + 1;
			}
			else
			{
				v2 = v1 + m_WidthVertices;
				v3 = v1 + m_WidthVertices + 1;
			}
		}
		else
		{
			if ((fmod(y, m_VertexSpacing) + fmod(x, m_VertexSpacing)) <= m_VertexSpacing)
			{
				v2 = v1 + m_WidthVertices;
				v3 = v1 + 1;
			}
			else
			{
				v2 = v1 + m_WidthVertices + 1;
				v3 = v1 + 1;
			}
		}

		result = ( (m_pTerrainInfoData[v1]&1) == 0) 
			|| ((m_pTerrainInfoData[v2]&1) == 0)
			|| ((m_pTerrainInfoData[v3]&1) == 0);

		Vector3 u = m_pVertices[v2] - m_pVertices[v1];
		Vector3 v = m_pVertices[v3] - m_pVertices[v1];
		
		float f;
		f = u.y;
		u.y = u.z;
		u.z = f;
		f = v.y;
		v.y = v.z;
		v.z = f;

		oNormal = u.crossProduct(v).normalise();
	}
	return result;
}

void Terrain::SetVisibleDataMask(uint32 mask,uint8_t bitOffset)
{
	if(mask != m_visibleDataMask)
		m_pEditorMeshVB.ReleaseBuffer();
	m_visibleDataMask = mask;
	m_visibleDataBitOffset = bitOffset;

	if(m_visibleDataMask != 0xffffffff)
		BuildHelperMeshBuffer();
}


////////////////////////////////////////////////////////////////


float Terrain::GetTextureTileWidth() const
{
	return m_TextureTileWidth;
}

float Terrain::GetTextureTileHeight() const
{
	return m_TextureTileHeight;
}

int Terrain::GetNumberOfTextureTilesWidth() const
{
	return m_NumberOfTextureTilesWidth;
}

int Terrain::GetNumberOfTextureTilesHeight() const
{
	return m_NumberOfTextureTilesHeight;
}

void Terrain::PreloadTextures()
{
	for (uint32 i = 0; i < m_TextureCells.size(); i++)
	{
		if(m_TextureCells[i])
		{
			m_TextureCells[i]->BindTexture();
			for (uint32 j = 0; j < (uint32)m_TextureCells[i]->GetNumberOfDetails(); j++)
			{
				if(m_TextureCells[i])
				{
					m_TextureCells[i]->BindMask(j);
					m_TextureCells[i]->BindDetail(j);
				}
			}
		}
	}
}

TextureCell *Terrain::GetTextureCell(int index)
{
	return m_TextureCells[index];
}

TextureCell *Terrain::GetTextureCell(int textureCellX, int textureCellY)
{
	return m_TextureCells[textureCellY * m_NumberOfTextureTilesWidth + textureCellX];
}

TextureCell * Terrain::GetTextureCellW(float x, float y)
{
	x -= m_OffsetX;
	y -= m_OffsetY;
	return GetTextureCell((int)(x / GetTextureTileWidth()), (int)(y / GetTextureTileHeight()));
}

bool Terrain::Pick(int mouseX, int mouseY, float &pickedX, float &pickedY, float &pickedZ) const
{
	// Thanks to Tero Kontkanen for providing this brilliant picking technique.
	bool bPickedTerrain = false;
#ifdef DIRECTX_PICK

	float depth[1];
	GLdouble modelm[16], projm[16], pos[3];
	int view[4];

	//D3DMATRIX modelm;
	//D3DMATRIX projm;
	//D3DVIEWPORT9  view;
	//Settings::GetInstance()->GettD3DDevice()->GetTransform(D3DTS_PROJECTION, &projm);
	//Settings::GetInstance()->GettD3DDevice()->GetTransform(D3DTS_VIEW, &modelm);
	//Settings::GetInstance()->GettD3DDevice()->GetViewport(&view);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelm);
	glGetDoublev(GL_PROJECTION_MATRIX, projm);
	glGetIntegerv(GL_VIEWPORT, (GLint *) view);

	glReadPixels(mouseX, mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

	gluUnProject(mouseX, mouseY, depth[0], modelm, projm, view, &pos[0], &pos[1], &pos[2]);

	pickedX = (float)pos[0];
	pickedY = (float)pos[1];
	pickedZ = (float)pos[2];

	float elev = GetElevation(pickedX, pickedY);
	bPickedTerrain = Settings::GetInstance()->GetPickThreshold() < 0.0f || (pickedZ - elev < Settings::GetInstance()->GetPickThreshold());	// Look for a "fudge factor" difference between the unprojected point and the actual terrain elevation to see if some application object was picked instead of the terrain
	if (!bPickedTerrain)
		cout << "TERRAIN: Picked point is not on terrain *" << (pickedZ - elev) << ")" << endl;
#endif
	return bPickedTerrain;
}

bool Terrain::Pick(int mouseX, int mouseY, float &pickedX, float &pickedY, float &pickedZ, int &textureCellX, int &textureCellY, float &texU, float &texV) const
{
	bool bPickedTerrain = Pick(mouseX, mouseY, pickedX, pickedY, pickedZ);
	if (bPickedTerrain)
		GetTextureCoordinates(pickedX, pickedY, textureCellX, textureCellY, texU, texV);
	return bPickedTerrain;
}

void Terrain::GetTextureCoordinates(float x, float y, int &textureCellX, int &textureCellY, float &texU, float &texV) const
{
	textureCellX = (int)(x / m_TextureTileWidth);
	textureCellY = (int)(y / m_TextureTileHeight);
	texU = (float)fmod(x, m_TextureTileWidth) / m_TextureTileWidth;
	texV = (float)fmod(y, m_TextureTileHeight) / m_TextureTileHeight;
}

void Terrain::BoxToRenderCoordinate( CShapeBox & cuboid ) const
{
	cuboid.m_Min+=m_vRenderOffset_internal;
	cuboid.m_Max+=m_vRenderOffset_internal;
}

/// Please notice that the original Demeter terrain has the height info saved in z axis,
/// yet, in ParaEngine, it is in y axis. So we secretly exchanged z,y during culling with
/// the camera.
int Terrain::CuboidInFrustum(const CShapeBox & cuboid) const
{
	//PERF1("TestFrustum");
/** set whether we will use 8 points of the cuboid for frustum test or else using standard AABB test 
* @note: there are no performance difference.
*/
//#define USE_BOX_POINT
#ifdef USE_BOX_POINT
	float minX = cuboid.m_Min.x+m_vRenderOffset_internal.x;
	float maxX = cuboid.m_Max.x+m_vRenderOffset_internal.x;
	float minY = cuboid.m_Min.y+m_vRenderOffset_internal.y;
	float maxY = cuboid.m_Max.y+m_vRenderOffset_internal.y;
	float minZ = cuboid.m_Min.z+m_vRenderOffset_internal.z;
	float maxZ = cuboid.m_Max.z+m_vRenderOffset_internal.z;

	Vector3 pt[8] = {
		Vector3(minX, minZ, minY), 
		Vector3(maxX, minZ, minY),
		Vector3(minX, maxZ, minY), 
		Vector3(minX, minZ, maxY),
		Vector3(maxX, maxZ, minY), 
		Vector3(minX, maxZ, maxY),
		Vector3(maxX, minZ, maxY), 
		Vector3(maxX, maxZ, maxY),
	};
	return CGlobals::GetScene()->GetCurrentCamera()->GetFrustum()->CullPointsWithFrustum(pt, 8) ? 2 : 0;
#else
	CShapeAABB box;
	box.SetMinMax(
		Vector3(cuboid.m_Min.x+m_vRenderOffset_internal.x, cuboid.m_Min.z+m_vRenderOffset_internal.z, cuboid.m_Min.y+m_vRenderOffset_internal.y), 
		Vector3(cuboid.m_Max.x+m_vRenderOffset_internal.x, cuboid.m_Max.z+m_vRenderOffset_internal.z, cuboid.m_Max.y+m_vRenderOffset_internal.y));

/** if true, we will treat the far plane as a spherical plane when doing occlusion testing */
#define USE_SPHERE_FAR_PLANE_TEST
#ifdef USE_SPHERE_FAR_PLANE_TEST
	return (CGlobals::GetScene()->GetCurrentCamera()->GetFrustum()->TestBox_sphere_far_plane(&box)) ? 2 : 0;
#else
	return (CGlobals::GetScene()->GetCurrentCamera()->GetFrustum()->TestBox(&box));
#endif	

#endif
}


float Terrain::GetBoxToEyeMatrics( const CShapeBox & cuboid ) const
{
	/**
	LiXizhi 2009.2.15: http://www.gamasutra.com/features/20000228/ulrich_02.htm
	This one just determines whether to tesselate according to camera-quad distance, instead of using screen space metrics. 
	bc[x,y,z] == coordinates of box center
	ex[x,y,z] == extent of box from the center (i.e. 1/2 the box dimensions) 
	L1 = max(abs(bcx - viewx) - exx, abs(bcy - viewy) - exy, abs(bcz - viewz) - exz)
	enabled = maxerror * Threshold < L1
	*/
	// we will return L1;
	float half = (cuboid.m_Max.x-cuboid.m_Min.x)*0.5f;
	float dx = fabs(cuboid.m_Min.x+half-m_vEye.x) - half;
	float dy = fabs(cuboid.m_Min.y+half-m_vEye.z) - half;
	float dz = fabs((cuboid.m_Min.z+cuboid.m_Max.z)*0.5f-m_vEye.y) - (cuboid.m_Max.z-cuboid.m_Min.z)/2;
	float	d = dx;
	if (dy > d) d = dy;
	if (dz > d) d = dz;
	return d;
}

bool Terrain::CuboidInFogRadius(const CShapeBox & cuboid) const
{
	CShapeSphere fog_sphere(Vector3(m_vEye.x, m_vEye.z, m_vEye.y), CGlobals::GetScene()->GetFogEnd());
	return cuboid.Intersect(fog_sphere);
}

Plane::Side Terrain::GetCuboidFogSide(const CShapeBox & cuboid) const
{
	CShapeAABB box;
	box.SetMinMax(
		Vector3(cuboid.m_Min.x+m_vRenderOffset_internal.x, cuboid.m_Min.z+m_vRenderOffset_internal.z, cuboid.m_Min.y+m_vRenderOffset_internal.y), 
		Vector3(cuboid.m_Max.x+m_vRenderOffset_internal.x, cuboid.m_Max.z+m_vRenderOffset_internal.z, cuboid.m_Max.y+m_vRenderOffset_internal.y));
	return CGlobals::GetScene()->GetCurrentCamera()->GetFogPlane().getSide(box);
}

float Terrain::IntersectRayW(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance)
{
	float fDist = IntersectRay(startX-m_OffsetX, startY-m_OffsetY, startZ, dirX, dirY, dirZ, intersectX, intersectY, intersectZ, fMaxDistance);
	intersectX+=m_OffsetX;
	intersectY+=m_OffsetY;
	return fDist;
}

float Terrain::IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance)
{
	if(m_pRootBlock == NULL)
	{
		// if the terrain is blank,suppose the terrain is a plane at y=DEFAULT_TERRAIN_HEIGHT,
		float fDist = (DEFAULT_TERRAIN_HEIGHT -startZ) / dirZ;
		intersectX = startX + fDist*dirX;
		intersectZ = DEFAULT_TERRAIN_HEIGHT;
		intersectY = startY + fDist*dirY;
		if((intersectX>=0) && (intersectY>=0) && 
			(intersectX<=(GetWidth())) && (intersectY<=GetHeight()))
			return fDist;
		else
			return -1.f;
	}

	CShapeRay ray;
	float distance = fMaxDistance;
	Vector3 point;
	point.x = point.y = point.z = -1.0f;

	ray.mOrig.x = startX;
	ray.mOrig.y = startY;
	ray.mOrig.z = startZ;
	ray.mDir.x = dirX;
	ray.mDir.y = dirY;
	ray.mDir.z = dirZ;
	
	if(m_useGeoMipmap)
	{
		m_pRootBlock->IntersectRayGeoMipmap(ray,point,distance,this);
	}
	else
	{
		m_pRootBlock->IntersectRay(ray, point, distance, this);
	}
	intersectX = point.x;
	intersectY = point.y;
	intersectZ = point.z;

	/// if the ray intersect with holes in the terrain, a negative value will be returned.
	if(IsHole(intersectX, intersectY))
	{
		distance = -1.f;
	}
	
	return (distance >= fMaxDistance) ? -1.f : distance;
}

float Terrain::IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, int &textureCellX, int &textureCellY, float &texU, float &texV, float fMaxDistance)
{
	float dist = IntersectRay(startX, startY, startZ, dirX, dirY, dirZ, intersectX, intersectY, intersectZ, fMaxDistance);
	if (0.0f <= intersectX)
		GetTextureCoordinates(intersectX, intersectY, textureCellX, textureCellY, texU, texV);
	return dist;
}

void Terrain::SetTextureCell(int index, TextureCell * pCell)
{
	m_TextureCells[index] = pCell;
}

TextureSet *Terrain::GetTextureSet()
{
	return m_pTextureSet;
}

void Terrain::SetTextureSet(TextureSet * pSet)
{
	if (m_pTextureSet)
		delete m_pTextureSet;
	m_pTextureSet = pSet;
}

void Terrain::SetLatticePosition(int x, int y)
{
	m_LatticePositionX = x;
	m_LatticePositionY = y;
}

void Terrain::GetLatticePosition(int &x, int &y)
{
	x = m_LatticePositionX;
	y = m_LatticePositionY;
}

void Terrain::AllocateTextureCells(int numCellsX, int numCellsY)
{
	for (uint32 i = 0; i < (int)m_TextureCells.size(); ++i)
	{
		SAFE_DELETE(m_TextureCells[i]);
	}
	m_TextureCells.clear();

	m_NumberOfTextureTilesWidth = numCellsX;
	m_NumberOfTextureTilesHeight = numCellsY;
	m_NumberOfTextureTiles = numCellsX * numCellsY;
	float verticesPerTileWidth = (float)(m_WidthVertices + m_NumberOfTextureTilesWidth - 1) / (float)m_NumberOfTextureTilesWidth;
	float verticesPerTileHeight = (float)(m_HeightVertices + m_NumberOfTextureTilesHeight - 1) / (float)m_NumberOfTextureTilesHeight;
	if(verticesPerTileWidth<m_MaximumVisibleBlockSize)
		m_MaximumVisibleBlockSize = (int)verticesPerTileWidth-1;
	m_TextureTileWidth = (verticesPerTileWidth - 1.0f) * m_VertexSpacing;
	m_TextureTileHeight = (verticesPerTileHeight - 1.0f) * m_VertexSpacing;

	int nCellCount= m_NumberOfTextureTilesWidth * m_NumberOfTextureTilesHeight;
	m_TextureCells.reserve(nCellCount);
	for (int i = 0; i < nCellCount; ++i)
		m_TextureCells.push_back(new TextureCell());
	if (m_WidthVertices > 0)
		StaticInitUV(m_WidthVertices, m_NumberOfTextureTilesWidth, m_NumberOfTextureTilesWidth * (int)m_DetailTextureRepeats);
}

void Terrain::DigCrater(float centerX, float centerY, float radius, int detailTextureId)
{
	const float depth = radius / 2.0f;
	vector < TerrainVertex > vertices;
	GetVertices(centerX, centerY, radius, vertices);
	vector < TerrainVertex >::iterator iter = vertices.begin();
	while (iter != vertices.end())
	{
		TerrainVertex v = *iter;
		//float craterBottom = sqrt(radius*radius - (sqrt(v.m_X*v.m_X + v.m_Y * v.m_Y) * sqrt(v.m_X*v.m_X + v.m_Y * v.m_Y)));
		float dx = v.m_X - centerX;
		float dy = v.m_Y - centerY;
		float distance = sqrtf(dx * dx + dy * dy);
		float distanceFactor = distance / (radius * 0.5f);
		if (distanceFactor < 0.0f)
			distanceFactor = 0.0f;
		if (1.0f < distanceFactor)
			distanceFactor = 1.0f;
		float craterBottom = GetElevation(v.m_X, v.m_Y) - (1.0f - distanceFactor) * depth;
		SetVertexElevation(v.m_Index, craterBottom, false);
		iter++;
	}
	RecalcGeometry(vertices[0].m_Index, vertices[vertices.size() - 1].m_Index);
}

void Terrain::GetVertices(float centerX, float centerY, float radius, vector < TerrainVertex > &vertices)
{
	int centerIndex = GetVertex(centerX, centerY);
	int span = (int)(radius / m_VertexSpacing);
	int topLeftIndex = centerIndex - (span / 2) * m_WidthVertices - (span / 2);
	int topLeftX = topLeftIndex % m_WidthVertices;
	int topLeftY = topLeftIndex / m_WidthVertices;
	for (int indexY = topLeftY; indexY < topLeftY + span; indexY++)
	{
		if (0 <= indexY && indexY < m_HeightVertices - 1)
		{
			for (int indexX = topLeftX; indexX < topLeftX + span; indexX++)
			{
				if (0 <= indexX && indexX < m_WidthVertices - 1)
				{
					float x = indexX * m_VertexSpacing;
					float y = indexY * m_VertexSpacing;
					TerrainVertex v(indexY * m_WidthVertices + indexX, x, y, GetElevation(x, y));
					vertices.push_back(v);
				}
			}
		}
	}
}

ReloadMaskRequest::ReloadMaskRequest(int textureCellX, int textureCellY, int detailIndex)
{
	m_TextureCellX = textureCellX;
	m_TextureCellY = textureCellY;
	m_DetailIndex = detailIndex;
}

ReloadMaskRequest::~ReloadMaskRequest()
{
}

int Terrain::GetTextureIDbyPoint(float x, float y, float& tileX, float& tileY)
{
	// low res texture
	tileY = (float)Math::Round(y / m_TextureTileHeight);
	tileX = (float)Math::Round(x / m_TextureTileWidth);
	return (int)(tileY * m_NumberOfTextureTilesWidth + tileX);
}


void Terrain::SetDetailTextureRepeats(float repeats)
{
	m_DetailTextureRepeats = repeats;
}

float Terrain::GetDetailTextureRepeats()
{
	return m_DetailTextureRepeats;
}

DWORD Terrain::GetRegionValue(const string& sLayerName, float x, float y)
{
	CTerrainRegions* pRegions = GetRegions();
	if(pRegions)
	{
		return pRegions->GetValue(sLayerName, x,y);
	}
	return 0;
}

DWORD Terrain::GetRegionValueW(const string& sLayerName, float x, float y)
{
	return GetRegionValue(sLayerName, x - m_OffsetX,y - m_OffsetY);
}

const string& Terrain::GetOnloadScript()
{
	return m_sOnLoadScript;
}
const string& Terrain::GetTerrainElevFile()
{
	return m_sElevFile;
}

string Terrain::GetRenderReport()
{
	char tmp[100] = "";
	if(m_TerrainBuffer.m_nNumOfTriangles>0)
		sprintf(tmp, "(%d,%d)=%d", m_LatticePositionX,m_LatticePositionY, m_TerrainBuffer.m_nNumOfTriangles);
	return string(tmp);
}
void  Terrain::SetLowestVisibleHeight(float fHeight)
{
	m_fLowestVisibleHeight = fHeight;
}

float Terrain::GetLowestVisibleHeight()
{
	return m_fLowestVisibleHeight;
}
bool Terrain::IsModified()
{
	return m_dwModified>0;
}

void Terrain::SetModified(bool bIsModified)
{
	m_dwModified = bIsModified ? MODIFIED_ALL : MODIFIED_NONE;
}

bool Terrain::IsModified(DWORD dwModifiedBits)
{
	return (m_dwModified & dwModifiedBits) > 0;
}

void Terrain::SetModified(bool bIsModified, DWORD dwModifiedBits)
{
	if(bIsModified)
		m_dwModified |= dwModifiedBits;
	else
		m_dwModified &= (~dwModifiedBits);
}

// function is not used
DetailTexture* Terrain::GetDefaultBaseLayerMask() 
{
	if(m_pDefaultBaseLayerMask)
		return m_pDefaultBaseLayerMask;
	else
	{
		m_pDefaultBaseLayerMask = new DetailTexture(NULL, (Texture *)NULL);
		m_pDefaultBaseLayerMask->RegenerateMask(0xff);
		return m_pDefaultBaseLayerMask;
	}
};

bool Terrain::LoadFromConfigFile(const char* pFileName, float fSize, const char* pRelativePath)
{
	string sFileName;
	CPathReplaceables::GetSingleton().DecodePath(sFileName, pFileName);
	string relativePath;
	CPathReplaceables::GetSingleton().DecodePath(relativePath, pRelativePath);

	CParaFile cFile;
	cFile.OpenAssetFile(sFileName.c_str(), true, relativePath.c_str());
	
	if(cFile.isEof() || (cFile.getBuffer()==0))
		return false;
	CParaFile::ToCanonicalFilePath(m_sConfigFile, cFile.GetFileName(), false);
	CPathReplaceables::GetSingleton().DecodePath(m_sConfigFile, m_sConfigFile);

	//char line[MAX_LINE+1];
	bool bLineProcessed = true;

	bLineProcessed = cFile.GetNextAttribute("OnLoadFile", m_sOnLoadScript);
	CPathReplaceables::GetSingleton().DecodePath(m_sOnLoadScript, m_sOnLoadScript);
	
	bLineProcessed = cFile.GetNextAttribute("Heightmapfile", m_sElevFile);
	CPathReplaceables::GetSingleton().DecodePath(m_sElevFile, m_sElevFile);

	bLineProcessed = cFile.GetNextAttribute("TerrainInfoFile", m_terrInfoFile);
	CPathReplaceables::GetSingleton().DecodePath(m_terrInfoFile, m_terrInfoFile);
	
	string MainTextureFile;
	bLineProcessed = cFile.GetNextAttribute("MainTextureFile", MainTextureFile);
	CPathReplaceables::GetSingleton().DecodePath(MainTextureFile, MainTextureFile);
	
	string CommonTextureFile;
	bLineProcessed = cFile.GetNextAttribute("CommonTextureFile", CommonTextureFile);
	CPathReplaceables::GetSingleton().DecodePath(CommonTextureFile, CommonTextureFile);

	float Size=533.333f;
	bLineProcessed = cFile.GetNextAttribute("Size", Size);
	if(fSize!=0)
		Size = fSize;

	float ElevScale=1.0f;
	bLineProcessed = cFile.GetNextAttribute("ElevScale", ElevScale);
	
	float Swapvertical=0;
	bLineProcessed = cFile.GetNextAttribute("Swapvertical", Swapvertical);
	
	float HighResRadius = 0;
	bLineProcessed = cFile.GetNextAttribute("HighResRadius", HighResRadius);
	
	float DetailThreshold = 9;
	bLineProcessed = cFile.GetNextAttribute("DetailThreshold", DetailThreshold);
	
	int MaxBlockSize = 16;
	bLineProcessed = cFile.GetNextAttribute("MaxBlockSize", MaxBlockSize);
	if(MaxBlockSize > 16)
	{
		OUTPUT_LOG("warning: MaxBlockSize in %s can not be larger than 16\n", pFileName);
		MaxBlockSize = 16;
	}

	int DetailTextureMatrixSize = 64;
	bLineProcessed = cFile.GetNextAttribute("DetailTextureMatrixSize", DetailTextureMatrixSize);
	
	this->SetMaximumVisibleBlockSize(MaxBlockSize);
	this->SetDetailThreshold(DetailThreshold);
	this->SetHighResTextureRadius(HighResRadius);
	ParaTerrain::Settings::GetInstance()->SetMediaPath(relativePath.c_str());
	ParaTerrain::Loader::GetInstance()->LoadElevations(this, m_sElevFile.c_str(),Size*OBJ_UNIT,ElevScale*OBJ_UNIT, Swapvertical>0);

	SetBaseTexture(MainTextureFile.c_str());
	SetCommonTexture(CommonTextureFile.c_str());
	

	/// add terrain holes
	if(cFile.GetNextAttribute("hole"))
	{
		float fHoleX=0.0f, fHoleY=0.0f;
		// add holes in the terrain.
		while(cFile.GetNextFormatted("(%f, %f)", &fHoleX, &fHoleY))
		{
			SetHoleLocal(fHoleX, fHoleY, true);
		}
		UpdateHoles();
	}
	
	/// add all detail textures used in this terrain tile
	int NumOfDetailTextures=0;
	bLineProcessed = cFile.GetNextAttribute("NumOfDetailTextures", NumOfDetailTextures);
	
	if(NumOfDetailTextures>0)
	{
		// read data from the mask file.
		LoadMaskFromDisk(true);
	}

	/// get regions layers
	int NumOfRegions = 0;
	bLineProcessed = cFile.GetNextAttribute("NumOfRegions", NumOfRegions);
	if(NumOfDetailTextures>0)
	{
		CTerrainRegions* pRegions = CreateGetRegions();
		
		string regionName;
		string regionFilePath;

		pRegions->SetSize(Size, Size);

		regex re("\\((\\w+), ([^\\)]+)\\)");

		char line[MAX_LINE+1];
		for(int i=0;i<NumOfRegions;++i)
		{
			if(cFile.GetNextLine(line, MAX_LINE)>0)
			{
				smatch match;
				std::string sLine = line;
				if (regex_search(sLine, match, re) && match.size() > 2)
				{
					regionName = match.str(1);
					regionFilePath = match.str(2);
					pRegions->LoadRegion(regionName, regionFilePath.c_str());
				}
			}
		}

	}

// OBSOLETED code block
#ifdef USE_UNTILED_DETAIL_TEXTURE
	char line[MAX_LINE+1];

	m_pDetailedTextureFactory->Init(DetailTextureMatrixSize, Size/DetailTextureMatrixSize);
	for(int i=0;i<NumOfDetailTextures;i++)
	{
		if (cFile.GetNextLine(line, MAX_LINE) > 0) {
			string sTmp(line);
			m_pDetailedTextureFactory->m_listTextures.push_back(sTmp);
		}
		else
			return false;
	}
	while(cFile.GetNextLine(line, MAX_LINE)>0)
	{
		int nX,nY;
		int nLayerIndex[4];
		int nNumTexLayers=0;
		if( (nNumTexLayers = (_snscanf(line, MAX_LINE, "(%d,%d) = {%d}{%d}{%d}{%d}", &nX, &nY,
			&nLayerIndex[0],&nLayerIndex[1],&nLayerIndex[2],&nLayerIndex[3])-2))>=1 )
		{
			if(nNumTexLayers> MAX_NUM_TEXTURE_LAYERS)
				nNumTexLayers = MAX_NUM_TEXTURE_LAYERS;
			for(int i=0;i<nNumTexLayers; i++)
			{
				m_pDetailedTextureFactory->m_TexturesMatrix[nX*m_pDetailedTextureFactory->m_nTileNum+nY].layers[i] = (short)nLayerIndex[i];
			}
		}
	}
#endif
	SetModified(false);

	ParaTerrain::Loader::GetInstance()->LoadTerrainInfo(this,m_terrInfoFile.c_str());
	
	if(m_useGeoMipmap)
		BuildGeoMipmapBuffer();

	return true;
}

void Terrain::LoadMaskFromDisk(bool bForceReload)
{
	
	// if it is editor, there is no need to load mask
	if(!bForceReload && (Settings::GetInstance()->IsEditor() || m_bMaskFileInited))
		return;
	m_bMaskFileInited = true;

	// read data from the mask file.	
	string sMaskFile;
	GetMaskFile(sMaskFile, m_sConfigFile);

	/// read mask data from file:

	CParaFile fileMask;
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(sMaskFile);
	if(pEntry)
	{
		if(pEntry->DoesFileExist())
		{
			// we already downloaded the file, so load it. 
			auto& filename = pEntry->GetLocalFileName();
			if(fileMask.OpenFile(filename.c_str(), true, NULL))
			{
				ReadMaskFile(fileMask);
			}
			else
			{
				OUTPUT_LOG1("warning: failed loading local mask file: %s \n", sMaskFile.c_str());
			}
		}
		else 
		{
			// download and async load the file 
			if(m_pMaskFileCallbackData == 0)
			{
				m_pMaskFileCallbackData = new CTerrainMaskFileCallbackData(this);
				pEntry->SyncFile_Async(boost::bind(&CTerrainMaskFileCallbackData::OnMaskFileDownloaded, 
					m_pMaskFileCallbackData , _1, _2));
			}
		}
	}
	else
	{
		string sTmp = string("ParaFile.OpenAssetFile using local file:") + sMaskFile + "\n";
		CAsyncLoader::GetSingleton().log(sTmp);
		if(fileMask.OpenFile(sMaskFile.c_str(), true, NULL))
		{
			ReadMaskFile(fileMask);
		}
	}
}

void CTerrainMaskFileCallbackData::OnMaskFileDownloaded(int nResult, AssetFileEntry* pAssetFileEntry)
{
	if(nResult == 0)
	{
		if(m_pTerrain)
		{
			CParaFile fileMask;
			if(fileMask.OpenFile(pAssetFileEntry->GetLocalFileName().c_str(), true, NULL))
			{
				m_pTerrain->ReadMaskFile(fileMask);
			}
		}
	}
}

bool Terrain::ReadMaskFile(CParaFile& fileMask)
{
	if(fileMask.isEof())
	{
		OUTPUT_LOG("warning: terrain mask file not found %s:\n", fileMask.GetFileName().c_str());
	}
	else
	{
		GetTextureSet()->ReadMask(fileMask, this);
		// number of cells, usually 8x8=64
		int nCellCount = fileMask.ReadDWORD();
		if(((int)m_TextureCells.size()) != nCellCount)
		{
			OUTPUT_LOG("warning: texture cell count in the mask does not match the cells number in the terrain:%s \n", fileMask.GetFileName().c_str());
		}
		else
		{
			for (int i = 0; i < nCellCount; ++i)
			{
				if (m_TextureCells[i])
					m_TextureCells[i]->ReadMask(fileMask, this);
			}
			return true;
		}
	}
	return false;
}

void Terrain::GetMaskFile(string& sMaskFile, const string& sConfigFile)
{
	sMaskFile = "";
	int nDotPos = 0;
	for(int i = (int)sConfigFile.size()-1;i>=0;--i)
	{
		if(sConfigFile[i] == '.')
			nDotPos = i;
		if(sConfigFile[i] == '\\' || sConfigFile[i] == '/'){
			sMaskFile = sConfigFile.substr(0, i+1);
			if(i<nDotPos)
				sMaskFile += sConfigFile.substr(i+1, nDotPos-i-1)+".mask";
			break;
		}
	}
	if(sMaskFile.empty())
		sMaskFile = sConfigFile+".mask";
}

bool Terrain::SaveElevation(const string& filePath)
{
	string sElevFile;
	if(filePath == "")
		sElevFile = m_sElevFile;
	else
		sElevFile = filePath;

	if(m_NumberOfVertices<0)
		return false;
	CParaFile cFile;
	if( cFile.CreateNewFile(sElevFile.c_str(), true) == false)
		return false;
	
	float * pElevation = new float[m_NumberOfVertices];
	if(pElevation)
	{
		for (int i=0; i<m_NumberOfVertices;++i)
		{
			pElevation[i] = m_pVertices[i].z;
		}
		cFile.write(pElevation, m_NumberOfVertices*4);
		delete []pElevation;
	}
	return true;
}
bool Terrain::SaveDetailTexture(const string& filePath)
{
	string sMaskFile;
	if(filePath == "")
		GetMaskFile(sMaskFile, m_sConfigFile);
	else
		sMaskFile = filePath;

	/// export all alpha mask data to disk with the name:
	int nCellCount = (int)m_TextureCells.size();
	CParaFile fileMask;
	if(fileMask.CreateNewFile(sMaskFile.c_str(), true) == false)
		return false;

	GetTextureSet()->WriteMask(fileMask, this);
	// number of cells, usually 8x8=64
	fileMask.WriteDWORD(nCellCount);
	for (int i = 0; i < nCellCount; ++i)
	{
		if(m_TextureCells[i])
			m_TextureCells[i]->WriteMask(fileMask, this);
	}
	return true;
}

bool Terrain::SaveConfig(const string& filePath)
{
	string sConfigFile;
	string sPath;
	if(filePath == "")
		sConfigFile = m_sConfigFile;
	else
		sConfigFile = filePath;

	CParaFile cFile;
	if( cFile.CreateNewFile(sConfigFile.c_str(), true) == false)
		return false;

	char line[MAX_LINE+1];
	memset(line, 0, sizeof(line));
	// header
	cFile.WriteString("-- auto gen by ParaEngine\n");

	// OnLoadFile = script/loader.lua
	if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
		CPathReplaceables::GetSingleton().EncodePath(sPath, m_sOnLoadScript, "WORLD");
	else
		sPath = m_sOnLoadScript;
	if(snprintf(line, MAX_LINE, "OnLoadFile = %s\n", sPath.c_str())<0)
		return false;
	cFile.WriteString(line);

	// Heightmapfile = texture/data/elevation.raw
	if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
		CPathReplaceables::GetSingleton().EncodePath(sPath, m_sElevFile, "WORLD");
	else
		sPath = m_sElevFile;
	if(snprintf(line, MAX_LINE, "Heightmapfile = %s\n", sPath.c_str())<0)
		return false;
	cFile.WriteString(line);

	//Additional terrain info data
	if(m_pTerrainInfoData != NULL)
	{
		if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
			CPathReplaceables::GetSingleton().EncodePath(sPath,m_terrInfoFile,"WORLD");
		else
			sPath = m_terrInfoFile;
		if(snprintf(line,MAX_LINE,"TerrainInfoFile = %s\n",sPath.c_str()) < 0)
			return false;

		cFile.WriteString(line);
	}


	// MainTextureFile = texture/data/main.jpg
	if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
		CPathReplaceables::GetSingleton().EncodePath(sPath, m_sBaseTextureFile, "WORLD");
	else
		sPath = m_sBaseTextureFile;
	if(snprintf(line, MAX_LINE, "MainTextureFile = %s\n", sPath.c_str())<0)
		return false;
	cFile.WriteString(line);

	// CommonTextureFile = texture/data/dirt.jpg
	if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
		CPathReplaceables::GetSingleton().EncodePath(sPath, m_sCommonTextureFile, "WORLD");
	else
		sPath = m_sCommonTextureFile;
	if(snprintf(line, MAX_LINE, "CommonTextureFile = %s\n", sPath.c_str())<0)
		return false;
	cFile.WriteString(line);

	// Size = 533.3333
	if(snprintf(line, MAX_LINE, "Size = %f\n", m_fTerrainSize)<0)
		return false;
	cFile.WriteString(line);

	// ElevScale = 1.0
	cFile.WriteString("ElevScale = 1.0\n");

	// Swapvertical = 1
	cFile.WriteString("Swapvertical = 1\n");

	// HighResRadius = 30
	cFile.WriteString("HighResRadius = 30\n");

	// DetailThreshold = 9.0
	if(snprintf(line, MAX_LINE, "DetailThreshold = %f\n", GetDetailThreshold())<0)
		return false;
	cFile.WriteString(line);

	// MaxBlockSize = 8
	if(snprintf(line, MAX_LINE, "MaxBlockSize = %d\n", m_MaximumVisibleBlockSize)<0)
		return false;
	cFile.WriteString(line);

	// DetailTextureMatrixSize = 64
	if(snprintf(line, MAX_LINE, "DetailTextureMatrixSize = %d\n", (int)m_TextureCells.size())<0)
		return false;
	cFile.WriteString(line);

	// terrain holes
	if(m_pHolemap!=NULL && m_nNumOfHoleVertices>0)
	{
		// hole
		bool bFirstHole = true;

		int nHoleDim = m_WidthVertices/m_nHoleScale;
		float fHoleSize = m_VertexSpacing * m_nHoleScale;
		for (int nRow=0;nRow<nHoleDim; ++nRow)
		{
			int vertexID0 = (nRow) * nHoleDim;
			for (int nCol=0;nCol<nHoleDim; ++nCol)
			{
				int vertexID = vertexID0+nCol;
				if(m_pHolemap[vertexID])
				{
					if(bFirstHole){
						// hole header
						cFile.WriteString("hole\n");
						bFirstHole = false;
					}
					// for each hole in the terrain
					if(snprintf(line, MAX_LINE, "(%f, %f)\n", (nCol+0.5f)*fHoleSize, (nRow+0.5f)*fHoleSize)<0)
						return false;
					cFile.WriteString(line);
				}
			}
		}
	}

	// NumOfDetailTextures = 0
	if(snprintf(line, MAX_LINE, "NumOfDetailTextures = %d\n", GetTextureSet()->GetNumTextures()+1) < 0)
		return false;
	cFile.WriteString(line);

	/// get regions layers
	CTerrainRegions* pRegions = GetRegions();
	if(pRegions)
	{
		int NumOfRegions = (int)pRegions->GetLayers().size();
		if(NumOfRegions>0)
		{
			cFile.WriteFormated("NumOfRegions=%d\n", NumOfRegions);

			CTerrainRegions::RegionLayer_Map_Type::iterator itCur, itEnd = pRegions->GetLayers().end();
			for(itCur = pRegions->GetLayers().begin(); itCur != itEnd; ++itCur)
			{
				cFile.WriteFormated("(%s, %s)\n", itCur->first.c_str(), itCur->second->GetFileName().c_str());
			}
		}
	}
	

	return true;
}
bool Terrain::SaveToFile(const char* filename)
{
	if(m_pRootBlock == NULL)
		return false;
	if(IsModified() == false)
		return false;
	
	if(GetTextureSet()!=NULL)
		GetTextureSet()->GarbageCollect(this);

	bool bForceUpdateConfig = false;

	// save mask file
	if(IsModified(MODIFIED_TEXTURE))
	{
		string sMaskFile = CGlobals::GetWorldInfo()->GetTerrainMaskFile(m_LatticePositionX, m_LatticePositionY);;
		//GetMaskFile(sMaskFile, sConfigFile);
		SaveDetailTexture(sMaskFile);
	}

	// save elevation file
	if(IsModified(MODIFIED_HEIGHTMAP))
	{
		string sConfig = CGlobals::GetWorldInfo()->GetTerrainElevationFile(m_LatticePositionX, m_LatticePositionY);
		if(m_sElevFile != sConfig)
		{
			m_sElevFile = sConfig;
			SetModified(true, MODIFIED_CONFIGURATION);
		}
		SaveElevation(m_sElevFile);
	}
	
	//save additional terrain info data
	if(m_pTerrainInfoData != NULL)
	{
		string infoFile = CGlobals::GetWorldInfo()->GetTerrainInfoDataFile(m_LatticePositionX, m_LatticePositionY);
		if(m_terrInfoFile != infoFile)
		{
			m_terrInfoFile = infoFile;
			SetModified(true, MODIFIED_CONFIGURATION);
		}
		SaveInfoData(infoFile);
	}


	if(IsModified(MODIFIED_ON_LOAD_SCRIPT))
	{
		CParaFile file;
		string sOnLoadScript = CGlobals::GetWorldInfo()->GetTerrainOnloadFile(m_LatticePositionX, m_LatticePositionY);
		if(sOnLoadScript != m_sOnLoadScript)
		{
			bForceUpdateConfig = true;
			m_sOnLoadScript = sOnLoadScript;
		}
		bool bBackupped = true;
		
		if(Settings::GetInstance()->IsBackupFilesOnSave())
		{
			if(file.DoesFileExist(sOnLoadScript.c_str(), false))
			{
				if(!file.BackupFile(sOnLoadScript.c_str()))
				{
					bBackupped = false;
					OUTPUT_LOG("error: unable to back up file %s\n", sOnLoadScript.c_str());
				}
			}
		}
		
		if(bBackupped)
		{
			if( file.CreateNewFile(sOnLoadScript.c_str()))
			{
				CSceneWriter writer(file);
				string sLoaderName =  CParaFile::GetFileName(sOnLoadScript);
				writer.BeginManagedLoader(sLoaderName);
				CShapeAABB aabb(Vector3(GetOffsetX() + GetWidth() / 2, 0, GetOffsetY() + GetHeight() / 2),
					Vector3(GetWidth() / 2, 0, GetHeight() / 2));
				writer.WriteRegion(aabb);
				writer.EndManagedLoader();
				writer.Close();
			}
			else
			{
				OUTPUT_LOG("error: unable to create file %s\n perhaps the file is read only.\n", sOnLoadScript.c_str());
			}
		}
	}
	
	// save config file, this one should come last.
	if(bForceUpdateConfig || IsModified(MODIFIED_CONFIGURATION|MODIFIED_HOLES|MODIFIED_TEXTURE))
	{
		string sConfigFile = (filename!=0) ? filename : m_sConfigFile;
		SaveConfig(sConfigFile);
	}
	return true;
}

void Terrain::OnLoad()
{
	// call the terrain tile's on load script;  
	if(!m_sOnLoadScript.empty())
	{
		// load from asset file if it is in asset manifest list
		string filename = m_sOnLoadScript;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(m_sOnLoadScript);
		if(pEntry)
		{
			if(pEntry->CheckSyncFile())
			{
				filename = pEntry->GetLocalFileName();
			}
		}
		else
		{
			string sTmp = string("Terrain.OnLoad using local file:") + filename + "\n";
			CAsyncLoader::GetSingleton().log(sTmp);
		}
		
		CGlobals::SetLoading(true);

		std::string sEncodedString;
		for(size_t i=0;i< filename.size(); ++i)
		{
			if(filename[i]!='\'')
				sEncodedString+=filename[i];
			else
			{
				sEncodedString+='\\';
				sEncodedString+=filename[i];
			}
		}
		
		std::string sCode = "if(ParaSandBox and ParaSandBox.ver == 1) then ParaSandBox.load('";
		sCode += sEncodedString;
		sCode += "'); else NPL.load('(gl)";
		sCode += sEncodedString;
		sCode += "'); end";
		CGlobals::GetAISim()->NPLDoString(sCode.c_str(), (int)(sCode.size()));

		/*
		const char * sSandBox = CGlobals::GetWorldInfo()->GetScriptSandBox();
		if(sSandBox==NULL)
		{
			CGlobals::GetAISim()->NPLLoadFile(filename.c_str(), true);
		}
		else
		{
			// use a sandbox 
			NPL::NPLRuntimeState_ptr runtime_state =  CGlobals::GetNPLRuntime()->CreateGetRuntimeState(sSandBox);
			NPL::NPLFileName filename(filename.c_str());
			runtime_state->LoadFile_any(filename.sRelativePath);
		}
		*/
		CGlobals::SetLoading(false);
	}

	// load all NPC from database in this terrain tile.
	CGlobals::GetScene()->DB_LoadNPCsByRegion(Vector3(GetOffsetX(), 0, GetOffsetY()), Vector3(GetOffsetX()+GetWidth(), 0, GetOffsetY()+GetHeight()), false );
}

void Terrain::SetTerrainBaseTextureFile(const string& str)
{
	if(m_sBaseTextureFile!=str)
	{
		m_sBaseTextureFile = str;
		SetModified(true, MODIFIED_CONFIGURATION);
	}
}

void Terrain::SetTerrainCommonTextureFile(const string& str)
{
	if(m_sCommonTextureFile != str)
	{
		SetCommonTexture(str.c_str());
		SetModified(true, MODIFIED_CONFIGURATION);
	}
}

int Terrain::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("IsModified", FieldType_Bool, (void*)SetModified_s, (void*)IsModified_s, NULL, NULL, bOverride);
	pClass->AddField("IsEmpty", FieldType_Bool, NULL, (void*)IsEmpty_s, NULL, NULL, bOverride);
	pClass->AddField("Size", FieldType_Float, NULL, (void*)GetSize_s, NULL, NULL, bOverride);

	pClass->AddField("OnloadScript", FieldType_String, NULL, (void*)GetOnloadScript_s, CAttributeField::GetSimpleSchemaOfScript(), NULL, bOverride);
	pClass->AddField("height map", FieldType_String, NULL, (void*)GetTerrainElevFile_s, NULL, NULL, bOverride);
	pClass->AddField("ConfigFile", FieldType_String, NULL, (void*)GetTerrainConfigFile_s, CAttributeField::GetSimpleSchemaOfScript(), NULL, bOverride);
	pClass->AddField("Base Texture", FieldType_String, (void*)SetTerrainBaseTextureFile_s, (void*)GetTerrainBaseTextureFile_s, CAttributeField::GetSimpleSchema(SCHEMA_FILE), NULL, bOverride);
	pClass->AddField("CommonTexture", FieldType_String, (void*)SetTerrainCommonTextureFile_s, (void*)GetTerrainCommonTextureFile_s, CAttributeField::GetSimpleSchema(SCHEMA_FILE), NULL, bOverride);
	
	pClass->AddField("NumOfRegions", FieldType_Int, NULL, (void*)GetNumOfRegions_s, NULL, NULL, bOverride);
	pClass->AddField("CurrentRegionIndex", FieldType_Int, (void*)SetCurrentRegionIndex_s, (void*)GetCurrentRegionIndex_s, NULL, NULL, bOverride);
	pClass->AddField("CurrentRegionName", FieldType_String, (void*)SetCurrentRegionName_s, (void*)GetCurrentRegionName_s, NULL, NULL, bOverride);
	pClass->AddField("CurrentRegionFilepath", FieldType_String, (void*)SetCurrentRegionFilepath_s, (void*)GetCurrentRegionFilepath_s, CAttributeField::GetSimpleSchema(SCHEMA_FILE), NULL, bOverride);

	return S_OK;
}


bool Terrain::InvokeEditor(int nFieldID, const string& sParameters)
{
	bool bFound = false;
	CAttributeField* pField = GetAttributeClass()->GetField(nFieldID);
	if(pField!=0)
	{
		bFound = true;
		if (pField->GetFieldname().substr(0, 2) == "On" || pField->GetFieldname() == "ConfigFile")
		{
			///////////////////////////////////////////////////////////////////////////
			// get the script file name from the field value

			const char* sScriptFile = NULL;
			pField->Get(this, &sScriptFile);
			if(sScriptFile!=NULL && sScriptFile[0]!='\0')
			{
				if(CParaFile::DoesFileExist(sScriptFile, false))
				{
					return OpenWithDefaultEditor(sScriptFile, false);
				}
				else 
					return false;
			}else 
				return false;
		}
		else
			bFound = false;
	}
	if(!bFound)
		bFound = IAttributeFields::InvokeEditor(nFieldID, sParameters);
	return bFound;
}

void Terrain::SetEditorMode(bool enable)
{
	m_isEditorMode = enable;
	m_pCollisionBuffer.ReleaseBuffer();
	m_pEditorMeshVB.ReleaseBuffer();

}
