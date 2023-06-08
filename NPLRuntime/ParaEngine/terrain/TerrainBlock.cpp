// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "TerrainBlock.h"
#include "Settings.h"
#include "Terrain.h"
#include "TextureEntity.h"
#include "memdebug.h"
#include <queue>
extern float numBlocks;
extern float numLevels;
extern float hashDelta;

using namespace ParaTerrain;
using namespace std;

TerrainBlock::TerrainBlock(TerrainBlock * pParent,bool useGeoMipmap)
:m_pParent(pParent), m_fMaxError(0.f), m_texture_group_id(0), m_texture_fog_id(0), m_texture_cell_id(0),m_fog_side(0),m_frustum_state(2)
{
	m_pChildren = NULL;
	m_nTriangleStripIndex = -1;
	m_bIsHole = false;
	m_bHasHole = false;
#if _USE_RAYTRACING_SUPPORT_ > 0
	m_pTriangles = NULL;
#endif

	//geoMipmapCode
	m_useGeoMipmap = useGeoMipmap;
}

TerrainBlock::TerrainBlock(int homeVertex, int stride, Terrain * pTerrain, TerrainBlock * pParent,bool useGeoMipmap)
:m_pParent(pParent), m_fMaxError(0.f), m_texture_group_id(0), m_fog_side(0),m_frustum_state(2)
{
	m_nTriangleStripIndex = -1;
	m_HomeIndex = homeVertex;
	m_Stride = stride;
	m_bIsHole = false;
	m_bHasHole = false;
	static int numBlocksBuilt = 0;

	m_useGeoMipmap = useGeoMipmap;

	if (Settings::GetInstance()->IsVerbose())
	{
		if (hashDelta <= numBlocksBuilt++)
		{
			cout << "#" << flush;
			numBlocksBuilt = 0;
		}
	}
	int nWidthVertices = pTerrain->GetWidthVertices();
	int nRow = (int)(homeVertex /nWidthVertices);
	int nCol = (int)((homeVertex - nRow * nWidthVertices) / pTerrain->m_MaximumVisibleBlockSize);
	nRow = (int)(nRow / pTerrain->m_MaximumVisibleBlockSize);
	m_texture_tile_x = (float) nCol;
	m_texture_tile_y = (float) nRow;
	m_texture_cell_id = nRow*((int)(nWidthVertices/pTerrain->m_MaximumVisibleBlockSize)) + nCol;
	m_texture_group_id = m_texture_cell_id;
	m_texture_fog_id = ((int)(nWidthVertices / pTerrain->m_MaximumVisibleBlockSize));
	m_texture_fog_id *= m_texture_fog_id;
	
	// Recursively build children blocks of this block.

	if(!m_useGeoMipmap)
	{
		int childrenStride = m_Stride / 2;
		if (2 < m_Stride)
		{
			m_pChildren = new TerrainBlock *[4];
			m_pChildren[0] = new TerrainBlock(homeVertex, childrenStride, pTerrain, this);
			m_pChildren[1] = new TerrainBlock(homeVertex + childrenStride, childrenStride, pTerrain, this);
			m_pChildren[2] = new TerrainBlock(homeVertex + childrenStride * nWidthVertices + childrenStride, childrenStride, pTerrain, this);
			m_pChildren[3] = new TerrainBlock(homeVertex + childrenStride * nWidthVertices, childrenStride, pTerrain, this);
		}
	}
	else
	{
		m_chunkCountX = m_Stride /  pTerrain->m_MaximumVisibleBlockSize;
		
	
		m_pChildren = new TerrainBlock *[m_chunkCountX];
		for (int i = 0;i<m_chunkCountX;i++)
		{
			m_pChildren[i] = new TerrainBlock[m_chunkCountX];

			for (int j=0;j<m_chunkCountX;j++)
			{
				m_pChildren[i][j].InitMipmapBlock(0,16,pTerrain,this,i,j);
			}
		}
		
		/*
		m_pChildren = new TerrainBlock * [m_chunkCountX*m_chunkCountX];
		for (int i = 0;i<m_chunkCountX;i++)
		{
			for (int j=0;j<m_chunkCountX;j++)
			{
				int idx = i * m_chunkCountX + j;
				m_pChildren[idx] = new TerrainBlock();
				m_pChildren[idx].InitMipmapBlock(0,16,pTerrain,this,i,j);
			}
		}
		*/
	}
}

TerrainBlock::~TerrainBlock()
{
	m_nTriangleStripIndex = -1;

	if(!m_useGeoMipmap)
	{
		if (m_pChildren != NULL && 2 < m_Stride)
		{
			for (int i = 0; i < 4; i++)
			{
				delete m_pChildren[i];
				m_pChildren[i] = NULL;
			}
			delete[]m_pChildren;
		}
	}
	else
	{
		if(m_pChildren != NULL)
		{
			for (int i = 0;i<m_chunkCountX;i++)
			{
				delete[] m_pChildren[i];
				m_pChildren[i] = NULL;
			}
			delete[] m_pChildren;
		}
	}
}

bool TerrainBlock::UpdateHoles(Terrain * pTerrain)
{
	m_bHasHole = false;
	if(pTerrain->m_nHoleScale >= m_Stride)
	{
		/// if the block is smaller than a terrain hole, check from the hole map whether this block is inside a hole
		int childrenStride = m_Stride / 2;
		int nIndex = m_HomeIndex + childrenStride * pTerrain->GetWidthVertices() + childrenStride;
		Vector3 v = pTerrain->m_pVertices[nIndex];
		m_bIsHole = pTerrain->IsHole(v.x, v.y);
	}
	else if(m_pChildren && (2 < m_Stride))
	{
		// recursively call the child functions.
		// if any of its child block contains a hole then the block contains a hole. 
		for(int i=0; i<4;i++)
		{
			m_bHasHole = m_pChildren[i]->UpdateHoles(pTerrain) || m_bHasHole;
		}

		/// if this block is a container block, then this block will only be a hole if all of its child is a hole block
		m_bIsHole = true;
		for(int i=0; i<4;i++)
		{
			if(m_pChildren[i]->m_bIsHole == false)
				m_bIsHole = false;
		}
	}
	m_bHasHole = m_bHasHole || m_bIsHole;
	return m_bHasHole;
}

