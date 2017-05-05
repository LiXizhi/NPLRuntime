//-----------------------------------------------------------------------------
// Class:	FBX importer
// Authors:	LiPeng, LiXizhi
// Emails:	
// Company: ParaEngine
// Date:	2015.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if defined(WIN32) && (defined(USE_DIRECTX_RENDERER)  || defined(USE_OPENGL_RENDERER))
#include "XFileHelper.h"
#include "FBXParser.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "StringHelper.h"
#include "TextureEntity.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"
#include <math.h>

#include "assimp/scene.h"

using namespace ParaEngine;
using namespace ParaEngine::XFile;

static const char* g_sDefaultTexture = "Texture/whitedot.png";

namespace ParaEngine
{
	inline Vector3 ConvertFBXVector3D(aiVector3D fbxVector3D)
	{
		return Vector3(fbxVector3D.x, fbxVector3D.y, fbxVector3D.z);
	}

	inline Quaternion ConvertFBXQuaternion(const aiQuaternion& fbxQuaternion)
	{
		return Quaternion(-fbxQuaternion.x, -fbxQuaternion.y, -fbxQuaternion.z, fbxQuaternion.w);
	}
}

FBXParser::FBXParser()
	:m_pScene(NULL), m_nMaterialIndex(0), m_nRootNodeIndex(0), m_bUsedVertexColor(true), m_bHasSkinnedMesh(false), m_unique_id(0)
{
}

FBXParser::FBXParser(const string& filename)
	: m_sFilename(filename),
	m_pScene(NULL), m_nMaterialIndex(0), m_nRootNodeIndex(0), m_bUsedVertexColor(true), m_bHasSkinnedMesh(false), m_unique_id(0)
{
}

FBXParser::~FBXParser()
{
}

XFile::Scene* ParaEngine::FBXParser::ParseFBXFile(const char* buffer, int nSize)
{
	Assimp::Importer importer;
	Reset();
	const aiScene* pFbxScene = importer.ReadFileFromMemory(buffer, nSize, aiProcess_Triangulate | aiProcess_GenSmoothNormals, "fbx");
	if (pFbxScene) {
		if (pFbxScene->HasMeshes())
		{
			m_pScene = new Scene;
			int numMeshes = pFbxScene->mNumMeshes;
			for (int i = 0; i < numMeshes; i++)
			{
				Mesh* mesh = new Mesh;
				aiMesh* test = pFbxScene->mMeshes[i];
				int numVertices = (pFbxScene->mMeshes[i])->mNumVertices;
				ProcessStaticFBXMesh(pFbxScene->mMeshes[i], mesh);
				m_pScene->mGlobalMeshes.push_back(mesh);

				if (i == 0)
				{
					Vector3 vMin, vMax;
					ParaComputeBoundingBox((Vector3*)(&mesh->mPositions[0]), mesh->mPositions.size(), sizeof(Vector3), &vMin, &vMax);
					m_pScene->m_header.minExtent = vMin;
					m_pScene->m_header.maxExtent = vMax;
				}
			}
			// OUTPUT_LOG("Successful parsing '%s' numMeshes:%d numMaterials:%d\n", m_sFilename.c_str(), numMeshes, pFbxScene->mNumMaterials);
		}
		if (pFbxScene->HasMaterials())
		{
			int materials_num = pFbxScene->mNumMaterials;
			for (int i = 0; i < materials_num; i++)
			{
				ProcessStaticFBXMaterial(pFbxScene, i);
			}
		}
	}
	else {
		OUTPUT_LOG("Error parsing '%s': '%s'\n", m_sFilename.c_str(), importer.GetErrorString());
	}
	return m_pScene;
}

void ParaEngine::FBXParser::Reset()
{
	ResetAABB();
	m_bHasSkinnedMesh = false;
	m_unique_id = 0;
}

void ParaEngine::FBXParser::SetAnimSplitterFilename()
{
	m_sAnimSplitterFilename = std::string(m_sFilename.c_str(), m_sFilename.size() - 3) + "xml";
}

CParaXModel* FBXParser::ParseParaXModel(const char* buffer, int nSize)
{
	CParaXModel* pMesh = NULL;
	Assimp::Importer importer;
	Reset();
	SetAnimSplitterFilename();
	// aiProcess_MakeLeftHanded | 
	const aiScene* pFbxScene = importer.ReadFileFromMemory(buffer, nSize, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs, "fbx");
	if (pFbxScene) {
		ParaXHeaderDef m_xheader;
		m_xheader.IsAnimated = pFbxScene->HasAnimations() ? 1 : 0;
		pMesh = new CParaXModel(m_xheader);

		if (pFbxScene->HasMaterials())
		{
			int materials_num = pFbxScene->mNumMaterials;
			for (int i = 0; i < materials_num; i++)
			{
				ProcessFBXMaterial(pFbxScene, i, pMesh);
			}
		}
		// get root node
		//m_nRootNodeIndex = CreateGetBoneIndex(pFbxScene->mRootNode->mName.C_Str());

		// must be called before ProcessFBXBoneNodes
		if (pFbxScene->HasAnimations())
		{
			int animations_num = pFbxScene->mNumAnimations;
			for (int i = 0; i < animations_num; i++)
			{
				ProcessFBXAnimation(pFbxScene, i, pMesh);
			}
		}

		// building the parent-child relationship of the bones, and add meshes if any;
		ProcessFBXBoneNodes(pFbxScene, pFbxScene->mRootNode, -1, pMesh);

		// MakeAxisY_UP();

		FillParaXModelData(pMesh, pFbxScene);

		PostProcessParaXModelData(pMesh);

#ifdef _DEBUG
		//PrintDebug(pFbxScene);
#endif
	}
	else {
		OUTPUT_LOG("Error parsing '%s': '%s'\n", m_sFilename.c_str(), importer.GetErrorString());
	}
	return pMesh;
}

void FBXParser::AddDefaultColors(CParaXModel *pMesh)
{
	pMesh->m_objNum.nColors = 1;

	pMesh->colors = new ModelColor[1];
	ModelColor &color = pMesh->colors[0];
	color.color.used = false;
	color.color.type = 0;
	color.color.seq = 0;
	color.color.globals = NULL;
	color.color.ranges.push_back(AnimRange(0, 0));
	color.color.times.push_back(0);
	color.color.data.push_back(Vector3(0, 0, 0));
	color.color.in.push_back(Vector3(0, 0, 0));
	color.color.out.push_back(Vector3(0, 0, 0));

	color.opacity.used = false;
	color.opacity.type = 0;
	color.opacity.seq = 0;
	color.opacity.globals = NULL;
	color.opacity.ranges.push_back(AnimRange(0, 0));
	color.opacity.times.push_back(0);
	color.opacity.data.push_back(-1.0f);
	color.opacity.in.push_back(0.0f);
	color.opacity.out.push_back(0.0f);
}

