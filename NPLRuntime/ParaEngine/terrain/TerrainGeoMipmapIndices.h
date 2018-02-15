

#pragma once
 
namespace ParaTerrain
{
	enum GeoMipmapChunkType
{
	gmc_normal = 0,
	gmc_surrounded,
	gmc_topLeft,
	gmc_left,
	gmc_bottomLeft,
	gmc_bottom,
	gmc_bottomRight,
	gmc_right,
	gmc_topRight,
	gmc_top,
	gmc_chunkTypeCount,
};

	class IndexInfo
{
	int m_startPos;
	int m_indexCount;

public:
	IndexInfo():m_startPos(0),m_indexCount(0){};
	int GetStartIndexPos(){return m_startPos;};
	int GetIndexCount(){return m_indexCount;};

	void SetStartIndex_(int startIndex){m_startPos = startIndex;}
	void SetIndexCount_(int count){m_indexCount = count;}
};

	class GeoMipmapIndicesGroup
	{
	public:
		GeoMipmapIndicesGroup();
		~GeoMipmapIndicesGroup();
		bool CreateIndex(int indexOffset,int vertexCountX,int minVertexLodStep,int lodCount);
		IndexInfo* GetChunkIndexInfo(int level,GeoMipmapChunkType type);
		short* GetIndexData(){return m_pIndices;};
		void ReleaseInternalIndices();
		int GetIndexCount(){return m_totalIndexCount;}

	private:
		int m_minVertexLodStep;
		int m_lodCount;
		int m_vertexCountX;
		int m_indexOffset;

		int m_totalIndexCount;
		short* m_pIndices;
		IndexInfo** m_lodIndexInfoGroup;

		int GetChunkIndexCount(int vertexLodStep,GeoMipmapChunkType type);
		int GetLevelIndexCount(int vertexLodStep);
		int FillLevelIndex(int startIdx,int level);
	};
}