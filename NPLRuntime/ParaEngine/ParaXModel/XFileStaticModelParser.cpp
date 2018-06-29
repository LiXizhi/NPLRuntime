//----------------------------------------------------------------------
// Class:	XFile Static Model Parser
// Authors:	LiXizhi
// Date:	2014.10.3
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "ParaWorldAsset.h"
#include "modelheaders.h"
#include "ParaXModel.h"
#include "ParaXBone.h"
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

ParaEngine::CParaXModel* ParaEngine::XFileStaticModelParser::LoadParaXModelFromScene(XFile::Scene* pScene)
{
	auto pFileData = pScene;
	// static mesh
	CParaXModel* pMesh = new CParaXModel(pFileData->m_header);

	// set to false, to disable animation
	pMesh->animated = true;

	if (pMesh->animated)
	{
		// static animation 0, just in case there are skinned mesh without any animation
		// we need to animate bones just in case external animations are used outside. 
		pMesh->anims = new ModelAnimation[1];
		ModelAnimation& anim = pMesh->anims[0];
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = 0;
		anim.timeEnd = 0;
		anim.animID = 0;

		pMesh->m_objNum.nAnimations = 1;
		pMesh->animBones = true;
	}

	int nVertexCount = 0;
	int nIndexCount = 0;

	// for each mesh
	for (XFile::Mesh* mesh : pFileData->mGlobalMeshes)
	{
		nVertexCount += (int)mesh->mPositions.size();
		nIndexCount += (int)mesh->mPosFaces.size() * 3;
	}

	// geosets
	int nGeosets = (int)pFileData->mGlobalMeshes.size();
	pMesh->showGeosets = new bool[nGeosets];
	for (int i = 0; i < nGeosets; ++i)
		pMesh->showGeosets[i] = true;
	pMesh->geosets.resize(nGeosets);

	ModelVertex* pVertices = new ModelVertex[nVertexCount];
	uint16 *pIndice = new uint16[nIndexCount];
	memset(pVertices, 0, sizeof(ModelVertex)*nVertexCount);
	auto pCurVert = pVertices;
	auto pCurIndice = pIndice;
	for (int nMesh = 0; nMesh < nGeosets; ++nMesh)
	{
		XFile::Mesh* mesh = pFileData->mGlobalMeshes[nMesh];
		int nCount = (int)mesh->mPositions.size();
		PE_ASSERT(nCount == mesh->mNormals.size());
		PE_ASSERT(nCount == mesh->mTexCoords[0].size());
		// prepare indices for the sub mesh
		for (int i = 0; i < nCount; ++i)
		{
			ModelVertex& vert = (*pCurVert);
			pCurVert++;
			vert.pos = mesh->mPositions[i];
			vert.normal = mesh->mNormals[i];
			vert.texcoords = mesh->mTexCoords[0][i];
		}
		// prepare indices for the sub mesh
		nCount = (int)mesh->mPosFaces.size();
		if (nCount > 0)
			memcpy(pCurIndice, &(mesh->mPosFaces[0]), sizeof(XFile::Face) * nCount);
		pCurIndice += nCount * 3;

		// geoset
		memset(&pMesh->geosets[nMesh], 0, sizeof(ModelGeoset));
		// TODO: this is not right for multi mesh
		pMesh->geosets[nMesh].SetVertexStart(0);
		pMesh->geosets[nMesh].icount = nCount * 3;
		pMesh->geosets[nMesh].vcount = nCount;
	}
	pMesh->initVertices(nVertexCount, pVertices);
	pMesh->initIndices(nIndexCount, pIndice);

	// bones
	int nBones = 1;
	pMesh->m_objNum.nBones = nBones;
	pMesh->bones = new ParaEngine::Bone[nBones];
	for (int i = 0; i < nBones; ++i)
	{ // at least one Bone
		pMesh->bones[i] = ParaEngine::Bone();
		pMesh->bones[i].parent = -1;
		pMesh->bones[i].pivot = Vector3(0.f, 0.f, 0.f);
		pMesh->bones[i].bUsePivot = true;
		pMesh->bones[i].SetStaticTransform(Matrix4::IDENTITY);
	}

	pMesh->calcBones();

	// render pass
	int32 nStartIndex = 0;
	int nRenderPass = 0;

	for (XFile::Mesh* mesh : pFileData->mGlobalMeshes)
	{
		int nTotalFaceCount = (int)mesh->mFaceMaterials.size();
		if (nTotalFaceCount == 0)
			continue;
		int32 nMaterialCount = (int32)mesh->mMaterials.size();
		pMesh->passes.resize(pMesh->passes.size() + nMaterialCount);
		int nMaterialIndex = mesh->mFaceMaterials[0];

		// textures
		pMesh->m_objNum.nTextures = nMaterialCount;
		pMesh->textures = new asset_ptr<TextureEntity>[nMaterialCount];

		for (int i = 0; i < nMaterialCount; ++i)
		{
			auto& material = mesh->mMaterials[nMaterialIndex];

			int nFaceCount = 0;
			{ // calculate face count in the material group
				for (int nStartFaceIndex = nStartIndex / 3; nStartFaceIndex < nTotalFaceCount; ++nStartFaceIndex)
				{
					if (mesh->mFaceMaterials[nStartFaceIndex] == nMaterialIndex)
						++nFaceCount;
					else
					{
						nMaterialIndex = mesh->mFaceMaterials[nStartFaceIndex];
						break;
					}
				}

				ModelRenderPass& pass = pMesh->passes[nRenderPass++];
				pass.indexStart = (uint16)(nStartIndex);
				pass.indexCount = nFaceCount * 3;
				nStartIndex = pass.indexStart + pass.indexCount;
				pass.is_rigid_body = true;
				pass.color = -1;
				pass.opacity = -1;
				pass.texanim = -1;
				pass.cull = true;

				pass.tex = i;
				pMesh->specialTextures[i] = -1;
				pMesh->useReplaceTextures[i] = false;

				if (!material.mTextures.empty())
				{
					// Initialize textures from texture file
					const std::string& packedFileNames = material.mTextures[0].mName;
					std::string texFile1, texFile2;
					size_t pos = packedFileNames.find(';');
					if (pos != string::npos) {
						texFile1.assign(packedFileNames.c_str(), pos);
						texFile2.assign(packedFileNames.c_str(), pos + 1, packedFileNames.size() - pos - 1);
					}
					else
						texFile1 = packedFileNames;
					int nFileSize = (int)texFile1.size();
					pMesh->textures[i] = CGlobals::GetAssetManager()->LoadTexture("", texFile1.c_str(), TextureEntity::StaticTexture);

					// check if there is any replaceable texture ID in texFile2, texFile2 will be "[0-9]*"
					// generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
					if (!texFile2.empty() && texFile2[0] >= '0' && texFile2[0] <= '9')
					{
						try
						{
							int nID = -1;
							nID = atoi(texFile2.c_str());
							assert(nID < 32);
							pMesh->specialTextures[i] = nID;
							pMesh->useReplaceTextures[i] = true;
							pMesh->replaceTextures[nID] = CGlobals::GetAssetManager()->LoadTexture("", "Texture/whitedot.png", TextureEntity::StaticTexture);
						}
						catch (...)
						{
						}
					}
				}
			}
		}
	}
	return pMesh;
}