void FBXParser::AddDefaultTransparency(CParaXModel *pMesh)
{
	pMesh->transparency = new ModelTransparency[1];
	ModelTransparency &tran = pMesh->transparency[0];

	tran.trans.used = false;
	tran.trans.type = 0;
	tran.trans.seq = 0;
	tran.trans.globals = NULL;
	tran.trans.ranges.push_back(AnimRange(0, 0));
	tran.trans.times.push_back(0);
	tran.trans.data.push_back(-1.0f);
	tran.trans.in.push_back(0.0f);
	tran.trans.out.push_back(0.0f);
}

void FBXParser::PostProcessParaXModelData(CParaXModel *pMesh)
{
	// we need to collapse all bone transform. 
	if (pMesh->m_objNum.nBones > 0)
	{
		AnimIndex blendingAnim;
		AnimIndex curAnim = pMesh->GetAnimIndexByID(0);
		pMesh->calcBones(NULL, curAnim, blendingAnim, 0.f);

		ModelVertex *ov = pMesh->m_origVertices;
		ParaEngine::Bone* bones = pMesh->bones;
		int nVertexCount = pMesh->m_objNum.nVertices;
		ResetAABB();
		if (!pMesh->animated)
		{
			for (int i = 0; i < nVertexCount; ++i, ++ov)
			{
				Bone& bone = bones[ov->bones[0]];
				ov->pos = ov->pos * bone.mat;
				ov->normal = ov->normal.TransformNormal(bone.mrot);
				CalculateMinMax(ov->pos);
			}
		}
		else
		{
			for (int i = 0; i < nVertexCount; ++i, ++ov)
			{
				Bone& bone = bones[ov->bones[0]];
				float weight = ov->weights[0] * (1 / 255.0f);
				Vector3 v = (ov->pos * bone.mat)*weight;
				for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
					weight = ov->weights[b] * (1 / 255.0f);
					Bone& bone = bones[ov->bones[b]];
					v += (ov->pos * bone.mat) * weight;
				}
				CalculateMinMax(v);
			}
		}
		pMesh->m_header.minExtent = m_minExtent;
		pMesh->m_header.maxExtent = m_maxExtent;
	}

	std::stable_sort(pMesh->passes.begin(), pMesh->passes.end());
}

void FBXParser::FillParaXModelData(CParaXModel *pMesh, const aiScene *pFbxScene)
{
	pMesh->m_objNum.nVertices = m_vertices.size();
	pMesh->m_objNum.nBones = m_bones.size();
	pMesh->m_objNum.nTextures = m_textures.size();
	pMesh->m_objNum.nIndices = m_indices.size();
	pMesh->m_header.minExtent = m_minExtent;
	pMesh->m_header.maxExtent = m_maxExtent;
	pMesh->m_vNeckYawAxis = m_modelInfo.m_vNeckYawAxis;
	pMesh->m_vNeckPitchAxis = m_modelInfo.m_vNeckPitchAxis;
	
	if (m_bones.size() > 0)
	{
		pMesh->bones = new ParaEngine::Bone[m_bones.size()];
		for (int i = 0; i < (int)m_bones.size(); ++i)
		{
			m_bones[i].RemoveRedundentKeys();
			pMesh->bones[i] = m_bones[i];
			if (m_bones[i].nBoneID > 0)
				pMesh->m_boneLookup[m_bones[i].nBoneID] = i;
			else if (m_bones[i].IsAttachment())
			{
				// TODO: pivot point
				pMesh->NewAttachment(true, m_bones[i].GetAttachmentId(), i, Vector3::ZERO);
			}
		}
	}

	if (m_bHasSkinnedMesh && pMesh->animated && m_anims.size() == 0)
	{
		// static animation 0, just in case there are skinned mesh without any animation
		// we need to animate bones just in case external animations are used outside. 
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = 0;
		anim.timeEnd = 0;
		anim.animID = 0;
		m_anims.push_back(anim);
	}

	if (m_anims.size() > 0 && m_bones.size() > 0)
	{
		pMesh->anims = new ModelAnimation[m_anims.size()];
		memcpy(pMesh->anims, &(m_anims[0]), sizeof(ModelAnimation)*m_anims.size());
		pMesh->animBones = true;
		pMesh->animated = true;
	}
	else
	{
		pMesh->animBones = false;
		pMesh->animated = false;
	}
	pMesh->m_objNum.nAnimations = m_bones.size() > 0 ? m_anims.size() : 0;

	if (m_textures.size() > 0)
	{
		pMesh->textures = new asset_ptr<TextureEntity>[m_textures.size()];
		for (int i = 0; i < (int)m_textures.size(); i++)
		{
			if (m_textureContentMapping.find(m_textures[i]) != m_textureContentMapping.end())
			{
				int nSize = m_textureContentMapping[m_textures[i]].size();
				if (nSize > 0)
				{
					TextureEntity *texEntity = CGlobals::GetAssetManager()->GetTextureManager().NewEntity(m_textures[i]);
					char* bufferCpy = new char[nSize];
					memcpy(bufferCpy, m_textureContentMapping[m_textures[i]].c_str(), nSize);
					texEntity->SetRawData(bufferCpy, nSize);
					pMesh->textures[i] = texEntity;
				}
			}
			else if (CParaFile::DoesAssetFileExist(m_textures[i].GetFileName().c_str()))
			{
				pMesh->textures[i] = CGlobals::GetAssetManager()->LoadTexture("", m_textures[i], TextureEntity::StaticTexture);
			}
			// for replaceable textures
			if (m_textures[i].nIsReplaceableIndex > 0 && i < 32)
			{
				pMesh->specialTextures[i] = m_textures[i].nIsReplaceableIndex;
			}
		}
	}

	pMesh->m_RenderMethod = pMesh->HasAnimation() ? CParaXModel::SOFT_ANIM : CParaXModel::NO_ANIM;
	// only enable bmax model, if there are vertex color channel.
	if (m_bUsedVertexColor)
		pMesh->SetBmaxModel();

	pMesh->initVertices(m_vertices.size(), &(m_vertices[0]));
	pMesh->initIndices(m_indices.size(), &(m_indices[0]));

	if (pMesh->geosets.size() > 0)
	{
		pMesh->showGeosets = new bool[pMesh->geosets.size()];
		memset(pMesh->showGeosets, true, pMesh->geosets.size()*sizeof(bool));
	}
	pMesh->m_radius = (m_maxExtent - m_minExtent).length() / 2;

	AddDefaultColors(pMesh);
	AddDefaultTransparency(pMesh);
}