bool TerrainBlock::IsActive()
{
	return m_bIsActive;
}

void TerrainBlock::Tessellate(uint32 *pCountStrips, Terrain * pTerrain)
{
	//GeoMipmapCode
	if(m_useGeoMipmap)
	{
		TessellateGeoMipmap(pTerrain);
		return;
	}

/** define to use depth first transversing*/
#define USE_DEPTH_FIRST_TESSELLATE
#ifdef USE_DEPTH_FIRST_TESSELLATE
	// depth first using recursive functions. 
	if(Tessellate_NonRecursive(pCountStrips, pTerrain))
	{
		m_pChildren[0]->Tessellate(pCountStrips, pTerrain);
		m_pChildren[1]->Tessellate(pCountStrips, pTerrain);
		m_pChildren[2]->Tessellate(pCountStrips, pTerrain);
		m_pChildren[3]->Tessellate(pCountStrips, pTerrain);
	}
#else
	queue_TerrainBlockPtr_Type queueBlocks;
	queueBlocks.push((TerrainBlock*)this);
	/// breadth first transversing the quad tree 
	while(!queueBlocks.empty())
	{
		TerrainBlock* pBlock = queueBlocks.front();
		queueBlocks.pop();
		if(pBlock->Tessellate_NonRecursive(pCountStrips, pTerrain))
		{
			queueBlocks.push(pBlock->m_pChildren[0]);
			queueBlocks.push(pBlock->m_pChildren[1]);
			queueBlocks.push(pBlock->m_pChildren[2]);
			queueBlocks.push(pBlock->m_pChildren[3]);
		}
	}
#endif
}

bool TerrainBlock::Tessellate_NonRecursive(uint32 *pCountStrips, Terrain * pTerrain)
{
	bool bProcessChild = false;
	/** Check if the terrain block is inside a hole, if so we will deactivate this terrain block as well as all of its child blocks
	so that they will not be rendered.*/
	if(!m_bIsHole)
	{
		CShapeBox boundingBox;
		float width = m_Stride * pTerrain->GetVertexSpacing();
		if(pTerrain->m_pVertices == NULL)
			return false;
		boundingBox.m_Min.x = pTerrain->m_pVertices[m_HomeIndex].x;
		boundingBox.m_Min.y = pTerrain->m_pVertices[m_HomeIndex].y;
		boundingBox.m_Min.z = m_MinElevation;
		boundingBox.m_Max.x = boundingBox.m_Min.x + width;
		boundingBox.m_Max.y = boundingBox.m_Min.y + width;
		boundingBox.m_Max.z = m_MaxElevation;
		
		if(m_pParent==0 || m_pParent->m_frustum_state == 2)
			m_frustum_state = pTerrain->CuboidInFrustum(boundingBox);
		else
			m_frustum_state = m_pParent->m_frustum_state;

		if (m_frustum_state>0)
		{
			/** block must be further tessellated if its size is larger than m_MaximumVisibleBlockSize or its bounding box is within the detailed (no tessellation) radius */
			if (pTerrain->m_MaximumVisibleBlockSize < m_Stride || m_bHasHole)
			{
				bProcessChild = true;
				m_bIsActive = false;
				m_bChildrenActive = true;
			}
			else
			{
				if(pTerrain->m_bFogEnabled)
				{
/** whether we will use sphere or plane to decide whether this block in completely in fog. 
Currently ParaEngine uses plane based fog, so we need to use fog plane.
*/
// #define USE_FOG_SPHERE
#ifdef USE_FOG_SPHERE
					// we will assign terrain block triangles that are in remote fog to a special m_texture_fog_id
					if( ! m_pParent->IsInFogRadius()) 
					{
						m_texture_group_id = m_texture_fog_id;	
					}
					else
					{
						m_texture_group_id = (pTerrain->CuboidInFogRadius(boundingBox)) ? m_texture_cell_id : m_texture_fog_id;
					}
#else // USE FOG PLANE
					if( m_pParent->GetFogSide() == Plane::POSITIVE_SIDE || m_pParent->GetFogSide() == Plane::NEGATIVE_SIDE) 
					{
						m_fog_side = m_pParent->GetFogSide();
						m_texture_group_id = m_pParent->m_texture_group_id;	
					}
					else
					{
						m_fog_side = pTerrain->GetCuboidFogSide(boundingBox);
						m_texture_group_id = ( m_fog_side != Plane::POSITIVE_SIDE) ? m_texture_cell_id : m_texture_fog_id;
					}
#endif
				}
				else
				{
					m_texture_group_id = m_texture_cell_id;
				}

				/**
				LiXizhi 2009.2.15: http://www.gamasutra.com/features/20000228/ulrich_02.htm
				Instead of screen-space geometric error, I advocate doing a similar test which results in 3D view-space error proportional to view distance. 
					bc[x,y,z] == coordinates of box center
					ex[x,y,z] == extent of box from the center (i.e. 1/2 the box dimensions) 
					L1 = max(abs(bcx - viewx) - exx, abs(bcy - viewy) - exy, abs(bcz - viewz) - exz)
					maxerror = max vertex error in the box in 3d space. 
					enabled = maxerror * Threshold < L1
				*/

				// the bigger, the finer(more triangles drawn).We can interpret the value as 
				// a unit 1 meter difference in height will be ignore over 50 meters from camera
				const float fThreshold = 50.f; 
				bool bSimplified = GetMaxError() * fThreshold<pTerrain->GetBoxToEyeMatrics(boundingBox);

				if (bSimplified)
				{
					// This block is simplified, so add its triangles to the list and stop recursing.
					CreateTriangleStrip(pCountStrips, pTerrain);
					m_bIsActive = true;
					m_bChildrenActive = false;

					// set lowest visible height
					if(m_MinElevation < pTerrain->GetLowestVisibleHeight())
						pTerrain->SetLowestVisibleHeight(m_MinElevation);
				}
				else
				{
					bProcessChild = true;
					m_bIsActive = false;
					m_bChildrenActive = true;
				}
			}
			/** if it is the leave node that need to be further tessellated, we will build the triangles directly.*/
			if ((m_Stride == 2) && (m_bChildrenActive))
			{
				// set lowest visible height
				if(m_MinElevation < pTerrain->GetLowestVisibleHeight())
					pTerrain->SetLowestVisibleHeight(m_MinElevation);

				int offset;
				int nWidthVertices = pTerrain->GetWidthVertices();
				int nLastStripCount = *pCountStrips;

				pTerrain->SetVertexStatus(m_HomeIndex, 1);
				TriangleStrip* tri_strip = CreateGetTriangleStrip(nLastStripCount, pTerrain);
				tri_strip->m_pVertices[0] = m_HomeIndex;
				offset = m_HomeIndex + nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[1] = offset;
				offset = m_HomeIndex + 1;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[2] = offset;
				offset += nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[3] = offset;
				offset = m_HomeIndex + 2;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[4] = offset;
				offset += nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[5] = offset;
				tri_strip->m_NumberOfVertices = 6;
				tri_strip->m_bEnabled = true;
				/// whether to use detailed texture

				offset = nWidthVertices + m_HomeIndex;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip = CreateGetTriangleStrip(nLastStripCount+1, pTerrain);
				tri_strip->m_pVertices[0] = offset;
				offset +=  nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[1] = offset;
				offset = nWidthVertices + m_HomeIndex + 1;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[2] = offset;
				offset += nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[3] = offset;
				offset = nWidthVertices + m_HomeIndex + 2;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[4] = offset;
				offset += nWidthVertices;
				pTerrain->SetVertexStatus(offset, 1);
				tri_strip->m_pVertices[5] = offset;
				tri_strip->m_NumberOfVertices = 6;
				tri_strip->m_bEnabled = true;
				*pCountStrips = nLastStripCount + 2;

				m_nTriangleStripIndex = -1;
				m_bIsActive = true;
				m_bChildrenActive = false;
				bProcessChild = false;
			}
		}
		else
		{
			m_bIsActive = false;
			m_bChildrenActive = false;
		}
	}
	else
	{
		m_bIsActive = false;
		m_bChildrenActive = false;
	}
	return bProcessChild;
}

