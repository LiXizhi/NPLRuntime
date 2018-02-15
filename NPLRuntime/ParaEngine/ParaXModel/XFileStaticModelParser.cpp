//----------------------------------------------------------------------
// Class:	XFile Static Model Parser
// Authors:	LiXizhi
// Date:	2014.10.3
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "modelheaders.h"
#include "XFileHelper.h"

using namespace ParaEngine;
using namespace ParaEngine::XFile;

#include "XFileStaticModelParser.h"

ParaEngine::XFileStaticModelParser::XFileStaticModelParser(const char* pBuffer, int32 nSize)
	:XFileParser(pBuffer, nSize)
{

}

ParaEngine::XFileStaticModelParser::~XFileStaticModelParser()
{

}

XFile::Scene* ParaEngine::XFileStaticModelParser::ParseParaXStaticModel()
{
	SAFE_DELETE(mScene);
	mScene = new Scene;
	
	bool running = true;
	while (running)
	{
		// read name of next object
		std::string objectName = GetNextToken();
		if (objectName.length() == 0)
			break;

		// parse specific object
		if (objectName == "template")
			ParseDataObjectTemplate();
		else if (objectName == "Frame")
			ParseDataObjectFrame(NULL);
		else if (objectName == "Mesh")
		{
			// some meshes have no frames at all
			Mesh* mesh = new Mesh;
			ParseDataObjectMesh(mesh);
			mScene->mGlobalMeshes.push_back(mesh);
		}
		else if (objectName == "ParaXHeader")
		{
			ParseDataObjectParaXHeader(mScene->m_header);
		}
		else if (objectName == "Material")
		{
			// Material outside of a mesh or node
			Material material;
			ParseDataObjectMaterial(&material);
			mScene->mGlobalMaterials.push_back(material);
		}
		else if (objectName == "ParaXRefSection")
		{
			ParseXRefSection();
		}
		else if (objectName == "}")
		{
			// whatever?
			// OUTPUT_DEBUG("} found in dataObject");
		}
		else
		{
			// unknown format
			// OUTPUT_DEBUG("Unknown data object in animation of .x file");
			ParseUnknownDataObject();
		}
	}

	// filter the imported hierarchy for some degenerated cases
	if (mScene->mRootNode) {
		FilterHierarchy(mScene->mRootNode);
	}
	return GetImportedData(true);
}

bool ParaEngine::XFileStaticModelParser::ParseXRefSection()
{
	if (mIsBinaryFormat)
	{
		// not supported for binary yet
		ParseUnknownDataObject();
	}
	else
	{
		readHeadOfDataObject();
		uint32 nRefCount = ReadInt();
		mScene->m_XRefObjects.resize(nRefCount);
		for (uint32 i = 0; i < nRefCount; ++i)
		{
			ParseDataObjectXRefObject(mScene->m_XRefObjects[i]);
		}
		ReadToEndOfDataObject();
	}
	return true;
}

bool ParaEngine::XFileStaticModelParser::ParseDataObjectXRefObject(ParaXRefObject& xref)
{
	GetNextTokenAsString(xref.m_filename);
	xref.m_data.type = ReadInt(); 
	float mat[16];
	for (int i = 0; i < 16; ++i)
	{
		mat[i] = ReadFloat();
	}
	memcpy(xref.m_data.localTransform.m, mat, sizeof(float) * 16);
	xref.m_data.origin = ReadVector3();
	xref.m_data.animID = ReadInt();
	xref.m_data.color = ReadInt();
	xref.m_data.reserved0 = ReadInt();
	xref.m_data.reserved1 = ReadFloat();
	xref.m_data.reserved2 = ReadFloat();
	xref.m_data.reserved3 = ReadFloat();
	TestForSeparator();
	return true;
}
