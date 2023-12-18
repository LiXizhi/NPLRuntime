//-----------------------------------------------------------------------------
// Class:	FBX importer
// Authors:	LiPeng, LiXizhi
// Emails:	
// Company: ParaEngine
// Date:	2015.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef SUPPORT_FBX_MODEL_FILE
#include "XFileHelper.h"
#include "FBXParser.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "StringHelper.h"
#include "TextureEntity.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"
#include "particle.h"
#include "StringHelper.h"
#include <math.h>

#include "assimp/scene.h"
/** define this use use assimp version 5, default to use ver 4 interface*/
#define ASSIMP5

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

using namespace ParaEngine;
using namespace ParaEngine::XFile;

static const char* g_sDefaultTexture = "Texture/whitedot.png";

namespace ParaEngine
{
	inline Vector3 ConvertFBXVector3D(const aiVector3D& fbxVector3D)
	{
		return Vector3(fbxVector3D.x, fbxVector3D.y, fbxVector3D.z);
	}

	inline Quaternion ConvertFBXQuaternion(const aiQuaternion& fbxQuaternion)
	{
		return Quaternion(-fbxQuaternion.x, -fbxQuaternion.y, -fbxQuaternion.z, fbxQuaternion.w);
	}
}

FBXParser::FBXParser()
	: m_pScene(NULL)
	, m_nMaterialIndex(0)
	, m_nRootNodeIndex(0)
	, m_bUsedVertexColor(true)
	, m_bHasSkinnedMesh(false)
	, m_unique_id(0)
	, m_bHasAABBMesh(false)
	, m_pLuaState(nullptr)
{
}

FBXParser::FBXParser(const string& filename)
	: m_sFilename(filename)
	, m_pScene(NULL)
	, m_nMaterialIndex(0)
	, m_nRootNodeIndex(0)
	, m_bUsedVertexColor(true)
	, m_bHasSkinnedMesh(false)
	, m_unique_id(0)
	, m_bHasAABBMesh(false)
	, m_pLuaState(nullptr)
{
	m_texAnims.reserve(10);
}

FBXParser::~FBXParser()
{
	if (m_pLuaState)
		lua_close(m_pLuaState);
}

XFile::Scene* ParaEngine::FBXParser::ParseFBXFile(const char* buffer, int nSize)
{
	Assimp::Importer importer;
	Reset();
	const aiScene* pFbxScene = importer.ReadFileFromMemory(buffer, nSize, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs, "fbx");
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
	m_bHasAABBMesh = false;
	m_unique_id = 0;
}

void ParaEngine::FBXParser::SetAnimSplitterFilename()
{
	m_sAnimSplitterFilename = std::string(m_sFilename.c_str(), m_sFilename.size() - 3) + "xml";
}