XFile::Scene* FBXParser::ParseFBXFile()
{
	CParaFile myFile(m_sFilename.c_str());
	if (!myFile.isEof())
	{
		return ParseFBXFile(myFile.getBuffer(), myFile.getSize());
	}
	else
	{
		OUTPUT_LOG("warn: unable to open file %s\n", m_sFilename.c_str());
		return NULL;
	}
}

CParaXModel* FBXParser::ParseParaXModel()
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

LinearColor FBXParser::GetRGBA(int colorTag)
{
	LinearColor color;
	switch (colorTag)
	{
		//1:diffuse
	case 1:
		color.r = 0.588f;
		color.g = 0.588f;
		color.b = 0.588f;
		color.a = 1.f;
		break;
		//2:specular
	case 2:
		color.r = 0.9f;
		color.g = 0.9f;
		color.b = 0.9f;
		color.a = 0.f;
		break;
		//3£ºemissive
	case 3:
		color.r = 1.f;
		color.g = 0.f;
		color.b = 0.f;
		color.a = 0.f;
		break;
	}
	return color;
}

void FBXParser::FillMaterial(XFile::Material *pMaterial)
{
	std::string matName;
	if (true)
	{
		char temp[16];
		ParaEngine::StringHelper::fast_itoa(m_nMaterialIndex, temp, 10);
		m_nMaterialIndex = m_nMaterialIndex + 1;
		matName = std::string("material") + temp;
	}

	pMaterial->mName = matName;
	pMaterial->mIsReference = false;

	// read material values
	pMaterial->mDiffuse = GetRGBA(1);
	pMaterial->mSpecularExponent = 0;
	pMaterial->mSpecular = GetRGBA(2).ToVector3();
	pMaterial->mEmissive = GetRGBA(3).ToVector3();

	std::string texname = g_sDefaultTexture;
	pMaterial->mTextures.push_back(TexEntry(texname));
}

void FBXParser::FillMaterialList(aiMesh *pFbxMesh, XFile::Mesh *pMesh)
{
	int numFaces = pFbxMesh->mNumFaces;
	int i;
	for (i = 0; i < numFaces; i++)
	{
		pMesh->mFaceMaterials.push_back(0);
	}

	m_nMaterialIndex = 1;
	Material material;
	FillMaterial(&material);
	pMesh->mMaterials.push_back(material);
}

Vector2 FBXParser::GetTextureCoords()
{
	Vector2 vector;
	vector.x = 0.0;
	vector.y = 1.0;
	return vector;
}

void FBXParser::FillTextureCoords(aiMesh *pFbxMesh, XFile::Mesh *pMesh)
{
	std::vector<Vector2>& coords = pMesh->mTexCoords[pMesh->mNumTextures++];
	coords.resize(pFbxMesh->mNumVertices);
	int numTextureCoords = pFbxMesh->mNumVertices;
	for (int a = 0; a < numTextureCoords; a++)
		coords[a] = GetTextureCoords();

}

std::string FBXParser::GetTexturePath(string textpath)
{
	string textname = CParaFile::GetParentDirectoryFromPath(m_sFilename) + CParaFile::GetFileName(textpath);
	return textname;
}

void FBXParser::ProcessStaticFBXMaterial(const aiScene* pFbxScene, unsigned int iIndex)
{
	aiMaterial* pfbxMaterial = pFbxScene->mMaterials[iIndex];
	unsigned int iMesh = 0;
	for (unsigned int i = 0; i < pFbxScene->mNumMaterials; ++i)
	{
		if (iIndex == pFbxScene->mMeshes[i]->mMaterialIndex)
		{
			iMesh = i;
			break;
		}
	}
	std::string sMatName;
	{
		aiString sMaterialName;
		if (AI_SUCCESS == aiGetMaterialString(pfbxMaterial, AI_MATKEY_NAME, &sMaterialName))
			sMatName = sMaterialName.C_Str();
	}

	unsigned int iUV;
	float fBlend;
	aiTextureOp eOp;
	aiString szPath;
	char* content_begin = NULL;
	int content_len = -1;

	bool bNoOpacity = true;
	for (unsigned int i = 0; i <= AI_TEXTURE_TYPE_MAX; ++i)
	{
		unsigned int iNum = 0;
		while (true)
		{
			if (AI_SUCCESS != aiGetMaterialTexture(pfbxMaterial, (aiTextureType)i, iNum,
				&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len))
			{
				break;
			}
			if (aiTextureType_OPACITY == i)bNoOpacity = false;
			++iNum;
		}
	}

	std::string texname = std::string(szPath.C_Str());
	texname = GetTexturePath(texname);
	std::string tex_content;
	if (content_begin)
		tex_content.append(content_begin, content_len);

	Material material;
	material.mIsReference = false;
	material.mName = sMatName;
	material.mDiffuse = GetRGBA(1);
	material.mSpecularExponent = 0;
	material.mSpecular = GetRGBA(2).ToVector3();
	material.mEmissive = GetRGBA(3).ToVector3();
	texname = g_sDefaultTexture;
	TexEntry tex(texname, true);
	material.mTextures.push_back(TexEntry(texname, true));
	m_pScene->mGlobalMeshes[iMesh]->mMaterials.push_back(material);
}