TriangleStrip* TerrainBlock::CreateGetTriangleStrip( int nIndex, Terrain * pTerrain )
{
	TriangleStrip* pTriangle = pTerrain->GetSafeTriStrip(nIndex);
	pTriangle->texture_group_id = m_texture_group_id;

	pTriangle->fOffsetTexU = m_texture_tile_x;
	pTriangle->fOffsetTexV = m_texture_tile_y;
	return pTriangle;
}

TriangleFan* TerrainBlock::CreateGetTriangleFan( int nIndex, Terrain * pTerrain )
{
	TriangleFan* pTriangle = pTerrain->GetSafeTriFan(nIndex);
	pTriangle->texture_group_id = m_texture_group_id;
	
	pTriangle->fOffsetTexU = m_texture_tile_x;
	pTriangle->fOffsetTexV = m_texture_tile_y;
	return pTriangle;
}

void TerrainBlock::CreateTriangleStrip(uint32 *pCount, Terrain * pTerrain)
{
	pTerrain->SetVertexStatus(m_HomeIndex, 1);
	int nLastCount =*pCount;
	TriangleStrip* tri_strip = CreateGetTriangleStrip(nLastCount, pTerrain);

	int nWidthVertices = pTerrain->GetWidthVertices();
	tri_strip->m_pVertices[0] = m_HomeIndex;
	int offset = m_HomeIndex + nWidthVertices * m_Stride;
	pTerrain->SetVertexStatus(offset, 1);
	tri_strip->m_pVertices[1] = offset;
	offset = m_HomeIndex + m_Stride;
	pTerrain->SetVertexStatus(offset, 1);
	tri_strip->m_pVertices[2] = offset;
	offset += nWidthVertices * m_Stride;
	pTerrain->SetVertexStatus(offset, 1);
	tri_strip->m_pVertices[3] = offset;
	tri_strip->m_bEnabled = true;
	tri_strip->m_NumberOfVertices = 4;
	
	m_nTriangleStripIndex = nLastCount;
	*pCount = nLastCount + 1;
}

void TerrainBlock::EnableStrip(Terrain* pTerrain, bool bEnabled)
{
	pTerrain->GetTriStrip(m_nTriangleStripIndex)->m_bEnabled = bEnabled;
}