CParaXModel* FBXParser::ParseParaXModel(const char* buffer, int nSize, const char* pHint)
{
	CParaXModel* pMesh = NULL;
	Assimp::Importer importer;
	Reset();
	SetAnimSplitterFilename();
	// this is not needed: aiProcess_MakeLeftHanded | 
	const aiScene* pFbxScene = importer.ReadFileFromMemory(buffer, nSize, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs, pHint);
	if (pFbxScene) {
		ParaXHeaderDef m_xheader;
		m_xheader.IsAnimated = pFbxScene->HasAnimations() ? 1 : 0;

		// RAF stands for _ref_anim_file_: marker for mutual animation file (fbx files with this marker can share the same animation file to avoid meaningless duplication)
		// RAF check
		if (m_sFilename.find("_RAF(") != std::string::npos)
		{
			m_xheader.IsAnimated = 1;
		}
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
		decltype(m_bones) raf_bones;
		decltype(m_boneMapping) raf_bonemapping;
		if (m_sFilename.find("_RAF(") != std::string::npos)
		{
			auto ref_file_start = m_sFilename.find("_RAF(") + strlen("_RAF(");
			auto ref_file_end = m_sFilename.find(')', ref_file_start) - 1;
			std::string anim_file_name = m_sFilename.substr(ref_file_start, ref_file_end - ref_file_start + 1) + ".fbx";
			anim_file_name = m_sFilename.substr(0, m_sFilename.find_last_of("/") + 1) + anim_file_name;
			FBXParser anim_parser(anim_file_name);
			anim_parser.ParseParaXModel();
			m_modelInfo.LoadFromFile(std::string(anim_parser.GetFilename().c_str(), anim_parser.GetFilename().size() - 3) + "xml");
			raf_bonemapping = m_boneMapping = anim_parser.m_boneMapping;
			raf_bones = m_bones = anim_parser.m_bones;
			m_anims = anim_parser.m_anims;
		}
		else if (pFbxScene->HasAnimations())
		{
			int animations_num = pFbxScene->mNumAnimations;
			for (int i = 0; i < animations_num; i++)
			{
				ProcessFBXAnimation(pFbxScene, i, pMesh);
			}
		}

		// building the parent-child relationship of the bones, and add meshes if any;
		ProcessFBXBoneNodes(pFbxScene, pFbxScene->mRootNode, -1, pMesh);

#ifdef ASSIMP5
		// MakeAxisY_UP();
#endif
		FillParaXModelData(pMesh, pFbxScene);

		CalculateScale(pMesh);

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

void FBXParser::CalculateScale(CParaXModel* pMesh)
{
	float scale = 1.0f;
	auto extend = m_maxExtent - m_minExtent;
	// float maxSize = 10.f;  // 模型最大大小, 超过此值进行缩小
	// while (extend.x * scale > maxSize || extend.y * scale > maxSize || extend.z * scale > maxSize) scale /= 10;
	if (extend.x > 50 || extend.y > 50 || extend.z > 50) scale = 0.01f;
	pMesh->m_header.minExtent = scale;
	pMesh->m_header.maxExtent = scale;
	int bones_size = pMesh->m_objNum.nBones;
	if (bones_size == 0) return;

	int root_bone_index = -1;
	for (int i = 0; i < bones_size; i++)
	{
		ParaEngine::Bone& bone = pMesh->bones[i];
		if (bone.parent < 0)
		{
			root_bone_index = i;
			break;
		}
	}

	ParaEngine::Bone& rootBone = pMesh->bones[root_bone_index];

	// 动画缩放
	int size = rootBone.scale.data.size();
	for (int i = 0; i < size; i++)
	{
		rootBone.scale.data[i] *= scale;
	}

	// 非动画缩放
	Matrix4 scaleMat;
	scaleMat.makeScale(scale, scale, scale);
	rootBone.matTransform = rootBone.matTransform * scaleMat;
	rootBone.mat = rootBone.mat * scaleMat;
}

void FBXParser::AddColors(CParaXModel* pMesh)
{
	auto size = m_colors.size();
	if (size == 0)
	{
		// addDefault colors

		pMesh->m_objNum.nColors = 1;

		pMesh->colors = new ModelColor[1];
		ModelColor& color = pMesh->colors[0];
		color.color.used = false;
		color.color.type = 0;
		color.color.seq = -1;
		color.color.globals = NULL;
		color.color.ranges.push_back(AnimRange(0, 0));
		color.color.times.push_back(0);
		color.color.data.push_back(Vector3::ZERO);
		color.color.in.push_back(Vector3::ZERO);
		color.color.out.push_back(Vector3::ZERO);

		color.opacity.used = false;
		color.opacity.type = 0;
		color.opacity.seq = -1;
		color.opacity.globals = NULL;
		color.opacity.ranges.push_back(AnimRange(0, 0));
		color.opacity.times.push_back(0);
		color.opacity.data.push_back(-1.0f);
		color.opacity.in.push_back(0.0f);
		color.opacity.out.push_back(0.0f);
	}
	else
	{
		pMesh->m_objNum.nColors = size;
		pMesh->colors = new ModelColor[size];

		for (size_t i = 0; i < size; i++)
		{
			pMesh->colors[i] = m_colors[i];
		}
	}


}

void FBXParser::AddTransparency(CParaXModel* pMesh)
{
	auto size = m_transparencys.size();

	if (size == 0)
	{
		// add default transparency

		pMesh->m_objNum.nTransparency = 1;
		pMesh->transparency = new ModelTransparency[1];
		ModelTransparency& tran = pMesh->transparency[0];

		tran.trans.used = false;
		tran.trans.type = 0;
		tran.trans.seq = -1;
		tran.trans.globals = NULL;
		tran.trans.ranges.push_back(AnimRange(0, 0));
		tran.trans.times.push_back(0);
		tran.trans.data.push_back(-1.0f);
		tran.trans.in.push_back(0.0f);
		tran.trans.out.push_back(0.0f);
	}
	else
	{
		pMesh->m_objNum.nTransparency = size;
		pMesh->transparency = new ModelTransparency[size];

		for (size_t i = 0; i < size; i++)
		{
			pMesh->transparency[i] = m_transparencys[i];
		}
	}

}

void FBXParser::PostProcessParaXModelData(CParaXModel* pMesh)
{
	// we need to collapse all bone transform. 
	if (pMesh->m_objNum.nBones > 0)
	{
		AnimIndex blendingAnim;
		AnimIndex curAnim = pMesh->GetAnimIndexByID(0);
		pMesh->calcBones(NULL, curAnim, blendingAnim, 0.f, curAnim, blendingAnim, 0.f);

		ModelVertex* ov = pMesh->m_origVertices;
		ParaEngine::Bone* bones = pMesh->bones;
		int nVertexCount = pMesh->m_objNum.nVertices;
		if (m_bHasAABBMesh)
		{
			int nBoneIndex = CreateGetBoneIndex("aabb");
			if (nBoneIndex >= 0)
			{
				Bone& bone = bones[nBoneIndex];
				auto v1 = m_minExtent * bone.mat;
				auto v2 = m_maxExtent * bone.mat;
				m_bHasAABBMesh = false;
				ResetAABB();
				CalculateMinMax(v1);
				CalculateMinMax(v2);
				m_bHasAABBMesh = true;
			}
		}
		else
		{
			ResetAABB();
		}

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
				Vector3 v = (ov->pos * bone.mat) * weight;
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

		for (uint32 i = 0; i < pMesh->m_objNum.nBones; ++i)
		{
			Bone& bone = bones[i];
			if (bone.IsTransformationNode() && !bone.CheckHasAnimation())
			{
				bone.SetStaticTransform(bone.matTransform);
			}
		}
#define COLLAPSE_STATIC_TRANSFORM_NODE false
		if (COLLAPSE_STATIC_TRANSFORM_NODE)
		{
			for (uint32 i = 0; i < pMesh->m_objNum.nBones; ++i)
			{
				Bone& bone = bones[i];
				if (bone.IsStaticTransform() && bone.IsTransformationNode())
				{
					// try to collapse multiple transform node into one to save computation. 
					while (bone.parent >= 0)
					{
						Bone& parent = bones[bone.parent];
						if (parent.IsStaticTransform() && parent.IsTransformationNode())
						{
							bone.matTransform *= parent.matTransform;
							bone.parent = parent.parent;
						}
						else
							break;
					}
				}
			}
		}

	}

	std::stable_sort(pMesh->passes.begin(), pMesh->passes.end());
}

void FBXParser::FillParaXModelData(CParaXModel* pMesh, const aiScene* pFbxScene)
{
	pMesh->m_objNum.nVertices = m_vertices.size();
	pMesh->m_objNum.nBones = m_bones.size();
	pMesh->m_objNum.nTextures = m_textures.size();
	pMesh->m_objNum.nIndices = m_indices.size();
	pMesh->m_header.minExtent = m_minExtent;
	pMesh->m_header.maxExtent = m_maxExtent;
	pMesh->m_vNeckYawAxis = m_modelInfo.m_vNeckYawAxis;
	pMesh->m_vNeckPitchAxis = m_modelInfo.m_vNeckPitchAxis;


	auto nTexAnims = this->m_texAnims.size();
	if (nTexAnims > 0)
	{
		//pMesh->animated = true;
		pMesh->animTextures = true;

		pMesh->texanims = new TextureAnim[nTexAnims];
		pMesh->m_objNum.nTexAnims = nTexAnims;

		for (size_t i = 0; i < nTexAnims; i++)
		{
			pMesh->texanims[i] = m_texAnims[i];
		}

		m_texAnims.clear();
	}

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

	auto psSize = m_particleSystem.size();
	{
		if (psSize > 0)
		{
			pMesh->m_objNum.nParticleEmitters = psSize;
			pMesh->particleSystems = new ParticleSystem[psSize];
			int index = 0;
			for (auto it = m_particleSystem.begin(); it != m_particleSystem.end(); it++)
			{
				ParticleSystem& ps = it->second;
				auto offset = (size_t)ps.parent;
				ps.parent = pMesh->bones + offset;
				auto& curPs = pMesh->particleSystems[index++];
				curPs = ps;
				ps.emitter = nullptr;

				if ((intptr_t)curPs.emitter == ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER)
					curPs.emitter = new SphereParticleEmitter(&curPs);
				else
					curPs.emitter = new PlaneParticleEmitter(&curPs);

			}

			m_particleSystem.clear();
		}
	}

	if ((m_bHasSkinnedMesh && pMesh->animated && m_anims.size() == 0) || nTexAnims > 0 || psSize > 0)
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
		memcpy(pMesh->anims, &(m_anims[0]), sizeof(ModelAnimation) * m_anims.size());
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
					TextureEntity* texEntity = CGlobals::GetAssetManager()->GetTextureManager().GetEntity(m_textures[i]);
					if (!texEntity)
					{
						texEntity = CGlobals::GetAssetManager()->GetTextureManager().NewEntity(m_textures[i]);
						char* bufferCpy = new char[nSize];
						auto src = m_textureContentMapping[m_textures[i]].c_str();
						memcpy(bufferCpy, src, nSize);
						texEntity->SetRawData(bufferCpy, nSize);
						texEntity->SetEmbeddedTexture(true);
					}
					pMesh->textures[i] = texEntity;
				}
			}
			else if (CParaFile::DoesFileExist(m_textures[i].GetFileName().c_str(), true))
			{
				pMesh->textures[i] = CGlobals::GetAssetManager()->LoadTexture("", m_textures[i], TextureEntity::StaticTexture);
			}
			// for replaceable textures
			if (m_textures[i].nIsReplaceableIndex >= 0 && i < 32)
			{
				pMesh->specialTextures[i] = m_textures[i].nIsReplaceableIndex;
			}
		}
	}

	//pMesh->m_RenderMethod = pMesh->HasAnimation() ? CParaXModel::SOFT_ANIM : CParaXModel::NO_ANIM;
	pMesh->SetRenderMethod(pMesh->HasAnimation() ? CParaXModel::SOFT_ANIM : CParaXModel::NO_ANIM);


	// only enable bmax model, if there are vertex color channel.
	if (m_bUsedVertexColor)
		pMesh->SetBmaxModel();

	if (m_vertices.size() > 0)
		pMesh->initVertices(m_vertices.size(), &(m_vertices[0]));
	if (m_indices.size() > 0)
		pMesh->initIndices(m_indices.size(), &(m_indices[0]));

	if (pMesh->geosets.size() > 0)
	{
		pMesh->showGeosets = new bool[pMesh->geosets.size()];
		memset(pMesh->showGeosets, true, pMesh->geosets.size() * sizeof(bool));
	}
	pMesh->m_radius = (m_maxExtent - m_minExtent).length() / 2;

	AddColors(pMesh);
	AddTransparency(pMesh);
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
		//3��emissive
	case 3:
		color.r = 1.f;
		color.g = 0.f;
		color.b = 0.f;
		color.a = 0.f;
		break;
	}
	return color;
}

void FBXParser::FillMaterial(XFile::Material* pMaterial)
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

void FBXParser::FillMaterialList(aiMesh* pFbxMesh, XFile::Mesh* pMesh)
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

void FBXParser::FillTextureCoords(aiMesh* pFbxMesh, XFile::Mesh* pMesh)
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
	int16 opacity = -1;

#ifdef ASSIMP5
	aiGetMaterialTexture(pfbxMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL);
#else
	aiGetMaterialTexture(pfbxMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len);