void ParaEngine::FBXParser::ParseMaterialByName(const std::string& sMatName, FBXMaterial* out)
{
	int nMarkIndex = sMatName.size() - 1;
	for (; nMarkIndex > 0; nMarkIndex -= 2)
	{
		int nID = -1;
		char symbol = '\0';
		if (nMarkIndex >= 0 && sMatName[nMarkIndex] >= '0' && sMatName[nMarkIndex] <= '9')
		{
			nID = sMatName[nMarkIndex] - '0';
			nMarkIndex--;
			if (nMarkIndex >= 0 && sMatName[nMarkIndex] >= '0' && sMatName[nMarkIndex] <= '9')
			{
				nID += (sMatName[nMarkIndex] - '0') * 10;
				nMarkIndex--;
				if (nMarkIndex >= 0 && sMatName[nMarkIndex] >= '0' && sMatName[nMarkIndex] <= '9')
				{
					nID += (sMatName[nMarkIndex] - '0') * 100;
					nMarkIndex--;
					if (nMarkIndex >= 0 && sMatName[nMarkIndex] >= '0' && sMatName[nMarkIndex] <= '9')
					{
						nID += (sMatName[nMarkIndex] - '0') * 1000;
						nMarkIndex--;
					}
				}
			}
		}

		if (nMarkIndex == 0 || sMatName[nMarkIndex - 1] == '_')
		{
			symbol = sMatName[nMarkIndex];
			switch (symbol)
			{
			case 'b':
				{
					// if the material name ends with "_b", alpha testing will be disabled. this is usually the case for fully blended textures with alpha channels, such as fire and effects, etc.
					// 2006.12.21 LXZ: for textures without alpha testing. 
					out->fAlphaTestingRef = 0.f;
					break;
				}
			case 't':
				{
					// if the material name ends with "_t", z buffer write will be disabled. this is usually the case for particle meshes, etc.
					// 2006.12.29 LXZ: for textures without z buffer disabled. 
					out->bDisableZWrite = true;
					break;
				}
			case 'l':
				{
					// if the material name ends with "_l", z buffer write will be disabled. However, the material will be rendered with the mesh even it is transparent. 
					// 2006.12.29 LXZ: for textures without z buffer disabled. 
					out->bForceLocalTranparency = true;

					// layer order. 
					if (nID > 0)
						out->m_nOrder = nID;
					break;
				}
			case 'a':
				{
					// added  2007.1.5 LXZ: if the material name ends with "_a", physics will be disabled. this is usually the case for leaves on a tree etc.
					out->bDisablePhysics = true;
					break;
				}
			case 'p':
				{
					// added  2016.9.8 LXZ: if the material name ends with "_p", physics will be enabled. 
					out->bForcePhysics = true;
					break;
				}
			case 'u':
				{
					// added  2007.11.5 LXZ: if the material name ends with "_u", it will be unlit, which means no lighting is applied to surface. 
					out->bUnlit = true;
					break;
				}
			case 'd':
				{
					// added  2008.12.1 LXZ: if the material name ends with "_d", it will be additive blending.
					out->bAddictive = true;
					break;
				}
			case 'c':
				{
					if (nID < 0)
					{
						// added  2008.12.4 LXZ: if the material name ends with "_c", it will face the camera.Note, it only works with static mesh. 
						// For animated model, use "_b" bone names.  Also note that the engine will use the center of the sub mesh as pivot point. If u have several billboarded faces in a single mesh, please name their materials differently, such as mat0_c, mat1_c, mat2_c.
						out->bBillboard = true;
						out->nForceUnique = ++m_unique_id;
					}
					else
					{
						out->SetCategoryID(nID);
					}
					break;
				}
			case 'y':
				{
					// added  2008.12.4 LXZ: if the material name ends with "_y", it will face the camera but UP axis aligned. Note, it only works with static mesh. 
					// For animated model, use "_u" bone names.  Also note that the engine will use the center of the sub mesh as pivot point. If u have several billboarded faces in a single mesh, please name their materials differently, such as mat0_c, mat1_c, mat2_c.
					out->bAABillboard = true;
					out->nForceUnique = ++m_unique_id;
					break;
				}
			case 'r':
			{
				// if "_r", it is replaceable texture "_r2"
				out->nIsReplaceableIndex = nID >= 0 ? nID : 2;
				break;
			}
			default:
				break;
			}
		}
		else
			nMarkIndex = 0;
	}
}

void FBXParser::ProcessFBXMaterial(const aiScene* pFbxScene, unsigned int iIndex, CParaXModel *pMesh)
{
	aiMaterial* pfbxMaterial = pFbxScene->mMaterials[iIndex];

	unsigned int iUV;
	float fBlend;
	aiTextureOp eOp;
	aiString szPath;
	char* content_begin = NULL;

	int content_len = -1;

	std::string sMatName;
	{
		aiString sMaterialName;
		if (AI_SUCCESS == aiGetMaterialString(pfbxMaterial, AI_MATKEY_NAME, &sMaterialName))
			sMatName = sMaterialName.C_Str();
	}

	aiGetMaterialTexture(pfbxMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len);

	std::string diffuseTexName(szPath.C_Str());
	if (diffuseTexName != "")
	{
		diffuseTexName = GetTexturePath(diffuseTexName);

		if (content_begin)
		{
			std::string sFileName = CParaFile::GetFileName(m_sFilename);
			diffuseTexName = CParaFile::GetParentDirectoryFromPath(diffuseTexName) + sFileName + "/" + CParaFile::GetFileName(diffuseTexName);
			m_textureContentMapping[diffuseTexName] = "";
			m_textureContentMapping[diffuseTexName].append(content_begin, content_len);
			// OUTPUT_LOG("embedded FBX texture %s used. size %d bytes\n", texname.c_str(), (int)m_textureContentMapping[texname].size());
		}
		else if (!CParaFile::DoesFileExist(diffuseTexName.c_str(), true))
		{
			OUTPUT_LOG("warn: FBX texture %s not exist\n", diffuseTexName.c_str());
			diffuseTexName = "";
		}
	}
	m_bUsedVertexColor = diffuseTexName.empty() && m_bUsedVertexColor;

	// parse material name
	FBXMaterial fbxMat;
	ParseMaterialByName(sMatName, &fbxMat);

	int16 blendmode = BM_OPAQUE;
	if (!diffuseTexName.empty())
	{
		if (AI_SUCCESS == aiGetMaterialTexture(pfbxMaterial, (aiTextureType)aiTextureType_OPACITY, 0,
			&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len))
		{
			if (fbxMat.isAlphaBlended())
				blendmode = BM_ALPHA_BLEND;
			else
				blendmode = BM_TRANSPARENT;
		}
	}
	if (fbxMat.bAddictive)
		blendmode = BM_ADDITIVE;

	if (m_bUsedVertexColor)
	{
		diffuseTexName = std::string(g_sDefaultTexture);
	}
	fbxMat.m_filename = diffuseTexName;
	m_textures.push_back(fbxMat);

	int texture_index = m_textures.size() - 1;
	ModelRenderPass pass;
	pass.tex = texture_index;
	pass.SetCategoryId(fbxMat.GetCategoryID());
	pass.texanim = -1;
	pass.color = -1;
	pass.opacity = -1;
	pass.unlit = fbxMat.bUnlit;
	pass.nozwrite = fbxMat.bDisableZWrite;
	pass.disable_physics = fbxMat.bDisablePhysics;
	pass.force_physics = fbxMat.bForcePhysics;
	
	pass.blendmode = blendmode;
	pass.cull = blendmode == BM_OPAQUE ? true : false;
	pass.order = fbxMat.m_nOrder;
	pass.geoset = -1; // make its geoset uninitialized
	//*(((DWORD*)&(pass.geoset)) + 1) = parser.ReadInt();
	pMesh->passes.push_back(pass);
}