void TerrainBlock::RepairCracks(Terrain * pTerrain, uint32 *pCountFans)
{
	//GeoMipmapCode
	if(m_useGeoMipmap)
		return;

	if(m_nTriangleStripIndex==-1 && m_bIsActive){
		// no need to repair, because it is the smallest
		PE_ASSERT(m_Stride==2);
		return;
	}
	int nWidthVertices = pTerrain->GetWidthVertices();
	
	if (2 <= m_Stride)
	{
		if (m_bIsActive)
		{
			int halfStride = m_Stride / 2;
			int bottomLeft = m_HomeIndex + m_Stride * nWidthVertices;
			int bottomRight = bottomLeft + m_Stride;
			int i, previousVertex;
			int v0;
			int numVertices = 0;

			bool bNeedToFix = false;
			/** for the special condition*/
			if(m_Stride==2){
				i = m_HomeIndex;
				if( (pTerrain->GetVertexStatus(i+1) == 1) || 
					(pTerrain->GetVertexStatus(i+nWidthVertices)==1) ||
					(pTerrain->GetVertexStatus(i+nWidthVertices+2)==1) || 
					(pTerrain->GetVertexStatus(i+(nWidthVertices*2)+1)==1) )
				{
					bNeedToFix =true;
				}
			}
			else
			{
				for (i = m_HomeIndex + m_Stride - 1; m_HomeIndex < i && !bNeedToFix; i--)
					bNeedToFix = (pTerrain->GetVertexStatus(i) == 1);
				if (!bNeedToFix)
				{
					for (i = m_HomeIndex + nWidthVertices; i < m_HomeIndex + m_Stride * nWidthVertices && !bNeedToFix; i += nWidthVertices)
						bNeedToFix = (pTerrain->GetVertexStatus(i) == 1);
					if (!bNeedToFix)
					{
						for (i = bottomLeft + 1; i < bottomRight && !bNeedToFix; i++)
							bNeedToFix = (pTerrain->GetVertexStatus(i) == 1);
						if (!bNeedToFix)
						{
							for (i = bottomRight - nWidthVertices; m_HomeIndex + m_Stride < i && !bNeedToFix; i -= nWidthVertices)
								bNeedToFix = (pTerrain->GetVertexStatus(i) == 1);
						}
					}
				}
			}
		
			if (bNeedToFix)
			{
				EnableStrip(pTerrain, false);
				v0 = m_HomeIndex + halfStride + halfStride * nWidthVertices;
				PE_ASSERT(0 <= v0);
				PE_ASSERT(v0 < pTerrain->GetNumberOfVertices());
				int nCurrentFan = *pCountFans;
				TriangleFan* pTriFan = CreateGetTriangleFan(nCurrentFan, pTerrain);
				pTriFan->m_pVertices[0] = v0;
				numVertices = 0;
				for (i = m_HomeIndex + m_Stride; m_HomeIndex <= i; i--)
				{
					PE_ASSERT(0 <= i);
					PE_ASSERT(i < pTerrain->GetNumberOfVertices());
					if (pTerrain->GetVertexStatus(i) == 1)
					{
						if (++numVertices == MAX_VERTICES_PER_FAN - 1)
						{
							// We have reached the maximum size for a fan, so start a new fan.
							pTriFan->m_NumberOfVertices = numVertices;
							++nCurrentFan;
							pTriFan = CreateGetTriangleFan(nCurrentFan, pTerrain);
							pTriFan->m_pVertices[0] = v0;
							pTriFan->m_pVertices[1] = previousVertex;
							numVertices = 2;
						}
						pTriFan->m_pVertices[numVertices] = i;
						previousVertex = i;
					}
				}
				for (i = m_HomeIndex + nWidthVertices; i <= m_HomeIndex + m_Stride * nWidthVertices; i += nWidthVertices)
				{
					PE_ASSERT(0 <= i);
					PE_ASSERT(i < pTerrain->GetNumberOfVertices());
					if (pTerrain->GetVertexStatus(i) == 1)
					{
						if (++numVertices == MAX_VERTICES_PER_FAN - 1)
						{
							// We have reached the maximum size for a fan, so start a new fan.
							pTriFan->m_NumberOfVertices = numVertices;
							++nCurrentFan;
							pTriFan = CreateGetTriangleFan(nCurrentFan, pTerrain);
							pTriFan->m_pVertices[0] = v0;
							pTriFan->m_pVertices[1] = previousVertex;
							numVertices = 2;
						}
						pTriFan->m_pVertices[numVertices] = i;
						previousVertex = i;
					}
				}
				for (i = bottomLeft; i <= bottomRight; ++i)
				{
					PE_ASSERT(0 <= i);
					PE_ASSERT(i < pTerrain->GetNumberOfVertices());
					if (pTerrain->GetVertexStatus(i) == 1)
					{
						if (++numVertices == MAX_VERTICES_PER_FAN - 1)
						{
							// We have reached the maximum size for a fan, so start a new fan.
							pTriFan->m_NumberOfVertices = numVertices;
							++nCurrentFan;
							pTriFan = CreateGetTriangleFan(nCurrentFan, pTerrain);
							pTriFan->m_pVertices[0] = v0;
							pTriFan->m_pVertices[1] = previousVertex;
							numVertices = 2;
						}
						pTriFan->m_pVertices[numVertices] = i;
						previousVertex = i;
					}
				}
				for (i = bottomRight - nWidthVertices; m_HomeIndex + m_Stride <= i; i -= nWidthVertices)
				{
					PE_ASSERT(0 <= i);
					PE_ASSERT(i < pTerrain->GetNumberOfVertices());
					if (pTerrain->GetVertexStatus(i) == 1)
					{
						if (++numVertices == MAX_VERTICES_PER_FAN - 1)
						{
							// We have reached the maximum size for a fan, so start a new fan.
							pTriFan->m_NumberOfVertices = numVertices;
							++nCurrentFan;
							pTriFan = CreateGetTriangleFan(nCurrentFan, pTerrain);
							pTriFan->m_pVertices[0] = v0;
							pTriFan->m_pVertices[1] = previousVertex;
							numVertices = 2;
						}
						pTriFan->m_pVertices[numVertices] = i;
						previousVertex = i;
					}
				}
				pTriFan->m_NumberOfVertices = numVertices + 1;
				++nCurrentFan;
				*pCountFans = nCurrentFan;
			}
		}
		else if (m_bChildrenActive)
		{
			m_pChildren[0]->RepairCracks(pTerrain, pCountFans);
			m_pChildren[1]->RepairCracks(pTerrain, pCountFans);
			m_pChildren[2]->RepairCracks(pTerrain, pCountFans);
			m_pChildren[3]->RepairCracks(pTerrain, pCountFans);
		}
	}
}

void TerrainBlock::VertexChanged(Terrain * pTerrain)
{
	CalculateGeometry(pTerrain);
}