#endif

	std::string diffuseTexName(szPath.C_Str());
	if (diffuseTexName != "")
	{
		diffuseTexName = GetTexturePath(diffuseTexName);

#ifdef ASSIMP5
		if (auto texture = pFbxScene->GetEmbeddedTexture(diffuseTexName.c_str()))
		{
			// If mHeight = 0, pcData is a pointer to a memory  buffer of size mWidth containing the compressed texture data.
			if (texture->mHeight == 0)
			{
				content_len = (int)(texture->mWidth);
				content_begin = (char*)(texture->pcData);
			}
		}
#endif
		if (content_begin)
		{
			std::string sFileName = CParaFile::GetFileName(m_sFilename);
			diffuseTexName = CParaFile::GetParentDirectoryFromPath(diffuseTexName) + sFileName + "/" + CParaFile::GetFileName(diffuseTexName);

			//m_textureContentMapping.insert(std::make_pair(diffuseTexName, std::string(content_begin, content_len)));
			TextureEntity* texEntity = CGlobals::GetAssetManager()->GetTextureManager().GetEntity(diffuseTexName);
			if (!texEntity)
			{
				texEntity = CGlobals::GetAssetManager()->GetTextureManager().NewEntity(diffuseTexName);
				char* bufferCpy = new char[content_len];
				memcpy(bufferCpy, content_begin, content_len);
				texEntity->SetRawData(bufferCpy, content_len);
				texEntity->SetEmbeddedTexture(true);
				CGlobals::GetAssetManager()->GetTextureManager().AddEntity(diffuseTexName, texEntity);
			}

			// OUTPUT_LOG("embedded FBX texture %s used. size %d bytes\n", texname.c_str(), (int)m_textureContentMapping[texname].size());
		}
		else if (!CParaFile::DoesFileExist(diffuseTexName.c_str(), true))
		{
			OUTPUT_LOG("warn: FBX texture %s not exist\n", diffuseTexName.c_str());
			diffuseTexName = "";
		}
	}


	if (diffuseTexName.empty())
	{
		diffuseTexName = std::string(g_sDefaultTexture);
	}

	Material material;
	material.mIsReference = false;
	material.mName = sMatName;
	material.mDiffuse = GetRGBA(1);
	material.mSpecularExponent = 0;
	material.mSpecular = GetRGBA(2).ToVector3();
	material.mEmissive = GetRGBA(3).ToVector3();
	material.mTextures.push_back(TexEntry(diffuseTexName, true));
	m_pScene->mGlobalMeshes[iMesh]->mMaterials.push_back(material);

	//ASSIMP_API aiReturn aiGetMaterialTexture(const C_STRUCT aiMaterial* mat,
	//	aiTextureType type,
	//	unsigned int  index,
	//	aiString* path,
	//	aiTextureMapping* mapping = NULL,
	//	unsigned int* uvindex = NULL,
	//	ai_real* blend = NULL,
	//	aiTextureOp* op = NULL,
	//	aiTextureMapMode* mapmode = NULL,
	//	unsigned int* flags = NULL);


	/*
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
	*/
}

