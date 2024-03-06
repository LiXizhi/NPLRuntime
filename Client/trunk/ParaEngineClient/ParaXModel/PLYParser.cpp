//-----------------------------------------------------------------------------
// Class: PLYParser
// Authors:	LiXizhi
// Emails: LiXizhi@yeah.net
// Date: 2024.3.6	
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaVoxelModel.h"
#include "PLYParser.h"


using namespace ParaEngine;

PLYParser::PLYParser()
{
}

PLYParser::PLYParser(const std::string& filename)
{
	m_sFilename = filename;
}

PLYParser::~PLYParser()
{
}

const std::string& PLYParser::GetFilename() const
{
	return m_sFilename;
}

void PLYParser::SetFilename(const std::string& val)
{
	m_sFilename = val;
}

CParaXModel* PLYParser::ParseParaXModel()
{
	CParaFile myFile(m_sFilename.c_str());
	if (!myFile.isEof())
	{
		return ParseParaXModel(myFile.getBuffer(), myFile.getSize());
	}
	else
	{
		OUTPUT_LOG("warn: unable to open file %s\n", m_sFilename.c_str());
		return NULL;
	}
}

// parse ply point cloud file
CParaXModel* PLYParser::ParseParaXModel(const char* buffer, int nSize, const char* pHint)
{
	struct PlyVertex
	{
		int32 x, y, z;
		DWORD color;
	};
	std::vector<PlyVertex> vertices;
	// read the header
	const char* p = buffer;
	if (strncmp(p, "ply", 3) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p += 3;
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "format ascii", 12) != 0)
	{
		OUTPUT_LOG("error: only ascii ply file format is supported\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	while (strncmp(p, "comment", 7) == 0)
	{
		p = strchr(p, '\n') + 1;
	}
	if (strncmp(p, "element vertex", 14) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p += 14;
	int nVertices = atoi(p);
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property float x", 16) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property float y", 16) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property float z", 16) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property uchar red", 18) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property uchar green", 20) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "property uchar blue", 19) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	if (strncmp(p, "end_header", 10) != 0)
	{
		OUTPUT_LOG("error: invalid ply file format\n");
		return NULL;
	}
	p = strchr(p, '\n') + 1;
	// read vertices
	int32 minX = 0, minY = 0, minZ = 0, maxX = 1, maxY = 1, maxZ = 1;

	for (int i = 0; i < nVertices; i++)
	{
		PlyVertex v;
		v.x = atoi(p);
		p = strchr(p, ' ') + 1;
		v.y = atoi(p);
		p = strchr(p, ' ') + 1;
		v.z = atoi(p);
		p = strchr(p, ' ') + 1;
		DWORD r = atoi(p);
		p = strchr(p, ' ') + 1;
		DWORD g = atoi(p);
		p = strchr(p, ' ') + 1;
		DWORD b = atoi(p);
		v.color = (r << 16) | (g << 8) | b;
		p = strchr(p, '\n') + 1;
		vertices.push_back(v);

		if (i == 0) {
			minX = maxX = v.x;
			minY = maxY = v.y;
			minZ = maxZ = v.z;
		}
		else
		{
			minX = min(minX, v.x);
			minY = min(minY, v.y);
			minZ = min(minZ, v.z);
			maxX = max(maxX, v.x);
			maxY = max(maxY, v.y);
			maxZ = max(maxZ, v.z);
		}
	}

	int32 dx = maxX - minX;
	int32 dy = maxY - minY;
	int32 dz = maxZ - minZ;
	int32 maxD = max(max(dx, dy), dz) + 1;
	int32 nLevel = maxD;
	int nDepth = 0;
	while (nLevel > 1)
	{
		nLevel >>= 1;
		nDepth++;
	}
	nLevel = (int32)pow(2, nDepth);
	if (nLevel < maxD)
	{
		nLevel *= 2;
	}
	dx = (int)((nLevel - dx) / 2);
	dy = (int)((nLevel - dy) / 2);

	// create model
	ParaXHeaderDef m_xheader;
	CParaXModel* pModel = new CParaXModel(m_xheader);
	pModel->SetBmaxModel();
	// always a cube
	pModel->m_header.minExtent = Vector3(-0.5f, 0, -0.5f);
	pModel->m_header.maxExtent = Vector3(0.5f, 1, 0.5f);
	pModel->SetRenderMethod(CParaXModel::BMAX_MODEL);

	ParaVoxelModel* pVoxelModel = pModel->CreateGetVoxelModel();
	if (pVoxelModel)
	{
		for (int i = 0; i < nVertices; i++)
		{
			auto& v = vertices[i];
			// swap y,z, since ply file assumes z up, but we assume y up
			pVoxelModel->SetBlock(v.x - minX + dx, v.z - minZ, v.y - minY + dy, nLevel, v.color);
		}
	}
	return pModel;
}