// speed up variant of VertexChanged by detecting blocks 
// where vertex changed
void TerrainBlock::VertexChanged(Terrain * pTerrain, int index)
{
	int nWidthVertices = pTerrain->GetWidthVertices();
	int y1 = m_HomeIndex / pTerrain->GetWidthVertices();
	int x1 = m_HomeIndex - y1 * nWidthVertices;
	int last_index = m_HomeIndex + m_Stride * (nWidthVertices + 1);
	int y2 = last_index / nWidthVertices;
	int x2 = last_index - y2 * nWidthVertices;
	int y = index / nWidthVertices;
	int x = index - y * nWidthVertices;
	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
	{
		CalculateGeometry(pTerrain);
	}
}

void TerrainBlock::VertexChanged(Terrain * pTerrain, int index1, int index2)
{
	int nWidthVertices = pTerrain->GetWidthVertices();
	// block coords
	int last_index = m_HomeIndex + m_Stride * (nWidthVertices + 1);

	// top left
	int block_y11 = m_HomeIndex / nWidthVertices;
	int block_x11 = m_HomeIndex - block_y11 * nWidthVertices;

	// bottom right
	int block_y22 = last_index / nWidthVertices;
	int block_x22 = last_index - block_y22 * nWidthVertices;

	// top right
	int block_x12 = block_x22;
	int block_y12 = block_y11;

	// bottom left
	int block_x21 = block_x11;
	int block_y21 = block_y22;

	// changed rectangle coords
	// top left
	int changed_y11 = index1 / nWidthVertices;
	int changed_x11 = index1 - changed_y11 * nWidthVertices;

	// bottom right
	int changed_y22 = index2 / nWidthVertices;
	int changed_x22 = index2 - changed_y22 * nWidthVertices;

	// top right
	int changed_x12 = changed_x22;
	int changed_y12 = changed_y11;

	// bottom left
	int changed_x21 = changed_x11;
	int changed_y21 = changed_y22;

	// detect of intersection of two boxes ((block_x11, block_y11) - (block_x22, block_y22))
	// and ((changed_x11, changed_y11) - (changed_x22, changed_y22))
	if ((changed_x11 >= block_x11 && changed_x11 <= block_x22 && changed_y11 >= block_y11 && changed_y11 <= block_y22) || (changed_x12 >= block_x11 && changed_x12 <= block_x22 && changed_y12 >= block_y11 && changed_y12 <= block_y22) || (changed_x21 >= block_x11 && changed_x21 <= block_x22 && changed_y21 >= block_y11 && changed_y21 <= block_y22) || (changed_x22 >= block_x11 && changed_x22 <= block_x22 && changed_y22 >= block_y11 && changed_y22 <= block_y22) || (block_x11 >= changed_x11 && block_x11 <= changed_x22 && block_y11 >= changed_y11 && block_y11 <= changed_y22) || (block_x12 >= changed_x11 && block_x12 <= changed_x22 && block_y12 >= changed_y11 && block_y12 <= changed_y22) || (block_x21 >= changed_x11 && block_x21 <= changed_x22 && block_y21 >= changed_y11 && block_y21 <= changed_y22) || (block_x22 >= changed_x11 && block_x22 <= changed_x22 && block_y22 >= changed_y11 && block_y22 <= changed_y22))
	{
		CalculateGeometry(pTerrain);
	}
}

void TerrainBlock::ComputeMaxVertexError( Terrain * pTerrain )
{
	float VertexSpacing = pTerrain->GetVertexSpacing();
	int nWidthVertices = pTerrain->GetWidthVertices();
	float cornerHeights[4] = {
		pTerrain->GetElevation(0),
		pTerrain->GetElevation(m_HomeIndex + nWidthVertices * m_Stride),
		pTerrain->GetElevation(m_HomeIndex + m_Stride),
		pTerrain->GetElevation(m_HomeIndex + m_Stride + nWidthVertices * m_Stride),
	};

	ParaEngine::Plane planeA(Vector3(0,0,cornerHeights[0]), Vector3(m_Stride,0,cornerHeights[1]), Vector3(0,(float)m_Stride,cornerHeights[2]));
	ParaEngine::Plane planeB(Vector3((float)m_Stride,0,cornerHeights[1]), Vector3(0,(float)m_Stride,cornerHeights[2]), Vector3((float)m_Stride,(float)m_Stride,cornerHeights[3]));

	m_fMaxError = 0.f;

	// for planeA
	for (int i = 0; i <= m_Stride; i++)
	{
		for (int j = 0; j <= (m_Stride-i); j++)
		{
			if(!((i==0 && j==0) || (i==0 && j==m_Stride) || (i==m_Stride && j==m_Stride)))
			{
				float fError = fabs(planeA.getDistance(Vector3((float)i,(float)j,pTerrain->GetElevation(m_HomeIndex + i * nWidthVertices + j))));
				if(fError>m_fMaxError)
					m_fMaxError = fError;
			}
		}
	}

	// for planeB
	for (int i = 1; i <= m_Stride; i++)
	{
		for (int j = m_Stride-i+1; j <= m_Stride; j++)
		{
			if(!((i==m_Stride && j==m_Stride)))
			{
				float fError = fabs(planeB.getDistance(Vector3((float)i,(float)j,pTerrain->GetElevation(m_HomeIndex + i * nWidthVertices + j))));
				if(fError>m_fMaxError)
					m_fMaxError = fError;
			}
		}
	}

	if(m_Stride > 2)
	{
		m_pChildren[0]->ComputeMaxVertexError(pTerrain);
		m_pChildren[1]->ComputeMaxVertexError(pTerrain);
		m_pChildren[2]->ComputeMaxVertexError(pTerrain);
		m_pChildren[3]->ComputeMaxVertexError(pTerrain);
	}

}

