//-----------------------------------------------------------------------------
// Class:	GeoMipmapIndices
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TerrainGeoMipmapIndices.h"
#include <math.h>

namespace ParaTerrain
{
	GeoMipmapIndicesGroup::GeoMipmapIndicesGroup()
		:m_minVertexLodStep(1),m_lodCount(0),m_vertexCountX(0),m_indexOffset(0),
		m_totalIndexCount(0),m_pIndices(NULL),m_lodIndexInfoGroup(NULL)
	{
	}

	GeoMipmapIndicesGroup::~GeoMipmapIndicesGroup()
	{
		if(m_lodIndexInfoGroup != NULL)
		{
			for(int i=0;i<m_lodCount;i++)
			{
				delete [] m_lodIndexInfoGroup[i];
			}
			delete[] m_lodIndexInfoGroup;
			m_lodIndexInfoGroup = NULL;
		}

		if(m_pIndices != NULL)
		{
			delete [] m_pIndices;
			m_pIndices = NULL;
		}
	}

	bool GeoMipmapIndicesGroup::CreateIndex(int indexOffset,int vertexCountX,int minVertexLodStep,int lodCount)
	{
		//todo:verify input parameter


		//release previouse data
		if(m_pIndices != NULL)
		{
			delete [] m_pIndices;
			m_pIndices = NULL;
		}

		if(m_lodIndexInfoGroup != NULL)
		{
			for(int i=0;i<m_lodCount;i++)
			{
				delete [] m_lodIndexInfoGroup[i];
			}
			delete[] m_lodIndexInfoGroup;
		}

		m_minVertexLodStep = minVertexLodStep;
		m_lodCount = lodCount;
		m_vertexCountX = vertexCountX;
		m_indexOffset = indexOffset;

		m_totalIndexCount = 0;
		m_lodIndexInfoGroup = new IndexInfo *[m_lodCount];
		for(int i=0;i<m_lodCount;i++)
		{
			m_lodIndexInfoGroup[i] = new IndexInfo[(int)gmc_chunkTypeCount];
			m_totalIndexCount += GetLevelIndexCount(i);
		}

		m_pIndices = new short[m_totalIndexCount];

		int idx = 0;
		for(int i=0;i<m_lodCount;i++)
		{
			idx = FillLevelIndex(idx,i);
		}

		return true;
	}

	IndexInfo* GeoMipmapIndicesGroup::GetChunkIndexInfo(int level,GeoMipmapChunkType type)
	{
		if(level < m_lodCount && type < gmc_chunkTypeCount)
		{
			return &m_lodIndexInfoGroup[level][(int)type];
		}
		else
		{
			return NULL;
		}
	}

	int GeoMipmapIndicesGroup::GetLevelIndexCount(int level)
	{
		int result = 0;
		int vertexLodStep = m_minVertexLodStep * (level+1);
		for(int i=0;i<gmc_chunkTypeCount;i++)
		{
			result += GetChunkIndexCount(vertexLodStep,(GeoMipmapChunkType)i);
		}
		return result;
	}