void ParaEngine::FBXParser::ParseMaterialByName(const std::string& sMatName, FBXMaterial* out)
{
	int nMarkIndex = sMatName.size() - 1;
	for (; nMarkIndex >= 0; nMarkIndex -= 2)
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

void FBXParser::ProcessFBXMaterial(const aiScene* pFbxScene, unsigned int iIndex, CParaXModel* pMesh)
{
	aiMaterial* pfbxMaterial = pFbxScene->mMaterials[iIndex];
	aiTextureType textureTypeList[2] = { aiTextureType_DIFFUSE, aiTextureType_NORMALS };
	pMesh->passes.resize(pMesh->passes.size() + 1);

	for (int i = 0; i < 2; i++)
	{
		aiTextureType textureType = textureTypeList[i];
		unsigned int iUV;
		float fBlend;
		aiTextureOp eOp;
		aiString szPath;
		char* content_begin = NULL;
		int content_len = -1;
		int16 opacity = -1;

		if (textureType != aiTextureType_DIFFUSE && pfbxMaterial->GetTextureCount(textureType) == 0) continue;

		std::string sMatName;
		{
			aiString sMaterialName;
			if (AI_SUCCESS == aiGetMaterialString(pfbxMaterial, AI_MATKEY_NAME, &sMaterialName)) sMatName = sMaterialName.C_Str();
		}

#ifdef ASSIMP5
		aiGetMaterialTexture(pfbxMaterial, textureType, 0, &szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL);
#else
		aiGetMaterialTexture(pfbxMaterial, textureType, 0, &szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len);
#endif

		std::string diffuseTexName(szPath.C_Str());
		if (diffuseTexName != "")
		{
			std::string sOriginalPath;
			CParaFile::ToCanonicalFilePath(sOriginalPath, diffuseTexName, false);
			diffuseTexName = GetTexturePath(diffuseTexName);

#ifdef ASSIMP5
			if (auto texture = pFbxScene->GetEmbeddedTexture(szPath.C_Str()))
			{
				// If mHeight = 0, pcData is a pointer to a memory  buffer of size mWidth containing the compressed texture data.
				if (texture->mHeight == 0)
				{
					content_len = (int)(texture->mWidth);
					content_begin = (char*)(texture->pcData);
				}
			}
#endif

			if (content_begin)
			{
				std::string sFileName = CParaFile::GetFileName(m_sFilename);
				diffuseTexName = CParaFile::GetParentDirectoryFromPath(diffuseTexName) + sFileName + "/" + CParaFile::GetFileName(diffuseTexName);

				m_textureContentMapping.insert(std::make_pair(diffuseTexName, std::string(content_begin, content_len)));
				// OUTPUT_LOG("embedded FBX texture %s used. size %d bytes\n", texname.c_str(), (int)m_textureContentMapping[texname].size());
			}
			else if (!CParaFile::DoesFileExist(diffuseTexName.c_str(), true))
			{
				bool bFound = false;
				if (CParaFile::IsAbsolutePath(sOriginalPath) || sOriginalPath[0] == '.')
				{
					// in case it is ../../Texture/abc.png, we will use relative path 
					if (sOriginalPath[1] == '.' && sOriginalPath[2] == '/')
					{
						// such as ../../
						std::string fullPath = CParaFile::GetParentDirectoryFromPath(diffuseTexName, 1);
						int nOffset = 3;
						while (sOriginalPath[nOffset] == '.' && sOriginalPath[nOffset + 1] == '.' && sOriginalPath[nOffset + 2] == '/')
						{
							fullPath = CParaFile::GetParentDirectoryFromPath(fullPath, 1);
							nOffset += 3;
						}
						fullPath.append(sOriginalPath.c_str() + nOffset);
						if (CParaFile::DoesFileExist(fullPath.c_str(), true))
						{
							diffuseTexName = fullPath;
							bFound = true;
						}
					}
					if (!bFound)
					{
						// search all parent directories for a possible global texture path. 
						auto nPos = sOriginalPath.find_first_of('/');
						int nCount = 0;
						while (nPos != std::string::npos)
						{
							if (nPos == 2 && sOriginalPath[0] == '.')
							{
								while (nPos == 2 && sOriginalPath[0] == '.') {
									sOriginalPath = sOriginalPath.substr(nPos + 1);
									nPos = sOriginalPath.find_first_of('/');
									++nCount;
								}
							}
							else
							{
								sOriginalPath = sOriginalPath.substr(nPos + 1);
								nPos = sOriginalPath.find_first_of('/');
								++nCount;
							}

							if (nCount > 1 && CParaFile::DoesFileExist(sOriginalPath.c_str(), true))
							{
								diffuseTexName = sOriginalPath;
								bFound = true;
								break;
							}
						}
					}
				}
				else if (CParaFile::DoesFileExist(sOriginalPath.c_str(), true))
				{
					bFound = true;
					diffuseTexName = sOriginalPath;
				}
				if (!bFound)
				{
					OUTPUT_LOG("warn: FBX texture %s not exist\n", diffuseTexName.c_str());
					diffuseTexName = g_sDefaultTexture;
				}
			}

			if (!content_begin && !diffuseTexName.empty() && CParaFile::IsAbsolutePath(diffuseTexName))
			{
				// try making it relative to project root
				const std::string& curDir = CParaFile::GetCurDirectory(0);
				if (curDir.size() < diffuseTexName.size() && diffuseTexName.compare(0, curDir.size(), curDir) == 0)
				{
					diffuseTexName = diffuseTexName.substr(curDir.size());
				}
			}
		}
		if (aiTextureType_DIFFUSE == textureType)
		{
			m_bUsedVertexColor = diffuseTexName.empty() && m_bUsedVertexColor;
			// parse material name
			FBXMaterial fbxMat;
			ParseMaterialByName(sMatName, &fbxMat);
			int16 blendmode = BM_OPAQUE;
			if (!diffuseTexName.empty())
			{
				if (AI_SUCCESS == aiGetMaterialTexture(pfbxMaterial, (aiTextureType)aiTextureType_OPACITY, 0, &szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL))
				{
					if (fbxMat.isAlphaBlended())
						blendmode = BM_ALPHA_BLEND;
					else
						blendmode = BM_TRANSPARENT;
				}
			}
			if (fbxMat.bAddictive) blendmode = BM_ADDITIVE;
			{
				float opacityValue;
				if (AI_SUCCESS == aiGetMaterialFloat(pfbxMaterial, AI_MATKEY_OPACITY, &opacityValue) && fabs(opacityValue - 1.0f) > FLT_EPSILON)
				{
					auto size = m_transparencys.size();
					opacity = (int16)size;
					m_transparencys.resize(size + 1);
					auto& tran = m_transparencys[size];
					tran.trans.used = false;
					tran.trans.type = 0;
					tran.trans.seq = -1;
					tran.trans.globals = NULL;
					tran.trans.ranges.push_back(AnimRange(0, 0));
					tran.trans.times.push_back(0);
					tran.trans.data.push_back(opacityValue);
					tran.trans.in.push_back(0.0f);
					tran.trans.out.push_back(0.0f);
				}
			}
			if (m_bUsedVertexColor)
			{
				diffuseTexName = std::string(g_sDefaultTexture);
			}
			fbxMat.m_filename = diffuseTexName;
			int texture_index = -1;
			for (int i = 0; i < (int)m_textures.size(); i++)
			{
				// ReplaceableTextureID usually [0-32)
				if (m_textures[i].nIsReplaceableIndex >= 0 && m_textures[i].nIsReplaceableIndex < 32 && m_textures[i].nIsReplaceableIndex == fbxMat.nIsReplaceableIndex)
				{
					texture_index = i;
					break;
				}

			}
			if (texture_index < 0)
			{
				for (int i = 0; i < (int)m_textures.size(); i++)
				{
					if (m_textures[i].m_filename == diffuseTexName)
					{
						texture_index = i;
						break;
					}
				}
				if (texture_index < 0)
				{
					m_textures.push_back(fbxMat);
					texture_index = m_textures.size() - 1;
				}
			}

			ModelRenderPass& pass = pMesh->passes[pMesh->passes.size() - 1];
			pass.tex = texture_index;
			pass.SetCategoryId(fbxMat.GetCategoryID());
			pass.texanim = -1;
			pass.color = -1;
			pass.opacity = opacity;
			pass.unlit = fbxMat.bUnlit;
			pass.nozwrite = fbxMat.bDisableZWrite;
			pass.disable_physics = fbxMat.bDisablePhysics;
			pass.force_physics = fbxMat.bForcePhysics;

			pass.blendmode = blendmode;
			pass.cull = blendmode == BM_OPAQUE ? true : false;
			pass.order = fbxMat.m_nOrder;
			pass.geoset = -1; // make its geoset uninitialized
			//*(((DWORD*)&(pass.geoset)) + 1) = parser.ReadInt();

			ParseUVAnimation(pass, pfbxMaterial, pMesh);
			ParseParticleEmitter(pass, pfbxMaterial, pMesh, sMatName, texture_index);
		}
		if (aiTextureType_NORMALS == textureType)
		{
			ModelRenderPass& pass = pMesh->passes[pMesh->passes.size() - 1];
			FBXMaterial fbxMat;
			ParseMaterialByName(sMatName, &fbxMat);
			fbxMat.m_filename = diffuseTexName;
			int texture_index = -1;
			for (int i = 0; i < (int)m_textures.size(); i++)
			{
				if (m_textures[i].m_filename == diffuseTexName)
				{
					texture_index = i;
					break;
				}
			}
			if (texture_index < 0)
			{
				m_textures.push_back(fbxMat);
				texture_index = m_textures.size() - 1;
			}
			pass.SetTexture2(texture_index);
		}
	}
}

lua_State* FBXParser::ParseScriptString(const char* str)
{
	if (!str)
		return nullptr;

	if (!m_pLuaState)
		m_pLuaState = luaL_newstate();

	int error = luaL_dostring(m_pLuaState, str);

	if (error == 0)
	{
		return m_pLuaState;
	}
	else
	{
		auto errString = lua_tostring(m_pLuaState, -1);
		OUTPUT_LOG("Error parsing: %s.\n %s", str, errString);
		lua_pop(m_pLuaState, 1); // remove error message from stack
		return nullptr;
	}
}

void FBXParser::ParseParticleEmitter(ModelRenderPass& pass, aiMaterial* pfbxMaterial, CParaXModel* pMesh, const std::string& sMatName, int texture_index)
{
#ifdef ASSIMP5
	aiString value;
	if (pfbxMaterial->Get("ps_param", (unsigned int)AI_AISTRING, 0, value) == AI_SUCCESS)
	{
		if (value.length == 0)
			return;

		std::string paramString(value.C_Str());
		paramString = "return {" + paramString;
		paramString += "}";


		auto L = ParseScriptString(paramString.c_str());
		if (!L)
			return;

		PE_ASSERT(m_particleSystem.find(sMatName) == m_particleSystem.end());

		m_particleSystem.insert(std::pair<std::string, ParticleSystem>(sMatName, ParticleSystem()));
		ParticleSystem& ps = m_particleSystem[sMatName];

		ParseParticleParam(ps, L);
		lua_pop(L, 1); // remove return value from stack

		ps.order = 0; // triangle winding order is fixed for all types.
		ps.model = pMesh;

		ps.grav2.globals = ps.areaw.globals = ps.areal.globals = ps.rate.globals = ps.lifespan.globals = ps.gravity.globals = ps.lat.globals = ps.spread.globals = ps.variation.globals = ps.speed.globals = pMesh->globalSequences;

		ps.tofs = frand();
		ps.m_texture_index = texture_index;
	}
#else
	auto metaData = pfbxMaterial->mMetaData;
	if (!metaData || metaData->mNumProperties == 0)
		return;

	for (unsigned int i = 0; i < metaData->mNumProperties; i++)
	{
		auto& key = metaData->mKeys[i];
		auto& value = metaData->mValues[i];

		if (strcmp(key.C_Str(), "ps_param") != 0)
			continue;

		PE_ASSERT(value.mType == AI_AISTRING);

		auto aiParam = static_cast<aiString*>(value.mData);

		if (!aiParam || aiParam->length == 0)
			continue;

		std::string paramString(aiParam->C_Str());
		paramString = "return {" + paramString;
		paramString += "}";


		auto L = ParseScriptString(paramString.c_str());
		if (!L)
			continue;

		PE_ASSERT(m_particleSystem.find(sMatName) == m_particleSystem.end());

		m_particleSystem.insert(std::pair<std::string, ParticleSystem>(sMatName, ParticleSystem()));
		ParticleSystem& ps = m_particleSystem[sMatName];

		ParseParticleParam(ps, L);
		lua_pop(L, 1); // remove return value from stack

		ps.order = 0; // triangle winding order is fixed for all types.
		ps.model = pMesh;

		ps.grav2.globals = ps.areaw.globals = ps.areal.globals = ps.rate.globals = ps.lifespan.globals = ps.gravity.globals = ps.lat.globals = ps.spread.globals = ps.variation.globals = ps.speed.globals = pMesh->globalSequences;

		ps.tofs = frand();
		ps.m_texture_index = texture_index;

		break;
	}
#endif
}

void FBXParser::ParseParticleParam(ParticleSystem& ps, lua_State* L)
{
	const float ticksPerSample = 1000.f / 30.f; // 30fps
	const float fEpsilon = 0.01f;

	// read mid
	{
		lua_pushstring(L, "mid");
		lua_gettable(L, -2);
		ps.mid = lua_isnil(L, -1) ? 0.5f : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	// read color
	{
		lua_pushstring(L, "color");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			size_t len = lua_objlen(L, -1);

			PE_ASSERT(len <= 3);

			for (size_t i = 0; i < len; i++)
			{
				auto& color = ps.colors[i];
				lua_pushnumber(L, i + 1);
				lua_gettable(L, -2);
				if (lua_istable(L, -1))
				{
					// read R
					lua_pushnumber(L, 1);
					lua_gettable(L, -2);
					color.x = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					// read G
					lua_pushnumber(L, 2);
					lua_gettable(L, -2);
					color.y = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
					lua_pop(L, 1);

					// read B
					lua_pushnumber(L, 3);
					lua_gettable(L, -2);
					color.z = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
					lua_pop(L, 1);
				}

				lua_pop(L, 1);
			}
		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: color must be table");
		}
		lua_pop(L, 1);
	}

	// read alpha
	{
		lua_pushstring(L, "alpha");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			size_t len = lua_objlen(L, -1);
			PE_ASSERT(len <= 3);
			for (size_t i = 0; i < len; i++)
			{
				auto& alpha = ps.colors[i].w;

				lua_pushnumber(L, i + 1);
				lua_gettable(L, -2);
				alpha = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}

		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: alpha must be table");
		}
		lua_pop(L, 1);
	}

	// read size
	{
		lua_pushstring(L, "size");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			size_t len = lua_objlen(L, -1);
			PE_ASSERT(len <= 3);
			for (size_t i = 0; i < len; i++)
			{
				auto& size = ps.sizes[i];

				lua_pushnumber(L, i + 1);
				lua_gettable(L, -2);
				size = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: size must be table");
		}
		lua_pop(L, 1);
	}

	// read rate 
	{
		lua_pushstring(L, "rate");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.rate.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.rate.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.rate.CompressKeyLinear(fEpsilon);
				ps.rate.used = true;
				ps.rate.seq = -2;
			}
			else
			{
				ps.rate.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}


		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: rate must be table");
		}
		lua_pop(L, 1);
	}

	// read speed
	{
		lua_pushstring(L, "speed");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.speed.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.speed.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.speed.CompressKeyLinear(fEpsilon);
				ps.speed.used = true;
				ps.speed.seq = -2;
			}
			else
			{
				ps.speed.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}


		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: speed must be table");
		}
		lua_pop(L, 1);
	}

	// read variation
	{
		lua_pushstring(L, "variation");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.variation.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.variation.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.variation.CompressKeyLinear(fEpsilon);
				ps.variation.used = true;
				ps.variation.seq = -2;
			}
			else
			{
				ps.variation.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}


		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: variation must be table");
		}
		lua_pop(L, 1);
	}


	// read lifeTime
	{
		lua_pushstring(L, "lifeTime");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.lifespan.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.lifespan.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.lifespan.CompressKeyLinear(fEpsilon);
				ps.lifespan.used = true;
				ps.lifespan.seq = -2;
			}
			else
			{
				ps.lifespan.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}


		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: lifeTime must be table");
		}
		lua_pop(L, 1);
	}

	// read emitterWidth
	{
		lua_pushstring(L, "emitterWidth");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.areaw.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.areaw.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.areaw.CompressKeyLinear(fEpsilon);
				ps.areaw.used = true;
				ps.areaw.seq = -2;
			}
			else
			{
				ps.areaw.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}
		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: emitterWidth must be table");
		}
		lua_pop(L, 1);
	}

	// read emitterHeight
	{
		lua_pushstring(L, "emitterHeight");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			std::vector<std::pair<int, float>> values;

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				auto key = lua_isnil(L, -2) ? 0 : (int)lua_tointeger(L, -2);
				auto value = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);

				values.push_back(std::pair<const int&, const float&>(key, value));

				lua_pop(L, 1);
			}

			std::sort(values.begin(), values.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
			{
				return a.first < b.first;
			});

			auto size = values.size();
			for (size_t i = 0; i < size; i++)
			{
				auto& one = values[i];
				ps.areal.AppendKey((int)(one.first * ticksPerSample), one.second);
			}

			if (size > 0)
			{
				ps.areal.SetRangeByAnimIndex(0, AnimRange(0, values[size - 1].first));
				ps.areal.CompressKeyLinear(fEpsilon);
				ps.areal.used = true;
				ps.areal.seq = -2;
			}
			else
			{
				ps.areal.SetRangeByAnimIndex(0, AnimRange(0, 0));
			}
		}
		else if (lua_isnil(L, -1))
		{

		}
		else
		{
			OUTPUT_LOG("warning: emitterHeight must be table");
		}
		lua_pop(L, 1);
	}

	// read pos
	{
		lua_pushstring(L, "pos");
		lua_gettable(L, -2);
		if (lua_istable(L, -1))
		{
			// read x
			lua_pushstring(L, "x");
			lua_gettable(L, -2);
			ps.pos.x = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_pushstring(L, "y");
			lua_gettable(L, -2);
			ps.pos.y = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_pushstring(L, "z");
			lua_gettable(L, -2);
			ps.pos.z = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		else if (lua_isnil(L, -1))
		{
			memset(&ps.pos, sizeof(ps.pos), 0);
		}
		else
		{
			OUTPUT_LOG("warning: emitterHeight must be table");
		}
		lua_pop(L, 1);
	}

	// read texRotateSpeed
	{
		lua_pushstring(L, "texRotateSpeed");
		lua_gettable(L, -2);
		float texRotateSpeed = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		ps.grav2.AppendKey(0, texRotateSpeed);
		ps.grav2.SetRangeByAnimIndex(0, AnimRange(0, 0));
		ps.grav2.used = true;
		ps.grav2.seq = -2;
	}

	// read row & cos
	{
		lua_pushstring(L, "row");
		lua_gettable(L, -2);
		ps.cols = lua_isnil(L, -1) ? 1 : lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "col");
		lua_gettable(L, -2);
		ps.rows = lua_isnil(L, -1) ? 1 : lua_tointeger(L, -1);
		lua_pop(L, 1);

		PE_ASSERT(ps.cols >= 1 && ps.rows >= 1);

		ps.SetTextureRowsCols(ps.rows, ps.cols);
	}

	// read displayType 
	{
		lua_pushstring(L, "displayType");
		lua_gettable(L, -2);
		ps.type = lua_isnil(L, -1) ? 0 : lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	// read emitterType 
	{
		lua_pushstring(L, "emitterType");
		lua_gettable(L, -2);
		auto typeString = lua_isnil(L, -1) ? "" : lua_tostring(L, -1);

		if (strcmp(typeString, "sphere") == 0)
		{
			//ps.type = (int)ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER;
			//ps.emitter = new SphereParticleEmitter(&ps);
			ps.emitter = (ParticleEmitter*)ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER;
		}
		else
		{
			//ps.type = (int)ParticleEmitter::TYPE_PLANE_PARTICLE_EMITTER;
			//ps.emitter = new PlaneParticleEmitter(&ps);
			ps.emitter = (ParticleEmitter*)ParticleEmitter::TYPE_PLANE_PARTICLE_EMITTER;
		}

		lua_pop(L, 1);
	}

	// read gravity 
	{
		lua_pushstring(L, "gravity");
		lua_gettable(L, -2);
		auto gravity = lua_isnil(L, -1) ? 9.8f : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		ps.gravity.AppendKey(0, gravity);
		ps.gravity.SetRangeByAnimIndex(0, AnimRange(0, 0));
		ps.gravity.used = true;
		ps.gravity.seq = -2;
	}

	// read forceRandom
	{
		lua_pushstring(L, "forceRandom");
		lua_gettable(L, -2);
		auto forceRandom = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		ps.spread.AppendKey(0, forceRandom);
		ps.spread.SetRangeByAnimIndex(0, AnimRange(0, 0));
		ps.spread.used = true;
		ps.spread.seq = -2;
	}

	// read latitude
	{
		lua_pushstring(L, "latitude");
		lua_gettable(L, -2);
		auto latitude = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		ps.lat.AppendKey(0, latitude);
		ps.lat.SetRangeByAnimIndex(0, AnimRange(0, 0));
		ps.lat.used = true;
		ps.lat.seq = -2;
	}

	// read slowdown
	{
		lua_pushstring(L, "slowdown");
		lua_gettable(L, -2);
		ps.slowdown = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	// read billboard
	{
		lua_pushstring(L, "billboard");
		lua_gettable(L, -2);
		ps.billboard = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);
	}

	// read blend 
	{
		lua_pushstring(L, "blend");
		lua_gettable(L, -2);
		ps.blend = lua_isnil(L, -1) ? 0 : lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	// read rotation 
	{
		lua_pushstring(L, "rotation");
		lua_gettable(L, -2);
		ps.rotation = lua_isnil(L, -1) ? 0 : (float)lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	// read rotate2SpeedDirection 
	{
		lua_pushstring(L, "rotate2SpeedDirection");
		lua_gettable(L, -2);
		ps.rotate2SpeedDirection = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);
	}
}

void FBXParser::ParseUVAnimation(ModelRenderPass& pass, aiMaterial* pfbxMaterial, CParaXModel* pMesh)
{
#ifdef ASSIMP5
	if (pfbxMaterial->mNumProperties == 0)
		return;

	const float error_trans = 0.00001f;
	const float error_scale = error_trans * 10;
	const float error_rot = 0.0001f;

	const float ticksPerSample = 1000.f / 30.f; // 30fps

	std::vector<std::pair<int, Vector3>> trans;
	std::vector<std::pair<int, Vector3>> scales;
	std::vector<std::pair<int, Vector3>> rots;

	for (unsigned int i = 0; i < pfbxMaterial->mNumProperties; i++)
	{
		auto property = pfbxMaterial->mProperties[i];

		auto& key = property->mKey;
		auto value = property->mData;

		auto keyFrame = strstr(key.C_Str(), "TexAnims_key");
		if (!keyFrame)
			continue;

		PE_ASSERT((key.length == strlen("TexAnims_key000_r")));
		PE_ASSERT(property->mType == AI_AIVECTOR3D);

		keyFrame += strlen("TexAnims_key");

		int keyIndex = (keyFrame[0] - '0') * 100 + (keyFrame[1] - '0') * 10 + (keyFrame[2] - '0');
		char type = keyFrame[4];
		PE_ASSERT(type == 't' || type == 'r' || type == 's');
		PE_ASSERT(sizeof(aiVector3D) == property->mDataLength);

		auto aiVec3 = reinterpret_cast<aiVector3D*>(value);

		switch (type)
		{
		case 't':
		{
			// translation
			trans.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x, aiVec3->y, aiVec3->z)));
			break;
		}
		case 'r':
		{
			// rotation
			rots.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x * 3.1415926f / 180
				, aiVec3->y
				, aiVec3->z)));
			break;
		}
		case 's':
		{
			// scaling
			scales.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x, aiVec3->y, aiVec3->z)));
			break;
		}
		default:
			break;
		}

	}

	if (trans.size() == 0 && rots.size() == 0 && scales.size() == 0)
		return;

	auto index = m_texAnims.size();
	m_texAnims.resize(index + 1);
	TextureAnim& anim = m_texAnims[index];
	pass.texanim = (int16)index;

	anim.rot.globals = anim.scale.globals = anim.trans.globals = pMesh->globalSequences;

	auto sortFunc = [](const std::pair<int, Vector3>& a, const std::pair<int, Vector3>& b)
	{
		return a.first < b.first;
	};

	std::sort(trans.begin(), trans.end(), sortFunc);
	std::sort(rots.begin(), rots.end(), sortFunc);
	std::sort(scales.begin(), scales.end(), sortFunc);

	std::vector<std::pair<int, Vector3>>* pVec[3] = { &trans, &rots, &scales };
	Animated<Vector3>* pAnimated[3] = { &anim.trans, &anim.rot, &anim.scale };
	float errorValues[3] = { error_trans , error_rot, error_scale };
	for (int times = 0; times < 3; times++)
	{
		auto& vec = *pVec[times];
		auto& animated = *pAnimated[times];
		auto& errorValue = errorValues[times];

		if (vec.size() > 0)
		{
			// In ParaX Model, UV animations are always global sequence with seq id equal to -2.
			// There is only one animation range at index 0, hence nCurAnimIndex=0
			animated.seq = -2;
			animated.used = true;

			int nKeyCount, nFirstKeyIndex, nLastKeyIndex, i;
			nKeyCount = (int)vec.size();
			nLastKeyIndex = nFirstKeyIndex = 0;

			int timeStart = (int)(vec[0].first * ticksPerSample);
			auto lastRotKey = vec[0].second;
			auto lastlastRotKey = lastRotKey;

			animated.AppendKey(timeStart, lastRotKey);

			for (i = 1; i < nKeyCount; i++)
			{
				auto& rotationKey = vec[i].second;
				auto predicatedKey = lastRotKey * 2 - lastlastRotKey;
				auto delta = rotationKey - predicatedKey;

				int time = (int)(vec[i].first * ticksPerSample);

				/*2006.9.6 we must export every translation key. even a very small delta will cause large error in the exported animation.
				e.g. //if(fabs(delta.x)>0.00001f || fabs(delta.y)>0.00001f || fabs(delta.z)>0.00001f || fabs(delta.w)>0.00001f) can cause large displacement of the mesh.
				*/
				if (fabs(delta.x) > errorValue || fabs(delta.y) > errorValue || fabs(delta.z) > errorValue)
				{
					// add new key
					if (nLastKeyIndex == nFirstKeyIndex)
					{
						// if this is the second key, modify the first key's time to animSequence.timeStart, and insert a constant key if necessary.
						int nKeyIndex = animated.GetKeyNum() - 1;
						int nTime = animated.times[nKeyIndex];
						animated.times[nKeyIndex] = timeStart;

						if (i > 2)
						{
							animated.AppendKey(nTime, lastRotKey);
						}

					}

					animated.AppendKey(time, rotationKey);
					predicatedKey = rotationKey;
					++nLastKeyIndex;
				}
				else
				{
					// override the last key
					animated.UpdateLastKey(time, predicatedKey);
				}

				lastlastRotKey = lastRotKey;
				lastRotKey = predicatedKey;
			}

			if (nFirstKeyIndex == nLastKeyIndex)
				animated.UpdateLastKey(timeStart, lastRotKey);
			animated.SetRangeByAnimIndex(index, AnimRange(nFirstKeyIndex, nLastKeyIndex));
		}
	}