void TerrainBlock::CalculateGeometry(Terrain * pTerrain)
{
	//GeoMipmapCode
	if(m_useGeoMipmap)
	{
		CalculateGeometryGeoMipmap(pTerrain);
		return;
	}
	
	float VertexSpacing = pTerrain->GetVertexSpacing();
	
	if (m_Stride == 2)
	{
		// Find this block's bounding box.
		m_MinElevation = pTerrain->GetElevation(m_HomeIndex);
		m_MaxElevation = m_MinElevation;
		int WidthVertices = pTerrain->GetWidthVertices();
		for (int i = 0; i <= m_Stride; i++)
		{
			for (int j = 0; j <= m_Stride; j++)
			{
				float elevation = pTerrain->GetElevation(m_HomeIndex + i * WidthVertices + j);
				if (elevation < m_MinElevation)
					m_MinElevation = elevation;
				if (m_MaxElevation < elevation)
					m_MaxElevation = elevation;
			}
		}
	}
	else if(m_Stride > 2)
	{
		m_pChildren[0]->CalculateGeometry(pTerrain);
		m_pChildren[1]->CalculateGeometry(pTerrain);
		m_pChildren[2]->CalculateGeometry(pTerrain);
		m_pChildren[3]->CalculateGeometry(pTerrain);

		m_MinElevation = m_pChildren[0]->m_MinElevation;
		m_MaxElevation = m_pChildren[0]->m_MaxElevation;
		for(int i=1;i<4;i++)
		{
			if (m_pChildren[i]->m_MinElevation < m_MinElevation)
				m_MinElevation = m_pChildren[i]->m_MinElevation;
			if (m_MaxElevation < m_pChildren[i]->m_MaxElevation)
				m_MaxElevation = m_pChildren[i]->m_MaxElevation;
		}
	}
	ComputeMaxVertexError(pTerrain);

#if _USE_RAYTRACING_SUPPORT_ > 0
// Build triangles for ray intersection and collision detection.
	if (m_Stride == 2)
	{
		m_pTriangles = new Triangle[8];
		int nWidthVertices = pTerrain->GetWidthVertices();
		m_pTriangles[0].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + 1]);
		m_pTriangles[1].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + 1], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1]);
		m_pTriangles[2].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + 1], pTerrain->m_pVertices[m_HomeIndex + 1 + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + 2]);
		m_pTriangles[3].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + 2], pTerrain->m_pVertices[m_HomeIndex + 1 + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + 2 + nWidthVertices]);
		m_pTriangles[4].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1]);
		m_pTriangles[5].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + nWidthVertices + 1]);
		m_pTriangles[6].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1 + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 2]);
		m_pTriangles[7].DefineFromPoints(pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 2], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 1 + nWidthVertices], pTerrain->m_pVertices[m_HomeIndex + nWidthVertices + 2 + nWidthVertices]);
	}
#endif
}

void TerrainBlock::IntersectRay(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain)
{
// First test ray against this block's bounding box.
	CShapeBox boundingBox;
	float width = m_Stride * pTerrain->GetVertexSpacing();
	Vector3* pVertices = pTerrain->m_pVertices;
	boundingBox.m_Min.x = pVertices[m_HomeIndex].x;
	boundingBox.m_Min.y = pVertices[m_HomeIndex].y;
	boundingBox.m_Min.z = m_MinElevation;
	boundingBox.m_Max.x = boundingBox.m_Min.x + width;
	boundingBox.m_Max.y = boundingBox.m_Min.y + width;
	boundingBox.m_Max.z = m_MaxElevation;
	float fDistance;
	if (ray.IntersectBox( &boundingBox, NULL, &fDistance) && fDistance < lowestDistance)
	{
		if (2 < m_Stride)
		{
			if(m_pChildren == NULL || m_pChildren[0] == NULL)
				return;

			m_pChildren[0]->IntersectRay(ray, intersectionPoint, lowestDistance, pTerrain);
			m_pChildren[1]->IntersectRay(ray, intersectionPoint, lowestDistance, pTerrain);
			m_pChildren[2]->IntersectRay(ray, intersectionPoint, lowestDistance, pTerrain);
			m_pChildren[3]->IntersectRay(ray, intersectionPoint, lowestDistance, pTerrain);
		}
		else
		{
			Triangle pTriangles[8];
			int nWidthVertices = pTerrain->GetWidthVertices();
			pTriangles[0].DefineFromPoints(pVertices[m_HomeIndex], pVertices[m_HomeIndex + nWidthVertices], pVertices[m_HomeIndex + 1]);
			pTriangles[1].DefineFromPoints(pVertices[m_HomeIndex + 1], pVertices[m_HomeIndex + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + 1]);
			pTriangles[2].DefineFromPoints(pVertices[m_HomeIndex + 1], pVertices[m_HomeIndex + 1 + nWidthVertices], pVertices[m_HomeIndex + 2]);
			pTriangles[3].DefineFromPoints(pVertices[m_HomeIndex + 2], pVertices[m_HomeIndex + 1 + nWidthVertices], pVertices[m_HomeIndex + 2 + nWidthVertices]);
			pTriangles[4].DefineFromPoints(pVertices[m_HomeIndex + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + 1]);
			pTriangles[5].DefineFromPoints(pVertices[m_HomeIndex + nWidthVertices + 1], pVertices[m_HomeIndex + nWidthVertices + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + nWidthVertices + 1]);
			pTriangles[6].DefineFromPoints(pVertices[m_HomeIndex + nWidthVertices + 1], pVertices[m_HomeIndex + nWidthVertices + 1 + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + 2]);
			pTriangles[7].DefineFromPoints(pVertices[m_HomeIndex + nWidthVertices + 2], pVertices[m_HomeIndex + nWidthVertices + 1 + nWidthVertices], pVertices[m_HomeIndex + nWidthVertices + 2 + nWidthVertices]);
		
			float distance;
			Vector3 point;
			for (int i = 0; i < 8; i++)
			{
				Triangle& tri = pTriangles[i];
				Plane p = tri.ComputePlane();
				if (ray.IntersectPlane(&p, &point, &distance) == 1)
				{
					if (i == 0 || i == 2 || i == 4 || i == 6)
					{
						if (tri.GetVertex(0)->x <= point.x && tri.GetVertex(0)->y <= point.y && point.x <= tri.GetVertex(2)->x && point.y <= tri.GetVertex(1)->y && ((fmod(point.y, pTerrain->m_VertexSpacing) + fmod(point.x, pTerrain->m_VertexSpacing)) <= pTerrain->m_VertexSpacing))
						{
							if (distance < lowestDistance)
							{
								lowestDistance = distance;
								intersectionPoint.x = point.x;
								intersectionPoint.y = point.y;
								intersectionPoint.z = point.z;
							}
						}
					}
					else
					{
						if (tri.GetVertex(1)->x <= point.x && tri.GetVertex(0)->y <= point.y && point.x <= tri.GetVertex(2)->x && point.y <= tri.GetVertex(1)->y && ((fmod(point.y, pTerrain->m_VertexSpacing) + fmod(point.x, pTerrain->m_VertexSpacing)) >= pTerrain->m_VertexSpacing))
						{
							if (distance < lowestDistance)
							{
								lowestDistance = distance;
								intersectionPoint.x = point.x;
								intersectionPoint.y = point.y;
								intersectionPoint.z = point.z;
							}
						}
					}
				}
			}
		}
	}
}

