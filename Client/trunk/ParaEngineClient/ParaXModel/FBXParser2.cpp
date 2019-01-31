#include "ParaEngine.h"

#include "FBXParser2.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXBone.h"

#include "assimp/scene.h"

namespace ParaEngine
{
	inline Vector3 ConvertFBXVector3D2(const aiVector3D& fbxVector3D)
	{
		return Vector3(fbxVector3D.x, fbxVector3D.y, fbxVector3D.z);
	}

	inline Quaternion ConvertFBXQuaternion2(const aiQuaternion& fbxQuaternion)
	{
		return Quaternion(-fbxQuaternion.x, -fbxQuaternion.y, -fbxQuaternion.z, fbxQuaternion.w);
	}
}

ParaEngine::FBXMaterial2::FBXMaterial2()
	: strDiffuseTexName("")
	, nDiffuseTexIndex(-1)
	, bHasOpacityTex(false)
	, fAlphaTestingRef(0.5f)
	, bDisableZWrite(false)
	, bForceLocalTranparency(false)
	, m_nCategoryID(0)
	, m_nOrder(0)
	, bDisablePhysics(false)
	, bForcePhysics(false)
	, bUnlit(false)
	, bAddictive(false)
	, bBillboard(false)
	, bAABillboard(false)
	, nForceUnique(0)
	, nIsReplaceableIndex(-1)
	, opacity(-1)
	, bIsUsed(false)
{
	
}

//ParaEngine::FBXNode2::FBXNode2()
//	: index(-1)
//	, parentIndex(-1)
//	, strNodeName("")
//	, mLocalMatrix(Matrix4::IDENTITY)
//	, mGlobalMatrix(Matrix4::IDENTITY)
//{
//
//}

//////////////////////////////////////////////////////////////////////////
//ParaEngine::FBXParser2::FBXParser2()
//	:m_pFBXScene(NULL)
//{
//
//}

ParaEngine::FBXParser2::FBXParser2(const string& path)
	: m_strFilePath(path)
	, m_pFBXScene(NULL)
	, minExtent(FLT_MAX)
	, maxExtent(-FLT_MAX)
{
	m_strAnimConfig = std::string(m_strFilePath.c_str(), m_strFilePath.size() - 3) + "xml";
}

ParaEngine::FBXParser2::~FBXParser2()
{

}

ParaEngine::CParaXModel* ParaEngine::FBXParser2::Parse(const aiScene* pFBXScene, const aiNode* pStartNode)
{
	m_pFBXScene = pFBXScene;
	if (m_pFBXScene)
	{
		for (int i = 0; i < m_pFBXScene->mNumMaterials; i++)
		{
			ParseMaterial(i, m_pFBXScene->mMaterials[i]);
		}

		for (int i = 0; i < m_pFBXScene->mNumAnimations; i++)
		{
			ParseAnimation(m_pFBXScene->mAnimations[i]);
		}

		ParseNode(pStartNode, -1);

		CParaXModel* pModel = NULL;
		if (/*!ReachMaxMaterials() &&*/ HasMesh())
		{
			ParaXHeaderDef m_xheader;
			m_xheader.IsAnimated = m_pFBXScene->HasAnimations() ? 1 : 0;
			pModel = new CParaXModel(m_xheader);
			//pModel->m_strName = pStartNode->mName.C_Str();
			FillData(pModel);
		}
		ClearData();
		return pModel;
	}

	return NULL;
}