	int GeoMipmapIndicesGroup::GetChunkIndexCount(int vertexLodStep,GeoMipmapChunkType type)
	{
		int gridCountX = m_vertexCountX - 1;
		gridCountX = 16;

		switch(type)	
		{
		case gmc_normal:
			{
				int gridWidth = gridCountX / vertexLodStep;
				return gridWidth * gridWidth * 6;
			}
			break;
		case gmc_surrounded:
			{
				int result;
				int temp;
				//center part
				temp = (gridCountX - vertexLodStep * 2) / vertexLodStep;
				result = temp*temp*6;
				//4 strip
				temp = (gridCountX - vertexLodStep * 4) / (vertexLodStep*2);
				result += temp * 4 * 9;
				//4 corner triangle
				result += 16 * 3;
				return result;
			}
			break;
		case gmc_left:
			{
				int result;
				int gridX = gridCountX / vertexLodStep;
				int gridY = (gridCountX - vertexLodStep) / vertexLodStep;
				result = gridX * gridY * 2 * 3;
				result +=  (gridCountX / (vertexLodStep*2)) * 9;
				return result;
			}
		case gmc_right:
			{
				int result;
				int gridX = gridCountX / vertexLodStep;
				int gridY = (gridCountX - vertexLodStep) / vertexLodStep;
				result = gridX * gridY * 2 * 3;
				result +=  (gridCountX / (vertexLodStep*2)) * 9;
				return result;
			}
		case gmc_top:
			{
				int result;
				int gridX = gridCountX / vertexLodStep;
				int gridY = (gridCountX - vertexLodStep) / vertexLodStep;
				result = gridX * gridY * 2 * 3;
				result +=  (gridCountX / (vertexLodStep*2)) * 9;
				return result;
			}
		case gmc_bottom:
			{
				int result;
				int gridX = gridCountX / vertexLodStep;
				int gridY = (gridCountX - vertexLodStep) / vertexLodStep;
				result = gridX * gridY * 2 * 3;
				result +=  (gridCountX / (vertexLodStep*2)) * 9;
				return result;
			}
		case gmc_topLeft:
			{
				int result;
				int gridX = gridCountX - vertexLodStep;
				gridX /= vertexLodStep;
				result = gridX * gridX * 6;

				gridX = (gridCountX - vertexLodStep * 2) / (vertexLodStep*2);
				result += gridX * 2 * 9;
				result += 12;
				return result;
			}
		case gmc_topRight:
			{
				int result;
				int gridX = gridCountX - vertexLodStep;
				gridX /= vertexLodStep;
				result = gridX * gridX * 6;

				gridX = (gridCountX - vertexLodStep * 2) / (vertexLodStep*2);
				result += gridX * 2 * 9;
				result += 12;
				return result;
			}
		case gmc_bottomLeft:
			{
				int result;
				int gridX = gridCountX - vertexLodStep;
				gridX /= vertexLodStep;
				result = gridX * gridX * 6;

				gridX = (gridCountX - vertexLodStep * 2) / (vertexLodStep*2);
				result += gridX * 2 * 9;
				result += 12;
				return result;
			}
		case gmc_bottomRight:
			{
				int result;
				int gridX = gridCountX - vertexLodStep;
				gridX /= vertexLodStep;
				result = gridX * gridX * 6;

				gridX = (gridCountX - vertexLodStep * 2) / (vertexLodStep*2);
				result += gridX * 2 * 9;
				result += 12;
				return result;
			}
		default:
			return 0;
		}
	}

	int GeoMipmapIndicesGroup::FillLevelIndex(int startIdx,int level)
	{	
		int gridCount = m_vertexCountX - 1;
		gridCount = 16;
		int idx = startIdx;
		int lastIdx = startIdx;
		int vertexLodStep = (int)(m_minVertexLodStep* pow(2.0f,(float)level));

		IndexInfo* indexInfoGroup = m_lodIndexInfoGroup[level];
		int vertexCountX = gridCount + 1;
		int vertexCountY = gridCount + 1;

		//===========normal chunk=========
		{
		for (int y = 0;y<gridCount;y+=vertexLodStep)
		{
			for(int x=0;x<gridCount;x+=vertexLodStep)
			{
				m_pIndices[idx++] = y * vertexCountX + x;
				m_pIndices[idx++] = (y+vertexLodStep)*vertexCountX + x + vertexLodStep;
				m_pIndices[idx++] = (y+vertexLodStep)*vertexCountX + x;

				m_pIndices[idx++] = y* vertexCountX + x;
				m_pIndices[idx++] = y*vertexCountX + x + vertexLodStep;
				m_pIndices[idx++] = (y+vertexLodStep)*vertexCountX + x + vertexLodStep;
			}
		}		

		indexInfoGroup[(int)gmc_normal].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_normal].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//=============surrounded chunk=======
		{
		//top strip
		for (int i = 0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i == 0)
			{
				m_pIndices[idx++] = 0;
				m_pIndices[idx++] = vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;

				m_pIndices[idx++] = vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;
			}
			else if(i == gridCount - vertexLodStep*2)
			{
				m_pIndices[idx++] = vertexCountX - vertexLodStep*2-1;
				m_pIndices[idx++] = vertexCountX * (vertexLodStep+1) - vertexLodStep-1;
				m_pIndices[idx++] = vertexCountX * (vertexLodStep+1) - vertexLodStep*2-1;

				m_pIndices[idx++] = vertexCountX - vertexLodStep*2-1;
				m_pIndices[idx++] = vertexCountX - 1;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) - vertexLodStep-1;
			}
			else
			{
				int nextLine = vertexCountX*(vertexLodStep) + i;

				m_pIndices[idx++] = i;
				m_pIndices[idx++] = nextLine + vertexLodStep;
				m_pIndices[idx++] = nextLine;

				m_pIndices[idx++] = i;
				m_pIndices[idx++] = i + vertexLodStep*2;
				m_pIndices[idx++] = nextLine + vertexLodStep;

				m_pIndices[idx++] = i + vertexLodStep*2;
				m_pIndices[idx++] = nextLine + vertexLodStep*2;
				m_pIndices[idx++] = nextLine + vertexLodStep;
			}
		}