void FBXParser::ProcessStaticFBXMesh(aiMesh *pFbxMesh, XFile::Mesh *pMesh)
{
	int numVertices = pFbxMesh->mNumVertices;
	pMesh->mPositions.resize(numVertices);
	pMesh->mNormals.resize(numVertices);
	std::vector<Vector2>& coords = pMesh->mTexCoords[pMesh->mNumTextures++];
	coords.resize(pFbxMesh->mNumVertices);
	//pMesh->mTexCoords.resize(numVertices);
	int i;
	for (i = 0; i < numVertices; i++)
	{
		pMesh->mPositions[i] = ConvertFBXVector3D(pFbxMesh->mVertices[i]);
		pMesh->mNormals[i] = ConvertFBXVector3D(pFbxMesh->mNormals[i]);
		if (pFbxMesh->HasTextureCoords(0))
			coords[i] = Vector2(pFbxMesh->mTextureCoords[0][i].x, pFbxMesh->mTextureCoords[0][i].y);
		else
			coords[i] = Vector2(0.5f, 0.5f);
	}

	int numFaces = pFbxMesh->mNumFaces;
	pMesh->mPosFaces.resize(numFaces);
	pMesh->mNormFaces.resize(numFaces);
	for (i = 0; i < numFaces; i++)
	{
		unsigned int *faceIndices = pFbxMesh->mFaces[i].mIndices;

		Face& face = pMesh->mPosFaces[i];
		face.mIndices[0] = faceIndices[0];
		face.mIndices[1] = faceIndices[1];
		face.mIndices[2] = faceIndices[2];

		Face& nface = pMesh->mNormFaces[i];
		nface.mIndices[0] = faceIndices[0];
		nface.mIndices[1] = faceIndices[1];
		nface.mIndices[2] = faceIndices[2];

		pMesh->mFaceMaterials.push_back(0);
	}

	//FillMaterialList(pFbxMesh, pMesh);
	//FillTextureCoords(pFbxMesh, pMesh);
}

void FBXParser::ConvertFBXBone(ParaEngine::Bone& bone, const aiBone *pfbxBone)
{
	/*auto invertOffsetMat = pfbxBone->mOffsetMatrix;
	invertOffsetMat.Inverse();
	aiVector3D scaling;
	aiQuaternion rotation;
	aiVector3D position;
	invertOffsetMat.Decompose(scaling, rotation, position);

	aiVector3D pivot = invertOffsetMat * aiVector3D(0, 0, 0);
	bone.pivot = ConvertFBXVector3D(pivot);

	// temporarily save the offset matrix (transform from mesh space in binding pose to local bone space) in the bone.mat
	bone.mat = (const Matrix4&)(pfbxBone->mOffsetMatrix);
	bone.mat = bone.mat.transpose();
	Matrix4 matBoneToMesh = bone.mat;
	matBoneToMesh.invert();
	Quaternion rot;
	Vector3 pos;
	Vector3 scaling;
	ParaMatrixDecompose(&scaling, &rot, &pos, &matBoneToMesh);
	bone.pivot = pos;
	rot.ToRotationMatrix(bone.mrot, Vector3::ZERO);
	*/
}

void FBXParser::CalculateMinMax(const Vector3& v)
{
	if (v.x > m_maxExtent.x) m_maxExtent.x = v.x;
	if (v.y > m_maxExtent.y) m_maxExtent.y = v.y;
	if (v.z > m_maxExtent.z) m_maxExtent.z = v.z;

	if (v.x < m_minExtent.x) m_minExtent.x = v.x;
	if (v.y < m_minExtent.y) m_minExtent.y = v.y;
	if (v.z < m_minExtent.z) m_minExtent.z = v.z;
}


int ParaEngine::FBXParser::CreateGetBoneIndex(const char* pNodeName)
{
	int nBoneIndex = -1;
	if (m_boneMapping.find(pNodeName) != m_boneMapping.end())
	{
		nBoneIndex = m_boneMapping[pNodeName];
	}
	else
	{
		ParaEngine::Bone bone;
		bone.nIndex = m_bones.size();
		bone.bUsePivot = false;
		nBoneIndex = bone.nIndex;
		bone.SetName(pNodeName);
		bone.AutoSetBoneInfoFromName();
		m_boneMapping[pNodeName] = bone.nIndex;
		m_bones.push_back(bone);
	}
	return nBoneIndex;
}