void ParaEngine::FBXParser2::ParseMaterial(int nMatIndex, const aiMaterial* pMat)
{
	aiString aiMatName;
	aiGetMaterialString(pMat, AI_MATKEY_NAME, &aiMatName);

	aiString aiOpacityPath;
	char* pOpacityContent = NULL;
	int nOpacityContentLength = -1;
	bool bHasOpacityTex = false;
	if (AI_SUCCESS == aiGetMaterialTexture(pMat, (aiTextureType)aiTextureType_OPACITY, 0,
		&aiOpacityPath, NULL, NULL, NULL, NULL, NULL, NULL, &pOpacityContent, &nOpacityContentLength))
	{
		bHasOpacityTex = true;
	}

	aiString aiDiffusePath;
	char* pDiffuseContent = NULL;
	int nDiffuseContentLength = -1;
	aiGetMaterialTexture(pMat, (aiTextureType)aiTextureType_DIFFUSE, 0,
		&aiDiffusePath, NULL, NULL, NULL, NULL, NULL, NULL, &pDiffuseContent, &nDiffuseContentLength);

	int opacity = -1;
	{
		float opacityValue;
		if (AI_SUCCESS == aiGetMaterialFloat(pMat, AI_MATKEY_OPACITY, &opacityValue) && fabs(opacityValue - 1.0f) > FLT_EPSILON)
		{
			auto size = m_vecTransparencys.size();
			opacity = (int16)size;
			m_vecTransparencys.resize(size + 1);
			auto& tran = m_vecTransparencys[size];

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
	//////////////////////////////////////////////////////////////////////////
	string strMatName = aiMatName.C_Str();
	FBXMaterial2 mat;
	ParseMaterialByName(strMatName, &mat);
	mat.bHasOpacityTex = bHasOpacityTex;
	mat.opacity = opacity;
	TextureEntity* pTexEntity = NULL;
	string strDiffuseTexName = m_strFilePath;
	string strDiffusePath = aiDiffusePath.C_Str();
	if ("" != strDiffusePath)
	{
		if (pDiffuseContent)
		{
			// 内嵌贴图使用：文件路径+生成的名字
			strDiffuseTexName = strDiffuseTexName + "/" + strDiffusePath;
		}
		else
		{
			// 外置贴图使用：模型中读取出来的path
			strDiffuseTexName = strDiffusePath;
		}
		mat.strDiffuseTexName = strDiffuseTexName;
		int diffuseTexIndex = GetDiffuseTexIndex(strDiffuseTexName);
		if (-1 != diffuseTexIndex)
		{
			mat.nDiffuseTexIndex = diffuseTexIndex;
		}
		else
		{
			// 比较贴图内容判断贴图是否已经存在
			if (pDiffuseContent)
			{
				for (int i = 0; i < m_vecTexEnity.size(); ++i)
				{
					int nRawDataSize = m_vecTexEnity[i]->GetRawDataSize();
					if (nRawDataSize == nDiffuseContentLength)
					{
						char* pRawData = m_vecTexEnity[i]->GetRawData();
						if (!memcmp(pRawData, pDiffuseContent, nRawDataSize))
						{
							pTexEntity = m_vecTexEnity[i];
							break;
						}
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////
			if (!pTexEntity)
			{
				pTexEntity = CGlobals::GetAssetManager()->GetTextureManager().GetEntity(strDiffuseTexName);
				if (!pTexEntity)
				{
					if (pDiffuseContent)
					{
						// 内嵌贴图
						pTexEntity = CGlobals::GetAssetManager()->GetTextureManager().NewEntity(strDiffuseTexName);
						char* bufferCpy = new char[nDiffuseContentLength];
						memcpy(bufferCpy, pDiffuseContent, nDiffuseContentLength);
						pTexEntity->SetRawData(bufferCpy, nDiffuseContentLength);
						//pTexEntity->SetRawDataForImage(pDiffuseContent, nDiffuseContentLength, false);
						CGlobals::GetAssetManager()->GetTextureManager().AddEntity(strDiffuseTexName, pTexEntity);
					}
					else if (CParaFile::DoesFileExist(strDiffuseTexName.c_str(), true))
					{
						// 外置贴图
						pTexEntity = CGlobals::GetAssetManager()->LoadTexture(strDiffuseTexName, strDiffuseTexName
							, ParaEngine::TextureEntity::StaticTexture);
					}
				}
			}
		}
	}

	if (!pTexEntity)
	{
		// 使用默认贴图
		strDiffuseTexName = TextureEntity::DEFAULT_STATIC_TEXTURE;
		pTexEntity = CGlobals::GetAssetManager()->LoadTexture(strDiffuseTexName, strDiffuseTexName
			, ParaEngine::TextureEntity::StaticTexture);
	}

	if (pTexEntity)
	{
		m_vecTexEnity.push_back(pTexEntity);
		mat.nDiffuseTexIndex = m_vecTexEnity.size() - 1;
	}

	m_vecMaterials.push_back(mat);
}

void ParaEngine::FBXParser2::ParseNode(const aiNode* pNode, int parentIndex)
{
	string nodeName = pNode->mName.C_Str();
	Bone& bone = m_vecBones[CreateOrGetBoneIndex(nodeName)];
	bone.parent = parentIndex;
	aiVector3D aiScaling;
	aiVector3D aiRot;
	aiVector3D aiTrans;
	pNode->mTransformation.Decompose(aiScaling, aiRot, aiTrans);
	bone.mOrigScale = ConvertFBXVector3D2(aiScaling);
	if (parentIndex >= 0)
	{
		//////////////////////////////////////////////////////////////////////////
		Matrix4 matTrans = reinterpret_cast<const Matrix4&>(pNode->mTransformation);
		bone.matTransform = matTrans.transpose();
		bone.mLocalMatrix = bone.matTransform;
		bone.mGlobalMatrix = bone.mLocalMatrix * m_vecBones[parentIndex].mGlobalMatrix;
		if (!bone.IsAnimated())
		{
			bone.flags |= ParaEngine::Bone::BONE_STATIC_TRANSFORM;
		}
	}

	bool bVisible = true;
	if (bVisible)
	{
		for (int i = 0; i < pNode->mNumMeshes; ++i)
		{
			ParseMesh(pNode->mMeshes[i], bone);
		}
	}

	for (int i = 0; i < pNode->mNumChildren; ++i)
	{
		//ParseNode(pNode->mChildren[i], bone.nIndex);
		ParseNode(pNode->mChildren[i], m_vecBones[CreateOrGetBoneIndex(nodeName)].nIndex);
	}
}

void ParaEngine::FBXParser2::ParseMesh(unsigned int meshIndex, const Bone& bone)
{
	aiMesh* mesh = m_pFBXScene->mMeshes[meshIndex];
	int vertexStart = m_vecVertices.size();
	int indexStart = m_vecIndices.size();
	m_vecVertices.reserve(m_vecVertices.size() + mesh->mNumVertices);
	m_vecIndices.reserve(m_vecIndices.size() + mesh->mNumFaces * 3);
	// 解析顶点数据
	unsigned int vertexCnt = mesh->mNumVertices;
	for (int i = 0; i < vertexCnt; ++i)
	{
		ModelVertex vertex;
		memset(&vertex, 0, sizeof(ModelVertex));

		const aiVector3D* pos = &(mesh->mVertices[i]);
		const aiVector3D* normal = mesh->HasNormals() ? &(mesh->mNormals[i]) : NULL;
		const aiVector3D* texCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : NULL;

		vertex.pos = Vector3(pos->x, pos->y, pos->z);

		if (normal)
		{
			vertex.normal = Vector3(normal->x, normal->y, normal->z);
		}
		else
		{
			vertex.normal = Vector3::ZERO;
		}

		if (texCoord)
		{
			vertex.texcoords.x = texCoord->x;
			vertex.texcoords.y = texCoord->y;
		}
		else
		{
			vertex.texcoords = Vector2::ZERO;
		}
		if (!m_pFBXScene->HasAnimations())
		{
			vertex.pos = vertex.pos * bone.mGlobalMatrix;
			//vertex.normal = vertex.normal * bone.mGlobalMatrix;
			vertex.normal = vertex.normal.TransformNormal(bone.mGlobalMatrix);
			//vertex.bones[0] = bone.nIndex;
			minExtent.makeFloor(vertex.pos);
			maxExtent.makeCeil(vertex.pos);
		}
		else if (!mesh->HasBones())
		{
			// 有动画，但是mesh没骨骼
			vertex.bones[0] = bone.nIndex;
			vertex.weights[0] = 255;
		}

		m_vecVertices.push_back(vertex);
	}

	// 解析索引数据
	unsigned int indexCnt = mesh->mNumFaces * 3;
	for (int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		if (face.mNumIndices == 3)
		{
			uint16 indices[3];
			indices[0] = face.mIndices[0];
			indices[1] = face.mIndices[1];
			indices[2] = face.mIndices[2];
			/*m_vecIndices.push_back(indices[0]);
			m_vecIndices.push_back(indices[1]);
			m_vecIndices.push_back(indices[2]);*/
			//////////////////////////////////////////////////////////////////////////
			if (bone.mOrigScale.x < 0.0f
				&& bone.mOrigScale.y < 0.0f
				&& bone.mOrigScale.z < 0.0f)
			//if (false)
			{
				m_vecIndices.push_back(indices[0]);
				m_vecIndices.push_back(indices[2]);
				m_vecIndices.push_back(indices[1]);
			}
			else
			{
				m_vecIndices.push_back(indices[0]);
				m_vecIndices.push_back(indices[1]);
				m_vecIndices.push_back(indices[2]);
			}
			//////////////////////////////////////////////////////////////////////////
			//// 矫正镜像后的三角面线序
			//Vector3 v1 = ConvertFBXVector3D2(mesh->mVertices[indices[0]]);
			//Vector3 v2 = ConvertFBXVector3D2(mesh->mVertices[indices[1]]);
			//Vector3 v3 = ConvertFBXVector3D2(mesh->mVertices[indices[2]]);
			//Vector3 facing = (v2 - v1).crossProduct(v3 - v2);
			//Vector3 n1 = ConvertFBXVector3D2(mesh->mNormals[indices[0]]);
			//Vector3 n2 = ConvertFBXVector3D2(mesh->mNormals[indices[1]]);
			//Vector3 n3 = ConvertFBXVector3D2(mesh->mNormals[indices[2]]);
			//Vector3 faceNormal = (n1 + n2 + n3) / 3;
			//if (facing.dotProduct(faceNormal) < 0)
			//{
			//	m_vecIndices.push_back(indices[0]);
			//	m_vecIndices.push_back(indices[1]);
			//	m_vecIndices.push_back(indices[2]);
			//}
			//else
			//{
			//	m_vecIndices.push_back(indices[0]);
			//	m_vecIndices.push_back(indices[2]);
			//	m_vecIndices.push_back(indices[1]);
			//}
		}
	}

	////////////////////////////////////////////////////////////////////////
	ModelRenderPass pass;
	pass.SetStartIndex(indexStart);
	pass.geoset = m_vecGeoset.size();
	pass.indexCount = indexCnt;
	FBXMaterial2& mat = m_vecMaterials[mesh->mMaterialIndex];
	mat.bIsUsed = true;
	pass.tex = mat.nDiffuseTexIndex;
	int16 blendmode = BM_OPAQUE;
	if (mat.bHasOpacityTex)
	{
		if (mat.isAlphaBlended())
			blendmode = BM_ALPHA_BLEND;
		else
			blendmode = BM_TRANSPARENT;
	}
	
	if (mat.bAddictive)
	{
		blendmode = BM_ADDITIVE;
	}
	pass.SetCategoryId(mat.GetCategoryID());
	pass.texanim = -1;
	pass.color = -1;
	pass.opacity = mat.opacity;
	pass.unlit = mat.bUnlit;
	pass.nozwrite = mat.bDisableZWrite;
	pass.disable_physics = mat.bDisablePhysics;
	pass.force_physics = mat.bForcePhysics;

	pass.blendmode = blendmode;
	pass.cull = blendmode == BM_OPAQUE ? true : false;
	pass.order = mat.m_nOrder;
	m_vecPasses.push_back(pass);
	mat.vecRefPassIndex.push_back(m_vecPasses.size() - 1);

	ModelGeoset geoset;
	geoset.vstart = vertexStart;
	geoset.istart = indexStart;
	geoset.SetVertexStart(vertexStart);
	geoset.vcount = vertexCnt;
	geoset.icount = indexCnt;
	m_vecGeoset.push_back(geoset);

	//if (mesh->HasBones())
	{
		for (int i = 0; i < mesh->mNumBones; ++i)
		{
			ParseBone(mesh->mBones[i], vertexStart);
		}
	}

}

void ParaEngine::FBXParser2::ParseBone(const aiBone* pBone, const int& nVertexStart)
{
	int index = CreateOrGetBoneIndex(pBone->mName.C_Str());

	if (index >= 0)
	{
		Bone& bone = m_vecBones[index];
		const Matrix4& offsetMat = reinterpret_cast<const Matrix4&>(pBone->mOffsetMatrix);
		bone.matOffset = offsetMat.transpose();
		bone.flags |= ParaEngine::Bone::BONE_OFFSET_MATRIX;
		bone.flags &= ~ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
		bone.pivot = Vector3(0, 0, 0) * bone.matOffset.InvertPRMatrix();
	}

	for (int i = 0; i < pBone->mNumWeights; i++)
	{
		aiVertexWeight& weight = pBone->mWeights[i];
		ModelVertex& vertex = m_vecVertices[weight.mVertexId + nVertexStart];
		uint8 vertexWeight = (uint8)(weight.mWeight * 255);
		int nBoneIndex = 0;
		for (; nBoneIndex < ParaEngine::Bone::s_MaxBonesPerVertex; nBoneIndex++)
		{
			if (0 == vertex.weights[nBoneIndex])
			{
				vertex.bones[nBoneIndex] = index;
				vertex.weights[nBoneIndex] = vertexWeight;
				break;
			}
		}

		if (nBoneIndex >= ParaEngine::Bone::s_MaxBonesPerVertex)
		{
			OUTPUT_LOG("warn: %s has more than 4 bones affecting it. overwrite the smallest one\n", m_strFilePath.c_str());
			int nSmallestIndex = 0;
			for (nBoneIndex = 1; nBoneIndex < ParaEngine::Bone::s_MaxBonesPerVertex; nBoneIndex++)
			{
				if (vertex.weights[nBoneIndex] <= vertex.weights[nSmallestIndex])
					nSmallestIndex = nBoneIndex;
			}
			vertex.bones[nSmallestIndex] = nBoneIndex;
			vertex.weights[nSmallestIndex] = vertexWeight;
		}

	}
}

void ParaEngine::FBXParser2::ParseAnimation(const aiAnimation* pAnim)
{
	for (int i = 0; i < pAnim->mNumChannels; i++)
	{
		aiNodeAnim* channel = pAnim->mChannels[i];
		int index = CreateOrGetBoneIndex(channel->mNodeName.C_Str());
		if (index >= 0)
		{
			float fTimeScale = 1000.f / pAnim->mTicksPerSecond;
			Bone& bone = m_vecBones[index];
			// bone.calc is true, if there is bone animation. 
			bone.calc = true;
			bone.scale.used = (channel->mNumScalingKeys > 0);
			if (bone.scale.used)
			{
				for (int j = 0; j < (int)channel->mNumScalingKeys; j++)
				{
					bone.scale.times.push_back((int)(channel->mScalingKeys[j].mTime * fTimeScale));
					bone.scale.data.push_back(ConvertFBXVector3D2(channel->mScalingKeys[j].mValue));
				}
			}

			bone.rot.used = (channel->mNumRotationKeys > 0);
			if (bone.rot.used)
			{
				for (int j = 0; j < (int)channel->mNumRotationKeys; j++)
				{
					bone.rot.times.push_back((int)(channel->mRotationKeys[j].mTime * fTimeScale));
					bone.rot.data.push_back(ConvertFBXQuaternion2(channel->mRotationKeys[j].mValue));
				}
			}

			bone.trans.used = (channel->mNumPositionKeys > 0);
			if (bone.trans.used)
			{
				for (int j = 0; j < (int)channel->mNumPositionKeys; j++)
				{
					bone.trans.times.push_back((int)(channel->mPositionKeys[j].mTime * fTimeScale));
					bone.trans.data.push_back(ConvertFBXVector3D2(channel->mPositionKeys[j].mValue));
				}
			}
		}
	}

	if (m_cAnimConfig.LoadFromFile(m_strAnimConfig))
	{
		for (int i = 0; i < m_cAnimConfig.GetAnimCount(); i++)
		{
			ParaEngine::AnimInfo& animinfo = m_cAnimConfig.m_Anims[i];
			ModelAnimation anim = CreateAnimation(pAnim, &animinfo);
			m_vecAnims.push_back(anim);
		}
	}
	else
	{
		uint32 time = (uint32)((1000.f / pAnim->mTicksPerSecond) * pAnim->mDuration);
		int animsCount = (int)floor(time / 10000) + 1;
		for (int i = 0; i < animsCount; i++)
		{
			ParaEngine::AnimInfo animinfo;
			animinfo.id = i;
			animinfo.startTick = (int)(i * 10 * pAnim->mTicksPerSecond);
			animinfo.endTick = (int)((i + 1) * 10 * pAnim->mTicksPerSecond - 1);
			animinfo.loopType = 0;
			ModelAnimation anim = CreateAnimation(pAnim, &animinfo);
			m_vecAnims.push_back(anim);
		}
	}
}

ParaEngine::ModelAnimation ParaEngine::FBXParser2::CreateAnimation(const aiAnimation* pAnim, const AnimInfo* pAnimInfo)
{
	float fTimeScale = (1000.f / pAnim->mTicksPerSecond);
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
		animID = m_vecAnims.size();
		tickStart = 0;
		tickEnd = (int)pAnim->mDuration;
		timeStart = 0;
		timeEnd = (uint32)(pAnim->mDuration * fTimeScale);
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

	ClipAnimation(timeStart, timeEnd);

	return anim;
}

void ParaEngine::FBXParser2::ClipAnimation(const uint32& timeStart, const uint32& timeEnd)
{
	for (Bone& bone : m_vecBones)
	{
		// support multiple animations
		int nAnimId = m_vecAnims.size();
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
}

bool ParaEngine::FBXParser2::ReachMaxMaterials() const
{
	return m_vecMaterials.size() >= CParaXModel::MAX_MODEL_TEXTURES;
}

bool ParaEngine::FBXParser2::HasMesh() const
{
	return !m_vecPasses.empty() && !m_vecGeoset.empty();
}

void ParaEngine::FBXParser2::FillData(CParaXModel* pModel)
{
	pModel->m_objNum.nBones = m_vecBones.size();
	pModel->m_objNum.nAnimations = m_vecAnims.size();
	pModel->m_objNum.nVertices = m_vecVertices.size();
	pModel->m_objNum.nIndices = m_vecIndices.size();
	//pModel->m_objNum.nTextures = m_vecTexEnity.size();
	pModel->m_vNeckYawAxis = m_cAnimConfig.m_vNeckYawAxis;
	pModel->m_vNeckPitchAxis = m_cAnimConfig.m_vNeckPitchAxis;
	//std::stable_sort(m_vecPasses.begin(), m_vecPasses.end());
	//pModel->passes.insert(pModel->passes.end(), m_vecPasses.begin(), m_vecPasses.end());
	pModel->geosets.insert(pModel->geosets.end(), m_vecGeoset.begin(), m_vecGeoset.end());
	pModel->showGeosets = new bool[pModel->geosets.size()];
	for (int i = 0; i < pModel->geosets.size(); ++i)
	{
		pModel->showGeosets[i] = true;
	}
	//////////////////////////////////////////////////////////////////////////
	vector<int> vecUsedMaterialsIndex;
	for (int i = 0; i < m_vecMaterials.size(); ++i)
	{
		if (m_vecMaterials[i].bIsUsed)
		{
			vecUsedMaterialsIndex.push_back(i);
		}
	}
	pModel->m_objNum.nTextures = vecUsedMaterialsIndex.size();
	int texSize = vecUsedMaterialsIndex.size();
	pModel->textures = new asset_ptr<TextureEntity>[texSize];
	for (int i = 0; i < texSize; ++i)
	{
		pModel->textures[i] = m_vecTexEnity[vecUsedMaterialsIndex[i]];
	}
	for (int i = 0; i < Math::Min(texSize, CParaXModel::MAX_MODEL_TEXTURES); ++i)
	{
		pModel->specialTextures[i] = m_vecMaterials[vecUsedMaterialsIndex[i]].nIsReplaceableIndex;
	}
	// 更新pass的材质球索引
	for (int i = 0; i < texSize; ++i)
	{
		FBXMaterial2& matRef = m_vecMaterials[vecUsedMaterialsIndex[i]];
		for (int j = 0; j < matRef.vecRefPassIndex.size(); ++j)
		{
			m_vecPasses[matRef.vecRefPassIndex[j]].tex = i;
		}
	}
	std::stable_sort(m_vecPasses.begin(), m_vecPasses.end());
	pModel->passes.insert(pModel->passes.end(), m_vecPasses.begin(), m_vecPasses.end());
	//////////////////////////////////////////////////////////////////////////
	int transparencySize = m_vecTransparencys.size();
	pModel->m_objNum.nTransparency = transparencySize;
	if (!m_vecTransparencys.empty())
	{
		pModel->transparency = new ModelTransparency[transparencySize];
		for (int i = 0; i < transparencySize; ++i)
		{
			pModel->transparency[i] = m_vecTransparencys[i];
		}
	}

	// 骨骼
	if (m_vecBones.size() > 0)
	{
		pModel->bones = new Bone[m_vecBones.size()];
		for (int i = 0; i < (int)m_vecBones.size(); ++i)
		{
			m_vecBones[i].RemoveRedundentKeys();
			pModel->bones[i] = m_vecBones[i];
			if (m_vecBones[i].nBoneID > 0)
				pModel->m_boneLookup[m_vecBones[i].nBoneID] = i;
			else if (m_vecBones[i].IsAttachment())
			{
				// TODO: pivot point
				pModel->NewAttachment(true, m_vecBones[i].GetAttachmentId(), i, Vector3::ZERO);
			}
		}
	}

	// 骨骼动画
	if (!m_vecAnims.empty())
	{
		pModel->anims = new ModelAnimation[m_vecAnims.size()];
		memcpy(pModel->anims, &(m_vecAnims[0]), sizeof(ModelAnimation)*m_vecAnims.size());
		pModel->animBones = true;
		pModel->animated = true;
	}
	else
	{
		pModel->animBones = false;
		pModel->animated = false;
	}
	pModel->SetRenderMethod(pModel->HasAnimation() ? CParaXModel::SOFT_ANIM : CParaXModel::NO_ANIM);

	// AABB
	if (pModel->animated)
	{
		AnimIndex blendingAnim;
		AnimIndex curAnim = pModel->GetAnimIndexByID(0);
		pModel->calcBones(NULL, curAnim, blendingAnim, 0.f);

		//ModelVertex *ov = pModel->m_origVertices;
		ModelVertex *ov = &(m_vecVertices[0]);
		ParaEngine::Bone* bones = pModel->bones;
		for (int i = 0; i < pModel->m_objNum.nVertices; ++i, ++ov)
		{
			Bone& bone = bones[ov->bones[0]];
			float weight = ov->weights[0] * (1 / 255.0f);
			Vector3 v = (ov->pos * bone.mat)*weight;
			for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
				weight = ov->weights[b] * (1 / 255.0f);
				Bone& bone = bones[ov->bones[b]];
				v += (ov->pos * bone.mat) * weight;
			}
			//ov->pos = v;
			minExtent.makeFloor(v);
			maxExtent.makeCeil(v);
		}
	}
	pModel->m_header.minExtent = minExtent;
	pModel->m_header.maxExtent = maxExtent;
	pModel->initVertices(m_vecVertices.size(), &(m_vecVertices[0]));
	pModel->initIndices(m_vecIndices.size(), &(m_vecIndices[0]));
}

void ParaEngine::FBXParser2::ClearData()
{
	m_pFBXScene = NULL;

	m_vecMaterials.clear();
	m_vecTexEnity.clear();
	m_vecTransparencys.clear();

	m_vecVertices.clear();
	m_vecIndices.clear();
	m_vecPasses.clear();
	m_vecGeoset.clear();
	minExtent = FLT_MAX;
	maxExtent = -FLT_MAX;
	m_vecBones.clear();
	m_mapBones.clear();
	m_vecAnims.clear();
}

void ParaEngine::FBXParser2::ParseMaterialByName(const string& sMatName, FBXMaterial2* out)
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

		if (nMarkIndex == -1)
		{
			return;
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
					//out->nForceUnique = ++m_unique_id;
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
				out->bBillboard = true;
				//out->nForceUnique = ++m_unique_id;
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

int ParaEngine::FBXParser2::GetDiffuseTexIndex(const string& strTexName) const
{
	for (int i = 0; i < m_vecTexEnity.size(); ++i)
	{
		if (strTexName == m_vecTexEnity[i]->GetLocalFileName())
		{
			return i;
		}
	}

	return -1;
}

int ParaEngine::FBXParser2::CreateOrGetBoneIndex(const string& strBoneName)
{
	auto fIt = m_mapBones.find(strBoneName);
	if (fIt != m_mapBones.end())
	{
		return fIt->second;
	}
	else
	{
		// 创建
		Bone bone;
		bone.nIndex = m_vecBones.size();
		bone.bUsePivot = false;
		bone.SetName(strBoneName);
		bone.AutoSetBoneInfoFromName();
		// if bone already has an ID, it is always a non-transform node. 
		if (bone.GetBoneID() <= 0)
			bone.flags = ParaEngine::Bone::BONE_TRANSFORMATION_NODE;
		m_mapBones.insert(make_pair(strBoneName, bone.nIndex));
		m_vecBones.push_back(bone);
		return bone.nIndex;
	}
}