#else
	auto metaData = pfbxMaterial->mMetaData;
	if (!metaData || metaData->mNumProperties == 0)
		return;

	const float error_trans = 0.00001f;
	const float error_scale = error_trans * 10;
	const float error_rot = 0.0001f;

	const float ticksPerSample = 1000.f / 30.f; // 30fps

	std::vector<std::pair<int, Vector3>> trans;
	std::vector<std::pair<int, Vector3>> scales;
	std::vector<std::pair<int, Vector3>> rots;

	for (unsigned int i = 0; i < metaData->mNumProperties; i++)
	{
		auto& key = metaData->mKeys[i];
		auto& value = metaData->mValues[i];

		auto keyFrame = strstr(key.C_Str(), "TexAnims_key");
		if (!keyFrame)
			continue;

		PE_ASSERT((key.length == strlen("TexAnims_key000_r")));
		PE_ASSERT(value.mType == AI_AIVECTOR3D);

		keyFrame += strlen("TexAnims_key");

		int keyIndex = (keyFrame[0] - '0') * 100 + (keyFrame[1] - '0') * 10 + (keyFrame[2] - '0');
		char type = keyFrame[4];
		PE_ASSERT(type == 't' || type == 'r' || type == 's');

		auto aiVec3 = static_cast<aiVector3D*>(value.mData);
		//auto time = ticksPerSample * keyIndex;

		switch (type)
		{
		case 't':
		{
			// translation
			trans.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x, aiVec3->y, aiVec3->z)));
			break;
		}
		case 'r':
		{
			// rotation
			rots.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x * 3.1415926f / 180
				, aiVec3->y
				, aiVec3->z)));
			break;
		}
		case 's':
		{
			// scaling
			scales.push_back(std::pair<int, Vector3>(keyIndex, Vector3(aiVec3->x, aiVec3->y, aiVec3->z)));
			break;
		}
		default:
			break;
		}
	}

	if (trans.size() == 0 && rots.size() == 0 && scales.size() == 0)
		return;

	auto index = m_texAnims.size();
	m_texAnims.resize(index + 1);
	TextureAnim& anim = m_texAnims[index];
	pass.texanim = (int16)index;

	anim.rot.globals = anim.scale.globals = anim.trans.globals = pMesh->globalSequences;

	auto sortFunc = [](const std::pair<int, Vector3>& a, const std::pair<int, Vector3>& b)
	{
		return a.first < b.first;
	};

	std::sort(trans.begin(), trans.end(), sortFunc);
	std::sort(rots.begin(), rots.end(), sortFunc);
	std::sort(scales.begin(), scales.end(), sortFunc);

	std::vector<std::pair<int, Vector3>>* pVec[3] = { &trans, &rots, &scales };
	Animated<Vector3>* pAnimated[3] = { &anim.trans, &anim.rot, &anim.scale };
	float errorValues[3] = { error_trans , error_rot, error_scale };
	for (int times = 0; times < 3; times++)
	{
		auto& vec = *pVec[times];
		auto& animated = *pAnimated[times];
		auto& errorValue = errorValues[times];

		if (vec.size() > 0)
		{
			// In ParaX Model, UV animations are always global sequence with seq id equal to -2.
			// There is only one animation range at index 0, hence nCurAnimIndex=0
			animated.seq = -2;
			animated.used = true;

			int nKeyCount, nFirstKeyIndex, nLastKeyIndex, i;
			nKeyCount = (int)vec.size();
			nLastKeyIndex = nFirstKeyIndex = 0;

			int timeStart = (int)(vec[0].first * ticksPerSample);
			auto lastRotKey = vec[0].second;
			auto lastlastRotKey = lastRotKey;

			animated.AppendKey(timeStart, lastRotKey);

			for (i = 1; i < nKeyCount; i++)
			{
				auto& rotationKey = vec[i].second;
				auto predicatedKey = lastRotKey * 2 - lastlastRotKey;
				auto delta = rotationKey - predicatedKey;

				int time = (int)(vec[i].first * ticksPerSample);

				/*2006.9.6 we must export every translation key. even a very small delta will cause large error in the exported animation.
				e.g. //if(fabs(delta.x)>0.00001f || fabs(delta.y)>0.00001f || fabs(delta.z)>0.00001f || fabs(delta.w)>0.00001f) can cause large displacement of the mesh.
				*/
				if (fabs(delta.x) > errorValue || fabs(delta.y) > errorValue || fabs(delta.z) > errorValue)
				{
					// add new key
					if (nLastKeyIndex == nFirstKeyIndex)
					{
						// if this is the second key, modify the first key's time to animSequence.timeStart, and insert a constant key if necessary.
						int nKeyIndex = animated.GetKeyNum() - 1;
						int nTime = animated.times[nKeyIndex];
						animated.times[nKeyIndex] = timeStart;

						if (i > 2)
						{
							animated.AppendKey(nTime, lastRotKey);
						}

					}

					animated.AppendKey(time, rotationKey);
					predicatedKey = rotationKey;
					++nLastKeyIndex;
				}
				else
				{
					// override the last key
					animated.UpdateLastKey(time, predicatedKey);
				}

				lastlastRotKey = lastRotKey;
				lastRotKey = predicatedKey;
			}

			if (nFirstKeyIndex == nLastKeyIndex)
				animated.UpdateLastKey(timeStart, lastRotKey);
			animated.SetRangeByAnimIndex(index, AnimRange(nFirstKeyIndex, nLastKeyIndex));
		}
	}
