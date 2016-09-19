//-----------------------------------------------------------------------------
// Class:	BlockModel Manager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.11.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockModel.h"
#include "ParaXModel/XFileHelper.h"
#include "ParaXModel/XFileStaticModelParser.h"
#include "ParaXModel/FBXParser.h"
#include "BlockModelManager.h"

using namespace ParaEngine;
using namespace XFile;

ParaEngine::BlockModelManager::BlockModelManager()
{

}

ParaEngine::BlockModelManager::~BlockModelManager()
{
	Cleanup();
}

BlockModelManager& ParaEngine::BlockModelManager::GetInstance()
{
	static BlockModelManager s_singleton;
	return s_singleton;
}

void ParaEngine::BlockModelManager::Cleanup()
{
	for (auto& item : m_Xfiles)
	{
		SAFE_DELETE(item.second);
	}
	m_Xfiles.clear();
}

XFile::Scene* ParaEngine::BlockModelManager::GetXFile(const std::string& filename)
{
	auto it = m_Xfiles.find(filename);
	if (it != m_Xfiles.end())
	{
		return it->second;
	}
	else
	{
		using namespace XFile;
		Scene* pScene = NULL;
		CParaFile file;
		file.OpenAssetFile(filename.c_str());
		if (!file.isEof())
		{
			std::string sFileExt = CParaFile::GetFileExtension(filename);
			if (sFileExt == "x")
			{
				XFileStaticModelParser parser(file.getBuffer(), file.getSize());
				pScene = parser.ParseParaXStaticModel();
			}
#ifdef SUPPORT_FBX_MODEL_FILE
			else if (sFileExt == "fbx" || sFileExt == "FBX")
			{
				FBXParser parser;
				pScene = parser.ParseFBXFile(file.getBuffer(), file.getSize());
			}
#endif
			if (pScene)
			{
				RemoveUntexturedFaces(*pScene);
				if (!OptimizeRectFaceModelInPlace(*pScene))
				{
					OUTPUT_LOG("warn: model: %s is not coplanar. this will cost 4 times more memory. \n", filename.c_str());
				}
			}
			else
			{
				OUTPUT_LOG("warn: unknown block model file extension: %s \n", filename.c_str());
			}
		}
		m_Xfiles[filename] = pScene;
		return pScene;
	}
}

void ShuffleFaceIndex(Face& face, int nOffset=1)
{
	Face face_old = face;
	for (int i = 0; i < 3;++i)
	{
		face.mIndices[i] = face_old.mIndices[(i + nOffset) % 3];
	}
}
//  check if two faces are coplanar, if so return true, and arrange the two faces in correct index order. 
// co-planer faces are repacked as face1(0,1,2)  face2(2,0,3) 
bool AreFacesCoplaner(Face& face1, Face & face2, Mesh& mesh)
{
	auto& mNormals = mesh.mNormals;
	auto& mPositions = mesh.mPositions;
	auto& mPosFaces = mesh.mPosFaces;
	for (int i = 0; i < 3;i++)
	{
		if (mNormals[face1.mIndices[i]].squaredDistance(mNormals[face2.mIndices[i]]) > 0.001f)
			return false;
	}
	int EqualFaces1[3] = { 0, 0, 0, };
	int EqualFaces2[3] = { 0, 0, 0, };

	int nEqualCount = 0;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (mPositions[face1.mIndices[i]].squaredDistance(mPositions[face2.mIndices[j]]) < 0.001f)
			{
				EqualFaces1[nEqualCount] = i;
				EqualFaces2[nEqualCount] = j;
				nEqualCount++;
				break;
			}
		}
	}
	if (nEqualCount < 2)
		return false;
	// index of the non-shared vertex in each face. 
	int nFace1NonSharedIndex = face1.mIndices[3 - EqualFaces1[1] - EqualFaces1[0]];
	int nFace2NonSharedIndex = face2.mIndices[3 - EqualFaces2[1] - EqualFaces2[0]];

	// make face2 to reuse index of face1
	face2.mIndices[EqualFaces2[0]] = face1.mIndices[EqualFaces1[0]];
	face2.mIndices[EqualFaces2[1]] = face1.mIndices[EqualFaces1[1]];

	// co-planer faces are repacked as face1(0,1,2)  face2(2,0,3) 
	while (face1.mIndices[1] != nFace1NonSharedIndex)
	{
		ShuffleFaceIndex(face1, 1);
	}
	
	while (face2.mIndices[2] != nFace2NonSharedIndex)
	{
		ShuffleFaceIndex(face2, 1);
	}
	return true;
}

bool IsFaceCoplaner(Face& face1, Mesh& mesh)
{
	auto& mNormals = mesh.mNormals;
	return (mNormals[face1.mIndices[0]].squaredDistance(mNormals[face1.mIndices[1]])< 0.001f &&
		mNormals[face1.mIndices[1]].squaredDistance(mNormals[face1.mIndices[2]]) < 0.001f);
}

void ParaEngine::BlockModelManager::RemoveUntexturedFaces(XFile::Scene& scene)
{
	for (Mesh* mesh : scene.mGlobalMeshes)
	{
		int nFaceIndex = 0;
		int nFaceCount = (int)mesh->mPosFaces.size();

		for (int i = 0; i < nFaceCount; ++i)
		{
			Material& material = mesh->mMaterials[mesh->mFaceMaterials[i]];
			if (mesh->mFaceMaterials[i] > 0)
			{
				if (material.mTextures.empty() || material.mTextures[0].mName.empty())
				{
					mesh->mPosFaces.resize(i + 1);
					break;
				}
			}
		}
	}
}

// non-rect face model will cost 4 times more memory to hold the triangle data. 
// and draws 2 times more triangles than rect face models, since the even number of triangles is always made invisible. 
bool ParaEngine::BlockModelManager::OptimizeRectFaceModelInPlace(XFile::Scene& scene)
{
	bool bHasNonCoplanerFace = false;
	// try to optimize model 
	for (Mesh* mesh : scene.mGlobalMeshes)
	{
		int nFaceIndex = 0;
		int nFaceCount = (int) mesh->mPosFaces.size();

		auto mPosFaces = mesh->mPosFaces;
		
		for (int i = 0; i < (nFaceCount-1); i+=2)
		{
			Face& face1 = mesh->mPosFaces[i];
			
			bool bFoundCoplanarFace = false;
			if (IsFaceCoplaner(face1, *mesh))
			{
				// find coplanar face sharing two vertices
				for (int j = i + 1; j < nFaceCount; ++j)
				{
					Face& face2 = mesh->mPosFaces[j];
					if (AreFacesCoplaner(face1, face2, *mesh))
					{
						Face tmp = mesh->mPosFaces[i + 1];
						mesh->mPosFaces[i + 1] = mesh->mPosFaces[j];
						mesh->mPosFaces[j] = tmp;
						bFoundCoplanarFace = true;
						break;
					}
				}
			}
			
			if (!bFoundCoplanarFace)
			{
				bHasNonCoplanerFace = true;
				mesh->mPosFaces = mPosFaces;
				break;
			}
		}
	}
	bool bIsRectFaceModel = (!bHasNonCoplanerFace);
	scene.m_header.nModelFormat = bIsRectFaceModel ? 1 : 0;
	return bIsRectFaceModel;
}