		//right strip
		for (int i = 0; i<gridCount;i+=vertexLodStep*2)
		{
			if(i == 0)
			{
				m_pIndices[idx++] = vertexCountX - 1;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) - vertexLodStep-1;

				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) - vertexLodStep-1;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2+1) - 1 - vertexLodStep;
			}
			else if(i == (gridCount-vertexLodStep*2))
			{
				m_pIndices[idx++] = vertexCountX * (i+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * vertexCountX - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
			}
			else 
			{
				m_pIndices[idx++] = vertexCountX * (i+1) - 1-vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1 - vertexLodStep;
			}
		}

		//bottom strip
		int tempIdx = (vertexCountX-vertexLodStep - 1)*vertexCountX;
		for(int i= 0;i<gridCount; i+=vertexLodStep*2)
		{
			if(i==0)
			{
				m_pIndices[idx++] = tempIdx + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + 2*vertexLodStep;

				m_pIndices[idx++] = tempIdx + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexLodStep*2;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + 2*vertexLodStep;
			}
			else if(i == gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = tempIdx + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i;

				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*(vertexLodStep+1) - 1;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i;
			}
			else
			{
				m_pIndices[idx++] = tempIdx + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + (vertexCountX*vertexLodStep) + i;

				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i + vertexLodStep*2;

				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep*2;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i + vertexLodStep*2;
			}
		}

		//left strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i == 0)
			{
				m_pIndices[idx++] = 0;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2);

				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2);
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2) + vertexLodStep;
			}
			else if(i == gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
			}
			else
			{
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep) + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);

				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep*2);
				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep*2) + vertexLodStep;
			}
		}

		//mid
		for (int i = vertexLodStep;i<gridCount-vertexLodStep;i+=vertexLodStep)
		{
			for (int j=vertexLodStep;j<gridCount-vertexLodStep;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX *i + j;
				m_pIndices[idx++] = vertexCountX *i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep) + j + vertexLodStep;
								    
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_surrounded].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_surrounded].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//==============top left chunck=========
		{
		//top strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i==0)
			{
				m_pIndices[idx++] = 0;
				m_pIndices[idx++] = vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;

				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;
				m_pIndices[idx++] = vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep * 2;
			}
			else
			{
				m_pIndices[idx++] = i;
				m_pIndices[idx++] = i + vertexLodStep * 2;
				m_pIndices[idx++] = (vertexCountX*vertexLodStep) + i + vertexLodStep;

				m_pIndices[idx++] = i;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i;

				m_pIndices[idx++] = i + vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
			}
		}

		//left trip
		for (int i=0;i<gridCount;i += vertexLodStep*2)
		{
			if(i==0)
			{
				m_pIndices[idx++] = 0;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep*2);

				m_pIndices[idx++] = vertexCountX *(vertexLodStep*2);
				m_pIndices[idx++] = vertexCountX * vertexLodStep + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (vertexLodStep*2) + vertexLodStep;
			}
			else
			{
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);

				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2) + vertexLodStep;
			}
		}

		//mid
		for (int i=vertexLodStep;i<gridCount;i+=vertexLodStep)
		{
			for (int j=vertexLodStep;j<gridCount;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX*i + j;
				m_pIndices[idx++] = vertexCountX*i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX*i + j;
				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep)+j+vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_topLeft].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_topLeft].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============left chunk==============
		{
		//left strip
		for (int i=0;i<gridCount - vertexLodStep;i+= vertexLodStep*2)
		{
			m_pIndices[idx++] = vertexCountX * i;
			m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
											   
			m_pIndices[idx++] = vertexCountX * i;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
											   
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2) + vertexLodStep;
		}

		//center
		for (int i=0;i<gridCount;i+=vertexLodStep)
		{
			for (int j=vertexLodStep;j<gridCount;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
													 
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_left].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_left].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============left bottom=============
		{
		//left strip
		for (int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i == gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
												   	 
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
			}									   	 
			else 								   	 
			{									   	 
				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);

				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2);
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2) + vertexLodStep;
			}
		}

		//bottom strip
		int tempIdx = vertexCountX * (vertexCountX-vertexLodStep-1);
		for (int i =0;i<gridCount;i+= vertexLodStep*2)
		{
			if(i == 0)
			{
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + vertexLodStep*2;

				m_pIndices[idx++] = tempIdx + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexLodStep * 2;
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + vertexLodStep * 2;
			}
			else
			{
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;
				m_pIndices[idx++] = tempIdx + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;

				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i + vertexLodStep * 2;

				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep * 2;
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i + vertexLodStep * 2;
			}
		}

		for (int i = 0;i<gridCount-vertexLodStep;i+=vertexLodStep)
		{
			for (int j=vertexLodStep;j<gridCount;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i + vertexLodStep) + j;
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
			}
		}
		indexInfoGroup[(int)gmc_bottomLeft].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_bottomLeft].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============bottom tile===============
		{
		//bottom strip
		int tempIdx = vertexCountX * (vertexCountX-vertexLodStep - 1);
		for (int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;
			m_pIndices[idx++] = tempIdx + i;
			m_pIndices[idx++] = tempIdx + i + vertexLodStep;

			m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;
			m_pIndices[idx++] = tempIdx + i + vertexLodStep;
			m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i + vertexLodStep * 2;

			m_pIndices[idx++] = tempIdx + i + vertexLodStep;
			m_pIndices[idx++] = tempIdx + i + vertexLodStep * 2;
			m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i + vertexLodStep * 2;
		}

		for (int i=0;i<gridCount-vertexLodStep;i+=vertexLodStep)
		{
			for(int j=0;j<gridCount;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_bottom].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_bottom].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============right bottom ==============
		{
		//right strip
		for (int i=0;i<gridCount;i+= vertexLodStep*2)
		{
			if(i == gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = vertexCountX * (i+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - vertexLodStep -1;
												   	 
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX *  vertexCountX -1;
			}									   	 
			else								   	 
			{									   	 
				m_pIndices[idx++] = vertexCountX * (i+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep) - 1 - vertexLodStep;
												   	 
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep*2) - 1;
												   	 
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep*2) - 1;
				m_pIndices[idx++] = vertexCountX * (i+1+vertexLodStep*2) - 1 - vertexLodStep;
			}
		}

		//bottom strip
		int tempIdx = vertexCountX * (vertexCountX-vertexLodStep -1);
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i==gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = tempIdx + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;

				m_pIndices[idx++] = tempIdx + vertexCountX * vertexLodStep + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * vertexCountX - 1;
			}
			else
			{
				m_pIndices[idx++] = tempIdx + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i;

				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i + vertexLodStep*2;

				m_pIndices[idx++] = tempIdx + vertexCountX*vertexLodStep + i + vertexLodStep*2;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep;
				m_pIndices[idx++] = tempIdx + i + vertexLodStep*2;
			}
		}

		for(int i=0;i<gridCount-vertexLodStep;i+=vertexLodStep)
		{
			for(int j=0;j<gridCount-vertexLodStep;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
			}
		}

		indexInfoGroup[(int)gmc_bottomRight].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_bottomRight].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============right chunck=================
		{
		//right strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			m_pIndices[idx++] = vertexCountX * (i+1) - 1 - vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+1) - 1;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
												 
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+1) - 1;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
												 
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
			m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1 - vertexLodStep;
		}

		for(int i=0;i<gridCount;i+=vertexLodStep)
		{
			for(int j=0;j<gridCount-vertexLodStep;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
												   
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_right].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_right].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//===============right top ====================
		{
		//top strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i == gridCount-vertexLodStep*2)
			{
				m_pIndices[idx++] = i;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) - 1 - vertexLodStep*2;
								    
				m_pIndices[idx++] = i;
				m_pIndices[idx++] = i + vertexLodStep*2;
				m_pIndices[idx++] = vertexCountX*(vertexLodStep+1) -1 - vertexLodStep;
			}
			else
			{
				m_pIndices[idx++] = i;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i;

				m_pIndices[idx++] = i;
				m_pIndices[idx++] = i + vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;

				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
				m_pIndices[idx++] = i + vertexLodStep * 2;
				m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep * 2;
			}
		}

		//right strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			if(i==0)
			{
				m_pIndices[idx++] = vertexCountX - 1;
				m_pIndices[idx++] = vertexCountX * (vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX * (vertexLodStep+1) - 1 - vertexLodStep;

				m_pIndices[idx++] =vertexCountX * (vertexLodStep+1) - 1 - vertexLodStep;
				m_pIndices[idx++] =vertexCountX * (vertexLodStep*2+1) - 1;
				m_pIndices[idx++] =vertexCountX * (vertexLodStep*2+1) - 1 - vertexLodStep;
			}
			else
			{
				m_pIndices[idx++] = vertexCountX *(i+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX *(i+1) - 1;
				m_pIndices[idx++] = vertexCountX *(i+vertexLodStep+1) - 1 - vertexLodStep;

				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 -vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
												   
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep+1) - 1 - vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep*2+1) - 1 - vertexLodStep;
			}
		}
		
		//mid
		for(int i=vertexLodStep;i<gridCount;i+=vertexLodStep)
		{
			for(int j=0;j<gridCount-vertexLodStep;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX * i + j;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX * (i+vertexLodStep) + j;
			}
		}

		indexInfoGroup[(int)gmc_topRight].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_topRight].SetIndexCount_(idx - lastIdx);
		lastIdx = idx;
		}

		//================top chunck======================
		{
		//top strip
		for(int i=0;i<gridCount;i+=vertexLodStep*2)
		{
			m_pIndices[idx++] = i;
			m_pIndices[idx++] = i + vertexLodStep * 2;
			m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
								
			m_pIndices[idx++] = i;
			m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
			m_pIndices[idx++] = vertexCountX*vertexLodStep + i;
								
			m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep;
			m_pIndices[idx++] = i + vertexLodStep * 2;
			m_pIndices[idx++] = vertexCountX*vertexLodStep + i + vertexLodStep * 2;
		}

		for(int i=vertexLodStep;i<gridCount;i+=vertexLodStep)
		{
			for(int j=0;j<gridCount;j+=vertexLodStep)
			{
				m_pIndices[idx++] = vertexCountX*i + j;
				m_pIndices[idx++] = vertexCountX*i + j + vertexLodStep;
				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep) + j + vertexLodStep;

				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep) + j;
				m_pIndices[idx++] = vertexCountX*i + j;
				m_pIndices[idx++] = vertexCountX*(i+vertexLodStep) + j + vertexLodStep;
			}
		}

		indexInfoGroup[(int)gmc_top].SetStartIndex_(lastIdx);
		indexInfoGroup[(int)gmc_top].SetIndexCount_(idx - lastIdx);
		}
		return idx;
	}

	void GeoMipmapIndicesGroup::ReleaseInternalIndices()
	{
		if(m_pIndices != NULL)
		{
			delete [] m_pIndices;
			m_pIndices = NULL;
		}
	}
}