//====================================
//geoMipmapCode
//=====================================
void TerrainBlock::InitMipmapBlock(int homeVertex,int stride,Terrain* pTerrain,TerrainBlock* pParent,int nCol,int nRow)
{
	m_pParent = pParent;
	m_fMaxError = 0.f;
	m_texture_group_id = 0;
	m_fog_side = 0;
	m_frustum_state = 2;

	m_nTriangleStripIndex = -1;
	m_HomeIndex = homeVertex;
	m_Stride = stride;
	m_bIsHole = false;
	static int numBlocksBuilt = 0;

	if (Settings::GetInstance()->IsVerbose())
	{
		if (hashDelta <= numBlocksBuilt++)
		{
			cout << "#" << flush;
			numBlocksBuilt = 0;
		}
	}

	m_chunkCountX = 0;
	m_useGeoMipmap = true;
	m_pChildren = NULL;

	m_texture_tile_x = (float)nCol;
	m_texture_tile_y = (float)nRow;
	m_texture_cell_id = nRow * pParent->m_chunkCountX + nCol;
	m_texture_group_id = m_texture_cell_id;
	m_texture_fog_id = pParent->m_chunkCountX * pParent->m_chunkCountX;
	m_HomeIndex = pTerrain->m_TerrainBuffer.GetChunkVertexOffset(nCol,nRow);
}

//update bounding box for each block
void TerrainBlock::CalculateGeometryGeoMipmap(Terrain * pTerrain)
{
	m_MinElevation = pTerrain->GetElevation(0);
	m_MaxElevation = m_MinElevation;

	if(m_pChildren != NULL)
	{
		for(int i=0;i<m_chunkCountX;i++)
		{
			for(int j=0;j<m_chunkCountX;j++)
			{
				int widthVertices = pTerrain->GetWidthVertices();
				TerrainBlock& block = m_pChildren[j][i];
				int startIndex = i * block.m_Stride * widthVertices + j * block.m_Stride;

				block.m_MinElevation = pTerrain->GetElevation(startIndex);
				block.m_MaxElevation = block.m_MinElevation;
				for(int y =0;y<=block.m_Stride;y++)
				{
					for(int x =0;x<=block.m_Stride;x++)
					{
						float elevation = pTerrain->GetElevation(startIndex + y * widthVertices + x);
						if(block.m_MinElevation > elevation)
							block.m_MinElevation = elevation;
						if(block.m_MaxElevation < elevation)
							block.m_MaxElevation = elevation;

						if(m_MinElevation > elevation)
							m_MinElevation = elevation;
						if(m_MaxElevation < elevation)
							m_MaxElevation = elevation;
					}
				}
			}
		}
	}
}

void TerrainBlock::TessellateGeoMipmap(Terrain* pTerrain)
{
	CShapeBox boundingBox;
	float width = m_Stride * pTerrain->GetVertexSpacing();
	int startIdx = (int)(m_texture_tile_y * m_Stride * pTerrain->GetWidthVertices() + m_texture_tile_x * m_Stride);

	boundingBox.m_Min.x = pTerrain->m_pVertices[startIdx].x;
	boundingBox.m_Min.y = pTerrain->m_pVertices[startIdx].y;
	boundingBox.m_Min.z = m_MinElevation;
	boundingBox.m_Max.x = boundingBox.m_Min.x + width;
	boundingBox.m_Max.y = boundingBox.m_Min.y + width;
	boundingBox.m_Max.z = m_MaxElevation;

	if(m_pParent == NULL)
	{
		m_frustum_state = pTerrain->CuboidInFrustum(boundingBox);
		for(int y = 0;y<m_chunkCountX;y++)
		{
			for(int x = 0;x<m_chunkCountX;x++)
			{
				m_pChildren[x][y].TessellateGeoMipmap(pTerrain);
			}
		}
	}
	else
	{
		if(m_pParent->m_frustum_state == 2)
			m_frustum_state = pTerrain->CuboidInFrustum(boundingBox);
		else
			m_frustum_state = m_pParent->m_frustum_state;

		if(m_frustum_state>0)
		{
			if(pTerrain->m_bFogEnabled)
			{
#ifdef USE_FOG_SPHERE
				// we will assign terrain block triangles that are in remote fog to a special m_texture_fog_id
				if(!m_pParent->IsInFogRadius()) 
				{
					m_texture_group_id = m_texture_fog_id;	
				}
				else
				{
					m_texture_group_id = (pTerrain->CuboidInFogRadius(boundingBox)) ? m_texture_cell_id : m_texture_fog_id;
				}
#else // USE FOG PLANE
				if( m_pParent->GetFogSide() == Plane::POSITIVE_SIDE || m_pParent->GetFogSide() == Plane::NEGATIVE_SIDE) 
				{
					m_texture_group_id = m_texture_cell_id;
				}
				else
				{
					m_fog_side = pTerrain->GetCuboidFogSide(boundingBox);
					m_texture_group_id = ( m_fog_side != Plane::POSITIVE_SIDE) ? m_texture_cell_id : m_texture_fog_id;
				}
#endif
			}
			else
			{
				m_texture_group_id = m_texture_cell_id;
			}
		}
	}
}