void FBXParser::ProcessFBXMesh(const aiScene* pFbxScene, aiMesh *pFbxMesh, aiNode* pFbxNode, CParaXModel *pMesh)
{
	int index_start = m_indices.size();
	int vertex_start = m_vertices.size();
	int numFaces = pFbxMesh->mNumFaces;
	int numVertices = pFbxMesh->mNumVertices;

	// add vertices
	m_vertices.reserve(m_vertices.size() + numVertices);
	aiVector3D* uvs = NULL;
	if (pFbxMesh->HasTextureCoords(0))
		uvs = pFbxMesh->mTextureCoords[0];
	int nBoneIndex = CreateGetBoneIndex(pFbxNode->mName.C_Str());

	// check diffuse color
	DWORD dwDiffuseColor = Color::White;
	aiMaterial* useMaterial = pFbxScene->mMaterials[pFbxMesh->mMaterialIndex];
	aiTextureOp eOp;
	aiString szPath;
	char* content_begin = NULL;
	int content_len = -1;
	unsigned int iUV;
	float fBlend;
	aiGetMaterialTexture(useMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len);
	std::string diffuseTexName(szPath.C_Str());
	if (diffuseTexName == "")
	{
		aiColor4D diffuseColor;
		aiGetMaterialColor(useMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
		dwDiffuseColor = LinearColor(diffuseColor.r, diffuseColor.g , diffuseColor.b, 1.0f);
	}

	for (int i = 0; i < numVertices; i++)
	{
		ModelVertex vertex;
		memset(&vertex, 0, sizeof(ModelVertex));
		vertex.pos = ConvertFBXVector3D(pFbxMesh->mVertices[i]);
		vertex.normal = ConvertFBXVector3D(pFbxMesh->mNormals[i]);
		if (uvs)
			vertex.texcoords = Vector2(uvs[i].x, uvs[i].y);
		else
			vertex.texcoords = Vector2(0.f, 0.f);

		vertex.color0 = dwDiffuseColor;

		// remember the vertex' bone index, but do not assign any weight at the moment
		vertex.bones[0] = nBoneIndex;
		m_vertices.push_back(vertex);
		CalculateMinMax(vertex.pos);
	}
	if (pFbxMesh->GetNumColorChannels() >= 2)
	{
		aiColor4D* colors0 = pFbxMesh->mColors[0];
		aiColor4D* colors1 = pFbxMesh->mColors[1];

		for (int i = 0; i < numVertices; i++)
		{
			ModelVertex& vertex = m_vertices[vertex_start + i];
			LinearColor color0(colors0[i].r, colors0[i].g, colors0[i].b, colors0[i].a);
			vertex.color0 = color0;
			LinearColor color1(colors1[i].r, colors1[i].g, colors1[i].b, colors1[i].a);
			vertex.color1 = color1;
		}
	}
	else if (pFbxMesh->GetNumColorChannels() >= 1)
	{
		aiColor4D* colors0 = pFbxMesh->mColors[0];

		for (int i = 0; i < numVertices; i++)
		{
			ModelVertex& vertex = m_vertices[vertex_start + i];
			LinearColor color(colors0[i].r, colors0[i].g, colors0[i].b, colors0[i].a);
			vertex.color0 = color;
		}
	}


	// add geoset (faces & indices)
	{
		const int maxFaceCount = (int)(0xffff / 3);
		int nFaceStart = 0;
		int nVertexOffset = vertex_start;
		int nSplitCount = 0;
		while (numFaces > 0 && (++nSplitCount)<100)
		{
			ModelGeoset geoset;
			geoset.id = (uint16)pMesh->geosets.size();
			vertex_start = nVertexOffset;
			int nFaceCount = std::min(maxFaceCount, numFaces);
			if (numFaces > maxFaceCount || nSplitCount>1)
			{
				// get vertex offset and max number of vertex
				vertex_start = 0;
				unsigned int nMinIndex = 0xffffffff;
				unsigned int nMaxIndex = 0;
				for (int i = 0; i < nFaceCount; i++)
				{
					const aiFace& fbxFace = pFbxMesh->mFaces[i + nFaceStart];
					assert(fbxFace.mNumIndices == 3);
					for (int j = 0; j < 3; j++)
					{
						auto nIndex = fbxFace.mIndices[j];
						if (nIndex < nMinIndex)
							nMinIndex = nIndex;
						if (nIndex > nMaxIndex)
							nMaxIndex = nIndex;
						if ((nMaxIndex - nMinIndex) > 0xffff)
						{
							nFaceCount = i;
							break;
						}
					}
					vertex_start = nMinIndex;
				}
				vertex_start += nVertexOffset;
			}
			
			if (nFaceCount == 0) 
			{
				// warning: skip this face, if we can not easily split large mesh without reordering index. 
				numFaces -= 1;
				nFaceStart += 1;
				continue;
			}
			numFaces -= nFaceCount;
			int nIndexOffset = nVertexOffset - vertex_start;
			for (int i = 0; i < nFaceCount; i++)
			{
				const aiFace& fbxFace = pFbxMesh->mFaces[i+ nFaceStart];
				assert(fbxFace.mNumIndices == 3);
				for (int j = 0; j < 3; j++)
				{
					int index_ = fbxFace.mIndices[j] + nIndexOffset;
					assert(index_ >= 0);
					m_indices.push_back((uint16)index_);
				}
			}
			geoset.istart = index_start;
			geoset.icount = nFaceCount * 3;
			geoset.vstart = vertex_start;
			geoset.vcount = numVertices;
			geoset.SetVertexStart(vertex_start);

			pMesh->geosets.push_back(geoset);

			ModelRenderPass* pPass = &(pMesh->passes[pFbxMesh->mMaterialIndex]);
			if (pPass->geoset >= 0) {
				// if there is already a render pass due to mesh split, create a new render pass that inherit the unsplited pass.
				pMesh->passes.push_back(*pPass);
				pPass = &(pMesh->passes[pMesh->passes.size() - 1]);
			}
			pPass->indexStart = index_start;
			pPass->indexCount = nFaceCount * 3;
			pPass->SetStartIndex(index_start);
			pPass->geoset = pMesh->geosets.size() - 1;

			nFaceStart += nFaceCount;
			index_start += nFaceCount * 3;
		}
	}

	// add bones and vertex weight
	if (pFbxMesh->HasBones())
	{
		int numBones = pFbxMesh->mNumBones;

		m_bHasSkinnedMesh = true;
		if (!pMesh->animated && numBones > 1)
		{
			// always regard as animated if there are skinned mesh
			pMesh->animated = true; 
		}

		for (int i = 0; i < numBones; i++)
		{
			const aiBone * fbxBone = pFbxMesh->mBones[i];
			int nBoneIndex = CreateGetBoneIndex(fbxBone->mName.C_Str());
			if (nBoneIndex >= 0)
			{
				ParaEngine::Bone& bone = m_bones[nBoneIndex];
				Matrix4 offsetMat = reinterpret_cast<const Matrix4&>(fbxBone->mOffsetMatrix);
				offsetMat = offsetMat.transpose();
				bone.matOffset = offsetMat;
				bone.flags |= ParaEngine::Bone::BONE_OFFSET_MATRIX;
				bone.pivot = Vector3(0, 0, 0) * bone.matOffset.InvertPRMatrix();
			}

			for (int j = 0; j < (int)fbxBone->mNumWeights; j++)
			{
				aiVertexWeight vertexWeight = fbxBone->mWeights[j];
				int vertex_id = vertexWeight.mVertexId + vertex_start;
				uint8 vertex_weight = (uint8)(vertexWeight.mWeight * 255);
				int nTotalWeight = 0;
				int bone_index = 0;
				ModelVertex & vertex = m_vertices[vertex_id];
				for (; bone_index < ParaEngine::Bone::s_MaxBonesPerVertex; bone_index++)
				{
					uint8 cur_vertex_weight = vertex.weights[bone_index];
					nTotalWeight += cur_vertex_weight;
					if (cur_vertex_weight == 0)
					{
						//if (nTotalWeight > 255)
						//	vertex_weight -= nTotalWeight - 255;
						if (nTotalWeight == 254)
							vertex_weight += 1;
						vertex.bones[bone_index] = nBoneIndex;
						vertex.weights[bone_index] = vertex_weight;
						break;
					}
				}

				if (bone_index >= ParaEngine::Bone::s_MaxBonesPerVertex)
				{
					OUTPUT_LOG("warn: %s vertex %d has more than 4 bones affecting it. overwrite the smallest one\n", m_modelInfo.m_sFilename.c_str(), vertex_id);
					int nSmallestIndex = 0;
					for (bone_index = 1; bone_index < ParaEngine::Bone::s_MaxBonesPerVertex; bone_index++)
					{
						uint8 cur_vertex_weight = vertex.weights[bone_index];
						if (cur_vertex_weight <= vertex.weights[nSmallestIndex])
							nSmallestIndex = bone_index;
					}
					vertex.bones[nSmallestIndex] = nBoneIndex;
					vertex.weights[nSmallestIndex] = vertex_weight;
				}
			}
		}
	}
}


ModelAnimation FBXParser::CreateModelAnimation(aiAnimation* pFbxAnim, ParaEngine::AnimInfo* pAnimInfo, int AnimIndex, bool beEndAnim)
{
	float fTimeScale = (float)(1000.f / pFbxAnim->mTicksPerSecond);
	int animID, tickStart, tickEnd;
	uint32 timeStart, timeEnd, loopType;
	float fMoveSpeed = 0.f;
	if (pAnimInfo)
	{
		animID = pAnimInfo->id;
		tickStart = pAnimInfo->startTick;
		tickEnd = pAnimInfo->endTick;
		fMoveSpeed = pAnimInfo->fSpeed;
		timeStart = (uint32)(pAnimInfo->startTick * fTimeScale);
		timeEnd = (uint32)(pAnimInfo->endTick * fTimeScale);
		loopType = (uint32)pAnimInfo->loopType;
	}
	else
	{
		animID = AnimIndex;
		tickStart = 0;
		tickEnd = (int)pFbxAnim->mDuration;
		timeStart = 0;
		timeEnd = (uint32)(pFbxAnim->mDuration * fTimeScale);
		loopType = 0;
	}
	if ((animID == 4 || animID == 5) && fMoveSpeed == 0.f)
	{
		fMoveSpeed = 4.f;
	}

	ModelAnimation anim;
	memset(&anim, 0, sizeof(ModelAnimation));
	anim.animID = animID;
	anim.timeStart = timeStart;
	anim.timeEnd = timeEnd;
	anim.loopType = loopType;
	anim.moveSpeed = fMoveSpeed;

	for (ParaEngine::Bone & bone : m_bones)
	{
		// support multiple animations
		int nAnimId = AnimIndex;
		if (bone.scale.times.size() > 0)
		{
			int nCount = bone.scale.times.size();
			bone.scale.ranges.resize(nAnimId + 1, AnimRange(0, 0));

			AnimRange range(0, nCount - 1);
			for (int i = 0; i < nCount; ++i)
			{
				uint32 time = (uint32)bone.scale.times[i];
				if (time <= timeStart){
					range.first = i;
				}
				else if (time >= timeEnd)
				{
					range.second = i;
					break;
				}
			}
			bone.scale.ranges[nAnimId] = range;
		}
		if (bone.trans.times.size() > 0)
		{
			int nCount = bone.trans.times.size();
			bone.trans.ranges.resize(nAnimId + 1, AnimRange(0, 0));

			AnimRange range(0, nCount - 1);
			for (int i = 0; i < nCount; ++i)
			{
				uint32 time = (uint32)bone.trans.times[i];
				if (time <= timeStart){
					range.first = i;
				}
				else if (time >= timeEnd)
				{
					range.second = i;
					break;
				}
			}
			bone.trans.ranges[nAnimId] = range;
		}
		if (bone.rot.times.size() > 0)
		{
			int nCount = bone.rot.times.size();
			bone.rot.ranges.resize(nAnimId + 1, AnimRange(0, 0));

			AnimRange range(0, nCount - 1);
			for (int i = 0; i < nCount; ++i)
			{
				uint32 time = (uint32)bone.rot.times[i];
				if (time <= timeStart){
					range.first = i;
				}
				else if (time >= timeEnd)
				{
					range.second = i;
					break;
				}
			}
			bone.rot.ranges[nAnimId] = range;
		}
	}
	return anim;
}

void FBXParser::ProcessFBXAnimation(const aiScene* pFbxScene, unsigned int nIndex, CParaXModel *pMesh)
{
	aiAnimation* pFbxAnim = pFbxScene->mAnimations[nIndex];

	float fTimeScale = (float)(1000.f / pFbxAnim->mTicksPerSecond);

	int nodeChannelNum = pFbxAnim->mNumChannels;

	for (int j = 0; j < nodeChannelNum; j++)
	{
		aiNodeAnim *nodeChannel = pFbxAnim->mChannels[j];
		int bone_index = CreateGetBoneIndex(nodeChannel->mNodeName.C_Str());
		if (bone_index >= 0)
		{
			ParaEngine::Bone & bone = m_bones[bone_index];
			bone.flags = ParaEngine::Bone::BONE_OFFSET_MATRIX;
			// bone.calc is true, if there is bone animation. 
			bone.calc = true;

			bone.scale.used = (nodeChannel->mNumScalingKeys > 0);
			if (bone.scale.used)
			{
				for (int k = 0; k < (int)nodeChannel->mNumScalingKeys; k++)
				{
					int nTime = (int)nodeChannel->mScalingKeys[k].mTime;
					Vector3 v = ConvertFBXVector3D(nodeChannel->mScalingKeys[k].mValue);
					bone.scale.times.push_back((int)(nTime * fTimeScale));
					bone.scale.data.push_back(v);
				}
			}

			bone.rot.used = (nodeChannel->mNumRotationKeys > 0);
			if (bone.rot.used)
			{
				for (int k = 0; k < (int)nodeChannel->mNumRotationKeys; k++)
				{
					int nTime = (int)nodeChannel->mRotationKeys[k].mTime;
					Quaternion q = ConvertFBXQuaternion(nodeChannel->mRotationKeys[k].mValue);
					bone.rot.times.push_back((int)(nTime * fTimeScale));
					bone.rot.data.push_back(q);
				}
			}

			bone.trans.used = (nodeChannel->mNumPositionKeys > 0);
			if (bone.trans.used)
			{
				for (int k = 0; k < (int)nodeChannel->mNumPositionKeys; k++)
				{
					int nTime = (int)nodeChannel->mPositionKeys[k].mTime;
					Vector3 v = ConvertFBXVector3D(nodeChannel->mPositionKeys[k].mValue);
					bone.trans.times.push_back((int)(nTime * fTimeScale));
					bone.trans.data.push_back(v);
				}
			}
		}
	}

	if (m_modelInfo.LoadFromFile(m_sAnimSplitterFilename))
	{
		int animsCount = m_modelInfo.GetAnimCount();
		for (int i = 0; i < animsCount; i++)
		{
			ParaEngine::AnimInfo &animinfo = m_modelInfo.m_Anims[i];
			ModelAnimation anim = CreateModelAnimation(pFbxAnim, &animinfo, (int)m_anims.size(), i == animsCount - 1);
			m_anims.push_back(anim);
		}
	}
	else
	{
		uint32 time = (uint32)((1000.f / pFbxAnim->mTicksPerSecond) * pFbxAnim->mDuration);
		int animsCount = (int)floor(time / 10000) + 1;
		for (int i = 0; i < animsCount; i++)
		{
			ParaEngine::AnimInfo animinfo;
			animinfo.id = i;
			animinfo.startTick = (int)(i * 10 * pFbxAnim->mTicksPerSecond);
			animinfo.endTick = (int)((i + 1) * 10 * pFbxAnim->mTicksPerSecond - 1);
			animinfo.loopType = 0;
			ModelAnimation anim = CreateModelAnimation(pFbxAnim, &animinfo, (int)m_anims.size(), i == animsCount - 1);
			m_anims.push_back(anim);
		}
	}
}

void FBXParser::ProcessFBXBoneNodes(const aiScene* pFbxScene, aiNode* pFbxNode, int parentBoneIndex, CParaXModel* pMesh)
{
	const std::string nodeName(pFbxNode->mName.C_Str());
	// this will force create a bone for every node. Bones without weights are just treated as ordinary nodes, 
	// so it is important to add them here
	int bone_index = CreateGetBoneIndex(pFbxNode->mName.C_Str());
	if (bone_index >= 0)
	{
		ParaEngine::Bone& bone = m_bones[bone_index];
		// use static transform for non-animated bones
		Matrix4 matTrans = reinterpret_cast<const Matrix4&>(pFbxNode->mTransformation);
		bone.matTransform = matTrans.transpose();
		// bone.calc is true, if there is bone animation. 
		if (!bone.IsAnimated())
		{
			bone.flags |= ParaEngine::Bone::BONE_STATIC_TRANSFORM;
		}
	}
	m_bones[bone_index].parent = parentBoneIndex;

	// process any mesh on the node
	int numMeshes = pFbxNode->mNumMeshes;
	for (int i = 0; i < numMeshes; i++)
	{
		ProcessFBXMesh(pFbxScene, pFbxScene->mMeshes[pFbxNode->mMeshes[i]], pFbxNode, pMesh);
	}

	// for children 
	for (int i = 0; i < (int)pFbxNode->mNumChildren; i++)
	{
		ProcessFBXBoneNodes(pFbxScene, pFbxNode->mChildren[i], bone_index, pMesh);
	}
}

void PrintBone(int nIndex, ParaEngine::Bone* bones)
{
	ParaEngine::Bone& bone = bones[nIndex];
	if (!bone.calc)
	{
		bone.calc = true;
		if (bone.parent >= 0)
			PrintBone(bone.parent, bones);

		OUTPUT_LOG("Bone %s %d (parent: %d) %s (flag:%d) pivot: %.4f %.4f %.4f\n", bone.GetName().c_str(), bone.nIndex, bone.parent,
			bone.IsAnimated() ? "animated" : "", bone.flags, bone.pivot.x, bone.pivot.y, bone.pivot.z);
		if (bone.rot.used){
			Quaternion rot = bone.rot.data[0];
			OUTPUT_LOG("\t\t quat(%d): %.4f %.4f %.4f %.4f\n", (int)bone.rot.data.size(), rot.x, rot.y, rot.z, rot.w);
		}
		if (bone.trans.used){
			Vector3 v = bone.trans.data[0];
			OUTPUT_LOG("\t\t trans(%d): %.4f %.4f %.4f\n", (int)bone.trans.data.size(), v.x, v.y, v.z);
		}
		if (bone.scale.used){
			Vector3 v = bone.scale.data[0];
			OUTPUT_LOG("\t\t scale(%d): %.4f %.4f %.4f\n", (int)bone.scale.data.size(), v.x, v.y, v.z);
		}
		if (bone.IsStaticTransform())
		{
		}
	}
}

void ParaEngine::FBXParser::PrintDebug(const aiScene* pFbxScene)
{
	for (int i = 0; i < (int)m_bones.size(); ++i)
	{
		m_bones[i].calc = false;
	}

	for (int i = 0; i < (int)m_bones.size(); ++i)
	{
		PrintBone(i, &(m_bones[0]));
	}

	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		OUTPUT_LOG("\t Vertex%d: %.4f %.4f %.4f: Bone:%d %d\n", i, m_vertices[i].pos.x, m_vertices[i].pos.y, m_vertices[i].pos.z,
			m_vertices[i].bones[0], m_vertices[i].bones[1]);
	}
}

