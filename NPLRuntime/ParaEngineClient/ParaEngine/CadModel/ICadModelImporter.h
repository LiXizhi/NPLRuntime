

#pragma once

#include <stdint.h>
#include <vector>

namespace ParaEngine
{
	
	enum CadModelPrimitiveType
	{
		cmpt_triangleList,
		cmpt_triangleStrip,
		cmpt_lineStrip,
	};

	struct CadVec3
	{
		float x;
		float y;
		float z;
	};

	struct VertexData
	{
		CadVec3 position;
		CadVec3 normal;
	};

	struct SubPartInfo
	{
		uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t vertexCount;
		uint32_t primitiveCount;
		uint32_t primitiveType;

	};

	struct CadModelData
	{
		std::vector<VertexData> VertexData;
		std::vector<int32_t> IndexData;
		std::vector<SubPartInfo> SubPartInfo;
		CadVec3 AABBMin;
		CadVec3 AABBMax;
	};

	class ICadModelImporter
	{
	public:
		virtual bool LoadFromFile(const char* fileName,float tessellationFactor,CadModelData** o) = 0;
		virtual void ReleaseModelData(CadModelData* pModelData) = 0;
	};

}