TerrainBlock* TerrainBlock::GetChildBlock(int index)
{
	if(m_useGeoMipmap)
	{
		if(index < m_chunkCountX * m_chunkCountX)
		{
			int i = index / m_chunkCountX;
			int j = index % m_chunkCountX;
			return &m_pChildren[i][j];
		}
	}
	else
	{
		if(index < 4)
		{
			return m_pChildren[index];
		}
	}
	return NULL;
}

void TerrainBlock::SetLod(int level,GeoMipmapChunkType type)
{
	m_lodLevel = level;
	m_chunkType = type;
}

void TerrainBlock::IntersectRayGeoMipmap(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain)
{
	// First test ray against this block's bounding box.
	CShapeBox boundingBox;
	float width = m_Stride * pTerrain->GetVertexSpacing();
	Vector3* pVertices = pTerrain->m_pVertices;
	boundingBox.m_Min.x = pVertices[m_HomeIndex].x;
	boundingBox.m_Min.y = pVertices[m_HomeIndex].y;
	boundingBox.m_Min.z = m_MinElevation;
	boundingBox.m_Max.x = boundingBox.m_Min.x + width;
	boundingBox.m_Max.y = boundingBox.m_Min.y + width;
	boundingBox.m_Max.z = m_MaxElevation;
	float fDistance;

	if (ray.IntersectBox( &boundingBox, NULL, &fDistance) && fDistance < lowestDistance)
	{
		//root block
		if(m_pChildren != NULL)
		{
			int chunkCountX = m_Stride /  pTerrain->m_MaximumVisibleBlockSize;
			int chunkVertexWidth = m_Stride / chunkCountX;
			for (int i = 0;i<chunkCountX;i++)
			{
				for(int j=0;j<chunkCountX;j++)
				{
					int startVertex = i * chunkVertexWidth * pTerrain->GetWidthVertices() + j * chunkVertexWidth;
					m_pChildren[j][i].IntersectRayTriangle(ray, intersectionPoint, lowestDistance, pTerrain,startVertex);
				}
			}
		}
	}
}

void TerrainBlock::IntersectRayTriangle(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain,int startVertex)
{
	// First test ray against this block's bounding box.
	CShapeBox boundingBox;
	float width = m_Stride * pTerrain->GetVertexSpacing();
	Vector3* pVertices = pTerrain->m_pVertices;
	boundingBox.m_Min.x = pVertices[startVertex].x;
	boundingBox.m_Min.y = pVertices[startVertex].y;
	boundingBox.m_Min.z = m_MinElevation;
	boundingBox.m_Max.x = boundingBox.m_Min.x + width;
	boundingBox.m_Max.y = boundingBox.m_Min.y + width;
	boundingBox.m_Max.z = m_MaxElevation;
	float fDistance;

	if (ray.IntersectBox( &boundingBox, NULL, &fDistance) && fDistance < lowestDistance)
	{
		int trangleCount = m_Stride * m_Stride * 2;
		Triangle* pTriangles = new Triangle[trangleCount];
		int nWidthVertices = pTerrain->GetWidthVertices();
		int trangleIdx = 0;
		for(int y = 0;y<m_Stride;y++)
		{
			for(int x = 0;x<m_Stride;x++)
			{
				int idx = startVertex + y * nWidthVertices + x;
				pTriangles[trangleIdx++].DefineFromPoints(pVertices[idx + nWidthVertices],pVertices[idx + nWidthVertices + 1],pVertices[idx]);
				pTriangles[trangleIdx++].DefineFromPoints(pVertices[idx + nWidthVertices + 1],pVertices[idx + 1],pVertices[idx]);
			}
		}

		float distance;
		Vector3 point;
		for(int i=0;i<trangleCount;i++)
		{
			Triangle& tri = pTriangles[i];
			Plane p = tri.ComputePlane();
			if (ray.IntersectPlane(&p, &point, &distance) == 1)
			{
				if(PointInTriangle(tri,point))
				{
					if (distance < lowestDistance)
					{
						lowestDistance = distance;
						intersectionPoint.x = point.x;
						intersectionPoint.y = point.y;
						intersectionPoint.z = point.z;
					}
				}
			}
		}
		delete[] pTriangles;
	}
}

bool TerrainBlock::PointInTriangle(Triangle& tri,Vector3& point)
{
	Vector3* a = tri.GetVertex(0);
	Vector3* b = tri.GetVertex(1);
	Vector3* c = tri.GetVertex(2);

	Vector3 v0 = (*b) - (*a);
	Vector3 v1 = (*c)- (*a);
	Vector3 v2 = point - (*a);

	float d00 = v0.dotProduct(v0);
	float d01 = v0.dotProduct(v1);
	float d02 = v0.dotProduct(v2);
	float d11 = v1.dotProduct(v1);
	float d12 = v1.dotProduct(v2);

	float invDenom = 1 / (d00 * d11 - d01 * d01);
	float u = (d11 * d02 - d01*d12) * invDenom;
	float v = (d00 * d12 - d01*d02) * invDenom;

	return (u >=0) && (v>=0) && (u+v<1);
}