#endif
}

void FBXParser::ProcessStaticFBXMesh(aiMesh* pFbxMesh, XFile::Mesh* pMesh)
{
	int numVertices = pFbxMesh->mNumVertices;
	pMesh->mPositions.resize(numVertices);
	pMesh->mNormals.resize(numVertices);
	std::vector<Vector2>& coords = pMesh->mTexCoords[pMesh->mNumTextures++];
	coords.resize(numVertices);
	//pMesh->mTexCoords.resize(numVertices);

	aiVector3D* uvs = nullptr;
	if (pFbxMesh->HasTextureCoords(0))
		uvs = pFbxMesh->mTextureCoords[0];

	int i;
	for (i = 0; i < numVertices; i++)
	{
		pMesh->mPositions[i] = ConvertFBXVector3D(pFbxMesh->mVertices[i]);
		pMesh->mNormals[i] = ConvertFBXVector3D(pFbxMesh->mNormals[i]);
		if (uvs)
			coords[i] = Vector2(uvs[i].x, uvs[i].y);
		else
			coords[i] = Vector2(0.0f, 0.0f);
	}

	int numFaces = pFbxMesh->mNumFaces;
	pMesh->mPosFaces.resize(numFaces);
	pMesh->mNormFaces.resize(numFaces);
	for (i = 0; i < numFaces; i++)
	{
		unsigned int* faceIndices = pFbxMesh->mFaces[i].mIndices;

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

void FBXParser::ConvertFBXBone(ParaEngine::Bone& bone, const aiBone* pfbxBone)
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
	if (!m_bHasAABBMesh)
	{
		if (v.x > m_maxExtent.x) m_maxExtent.x = v.x;
		if (v.y > m_maxExtent.y) m_maxExtent.y = v.y;
		if (v.z > m_maxExtent.z) m_maxExtent.z = v.z;

		if (v.x < m_minExtent.x) m_minExtent.x = v.x;
		if (v.y < m_minExtent.y) m_minExtent.y = v.y;
		if (v.z < m_minExtent.z) m_minExtent.z = v.z;
	}
}


int ParaEngine::FBXParser::CreateGetBoneIndex(const char* pNodeName)
{
	int nBoneIndex = -1;
	auto it = m_boneMapping.find(pNodeName);
	if (it != m_boneMapping.end())
	{
		nBoneIndex = it->second;
	}
	else
	{
		nBoneIndex = m_bones.size();
		m_bones.resize(nBoneIndex + 1);

		ParaEngine::Bone& bone = m_bones.back();
		bone.nIndex = nBoneIndex;
		bone.bUsePivot = false;
		bone.SetName(pNodeName);
		bone.AutoSetBoneInfoFromName();
		// if bone already has an ID, it is always a non-transform node. 
		if (bone.GetBoneID() <= 0)
			bone.flags = ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
		m_boneMapping[pNodeName] = bone.nIndex;
	}
	return nBoneIndex;
}

/**
Each geoset has an id of format "ccdd", where "cc" is the geoset category CharModelInstance::CharGeosets
and "dd" is individual geoset style starting from 1 of the given category.
if id == 0, it means an always displayed mesh. There can be at most one geoset in the same category to show at the same time.
For example, either 501 or 502 can be shown at the same time. Please note, multiple geosets can share the same id.

read geoset id from the mesh name, the name is of format [name|number]_[number], such as
"Hair_01" = "00_01" = "1",
"hair_02" = "00_02" = "2",
"Boots" = "05_01" = "5_1" = "501",
"Boots_02" = "05_02" = "502",
"Unknown name" = "0",

name: can be a number or a string, string is interpreted to a number cc. by the following mapping (case insensitive)
"hair" = 0,
"facialhair" = 1,
"eyeaddon" = 2,
"head" = 3,
"hand" = "gloves" = 4,
"boots" = 5,
"ears" = 7,
"shirt" = "armsleeves" = "sleeves" = = 8,
"pants" = 9,
"wings" = 10,
"tabard" = 12,
"robe" = 13,
"skirt" = 14,
"cape" = 15,
*/
int GetGeosetIdFromMeshName(const char* meshname)
{
	int geosetID = 0;
	if (meshname)
	{
		int cc = 0;
		int dd = 0;
		char c = meshname[0];
		if (c >= '0' && c <= '9')
		{
			for (int i = 0; meshname[i] != '\0'; ++i)
			{
				c = meshname[i];
				if (c == '_')
				{
					++i;
					for (; meshname[i] != '\0'; ++i)
					{
						c = meshname[i];
						if (c >= '0' && c <= '9')
						{
							dd = dd * 10 + (int)(c - '0');
						}
					}
					break;
				}
				else if (c >= '0' && c <= '9')
				{
					cc = cc * 10 + (int)(c - '0');
				}
				else
				{
					// unknown name
					cc = 0;
					break;
				}
			}
		}
		else
		{
			char prename[256];
			int j = 0;
			memset(prename, 0, sizeof(prename));
			for (int i = 0; meshname[i] != '\0'; ++i)
			{
				c = meshname[i];
				if (meshname[i] == '_')
				{
					++i;
					for (; meshname[i] != '\0'; ++i)
					{
						c = meshname[i];
						if (c >= '0' && c <= '9')
						{
							dd = dd * 10 + (int)(c - '0');
						}
					}
					break;
				}
				else
				{
					// to lower case
					if (c >= 'A' && c <= 'Z')
						c += ('a' - 'A');
					prename[j++] = c;
				}
			}
			prename[j] = '\0';

			enum CharGeosets {
				CSET_HAIR = 0,
				CSET_FACIAL_HAIR1 = 1,
				CSET_FACIAL_HAIR2 = 2,
				CSET_FACIAL_HAIR3 = 3,
				CSET_GLOVES = 4,
				CSET_BOOTS = 5,
				CSET_EARS = 7,
				CSET_ARM_SLEEVES = 8,
				CSET_PANTS = 9,
				CSET_WINGS = 10, // newly added 2007.7.7
				CSET_TABARD = 12,
				CSET_ROBE = 13,
				CSET_SKIRT = 14,
				CSET_CAPE = 15,
				// TOTAL
				NUM_CHAR_GEOSETS = 16
			};
			if (strcmp(prename, "hair") == 0)
			{
				cc = CSET_HAIR;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "facialhair") == 0)
			{
				cc = CSET_FACIAL_HAIR1;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "eyeaddon") == 0)
			{
				cc = CSET_FACIAL_HAIR2;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "head") == 0)
			{
				cc = CSET_FACIAL_HAIR3;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "gloves") == 0 || strcmp(prename, "hand") == 0)
			{
				cc = CSET_GLOVES;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "boots") == 0)
			{
				cc = CSET_BOOTS;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "ears") == 0)
			{
				cc = CSET_EARS;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "armsleeves") == 0 || strcmp(prename, "sleeves") == 0 || strcmp(prename, "shirt") == 0)
			{
				cc = CSET_ARM_SLEEVES;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "pants") == 0)
			{
				cc = CSET_PANTS;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "wings") == 0)
			{
				cc = CSET_WINGS;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "tabard") == 0)
			{
				cc = CSET_TABARD;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "robe") == 0)
			{
				cc = CSET_ROBE;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "skirt") == 0)
			{
				cc = CSET_SKIRT;
				dd = (dd == 0) ? 1 : dd;
			}
			else if (strcmp(prename, "cape") == 0)
			{
				cc = CSET_CAPE;
				dd = (dd == 0) ? 1 : dd;
			}
			else
			{
				cc = 0;
			}
		}
		// compose the final id
		geosetID = cc * 100 + dd;
	}
	return geosetID;
}