bool ParaEngine::FBXParser::HasAnimations()
{
	return m_anims.size() > 0;
}

// not used, the exporter is required to do it. 
void ParaEngine::FBXParser::MakeAxisY_UP()
{
	if (m_bones.size()>0)
	{
		// for animated model, rotate the root bone. 
		Bone& bone = m_bones[m_nRootNodeIndex];
		// one should not animate the root bone. 
		assert(!bone.IsAnimated());
		// root transform is usually identity for most cases
		Matrix4 mat = bone.matTransform;
		Quaternion q(Radian(-ParaEngine::Math::PI / 2.0f), Vector3::UNIT_X);
		Matrix3 matRot;
		q.ToRotationMatrix(matRot);
		if (bone.IsStaticTransform())
			mat = mat * matRot;
		else
			mat = matRot;
		bone.SetStaticTransform(mat);
	}
}

const std::string& ParaEngine::FBXParser::GetFilename() const
{
	return m_sFilename;
}

void ParaEngine::FBXParser::SetFilename(const std::string& val)
{
	m_sFilename = val;
}

void ParaEngine::FBXParser::ResetAABB()
{
	m_maxExtent = Vector3(-1000.0f, -1000.0f, -1000.0f);
	m_minExtent = Vector3(1000.0f, 1000.0f, 1000.0f);
}
#endif