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
			XFileStaticModelParser parser(file.getBuffer(), file.getSize());
			pScene = parser.ParseParaXStaticModel();
			if (pScene)
			{
				RemoveUntexturedFaces(*pScene);
				if (!OptimizeRectFaceModelInPlace(*pScene))
				{
					OUTPUT_LOG("warn: model: %s is not coplanar. this will cost 4 times more memory. \n", filename.c_str());
				}
			}
		}
		m_Xfiles[filename] = pScene;
		return pScene;
	}
}


//  check if two faces are coplanar, if so return true, and arrange the two faces in correct index order. 
// co-planer faces are repacked as face1(0,1,3)  face2(1,2,3) 
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
	// store the remaining face index in to the third position. 
	EqualFaces1[2] = 3 - EqualFaces1[1] - EqualFaces1[0];
	EqualFaces2[2] = 3 - EqualFaces2[1] - EqualFaces2[0];

	Face face1_old = face1;
	Face face2_old = face2;
	face1.mIndices[0] = face1_old.mIndices[2];
	face1.mIndices[1] = face1_old.mIndices[0];
	face1.mIndices[2] = face2_old.mIndices[2];
	face2.mIndices[0] = face1_old.mIndices[0];
	face2.mIndices[1] = face2_old.mIndices[2];
	face2.mIndices[2] = face1_old.mIndices[1];
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