void FBXParser::ProcessFBXMesh(const aiScene* pFbxScene, aiMesh* pFbxMesh, aiNode* pFbxNode, CParaXModel* pMesh)
{
	int index_start = m_indices.size();
	int vertex_start = m_vertices.size();
	int numFaces = pFbxMesh->mNumFaces;
	int numVertices = pFbxMesh->mNumVertices;

	aiVector3D* uvs = NULL;
	if (pFbxMesh->HasTextureCoords(0))
		uvs = pFbxMesh->mTextureCoords[0];
	std::string nodeName(pFbxNode->mName.C_Str());
	int nBoneIndex = CreateGetBoneIndex(nodeName.c_str());

	bool bIsAABBMesh = false;
	if (nodeName == "aabb")
	{
		bIsAABBMesh = true;
		ResetAABB();
		m_bHasAABBMesh = false;
		for (int i = 0; i < numVertices; i++)
		{
			auto pos = ConvertFBXVector3D(pFbxMesh->mVertices[i]);
			CalculateMinMax(pos);
		}
		m_bHasAABBMesh = true;

		// add bones
		if (pFbxMesh->HasBones())
		{
			int numBones = pFbxMesh->mNumBones;
			for (int i = 0; i < numBones; i++)
			{
				const aiBone* fbxBone = pFbxMesh->mBones[i];
				int nBoneIndex = CreateGetBoneIndex(fbxBone->mName.C_Str());
				if (nBoneIndex >= 0)
				{
					ParaEngine::Bone& bone = m_bones[nBoneIndex];
					const Matrix4& offsetMat = reinterpret_cast<const Matrix4&>(fbxBone->mOffsetMatrix);
					bone.matOffset = offsetMat.transpose();
					bone.flags |= ParaEngine::Bone::BONE_OFFSET_MATRIX;
					bone.flags &= ~ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
					bone.pivot = Vector3(0, 0, 0) * bone.matOffset.inverse();
				}
			}
		}
		return;
	}

	// add vertices
	if (numVertices > 0)
		m_vertices.reserve(m_vertices.size() + numVertices);

	// check diffuse color
	DWORD dwDiffuseColor = Color::White;
	aiMaterial* useMaterial = pFbxScene->mMaterials[pFbxMesh->mMaterialIndex];
	aiTextureOp eOp;
	aiString szPath;
	unsigned int iUV;
	float fBlend;
#ifdef ASSIMP5
	aiGetMaterialTexture(useMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL);
#else
	char* content_begin = NULL;
	int content_len = -1;
	aiGetMaterialTexture(useMaterial, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&szPath, NULL, &iUV, &fBlend, &eOp, NULL, NULL, &content_begin, &content_len);
#endif
	std::string diffuseTexName(szPath.C_Str());
	if (diffuseTexName == "")
	{
		aiColor4D diffuseColor;
		aiGetMaterialColor(useMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);

		dwDiffuseColor = LinearColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f);
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
		while (numFaces > 0 && (++nSplitCount) < 100)
		{
			ModelGeoset geoset;
			geoset.id = GetGeosetIdFromMeshName(pFbxNode->mName.C_Str());

			vertex_start = nVertexOffset;
			int nFaceCount = (std::min)(maxFaceCount, numFaces);
			if (numFaces > maxFaceCount || nSplitCount > 1)
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
				const aiFace& fbxFace = pFbxMesh->mFaces[i + nFaceStart];
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

		vertex_start = nVertexOffset;
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
			const aiBone* fbxBone = pFbxMesh->mBones[i];
			int nBoneIndex = CreateGetBoneIndex(fbxBone->mName.C_Str());

			if (nBoneIndex >= 0)
			{
				ParaEngine::Bone& bone = m_bones[nBoneIndex];
				const Matrix4& offsetMat = reinterpret_cast<const Matrix4&>(fbxBone->mOffsetMatrix);
				bone.matOffset = offsetMat.transpose();
				bone.flags |= ParaEngine::Bone::BONE_OFFSET_MATRIX;
				bone.flags &= ~ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
				bone.pivot = Vector3(0, 0, 0) * bone.matOffset.inverse();
			}

			for (int j = 0; j < (int)fbxBone->mNumWeights; j++)
			{
				aiVertexWeight& vertexWeight = fbxBone->mWeights[j];
				int vertex_id = vertexWeight.mVertexId + vertex_start;
				uint8 vertex_weight = (uint8)(vertexWeight.mWeight * 255);
				int nTotalWeight = 0;
				int bone_index = 0;
				ModelVertex& vertex = m_vertices[vertex_id];
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

	for (ParaEngine::Bone& bone : m_bones)
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
				if (time <= timeStart) {
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
				if (time <= timeStart) {
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
				if (time <= timeStart) {
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

void FBXParser::ProcessFBXAnimation(const aiScene* pFbxScene, unsigned int nIndex, CParaXModel* pMesh)
{
	aiAnimation* pFbxAnim = pFbxScene->mAnimations[nIndex];

	float fTimeScale = (float)(1000.f / pFbxAnim->mTicksPerSecond);

	int nodeChannelNum = pFbxAnim->mNumChannels;

	for (int j = 0; j < nodeChannelNum; j++)
	{
		aiNodeAnim* nodeChannel = pFbxAnim->mChannels[j];
		int bone_index = CreateGetBoneIndex(nodeChannel->mNodeName.C_Str());
		if (bone_index >= 0)
		{
			ParaEngine::Bone& bone = m_bones[bone_index];
			bone.flags |= ParaEngine::Bone::BONE_OFFSET_MATRIX;
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
			ParaEngine::AnimInfo& animinfo = m_modelInfo.m_Anims[i];
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

		//if (pFbxNode->mIsComplex)
		//{
		//	bone.flags |= ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
		//}
	}
	m_bones[bone_index].parent = parentBoneIndex;

	bool bVisible = true;
	auto metaData = pFbxNode->mMetaData;
	if (metaData)
	{
		for (unsigned int i = 0; i < metaData->mNumProperties; i++)
		{
			auto& key = metaData->mKeys[i];
			auto& value = metaData->mValues[i];
			const char* pKeyStr = key.C_Str();
			if (strcmp(pKeyStr, "Show") == 0)
			{
				PE_ASSERT(value.mType == AI_BOOL);
				bVisible = *static_cast<bool*>(value.mData);
				continue;
			}

			//ps_material
			if (strstr(pKeyStr, "ps_material") == nullptr)
				continue;

			PE_ASSERT(value.mType == AI_AISTRING);

			auto matName = static_cast<aiString*>(value.mData)->C_Str();
			auto it = m_particleSystem.find(matName);
			if (it == m_particleSystem.end())
				continue;

			ParticleSystem& ps = it->second;
			ps.parent = (Bone*)&(m_bones[bone_index]);
		}
	}

	if (bVisible)
	{
		// process any mesh on the node
		int numMeshes = pFbxNode->mNumMeshes;
		for (int i = 0; i < numMeshes; i++)
		{
			ProcessFBXMesh(pFbxScene, pFbxScene->mMeshes[pFbxNode->mMeshes[i]], pFbxNode, pMesh);
		}
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
		if (bone.rot.used) {
			Quaternion rot = bone.rot.data[0];
			OUTPUT_LOG("\t\t quat(%d): %.4f %.4f %.4f %.4f\n", (int)bone.rot.data.size(), rot.x, rot.y, rot.z, rot.w);
		}
		if (bone.trans.used) {
			Vector3 v = bone.trans.data[0];
			OUTPUT_LOG("\t\t trans(%d): %.4f %.4f %.4f\n", (int)bone.trans.data.size(), v.x, v.y, v.z);
		}
		if (bone.scale.used) {
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
	int nBoneCount = (int)m_bones.size();
	if (nBoneCount > 0)
	{
		// root transform is usually identity for most cases
		Quaternion q(Radian(-ParaEngine::Math::PI / 2.0f), Vector3::UNIT_X);
		Matrix3 matRot;
		q.ToRotationMatrix(matRot);

		for (int i = 0; i < nBoneCount; ++i)
		{
			Bone& bone = m_bones[i];
			if (bone.parent < 0)
			{
				if (bone.IsOffsetMatrixBone())
				{
					Matrix4 mat = bone.matOffset;
					bone.matOffset = bone.matOffset * matRot;
				}
				else
				{
					Matrix4 mat = bone.matTransform;
					if (bone.IsStaticTransform())
						mat = mat * matRot;
					else
						mat = matRot;
					bone.SetStaticTransform(mat);
				}
			}
		}
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
