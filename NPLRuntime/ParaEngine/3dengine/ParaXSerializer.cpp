//-----------------------------------------------------------------------------
// Class:	CParaXSerializer
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.1.5
// Note: serialization functions for ParaX model.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/particle.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/ParaVoxelModel.h"
#include "ParaXModel/XFileCharModelExporter.h"
#include "ZipArchive.h"

#include "ParaXSerializer.h"
#include "ParaXModel/XFileCharModelParser.h"

namespace ParaEngine
{
	extern float frand();
	SerializerOptions CParaXSerializer::g_pDefaultOption;

	void CParaXSerializer::ExportParaXMesh(const string& filePath, CParaXModel* pMesh)
	{
		XFileCharModelExporter::Export(filePath, pMesh);
	}
}

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
/// CParaXSerializer													//
//////////////////////////////////////////////////////////////////////////

CParaXSerializer::CParaXSerializer(void)
	:m_pRaw(NULL)
{
}

CParaXSerializer::~CParaXSerializer(void)
{
}

const std::string& CParaXSerializer::GetFilename() const
{
	return m_sFilename;
}

void CParaXSerializer::SetFilename(std::string val)
{
	m_sFilename = val;
}

void* CParaXSerializer::LoadParaXMesh(CParaFile& f)
{
	void* pMesh = NULL;
	const char* pBuffer = f.getBuffer();
	int32 nSize = f.getSize();
	std::string uncompressedData;
	if (ParaEngine::IsZipData(pBuffer, nSize))
	{
		if (ParaEngine::GetFirstFileData(pBuffer, nSize, uncompressedData))
		{
			pBuffer = uncompressedData.c_str();
			nSize = (int32)uncompressedData.size();
		}
	}

	try
	{
		XFileCharModelParser p(pBuffer, nSize);
		p.SetFilename(GetFilename());
		pMesh = p.ParseParaXModel();
	}
	catch (runtime_error* e)
	{
		OUTPUT_LOG("warn: LoadParaXMesh error:%s\n", e->what());
	}
	return pMesh;
}

bool CParaXSerializer::SaveParaXMesh(const string& filename, const CParaXModel& xmesh, SerializerOptions* pOptions)
{
	return SUCCEEDED(XFileCharModelExporter::Export(filename, const_cast<CParaXModel*>(&xmesh)));
}
