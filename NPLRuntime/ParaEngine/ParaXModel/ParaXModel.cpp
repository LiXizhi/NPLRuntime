//-----------------------------------------------------------------------------
// Class:	CParaXModel
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8
// Note: some logics is based on the open source code of WOWMAPVIEW
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <algorithm>
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "effect_file.h"
#ifdef USE_DIRECTX_RENDERER
#include "ShadowVolume.h"
#include "EdgeBuilder.h"
#endif
#include "SortedFaceGroups.h"
#include "CustomCharCommon.h"
#include "particle.h"
#include "ParaXBone.h"
#include "ParaXModel.h"
#include "BoneChain.h"
#include "memdebug.h"
#include "XFileCharModelExporter.h"
#include "IO/FileUtils.h"
#include "ViewportManager.h"
#include "glTFModelExporter.h"
#include "ParaVoxelModel.h"
#include "GltfModel.h"

/** def this, if one wants the animation to be very accurate. */
//#define	ONLY_REMOVE_EQUAL_KEYS

/** @def if this macro is defined, then vertices and normals of the animated mesh will be precalculated.
* uncomment this macro, if you believe that the reuse of model vertices is not much and that the model
* has many hidden meshes. */
// #define PRECALCULATE_VERTICES_NORMALS

namespace ParaEngine
{
	int64_t globalTime = 0;
	VertexDeclarationPtr CParaXModel::m_pVertexDeclaration = NULL;
	CEffectFile* CParaXModel::m_pEffectFile = NULL;
}
using namespace ParaEngine;

size_t CParaXModel::m_uUsedVB = 0;

void CParaXModel::SetHeader(const ParaXHeaderDef& xheader)
{
	// for xheader
	m_header = xheader;

	//check rotation property first since it's not really a "animation" property
	//clear it after we get value                  -- clayman 2012.7.23
	uint32 mask = (1 << 5);
	rotatePartice2SpeedVector = (m_header.IsAnimated & (1 << 5)) > 0;
	m_header.IsAnimated &= ~mask;

	animated = m_header.IsAnimated > 0;
	animGeometry = (m_header.IsAnimated & (1 << 0)) > 0;
	animTextures = (m_header.IsAnimated & (1 << 1)) > 0;
	animBones = (m_header.IsAnimated & (1 << 2)) > 0;
	//to support arg channel only texture animation  -clayman 2011.8.5
	animTexRGB = (m_header.IsAnimated & (1 << 4)) > 0;

	if (IsBmaxModel())
		m_RenderMethod = BMAX_MODEL;
	else if (animated)
		m_RenderMethod = SOFT_ANIM;
	else
		m_RenderMethod = NO_ANIM;
}

CParaXModel::CParaXModel(const ParaXHeaderDef& xheader)
	: m_bIsValid(true), m_nCurrentFrameNumber(0), m_nHasAlphaBlendedRenderPass(-1), m_bTextureLoaded(false)
	, m_vNeckYawAxis(Vector3::UNIT_Y), m_vNeckPitchAxis(Vector3::UNIT_Z)
	, m_vbState(NOT_SET), m_pVoxelModel(NULL)
{
	SetHeader(xheader);

	// set to default for all others.
	memset(&m_objNum, 0, sizeof(m_objNum));
	m_trans = 1.0f;
	m_radius = 1.0f;

	int i = 0;
	for (i = 0; i < MAX_MODEL_TEXTURES; ++i)
	{
		specialTextures[i] = -1;
		replaceTextures[i] = 0;
		useReplaceTextures[i] = false;
	}
	for (i = 0; i < MAX_MODEL_ATTACHMENTS; ++i)
		m_attLookup[i] = -1;

	for (i = 0; i < MAX_KNOWN_BONE_NODE; ++i)
		m_boneLookup[i] = -1;
	bounds = 0;
	boundTris = 0;
	showGeosets = 0;

	hasCamera = false;
	m_origVertices = NULL;
	m_frame_number_vertices = NULL;
	m_vertices = NULL;
	m_normals = NULL;
	texcoords1 = NULL;
	m_indices = NULL;
	anims = NULL;
	bones = NULL;
	texanims = NULL;
	globalSequences = NULL;
	colors = NULL;
	lights = NULL;
	transparency = NULL;
	particleSystems = NULL;
	ribbons = NULL;
	globalSequences = NULL;

	// animation states
	m_CurrentAnim.Reset();
	m_NextAnim.MakeInvalid();
	m_BlendingAnim.Reset();
	blendingFactor = 0;

	mUpperAnim.Reset();
	mUpperBlendingAnim.Reset();
	mUpperBlendingFactor = 0;

	fBlendingTime = 0.25f;	// this is the default value.
}

CParaXModel::~CParaXModel(void)
{
	if (m_bIsValid)
	{
		if (m_vbState == INITED)
		{
			m_uUsedVB -= m_pVertexBuffer.GetBufferSize();
		}

		m_pIndexBuffer.ReleaseBuffer();
		m_pVertexBuffer.ReleaseBuffer();

		if (GetObjectNum().nTextures) {
			for (size_t i = 0; i < GetObjectNum().nTextures; i++) {
				auto pTexture = textures[i].get();
				if (pTexture) {
					textures[i].reset();
					if (pTexture->IsEmbeddedTexture() && (pTexture->GetRawData() || pTexture->GetImage()))
					{
						pTexture->UnloadAsset();
						pTexture->SetRawDataForImage(NULL, 0);
						pTexture->SetRawData(NULL, 0);
						pTexture->MakeInvalid();
						if (pTexture->GetRefCount() == 1)
						{
							// for embedded textures, this will release the texture memory as well. 
							auto& texManager = CGlobals::GetAssetManager()->GetTextureManager();
							texManager.DeleteEntity(pTexture);
						}
					}
				}
			}
			SAFE_DELETE_ARRAY(textures);
		}

		SAFE_DELETE_ARRAY(globalSequences);

		SAFE_DELETE_ARRAY(bounds);
		SAFE_DELETE_ARRAY(boundTris);
		SAFE_DELETE_ARRAY(showGeosets);


		// unload all sorts of crap
		SAFE_DELETE_ARRAY(m_vertices);
		SAFE_DELETE_ARRAY(m_normals);
		SAFE_DELETE_ARRAY(texcoords1);
		SAFE_DELETE_ARRAY(m_indices);
		SAFE_DELETE_ARRAY(anims);
		SAFE_DELETE_ARRAY(m_origVertices);
		SAFE_DELETE_ARRAY(m_frame_number_vertices);

		SAFE_DELETE_ARRAY(bones);
		SAFE_DELETE(m_pVoxelModel);

		if (animTextures)
			SAFE_DELETE_ARRAY(texanims);
		SAFE_DELETE_ARRAY(colors);
		SAFE_DELETE_ARRAY(transparency);
		SAFE_DELETE_ARRAY(lights);

		SAFE_DELETE_ARRAY(particleSystems);
		SAFE_DELETE_ARRAY(ribbons);

		ClearFaceGroups();
	}
}

void CParaXModel::SetVertexBufferDirty()
{
	if (m_vbState == INITED)
	{
		m_uUsedVB -= m_pVertexBuffer.GetBufferSize();
		m_pVertexBuffer.ReleaseBuffer();
		m_pIndexBuffer.ReleaseBuffer();

		m_vbState = NEED_INIT;
	}
}

void CParaXModel::SetRenderMethod(RENDER_METHOD method)
{
	m_RenderMethod = method;
}

bool CParaXModel::CheckMinVersion(int v0, int v1/*=0*/, int v2/*=0*/, int v3/*=0*/)
{
	return (GetHeader().version[0] >= v0) && (GetHeader().version[1] >= v1) && (GetHeader().version[2] >= v2) && (GetHeader().version[3] >= v3);
}

bool CParaXModel::InitDeviceObjects()
{
	LoadTextures();

	return true;
}

bool CParaXModel::DeleteDeviceObjects()
{
	return true;
}

void CParaXModel::LoadTextures()
{
	if (m_bTextureLoaded)
		return;
	m_bTextureLoaded = true;
	auto& texManager = CGlobals::GetAssetManager()->GetTextureManager();
	for (int i = 0; i < (int)m_objNum.nTextures; ++i)
	{
		asset_ptr<TextureEntity> pTexture = textures[i];
		if (pTexture)
		{
			textures[i] = CGlobals::GetAssetManager()->LoadTexture("", pTexture->GetKey(), TextureEntity::StaticTexture);
			if (pTexture != textures[i])
			{
				if (pTexture->GetRawData())
				{
					textures[i]->SetRawData(pTexture->GetRawData(), pTexture->GetRawDataSize());
					// OUTPUT_LOG("%s assigned buffer from raw data \n", pTexture->GetKey().c_str());
					pTexture->GiveupRawDataOwnership();
				}

				if (pTexture->GetImage())
				{
					pTexture->SwapImage(textures[i].get());
				}
			}
		}
	}

	initTranslucentFaceGroups();
}

bool CParaXModel::IsBmaxModel()
{
	return m_header.type == PARAX_MODEL_BMAX;
}

void CParaXModel::SetBmaxModel()
{
	m_header.type = PARAX_MODEL_BMAX;
	SetRenderMethod(BMAX_MODEL);
}

void CParaXModel::ClearFaceGroups()
{
	//////////////////////////////////////////////////////////////////////////
	// delete all face group. 
	for (int i = 0; i < (int)m_faceGroups.size(); ++i)
	{
		SAFE_DELETE(m_faceGroups[i]);
	}
	m_faceGroups.clear();
}

bool CParaXModel::IsValid()
{
	return m_bIsValid;
}

AnimIndex CParaXModel::GetAnimIndexByID(int nAnimID)
{
	int nAnim = (int)GetObjectNum().nAnimations;
	for (int i = 0; i < nAnim; i++)
	{
		if (anims[i].animID == nAnimID)
		{
			return AnimIndex(i, 0, anims[i].timeStart, anims[i].timeEnd, (unsigned char)(anims[i].loopType), nAnimID);
		}
	}
	return AnimIndex(-1);
}

int CParaXModel::GetAnimIDByIndex(int nAnimIndex)
{
	if (nAnimIndex < (int)GetObjectNum().nAnimations && nAnimIndex >= 0)
		return anims[nAnimIndex].animID;
	else
		return 0;
}

const ModelAnimation* CParaXModel::GetModelAnimByIndex(int nAnimIndex)
{
	if (nAnimIndex < (int)GetObjectNum().nAnimations && nAnimIndex >= 0)
		return &(anims[nAnimIndex]);
	else
		return NULL;
}

void CParaXModel::InitVertexBuffer()
{
	switch (m_RenderMethod)
	{
	case ParaEngine::CParaXModel::NO_ANIM:
		InitVertexBuffer_NOANIM();
		break;
	case ParaEngine::CParaXModel::BMAX_MODEL:
		InitVertexBuffer_BMAX();
		break;
	default:
		break;
	}
}

void CParaXModel::InitVertexBuffer_BMAX()
{
	do
	{
		if (m_pVertexBuffer.IsValid()
			|| m_pVertexBuffer.IsValid()
			|| passes.size() == 0
			|| m_origVertices == nullptr
			|| m_indices == nullptr)
		{
			break;
		}

		auto nPasses = passes.size();

		size_t count = 0;
		for (size_t i = 0; i < nPasses; i++)
		{
			auto& p = passes[i];

			if (p.geoset < 0 || !showGeosets[p.geoset])
				continue;

			count += p.indexCount;
		}

		if (!m_pVertexBuffer.CreateBuffer((uint32)(count * sizeof(bmax_vertex)), 0, D3DUSAGE_WRITEONLY))
			break;

		bmax_vertex* pBuffer;
		if (!m_pVertexBuffer.Lock((void**)&pBuffer, 0, 0))
			break;

		size_t index = 0;
		for (size_t pass = 0; pass < nPasses; pass++)
		{
			auto& p = passes[pass];

			if (p.geoset < 0 || !showGeosets[p.geoset])
				continue;

			size_t nLockedNum = p.indexCount / 3;
			int nVertexOffset = p.GetVertexStart(this);
			int nIndexOffset = p.m_nIndexStart;

			for (size_t i = 0; i < nLockedNum; ++i)
			{
				size_t nVB = 3 * i;
				for (int k = 0; k < 3; ++k, ++nVB)
				{
					int a = m_indices[nIndexOffset + nVB] + nVertexOffset;
					auto& out_vertex = pBuffer[index++];
					auto& ov = m_origVertices[a];
					out_vertex.p = ov.pos;
					out_vertex.n = ov.normal;
					out_vertex.color = ov.color0;
				}
			}
		}


		m_pVertexBuffer.Unlock();
		auto usedSize = count * sizeof(bmax_vertex);
		m_uUsedVB += usedSize;

		m_vbState = INITED;

		return;

	} while (false);

	m_pVertexBuffer.ReleaseBuffer();
	m_pIndexBuffer.ReleaseBuffer();
	m_vbState = NOT_USE;
}

void CParaXModel::InitVertexBuffer_NOANIM()
{
	do
	{
		if (m_pVertexBuffer.IsValid()
			|| m_pVertexBuffer.IsValid()
			|| passes.size() == 0
			|| m_origVertices == nullptr
			|| m_indices == nullptr)
		{
			break;
		}

		auto nPasses = passes.size();

		size_t count = 0;
		for (size_t i = 0; i < nPasses; i++)
		{
			auto& p = passes[i];

			if (p.geoset < 0 || !showGeosets[p.geoset])
				continue;

			count += p.indexCount;
		}

		if (!m_pVertexBuffer.CreateBuffer((uint32)(count * sizeof(mesh_vertex_normal)), 0, D3DUSAGE_WRITEONLY))
			break;

		mesh_vertex_normal* pBuffer;
		if (!m_pVertexBuffer.Lock((void**)&pBuffer, 0, 0))
			break;


		size_t index = 0;
		for (size_t pass = 0; pass < nPasses; pass++)
		{
			auto& p = passes[pass];

			if (p.geoset < 0 || !showGeosets[p.geoset])
				continue;

			size_t nLockedNum = p.indexCount / 3;
			int nVertexOffset = p.GetVertexStart(this);
			int nIndexOffset = p.m_nIndexStart;

			for (size_t i = 0; i < nLockedNum; ++i)
			{
				size_t nVB = 3 * i;
				for (int k = 0; k < 3; ++k, ++nVB)
				{
					int a = m_indices[nIndexOffset + nVB] + nVertexOffset;
					auto& out_vertex = pBuffer[index++];
					auto& ov = m_origVertices[a];
					out_vertex.p = ov.pos;
					out_vertex.n = ov.normal;
					out_vertex.uv = ov.texcoords;
				}
			}
		}


		m_pVertexBuffer.Unlock();
		auto usedSize = count * sizeof(mesh_vertex_normal);
		m_uUsedVB += usedSize;

		m_vbState = INITED;

		return;

	} while (false);

	m_pVertexBuffer.ReleaseBuffer();
	m_pIndexBuffer.ReleaseBuffer();
	m_vbState = NOT_USE;
}

void CParaXModel::initVertices(int nVertices, ModelVertex* pVertices)
{
	if (pVertices == NULL) return;
	// delete old
	m_objNum.nVertices = nVertices;
	SAFE_DELETE_ARRAY(m_origVertices);
	SAFE_DELETE_ARRAY(m_frame_number_vertices);

	// radius
	m_radius = (m_header.maxExtent - m_header.minExtent).length() / 2;

	/// read m_origVertices
	if (m_RenderMethod == SOFT_ANIM || m_RenderMethod == NO_ANIM || m_RenderMethod == BMAX_MODEL)
	{
		m_origVertices = new ModelVertex[nVertices];
		if (m_origVertices != 0)
			memcpy(m_origVertices, pVertices, nVertices * sizeof(ModelVertex));
		if (m_RenderMethod != BMAX_MODEL && m_RenderMethod != NO_ANIM)
		{
			m_frame_number_vertices = new int[nVertices];
			memset(m_frame_number_vertices, 0, sizeof(int) * nVertices);
			m_vertices = new Vector3[nVertices];
			m_normals = new Vector3[nVertices];
		}
		else
		{
			SAFE_DELETE_ARRAY(m_frame_number_vertices);
			SAFE_DELETE_ARRAY(m_vertices);
			SAFE_DELETE_ARRAY(m_normals);
		}
	}
	else if (m_RenderMethod == SHADER_ANIM)
	{
		/**
		* Create vertex buffer with skinning information
		*/
		if (!m_pVertexBuffer.IsValid())
		{
			if (m_pVertexBuffer.CreateBuffer(nVertices * sizeof(ModelVertex), 0, D3DUSAGE_WRITEONLY))
			{
				ModelVertex* pBuffer = NULL;
				if (m_pVertexBuffer.Lock((void**)&pBuffer, 0, 0))
				{
					memcpy(pBuffer, pVertices, nVertices * sizeof(ModelVertex));
					m_pVertexBuffer.Unlock();
				}
			}
		}
	}
	else if (m_RenderMethod == NO_ANIM) // NOT USED
	{
		/**
		* NOT USED: Create vertex buffer with only vertex information
		*/
		if (!m_pVertexBuffer.IsValid())
		{
			if (m_pVertexBuffer.CreateBuffer(nVertices * sizeof(mesh_vertex_normal), 0, D3DUSAGE_WRITEONLY))
			{
				mesh_vertex_normal* pBuffer = NULL;
				if (m_pVertexBuffer.Lock((void**)&pBuffer, 0, 0))
				{
					for (int i = 0; i < nVertices; i++) {
						pBuffer[i].p = (Vector3)pVertices[i].pos;
						pBuffer[i].n = (Vector3)pVertices[i].normal;
						pBuffer[i].uv = (Vector2)pVertices[i].texcoords;
					}
					m_pVertexBuffer.Unlock();
				}
			}
		}
	}
}

void CParaXModel::initIndices(int nIndices, uint16* pIndices)
{
	if (pIndices == 0) return;

	RenderDevicePtr pD3dDevice = CGlobals::GetRenderDevice();
	// delete old
	m_objNum.nIndices = nIndices;
	SAFE_DELETE_ARRAY(m_indices);
	if (m_RenderMethod == SOFT_ANIM || m_RenderMethod == NO_ANIM || m_RenderMethod == BMAX_MODEL)
	{
		m_indices = new uint16[nIndices];
		if (m_indices != 0)
			memcpy(m_indices, pIndices, sizeof(uint16) * nIndices);
	}
	else
	{
		if (!m_pIndexBuffer.IsValid())
		{
			if (m_pIndexBuffer.CreateIndexBuffer(sizeof(uint16) * (UINT)nIndices, D3DFMT_INDEX16, D3DUSAGE_WRITEONLY))
			{
				uint16* pIndexValues = NULL;
				if (m_pIndexBuffer.Lock((void**)&pIndexValues, 0, 0))
				{
					memcpy(pIndexValues, pIndices, sizeof(uint16) * nIndices);
					m_pIndexBuffer.Unlock();
				}
			}
		}
	}
}

void CParaXModel::initTranslucentFaceGroups()
{
	int nPasses = (int)passes.size();
	if (nPasses <= 0 || m_origVertices == 0)
		return;

	for (int nPass = 0; nPass < nPasses; nPass++)
	{
		ModelRenderPass& p = passes[nPass];
		// this is an transparent pass.
		if (p.blendmode != BM_OPAQUE && p.nozwrite && !p.force_local_tranparency)
		{
			CFaceGroup* pFaceGroup = new CFaceGroup();
			m_faceGroups.push_back(pFaceGroup);

			m_TranslucentPassIndice.resize(nPass + 1, -1);
			m_TranslucentPassIndice[nPass] = (int)(m_faceGroups.size() - 1);

			// copy data from mesh and material to face group.

			/// Set the texture
			TextureEntity* bindtex = NULL;
			if (specialTextures[p.tex] == -1)
				bindtex = textures[p.tex].get();
			else
			{
				bindtex = replaceTextures[specialTextures[p.tex]];
				// use default texture if replaceable texture is not specified. 
				if (bindtex == 0)
					bindtex = textures[p.tex].get();
			}
			pFaceGroup->m_pTexture = bindtex;
			pFaceGroup->m_alphaBlending = true;
			pFaceGroup->m_alphaTesting = false;
			pFaceGroup->m_bHasLighting = !(p.unlit);
			pFaceGroup->m_disableZWrite = p.nozwrite;
			pFaceGroup->m_bAdditive = (p.blendmode == BM_ADDITIVE) || (p.blendmode == BM_ADDITIVE_ALPHA);
			pFaceGroup->m_stripLength = p.m_fStripLength;
			pFaceGroup->m_bSkinningAni = p.skinningAni;
			// any material is ok. 
			ParaMaterial mat = CGlobals::GetSceneState()->GetCurrentMaterial();
			mat.Ambient = LinearColor(0.6f, 0.6f, 0.6f, 1.f);
			mat.Diffuse = LinearColor(1.f, 1.f, 1.f, 1.f);
			pFaceGroup->m_material = mat;

			// this gives zwrite enabled face a high priority to be rendered higher.
			if (!pFaceGroup->m_disableZWrite)
				pFaceGroup->m_order = 0;
			else
				pFaceGroup->m_order = 1;

			pFaceGroup->m_nNumTriangles = p.indexCount / 3;;

			for (int k = 0; k < pFaceGroup->m_nNumTriangles; ++k)
			{
				for (int j = 0; j < 3; j++)
				{
					ModelVertex& v = m_origVertices[m_indices[p.m_nIndexStart + k * 3 + j]];
					pFaceGroup->m_vertices.push_back((Vector3)(v.pos));
					pFaceGroup->m_normals.push_back((Vector3)(v.normal));
					pFaceGroup->m_UVs.push_back(v.texcoords);

					if (pFaceGroup->m_bSkinningAni)
					{
						uint32 packedValue = *((int32*)v.bones);
						pFaceGroup->m_boneIndices.push_back(packedValue);

						packedValue = *((int32*)v.weights);
						pFaceGroup->m_vertexWeights.push_back(packedValue);
					}
				}
			}
			pFaceGroup->UpdateCenterPos();
		}
	}
}

ModelAttachment& CParaXModel::NewAttachment(bool bOverwrite, int nAttachmentID, int nBoneIndex, const Vector3& pivotPoint)
{
	if (m_attLookup[nAttachmentID] >= 0 && !bOverwrite)
	{
		return m_atts[m_attLookup[nAttachmentID]];
	}
	else
	{
		if (m_attLookup[nAttachmentID] >= 0 && bOverwrite)
		{
			ModelAttachment& att = m_atts[m_attLookup[nAttachmentID]];
			att.bone = nBoneIndex;
			att.pos = pivotPoint;
			return att;
		}
		else
		{
			m_atts.push_back(ModelAttachment());
			int nAttachmentIndex = (int)m_atts.size() - 1;
			ModelAttachment& att = m_atts[nAttachmentIndex];
			att.id = nAttachmentID;
			m_attLookup[nAttachmentID] = nAttachmentIndex;

			att.bone = nBoneIndex;
			att.pos = pivotPoint;
			m_objNum.nAttachments = (int)m_atts.size();

			if ((int)m_objNum.nAttachLookup <= nAttachmentID)
				m_objNum.nAttachLookup = nAttachmentID + 1;
			return att;
		}
	}
}

bool CParaXModel::SetupTransformByID(int nID)
{
	int nAttachmentIndex = m_attLookup[nID];
	if (nAttachmentIndex >= 0)
	{
		m_atts[nAttachmentIndex].setup(this);
		return true;
	}
	else
		return false;
}


Matrix4* CParaXModel::GetAttachmentMatrix(Matrix4* pOut, int nAttachmentID, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, const AnimIndex& upperAnim, const AnimIndex& upperBlendingAnim, float upperBlendingFactor, bool bRecalcBone, IAttributeFields* pAnimInstance)
{
	int nAttachmentIndex = m_attLookup[nAttachmentID];
	if (nAttachmentIndex >= 0)
	{
		// the bone index at this attachment
		int nBoneIndex = m_atts[nAttachmentIndex].bone;
		Vector3 pos = m_atts[nAttachmentIndex].pos;

		uint32 nBones = (uint32)GetObjectNum().nBones;
		if (nBoneIndex < (int)nBones)
		{
			if (bRecalcBone)
			{
				/** calculate the bone and its parent bones */
				for (uint32 i = 0; i < nBones; i++)
				{
					bones[i].MakeDirty();
				}
			}
			if (bones[nBoneIndex].calcMatrix(bones, (bones[nBoneIndex].mIsUpper && upperAnim.IsValid()) ? upperAnim : CurrentAnim, (bones[nBoneIndex].mIsUpper && upperAnim.IsValid()) ? upperBlendingAnim : BlendingAnim, (bones[nBoneIndex].mIsUpper && upperAnim.IsValid()) ? upperBlendingFactor : blendingFactor, pAnimInstance))
			{
				Matrix4 mat, mat1;
				mat1 = (bones[nBoneIndex].mat);
				mat.makeTrans(pos.x, pos.y, pos.z);
				mat1 = mat.Multiply4x3(mat1);
				*pOut = mat1;
				return pOut;
			}
		}
	}
	return NULL;
}


bool CParaXModel::HasAttachmentMatrix(int nAttachmentID)
{
	return (m_attLookup[nAttachmentID] >= 0);
}

void CParaXModel::PostCalculateBoneMatrix(uint32 nBones)
{
	for (uint32 i = 0; i < nBones; i++)
	{
		bones[i].PostCalculateBoneMatrix();
	}
}

void CParaXModel::calcBones()
{
	uint32 nBones = (uint32)GetObjectNum().nBones;
	for (uint32 i = 0; i < nBones; i++)
	{
		bones[i].MakeDirty();
	}

	for (uint32 i = 0; i < nBones; i++) {
		bones[i].calcMatrix(bones);
	}
	PostCalculateBoneMatrix(nBones);
}

void CParaXModel::calcBones(CharacterPose* pPose, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, const AnimIndex& upperAnim, const AnimIndex& upperBlendingAnim, float upperBlendingFactor, IAttributeFields* pAnimInstance)
{
	uint32 nBones = (uint32)GetObjectNum().nBones;

	// uncomment to fine tune performances for this part of code. 
	//#define PERFOAMRNCE_TEST_calcBones
#ifdef PERFOAMRNCE_TEST_calcBones
	if (nBones < 30)
		return;
	// PERF1("calcBones");

#endif

	for (uint32 i = 0; i < nBones; i++) {
		bones[i].MakeDirty();
	}

	if (pPose)
	{
		// TODO: check if this is an valid character model.
		if (pPose->m_fUpperBodyFacingAngle != 0.f && m_vNeckYawAxis != Vector3::ZERO)
		{
			int nHeadAttachmentIndex = m_attLookup[ATT_ID_HEAD];
			if (nHeadAttachmentIndex >= 0)
			{
				int nParent = m_atts[nHeadAttachmentIndex].bone;
				int nSpine = m_boneLookup[Bone_Spine];
				if (nParent >= 0 && nSpine >= 0)
				{
					int nStart = nParent;

					// tricky code: try to find if there are at least 4 spline bones from head to spine, in most cases, it is head, neck, spline1, spline
					int i = 4;
					for (; i >= 0 && nStart >= 0 && (nSpine != nStart); i--)
					{
						nStart = bones[nStart].parent;
					}
					bool bHasEnoughSpineBones = (i == 0);

					// just in case, some animator connect Thigh bones to spine and we have limited bones, we will ignore rotation
					if (bHasEnoughSpineBones && (nSpine == nStart) && m_boneLookup[Bone_L_Thigh] > 0 && bones[m_boneLookup[Bone_L_Thigh]].parent == nSpine) {
						bHasEnoughSpineBones = false;
					}

					if (!bHasEnoughSpineBones)
					{
						// rotate only head
						int nHead = m_boneLookup[Bone_Head];
						CBoneChain UpperBodyBoneChain(1);
						UpperBodyBoneChain.SetStartBone(bones, nHead, m_boneLookup);
						UpperBodyBoneChain.RotateBoneChain(m_vNeckYawAxis, bones, nBones, pPose->m_fUpperBodyFacingAngle, CurrentAnim, BlendingAnim, blendingFactor, upperAnim, upperBlendingAnim, upperBlendingFactor, pAnimInstance);
					}
					else
					{
						int nNeck = bones[nParent].parent; // get the NECK bone index
						int nRotateSpineBoneCount = 4;

						CBoneChain UpperBodyBoneChain(nRotateSpineBoneCount);
						UpperBodyBoneChain.SetStartBone(bones, nNeck, m_boneLookup);
						UpperBodyBoneChain.RotateBoneChain(m_vNeckYawAxis, bones, nBones, pPose->m_fUpperBodyFacingAngle, CurrentAnim, BlendingAnim, blendingFactor, upperAnim, upperBlendingAnim, upperBlendingFactor, pAnimInstance);
					}
				}
			}
			else
			{
				int nHeadIndex = m_boneLookup[Bone_Head];
				if (nHeadIndex >= 0)
				{
					CBoneChain UpperBodyBoneChain(1);
					UpperBodyBoneChain.SetStartBone(bones, nHeadIndex, m_boneLookup);
					UpperBodyBoneChain.RotateBoneChain(m_vNeckYawAxis, bones, nBones, pPose->m_fUpperBodyFacingAngle, CurrentAnim, BlendingAnim, blendingFactor, upperAnim, upperBlendingAnim, upperBlendingFactor, pAnimInstance);
				}
			}
		}
		if (pPose->m_fUpperBodyUpDownAngle != 0.f && m_vNeckPitchAxis != Vector3::ZERO)
		{
			int nHeadIndex = m_boneLookup[Bone_Head];
			if (nHeadIndex >= 0)
			{
				CBoneChain UpperBodyBoneChain(1);
				UpperBodyBoneChain.SetStartBone(bones, nHeadIndex);
				UpperBodyBoneChain.RotateBoneChain(m_vNeckPitchAxis, bones, nBones, pPose->m_fUpperBodyUpDownAngle, CurrentAnim, BlendingAnim, blendingFactor, upperAnim, upperBlendingAnim, upperBlendingFactor, pAnimInstance);
			}
		}
	}
#ifdef PERFOAMRNCE_TEST_calcBones
	PERF1("calcBones");
#endif
	vector<Matrix4> lower_mats;
	vector<Matrix4> upper_mats;
	lower_mats.reserve(nBones);
	for (uint32 i = 0; i < nBones; i++) {
		bones[i].calcMatrix(bones, CurrentAnim, BlendingAnim, blendingFactor, pAnimInstance);
		lower_mats.push_back(bones[i].mat);
	}
	if (upperAnim.IsValid())
	{
		for (uint32 i = 0; i < nBones; i++) {
			bones[i].MakeDirty();
		}
		for (uint32 i = 0; i < nBones; i++) {
			bones[i].calcMatrix(bones, upperAnim, upperBlendingAnim, upperBlendingFactor, pAnimInstance);
			upper_mats.push_back(bones[i].mat);
		}
		for (uint32 i = 0; i < nBones; i++) {
			if (bones[i].mIsUpper)
				bones[i].mat = upper_mats[i];
			else
				bones[i].mat = lower_mats[i];
		}
	}
	PostCalculateBoneMatrix(nBones);
}

bool CParaXModel::HasAnimation()
{
	return animated;
}

void CParaXModel::animate(SceneState* pSceneState, CharacterPose* pPose, IAttributeFields* pAnimInstance)
{
	if (animated == false || !m_CurrentAnim.IsValid())
		return;

	if (animBones) {
		calcBones(pPose, m_CurrentAnim, m_BlendingAnim, blendingFactor, mUpperAnim, mUpperBlendingAnim, mUpperBlendingFactor, pAnimInstance);
	}

	uint32 nLights = GetObjectNum().nLights;
	for (size_t i = 0; i < nLights; i++) {
		if (lights[i].parent >= 0) {
			lights[i].tpos = bones[lights[i].parent].mat * lights[i].pos;
			lights[i].tdir = bones[lights[i].parent].mrot * lights[i].dir;
		}
	}

	if (m_CurrentAnim.Provider == 0)
	{
		if (pSceneState)
		{
			// TODO: non-local provider?	
			uint32 nParticleEmitters = GetObjectNum().nParticleEmitters;
			for (size_t i = 0; i < nParticleEmitters; i++) {
				// TODO:random time distribution
				// int pt = a.timeStart + (currentFrame + (int)(tmax*particleSystems[i].tofs)) % tmax;
				particleSystems[i].setup(pSceneState, m_CurrentAnim.nIndex, m_CurrentAnim.nCurrentFrame);
			}

			uint32 nRibbonEmitters = GetObjectNum().nRibbonEmitters;
			for (size_t i = 0; i < nRibbonEmitters; i++) {
				ribbons[i].setup(pSceneState, m_CurrentAnim.nIndex, m_CurrentAnim.nCurrentFrame);
			}

			if (animTextures) {
				uint32 nTexAnims = GetObjectNum().nTexAnims;
				for (size_t i = 0; i < nTexAnims; i++) {
					texanims[i].calc(m_CurrentAnim.nIndex, m_CurrentAnim.nCurrentFrame);
				}
			}
		}
	}
	else
	{
		// TODO: what happens when animating textures during external animation, such as the blink of eyes.
	}
}

// not used
void CParaXModel::RenderNoAnim(SceneState* pSceneState)
{
	int nPasses = (int)passes.size();
	if (nPasses <= 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	pd3dDevice->SetStreamSource(0, m_pVertexBuffer.GetDevicePointer(), 0, sizeof(mesh_vertex_normal));
	pd3dDevice->SetIndices(m_pIndexBuffer.GetDevicePointer());

	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if (pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		///////////////////////////////////////////////////////////////////////////
		// fixed function pipeline
		for (int i = 0; i < nPasses; i++)
		{
			ModelRenderPass& p = passes[i];
			if (p.init(this, pSceneState))
			{
				// we don't want to render completely transparent parts
				RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, 0, 0, m_objNum.nVertices, p.m_nIndexStart, p.indexCount / 3);
				p.deinit();
			}
		}
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		if (pEffect->begin())
		{
			if (pEffect->BeginPass(GetRenderPass()))
			{
				for (int i = 0; i < nPasses; i++)
				{
					ModelRenderPass& p = passes[i];
					if (p.init_FX(this, pSceneState))
					{
						// we don't want to render completely transparent parts
						pEffect->CommitChanges();
						RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, 0, 0, m_objNum.nVertices, p.m_nIndexStart, p.indexCount / 3);
						p.deinit_FX(pSceneState);
					}
				}
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}
	pd3dDevice->SetIndices(0);
}



void CParaXModel::RenderSoftNoAnim(SceneState* pSceneState, CParameterBlock* pMaterialParams)
{
	int nPasses = (int)passes.size();
	if (nPasses <= 0)
		return;


	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	if (m_vbState == INITED)
	{
		pd3dDevice->SetStreamSource(0, m_pVertexBuffer.GetDevicePointer(), 0, sizeof(mesh_vertex_normal));
	}
	else
	{
		DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
		pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal));
	}


	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	size_t startVB = 0;
	if (pEffect == 0)
	{
		///////////////////////////////////////////////////////////////////////////
		// fixed function pipeline
		for (int nPass = 0; nPass < nPasses; nPass++)
		{
			ModelRenderPass& p = passes[nPass];

			if (p.geoset >= 0 && showGeosets[p.geoset])
			{
				// skip and build for translucent pass
				if (pSceneState->m_bEnableTranslucentFaceSorting &&
					((int)m_TranslucentPassIndice.size()) > nPass && m_TranslucentPassIndice[nPass] >= 0)
				{
					if (!pSceneState->IsIgnoreTransparent())
					{
						const Matrix4& mat = CGlobals::GetWorldMatrixStack().top();

						const ModelVertex& ov = m_origVertices[m_indices[p.m_nIndexStart]];
						CFaceGroupInstance faceGroup(&mat, m_faceGroups[m_TranslucentPassIndice[nPass]]);
						if (p.texanim != -1) {
							const TextureAnim& texAnim = texanims[p.texanim];
							faceGroup.m_vUVOffset.x = texAnim.tval.x;
							faceGroup.m_vUVOffset.y = texAnim.tval.y;

							faceGroup.m_vUVRotate = texAnim.rval;

							if (texAnim.scale.used)
							{
								faceGroup.m_vUVScale.x = texAnim.sval.x;
								faceGroup.m_vUVScale.y = texAnim.sval.y;
							}
						}
						pSceneState->GetFaceGroups()->AddFaceGroup(faceGroup);
					}

					continue;
				}

				if (p.init(this, pSceneState))
				{
					//DrawPass_NoAnim(p);
					DrawPass_NoAnim_VB(p, startVB);
					p.deinit();
				}
				startVB += p.indexCount;
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		if (pEffect->begin())
		{
			if (pEffect->BeginPass(GetRenderPass(pMaterialParams)))
			{
				/* if this is defined, we will combine render pass with similar textures and attributes.
				It is very strange that combining render pass is slower. */
				// #define COMBINE_RENDER_PASS

#ifdef COMBINE_RENDER_PASS
				ModelRenderPass* pLastPass = NULL;
#endif
				for (int nPass = 0; nPass < nPasses; nPass++)
				{
					ModelRenderPass& p = passes[nPass];

					if (p.geoset >= 0 && showGeosets[p.geoset])
					{
						// skip and build for translucent pass
						if (pSceneState->m_bEnableTranslucentFaceSorting &&
							((int)m_TranslucentPassIndice.size()) > nPass && m_TranslucentPassIndice[nPass] >= 0)
						{
							if (!pSceneState->IsIgnoreTransparent())
							{
								const Matrix4& mat = CGlobals::GetWorldMatrixStack().top();

								const ModelVertex& ov = m_origVertices[m_indices[p.m_nIndexStart]];
								CFaceGroupInstance faceGroup(&mat, m_faceGroups[m_TranslucentPassIndice[nPass]]);
								if (p.texanim != -1) {
									const TextureAnim& texAnim = texanims[p.texanim];
									faceGroup.m_vUVOffset.x = texAnim.tval.x;
									faceGroup.m_vUVOffset.y = texAnim.tval.y;

									faceGroup.m_vUVRotate = texAnim.rval;

									if (texAnim.scale.used)
									{
										faceGroup.m_vUVScale.x = texAnim.sval.x;
										faceGroup.m_vUVScale.y = texAnim.sval.y;
									}


									//support texture uv rgb animation --clayman 2011.8.8
									if (animTexRGB)
										faceGroup.m_UVRgbAnim = true;

								}
								pSceneState->GetFaceGroups()->AddFaceGroup(faceGroup);
							}

							continue;
						}
#ifdef COMBINE_RENDER_PASS
						// we shall combine render pass if current one is same as previous, using the overloaded p.operator == 
						if (pLastPass == NULL)
						{
							if (p.init_FX(this))
							{
								pLastPass = &p;
								pEffect->CommitChanges();
								//DrawPass_NoAnim(p);
								DrawPass_NoAnim_VB(p, startVB);
							}

							startVB += p.indexCount;
						}
						else
						{
							if ((*pLastPass == p))
							{
								DrawPass_NoAnim_VB(p, startVB);
								startVB += p.indexCount;
							}
							else
							{
								pLastPass->deinit_FX(pSceneState, pMaterialParams);
								if (p.init_FX(this, pMaterialParams))
								{
									pLastPass = &p;
									pEffect->CommitChanges();
									DrawPass_NoAnim_VB(p, startVB);
								}
								startVB += p.indexCount;
							}
						}
#else
						// do not combine render pass. this appears to be faster than combined render passes. 
						if (p.init_FX(this, pSceneState, pMaterialParams))
						{
							pEffect->CommitChanges();
							DrawPass_NoAnim_VB(p, startVB);
							p.deinit_FX(pSceneState, pMaterialParams);
						}
						startVB += p.indexCount;
#endif

					}
				}
#ifdef COMBINE_RENDER_PASS
				if (pLastPass != NULL)
				{
					pLastPass->deinit_FX(pSceneState, pMaterialParams);
				}
#endif
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}

}

int CParaXModel::GetRenderPass(CParameterBlock* pMaterialParams)
{
	if (pMaterialParams == NULL) return 0;
	auto materialID = pMaterialParams->GetParameter("MaterialID");
	if (materialID && ((int)(*materialID)) > 0) return 1;
	return 0;
}

void CParaXModel::RenderBMaxModel(SceneState* pSceneState, CParameterBlock* pMaterialParams)
{
	int nPasses = (int)passes.size();
	if (nPasses <= 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	if (m_vbState == INITED)
	{
		pd3dDevice->SetStreamSource(0, m_pVertexBuffer.GetDevicePointer(), 0, sizeof(bmax_vertex));
	}
	else
	{
		DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);
		pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(bmax_vertex));
	}

	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	size_t startVB = 0;
	if (pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		///////////////////////////////////////////////////////////////////////////
		// fixed function pipeline
		for (int nPass = 0; nPass < nPasses; nPass++)
		{
			ModelRenderPass& p = passes[nPass];
			if (p.geoset >= 0 && showGeosets[p.geoset])
			{
				if (p.init_bmax_FX(this, pSceneState))
				{
					//DrawPass_BMax(p);
					DrawPass_BMax_VB(p, startVB);
					p.deinit_bmax_FX(pSceneState);
				}

				startVB += p.indexCount;
			}
		}
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		if (pEffect->begin())
		{
			if (pEffect->BeginPass(GetRenderPass(pMaterialParams)))
			{
				for (int nPass = 0; nPass < nPasses; nPass++)
				{
					ModelRenderPass& p = passes[nPass];

					if (p.geoset >= 0 && showGeosets[p.geoset])
					{
						// do not combine render pass. this appears to be faster than combined render passes. 
						if (p.init_bmax_FX(this, pSceneState, pMaterialParams))
						{
							pEffect->CommitChanges();
							DrawPass_BMax_VB(p, startVB);

							// TODO: do voxel model rendering in its own render pass
							if (m_pVoxelModel && nPass == 0)
							{
								m_pVoxelModel->Draw(pSceneState);
							}

							p.deinit_bmax_FX(pSceneState);
						}
						startVB += p.indexCount;
					}
				}

				pEffect->EndPass();
			}
			pEffect->end();
		}
	}
}

void CParaXModel::RenderSoftAnim(SceneState* pSceneState, CParameterBlock* pMaterialParams)
{
	int nPasses = (int)passes.size();
	if (nPasses <= 0)
		return;
	// define this to generate performance report
	//#define RenderSoftAnim_PERFORMANCE_TEST
#ifdef RenderSoftAnim_PERFORMANCE_TEST
	PERF1("RenderSoftAnim");
#endif


	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal));

	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if (pEffect == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		///////////////////////////////////////////////////////////////////////////
		// fixed function pipeline
		for (int nPass = 0; nPass < nPasses; nPass++)
		{
			ModelRenderPass& p = passes[nPass];

			if (p.geoset >= 0 && showGeosets[p.geoset])
			{
				// skip and build for translucent pass
				if (pSceneState->m_bEnableTranslucentFaceSorting &&
					((int)m_TranslucentPassIndice.size()) > nPass && m_TranslucentPassIndice[nPass] >= 0)
				{
					if (!pSceneState->IsIgnoreTransparent())
					{
						Matrix4 mat = CGlobals::GetWorldMatrixStack().top();

						const ModelVertex& ov = m_origVertices[m_indices[p.m_nIndexStart]];
						// use the four bone matrix. 
						if (ov.weights[0] > 0)
						{
							Matrix4 matLocal = bones[ov.bones[0]].mat * ((float)ov.weights[0] * (1 / 255.0f));
							for (int b = 1; b < 4 && ov.weights[b]>0; b++) {
								matLocal += bones[ov.bones[b]].mat * ((float)ov.weights[b] * (1 / 255.0f));
							}
							mat = matLocal * mat;
						}
						CFaceGroupInstance faceGroup(&mat, m_faceGroups[m_TranslucentPassIndice[nPass]]);
						if (p.texanim != -1) {
							const TextureAnim& texAnim = texanims[p.texanim];
							faceGroup.m_vUVOffset.x = texAnim.tval.x;
							faceGroup.m_vUVOffset.y = texAnim.tval.y;

							faceGroup.m_vUVRotate = texAnim.rval;

							if (texAnim.scale.used)
							{
								faceGroup.m_vUVScale.x = texAnim.sval.x;
								faceGroup.m_vUVScale.y = texAnim.sval.y;
							}
						}
						pSceneState->GetFaceGroups()->AddFaceGroup(faceGroup);
					}

					continue;
				}

				if (p.init(this, pSceneState))
				{
					DrawPass(p);
					p.deinit();
				}
			}
		}
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		if (pEffect->begin())
		{
			if (pEffect->BeginPass(GetRenderPass(pMaterialParams)))
			{
				/* if this is defined, we will combine render pass with similar textures and attributes.
				It is very strange that combining render pass is slower. */
				// #define COMBINE_RENDER_PASS

#ifdef COMBINE_RENDER_PASS
				ModelRenderPass* pLastPass = NULL;
#endif
				for (int nPass = 0; nPass < nPasses; nPass++)
				{
					ModelRenderPass& p = passes[nPass];

					if (p.geoset >= 0 && showGeosets[p.geoset])
					{
						// skip and build for translucent pass
						if (pSceneState->m_bEnableTranslucentFaceSorting &&
							((int)m_TranslucentPassIndice.size()) > nPass && m_TranslucentPassIndice[nPass] >= 0)
						{
							if (!pSceneState->IsIgnoreTransparent())
							{
								float alpha = 1.f;
								if (p.opacity != -1)
								{
									//typing error,m_CurrentAnim.nIndex is always 0?? 
									//change m_CurrentAnim.nIndex to m_CurrentAnim  --clayman 2012.8.21
									//alpha = transparency[p.opacity].trans.getValue(m_CurrentAnim.nIndex);
									alpha = transparency[p.opacity].trans.getValue(m_CurrentAnim);
									if (alpha < 0)
										continue;
								}

								Matrix4 mat = CGlobals::GetWorldMatrixStack().top();

								const ModelVertex& ov = m_origVertices[m_indices[p.m_nIndexStart]];
								// use four bone matrix. 
								if (ov.weights[0] > 0 && m_faceGroups[m_TranslucentPassIndice[nPass]]->m_bSkinningAni == false)
								{
									Matrix4 matLocal = bones[ov.bones[0]].mat * ((float)ov.weights[0] * (1 / 255.0f));
									for (int b = 1; b < 4 && ov.weights[b]>0; b++) {
										matLocal += bones[ov.bones[b]].mat * ((float)ov.weights[b] * (1 / 255.0f));
									}
									mat = matLocal * mat;
								}
								CFaceGroupInstance faceGroup(&mat, m_faceGroups[m_TranslucentPassIndice[nPass]]);
								if (p.texanim != -1) {
									const TextureAnim& texAnim = texanims[p.texanim];
									faceGroup.m_vUVOffset.x = texAnim.tval.x;
									faceGroup.m_vUVOffset.y = texAnim.tval.y;

									faceGroup.m_vUVRotate = texAnim.rval;

									if (texAnim.scale.used)
									{
										faceGroup.m_vUVScale.x = texAnim.sval.x;
										faceGroup.m_vUVScale.y = texAnim.sval.y;
									}

									//support texture uv rgb animation --clayman 2011.8.8
									if (animTexRGB)
										faceGroup.m_UVRgbAnim = true;
								}
								// opacity
								faceGroup.m_fAlpha = alpha;
								faceGroup.m_bones = bones;
								pSceneState->GetFaceGroups()->AddFaceGroup(faceGroup);
							}

							continue;
						}
#ifdef COMBINE_RENDER_PASS
						// we shall combine render pass if current one is same as previous, using the overloaded p.operator == 
						if (pLastPass == NULL)
						{
							if (p.init_FX(this))
							{
								pLastPass = &p;
								pEffect->CommitChanges();
								DrawPass(p);
							}
						}
						else
						{
							if ((*pLastPass == p))
							{
								DrawPass(p);
							}
							else
							{
								pLastPass->deinit_FX(pSceneState, pMaterialParams);
								if (p.init_FX(this))
								{
									pLastPass = &p;
									pEffect->CommitChanges();
									DrawPass(p);
								}
							}
						}
#else
						// do not combine render pass. this appears to be faster than combined render passes. 
						if (p.init_FX(this, pSceneState, pMaterialParams))
						{
							pEffect->onDrawPass(pMaterialParams, nPass);
							pEffect->CommitChanges();
							DrawPass(p);
							p.deinit_FX(pSceneState, pMaterialParams);
						}
#endif
					}
				}
#ifdef COMBINE_RENDER_PASS
				if (pLastPass != NULL)
				{
					pLastPass->deinit_FX(pSceneState, pMaterialParams);
				}
#endif
				pEffect->EndPass(0);
			}
			pEffect->end();
		}
	}
}

void CParaXModel::DrawPass_BMax_VB(ModelRenderPass& p, size_t start)
{
	if (m_vbState != INITED)
	{
		DrawPass_BMax(p);
		return;
	}

	if (p.indexCount == 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, (UINT)start, p.indexCount / 3);
}

void CParaXModel::DrawPass_NoAnim_VB(ModelRenderPass& p, size_t start)
{
	if (m_vbState != INITED)
	{
		DrawPass_NoAnim(p);
		return;
	}

	if (p.indexCount == 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, (UINT)start, p.indexCount / 3);
}


void CParaXModel::DrawPass_NoAnim(ModelRenderPass& p)
{
	if (p.indexCount == 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	{
		mesh_vertex_normal* vb_vertices = NULL;
		int nVertexOffset = p.GetVertexStart(this);
		ModelVertex* ov = m_origVertices;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((p.indexCount - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;

				int nIndexOffset = p.m_nIndexStart + nNumFinishedVertice;
				for (int i = 0; i < nLockedNum; ++i)
				{
					int nVB = 3 * i;
					for (int k = 0; k < 3; ++k, ++nVB)
					{
						int a = m_indices[nIndexOffset + nVB] + nVertexOffset;
						mesh_vertex_normal& out_vertex = vb_vertices[nVB];
						// weighted vertex
						ov = m_origVertices + a;
						out_vertex.p = ov->pos;
						out_vertex.n = ov->normal;
						out_vertex.uv = ov->texcoords;
					}
				}
				pBufEntity->Unlock();

				if (pBufEntity->IsMemoryBuffer())
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((p.indexCount - nNumFinishedVertice) > nNumLockedVertice)
				{
					nNumFinishedVertice += nNumLockedVertice;
				}
				else
					break;
			}
			else
				break;
		} while (1);
	}
}


void CParaXModel::DrawPass_BMax(ModelRenderPass& p)
{
	if (p.indexCount == 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	bmax_vertex* vb_vertices = NULL;
	ModelVertex* ov = m_origVertices;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);
	do
	{
		if ((nNumLockedVertice = pBufEntity->Lock((p.indexCount - nNumFinishedVertice),
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice / 3;

			bmax_vertex  vertex;
			int nIndexOffset = p.m_nIndexStart + nNumFinishedVertice;
			int nVertexOffset = p.GetVertexStart(this);
			ModelVertex* origVertices = m_origVertices + nVertexOffset;
			if (HasAnimation())
			{
				for (int i = 0; i < nLockedNum; ++i)
				{
					int nVB = 3 * i;
					for (int k = 0; k < 3; ++k, ++nVB)
					{
						int a = m_indices[nIndexOffset + nVB];
						bmax_vertex& out_vertex = vb_vertices[nVB];
						// weighted vertex
						ov = origVertices + a;

						float weight = ov->weights[0] * (1 / 255.0f);
						Bone& bone = bones[ov->bones[0]];
						Vector3 v = (ov->pos * bone.mat) * weight;
						Vector3 n = ov->normal.TransformNormal(bone.mrot) * weight;
						for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
							weight = ov->weights[b] * (1 / 255.0f);
							Bone& bone = bones[ov->bones[b]];
							v += (ov->pos * bone.mat) * weight;
							n += ov->normal.TransformNormal(bone.mrot) * weight;
						}
						out_vertex.p = v;
						out_vertex.n = n;
						out_vertex.color = ov->color0;
					}
				}
			}
			else
			{
				for (int i = 0; i < nLockedNum; ++i)
				{
					int nVB = 3 * i;
					for (int k = 0; k < 3; ++k, ++nVB)
					{
						uint16 a = m_indices[nIndexOffset + nVB];
						bmax_vertex& out_vertex = vb_vertices[nVB];
						ov = origVertices + a;
						out_vertex.p = ov->pos;
						out_vertex.n = ov->normal;
						out_vertex.color = ov->color0;
					}
				}
			}

			pBufEntity->Unlock();

			if (pBufEntity->IsMemoryBuffer())
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
			else
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

			if ((p.indexCount - nNumFinishedVertice) > nNumLockedVertice)
			{
				nNumFinishedVertice += nNumLockedVertice;
			}
			else
				break;
		}
		else
			break;
	} while (1);
}

void CParaXModel::DrawPass(ModelRenderPass& p)
{
	// uncomment to generate performance result in order to fine tune optimizations.  
	// #define DO_PERFORMANCE_TEST
	if (p.indexCount == 0)
		return;
	if (p.is_rigid_body)
	{
		// for rigid body with many vertices, do skinning on GPU instead of CPU. 
		DrawPass_NoAnim(p);
		return;
	}

#ifdef DO_PERFORMANCE_TEST
	if (p.indexCount < 500)
		return;
	PERF1("SoftSkinningDrawPass");
#endif
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	mesh_vertex_normal* vb_vertices = NULL;
	ModelVertex* ov = m_origVertices;
	int nVertexOffset = p.GetVertexStart(this);
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);

	do
	{
		if ((nNumLockedVertice = pBufEntity->Lock((p.indexCount - nNumFinishedVertice),
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice / 3;

#ifdef DO_PERFORMANCE_TEST
			if (nLockedNum > 200)
			{
				PERF1("SoftSkinning");
#endif
				mesh_vertex_normal  vertex;
				int nIndexOffset = p.m_nIndexStart + nNumFinishedVertice;
				for (int i = 0; i < nLockedNum; ++i)
				{
					int nVB = 3 * i;
					for (int k = 0; k < 3; ++k, ++nVB)
					{
						int a = m_indices[nIndexOffset + nVB] + nVertexOffset;
						mesh_vertex_normal& out_vertex = vb_vertices[nVB];
						// weighted vertex
						ov = m_origVertices + a;

						// uncomment to detect incorrect index. 
						// assert(a < m_objNum.nVertices, "index overflow");

						// TODO: m_nCurrentFrameNumber can not be replaced by CGlobals::GetViewportManager()->getCurrentFrameNumber()
						if (m_frame_number_vertices[a] != m_nCurrentFrameNumber)
						{
							m_frame_number_vertices[a] = m_nCurrentFrameNumber;

							float weight = ov->weights[0] * (1 / 255.0f);
							Bone& bone = bones[ov->bones[0]];
							Vector3 v = (ov->pos * bone.mat) * weight;
							Vector3 n = ov->normal.TransformNormal(bone.mrot) * weight;
							for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
								weight = ov->weights[b] * (1 / 255.0f);
								Bone& bone = bones[ov->bones[b]];
								v += (ov->pos * bone.mat) * weight;
								n += ov->normal.TransformNormal(bone.mrot) * weight;
							}
							out_vertex.p = v;
							out_vertex.n = n;

							// save the animated vertex in case it is reused in the same frame. 
							m_vertices[a] = v;
							m_normals[a] = n;
						}
						else
						{
							out_vertex.p = m_vertices[a];
							out_vertex.n = m_normals[a];
						}
						out_vertex.uv = ov->texcoords;
					}
				}
#ifdef DO_PERFORMANCE_TEST
			}
#endif
			pBufEntity->Unlock();

			if (pBufEntity->IsMemoryBuffer())
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
			else
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

			if ((p.indexCount - nNumFinishedVertice) > nNumLockedVertice)
			{
				nNumFinishedVertice += nNumLockedVertice;
			}
			else
				break;
		}
		else
			break;
	} while (1);
}

void CParaXModel::RenderShaderAnim(SceneState* pSceneState)
{

}

void CParaXModel::drawModel(SceneState* pSceneState, CParameterBlock* pMaterialParam, int nRenderMethod)
{
	if (passes.size() == 0)
		return;
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if (pEffect == 0)
		CGlobals::GetRenderDevice()->SetTransform(ETransformsStateType::WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
	else
	{
		/// apply surface materials
		bool bEnableLight = pSceneState->GetScene()->IsLightEnabled();
		if (bEnableLight)
		{
			ParaMaterial mat = pSceneState->GetCurrentMaterial();
			if (!pSceneState->HasLocalMaterial())
				mat.Diffuse = LinearColor(1.f, 1.f, 1.f, 1.f);
			pEffect->applySurfaceMaterial(&mat);
		}
		else
		{
			ParaMaterial mat = pSceneState->GetCurrentMaterial();
			if (!pSceneState->HasLocalMaterial())
			{
				mat.Ambient = LinearColor(0.6f, 0.6f, 0.6f, 1.f); // shall we use ambient to simulate lighting, when lighting is disabled.
				mat.Diffuse = LinearColor(1.f, 1.f, 1.f, 1.f);
			}
			pEffect->applySurfaceMaterial(&mat);
		}
	}



	if (nRenderMethod < 0)
		nRenderMethod = m_RenderMethod;

	if (m_vbState == NOT_SET)
	{
		if (nRenderMethod == NO_ANIM
			|| (nRenderMethod == BMAX_MODEL && !HasAnimation()))
		{
			if (m_uUsedVB < MAX_USE_VERTEX_BUFFER_SIZE)
			{
				m_vbState = NEED_INIT;
				InitVertexBuffer();
			}
		}
		else
		{
			m_vbState = NOT_USE;
		}
	}

	switch (nRenderMethod)
	{
	case SHADER_ANIM:
		RenderShaderAnim(pSceneState);
		break;
	case SOFT_ANIM:
		RenderSoftAnim(pSceneState, pMaterialParam);
		break;
	case NO_ANIM:
	{
		RenderSoftNoAnim(pSceneState, pMaterialParam);
	}
	break;
	case BMAX_MODEL:
	{
		RenderBMaxModel(pSceneState, pMaterialParam);
	}

	break;
	default:
		break;
	}
}

void CParaXModel::BuildShadowVolume(ShadowVolume* pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
	// TODO: for animation, the m_indices are null. figure out how.
	if (m_RenderMethod == NO_ANIM)
	{
		return;
	}
#ifdef USE_DIRECTX_RENDERER
#ifdef SHADOW_ZFAIL_WITHOUTCAPS
	pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_PASS;
	//geosets.BuildShadowVolume(this,currentAnimInfo, pShadowVolume, pLight, mxWorld);
	pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_FAIL;
#else
	// get the bounding box and check which shadow rendering method we use.
	const Vector3& mMinimum = m_header.minExtent;
	const Vector3& mMaximum = m_header.maxExtent;

	/** check if object is too small, if so its shadow will not be rendered */
	{
		Vector3 bottom, top;
		bottom = (mMinimum + mMaximum) / 2.0f;
		bottom.y = mMinimum.y;
		top = bottom;
		top.y = mMaximum.y;
		Vector3 screenTop, screenBottom;
		pShadowVolume->ProjectPoint(&screenBottom, &bottom, mxWorld);
		pShadowVolume->ProjectPoint(&screenTop, &top, mxWorld);

		float screenDistX = (float)fabs(screenTop.x - screenBottom.x);
		float screenDistY = (float)fabs(screenTop.y - screenBottom.y);
		if (screenDistX < pShadowVolume->m_fMinShadowCastDistance && screenDistY < pShadowVolume->m_fMinShadowCastDistance)
		{
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_NONE;
			return;
		}
	}
	/** check to see if object may be in shadow. We only performance this test if object does not
	enforce capping */
	// if(not capping)
	{
		Vector3 mCorners[8];
		mCorners[0] = mMinimum;
		mCorners[1].x = mMinimum.x; mCorners[1].y = mMaximum.y; mCorners[1].z = mMinimum.z;
		mCorners[2].x = mMaximum.x; mCorners[2].y = mMaximum.y; mCorners[2].z = mMinimum.z;
		mCorners[3].x = mMaximum.x; mCorners[3].y = mMinimum.y; mCorners[3].z = mMinimum.z;

		mCorners[4] = mMaximum;
		mCorners[5].x = mMinimum.x; mCorners[5].y = mMaximum.y; mCorners[5].z = mMaximum.z;
		mCorners[6].x = mMinimum.x; mCorners[6].y = mMinimum.y; mCorners[6].z = mMaximum.z;
		mCorners[7].x = mMaximum.x; mCorners[7].y = mMinimum.y; mCorners[7].z = mMaximum.z;

		for (int i = 0; i < 8; i++)
		{
			ParaVec3TransformCoord(&mCorners[i], &mCorners[i], mxWorld);
		}

		if (pShadowVolume->PointsInsideOcclusionPyramid(mCorners, 8))
		{
			/** we use z-fail algorithm, if camera may be in the shadow or the object does not
			performance screen distance testing */
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_FAIL;
		}
		else
			pShadowVolume->m_shadowMethod = ShadowVolume::SHADOW_Z_PASS;
	}
	/** build the shadow volume */

	// TODO: light's direction relative to the object.
	// here we assume the light is a directional light.
	Vector3 vLight = pLight->Direction;
	float fRange = pLight->Range;
	vLight = -vLight;

	int nUseCap = ((pShadowVolume->m_shadowMethod == ShadowVolume::SHADOW_Z_FAIL) ? 1 : 0);
	bool bBaseModelRendered = false;
	for (size_t i = 0; i < passes.size(); i++) {
		ModelRenderPass& p = passes[i];

		/**
		* TODO: although, we don't want to render completely transparent parts,
		* we will render it anyway if the geoset is 0, which is usually the base model.
		* this is just a work around. In future, I will specify a certain geoset ID as the shadow model and render it only.
		*/
		if ((p.geoset >= 0 && showGeosets[p.geoset]) && (p.blendmode == BM_OPAQUE || (p.geoset == 0 && !bBaseModelRendered)))
		{
			if (p.geoset == 0)
				bBaseModelRendered = true;
			ModelVertex* ov = m_origVertices;
			int nNumFaces = p.indexCount / 3;
			Vector3* pVertices = NULL;
			DWORD dwNumVertices = 0;
			DWORD dwNumEdges = 0;

			// Allocate a temporary edge list
			std::unordered_set <EdgeHash, hash_compare_edge> m_edgeTable;
			if (nUseCap > 0)
				pShadowVolume->ReserveNewBlock(&pVertices, nNumFaces * 3);

			// the three vertices of each face
			Vector3 FaceV[3];
			WORD wFace[3]; // index of the three vertices of each face
			for (int i = 0; i < nNumFaces; ++i)
			{
				// compute the three vertices of each face
				for (int k = 0; k < 3; ++k)
				{
					int nVB = 3 * i + k;
					wFace[k] = m_indices[p.m_nIndexStart + nVB];
					// weighted vertex
					ParaVec3TransformCoord(&(FaceV[k]), &GetWeightedVertexByIndex(wFace[k]), mxWorld);
				}
				// Transform vertices or transform light?
				// we use vertex transform, it may be more accurate to use light transform
				Vector3 vCross1(FaceV[2] - FaceV[1]);
				Vector3 vCross2(FaceV[1] - FaceV[0]);
				Vector3 vNormal;
				vNormal = vCross1.crossProduct(vCross2);

				if (vNormal.dotProduct(vLight) >= 0.0f)
				{
					CEdgeBuilder::AddEdge(m_edgeTable, dwNumEdges, wFace[0], wFace[1]);
					CEdgeBuilder::AddEdge(m_edgeTable, dwNumEdges, wFace[1], wFace[2]);
					CEdgeBuilder::AddEdge(m_edgeTable, dwNumEdges, wFace[2], wFace[0]);

					if (nUseCap > 0)
					{
						pVertices[dwNumVertices++] = FaceV[0];
						pVertices[dwNumVertices++] = FaceV[2];
						pVertices[dwNumVertices++] = FaceV[1];
					}
				}
			} // for(int i=0;i<nNumFaces;++i)

			if (nUseCap > 0)
			{
				// commit shadow volume front cap vertices
				pShadowVolume->CommitBlock(dwNumVertices);
				dwNumVertices = 0;
				pVertices = NULL;
			}
			/**
			build shadow volume for the edge array
			Interestingly, the extrusion of geometries for point light sources and
			infinite directional light sources are different. see below.
			*/
			if (pLight->bIsDirectional)
			{
				/**
				infinite directional light sources would extrude all silhouette edges to
				a single point at infinity.
				*/
				pShadowVolume->ReserveNewBlock(&pVertices, dwNumEdges * 3);
				Vector3 v3 = Vector3(mxWorld->_41, mxWorld->_42, mxWorld->_43) + pLight->Direction * pLight->Range;

				std::unordered_set <EdgeHash, hash_compare_edge>::iterator itCurCP, itEndCP = m_edgeTable.end();

				// first shutdown all connections
				for (itCurCP = m_edgeTable.begin(); itCurCP != itEndCP; ++itCurCP)
				{
					int index1 = (*itCurCP).m_v0;
					int index2 = (*itCurCP).m_v1;

					Vector3 v1 = GetWeightedVertexByIndex(index1);
					Vector3 v2 = GetWeightedVertexByIndex(index2);
					ParaVec3TransformCoord(&v1, &v1, mxWorld);
					ParaVec3TransformCoord(&v2, &v2, mxWorld);

					// Add a quad (two triangles) to the vertex list
					pVertices[dwNumVertices++] = v1;
					pVertices[dwNumVertices++] = v2;
					pVertices[dwNumVertices++] = v3;
				}
				pShadowVolume->CommitBlock(dwNumVertices);
			}
		}
	}
#endif
#endif
}

Vector3 CParaXModel::GetWeightedVertexByIndex(unsigned short nIndex)
{
	ModelVertex* ov = m_origVertices + nIndex;
	Vector3 v = bones[ov->bones[0]].mat * ov->pos * ((float)ov->weights[0] * (1 / 255.0f));
	for (int b = 1; b < 4 && ov->weights[b]>0; b++) {
		v += bones[ov->bones[b]].mat * ov->pos * ((float)ov->weights[b] * (1 / 255.0f));
	}
	return (Vector3)v;
}


void CParaXModel::draw(SceneState* pSceneState, CParameterBlock* materialParams, int nRenderMethod)
{
	if (!m_bIsValid) return;

	m_nCurrentFrameNumber++;

	if (!animated) {
		// TODO: maybe a faster way to render using Static mesh interface
		// so there is no need to go with render passes
		drawModel(pSceneState, materialParams, nRenderMethod);
	}
	else {
		/// assume that the animate() function has already been called to set the bone matrices

		drawModel(pSceneState, materialParams, nRenderMethod);

		if (!pSceneState->IsIgnoreTransparent())
		{
			// update particles
			float fDeltaTime = (float)pSceneState->dTimeDelta;
			if (fDeltaTime > 0)
			{
				updateEmitters(pSceneState, fDeltaTime);
			}
			// draw particle systems
			uint32 nParticleEmitters = GetObjectNum().nParticleEmitters;
			for (size_t i = 0; i < nParticleEmitters; i++) {
				particleSystems[i].draw(pSceneState);
			}

			// draw ribbons
			uint32 nRibbonEmitters = GetObjectNum().nRibbonEmitters;
			for (size_t i = 0; i < nRibbonEmitters; i++) {
				ribbons[i].draw(pSceneState);
			}
		}
	}
}

void CParaXModel::lightsOn(uint32 lbase)
{
	// setup lights
	uint32 nLights = GetObjectNum().nLights;
	for (uint32 i = 0, l = lbase; i < nLights; i++)
		lights[i].setup(m_CurrentAnim.nCurrentFrame, l++);
}

void CParaXModel::lightsOff(uint32 lbase)
{
}

void CParaXModel::updateEmitters(SceneState* pSceneState, float dt)
{
	if (!m_bIsValid) return;
	uint32 nParticleEmitters = GetObjectNum().nParticleEmitters;
	for (size_t i = 0; i < nParticleEmitters; i++) {
		particleSystems[i].update(pSceneState, dt);
	}
}

float CParaXModel::GetBoundingRadius()
{
	return m_radius;
};

void CParaXModel::drawBones()
{

}

void CParaXModel::drawBoundingVolume()
{

}
bool CParaXModel::canAttach(int id)
{
	return ((int)GetObjectNum().nAttachLookup > id) && (m_attLookup[id] != -1);
}

int CParaXModel::GetPolyCount()
{
	int nCount = 0;
	int nPasses = (int)passes.size();
	for (int nPass = 0; nPass < nPasses; nPass++)
	{
		ModelRenderPass& p = passes[nPass];
		nCount += p.indexCount / 3;
	}
	return nCount;
}

int CParaXModel::GetPhysicsCount()
{
	return 0;
}

const char* CParaXModel::DumpTextureUsage()
{
	static string g_output;
	g_output.clear();

	char temp[200];
	snprintf(temp, 200, "cnt:%d;", GetObjectNum().nTextures);
	g_output.append(temp);

	for (size_t i = 0; i < GetObjectNum().nTextures; i++)
	{
		if (textures[i])
		{
			const TextureEntity::TextureInfo* pInfo = textures[i]->GetTextureInfo();
			if (pInfo)
			{
				char temp[200];
				snprintf(temp, 200, "%d*%d(%d)", pInfo->m_width, pInfo->m_height, pInfo->m_format);
				g_output.append(temp);
			}
			g_output.append(textures[i]->GetKey());
			g_output.append(";");
		}
	}
	return g_output.c_str();
}

bool CParaXModel::HasAlphaBlendedObjects()
{
	if (m_nHasAlphaBlendedRenderPass >= 0)
		return m_nHasAlphaBlendedRenderPass > 0;
	else
	{
		for (ModelRenderPass& p : passes)
		{
			if (p.blendmode != BM_OPAQUE && p.blendmode != BM_TRANSPARENT)
			{
				m_nHasAlphaBlendedRenderPass = 1;
				return true;
			}
		}
		m_nHasAlphaBlendedRenderPass = 0;
		return false;
	}
}

int CParaXModel::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0) {
		return (int)GetObjectNum().nBones;
	}
	else if (nColumnIndex == 1) {
		return (int)GetObjectNum().nTextures;
	}
	else if (nColumnIndex == 2) {
		return (m_pVoxelModel != NULL) ? 1 : 0;
	}
	return 0;
}

IAttributeFields* CParaXModel::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		if (nRowIndex < (int)GetObjectNum().nBones)
			return &(bones[nRowIndex]);
	}
	else if (nColumnIndex == 1)
	{
		if (nRowIndex < (int)GetObjectNum().nTextures)
		{
			if (textures[nRowIndex])
			{
				return textures[nRowIndex].get();
			}
		}
	}
	else if (nColumnIndex == 2)
	{
		return m_pVoxelModel;
	}
	return 0;
}

IAttributeFields* CParaXModel::GetChildAttributeObject(const char* sName)
{
	if (strcmp(sName, "VoxelModel") == 0)
	{
		if (m_pVoxelModel == NULL) {
			m_pVoxelModel = new ParaVoxelModel();
		}
		return m_pVoxelModel;
	}
	return 0;
}

int CParaXModel::GetChildAttributeColumnCount()
{
	return 3;
}

int CParaXModel::GetNextPhysicsGroupID(int nPhysicsGroup)
{
	int nNextID = -1;
	for (ModelRenderPass& pass : passes)
	{
		if (pass.hasPhysics() && pass.GetPhysicsGroup() > nPhysicsGroup)
		{
			if (nNextID > pass.GetPhysicsGroup() || nNextID == -1)
			{
				nNextID = pass.GetPhysicsGroup();
			}
		}
	}
	return nNextID;
}

HRESULT CParaXModel::RendererRecreated()
{
	m_pIndexBuffer.RendererRecreated();
	m_pVertexBuffer.RendererRecreated();

	this->SetVertexBufferDirty();

	return S_OK;
}

HRESULT CParaXModel::ClonePhysicsMesh(DWORD* pNumVertices, Vector3** ppVerts, DWORD* pNumTriangles, DWORD** ppIndices, int* pnMeshPhysicsGroup /*= NULL*/, int* pnTotalMeshGroupCount /*= NULL*/)
{
	if (m_objNum.nVertices == 0 || !m_indices)
		return E_FAIL;

	if (pnTotalMeshGroupCount)
	{
		int nTotalMeshGroupCount = 0;
		int nPhysicsGroup = -1;
		while ((nPhysicsGroup = GetNextPhysicsGroupID(nPhysicsGroup)) >= 0)
		{
			nTotalMeshGroupCount++;
		}
		*pnTotalMeshGroupCount = nTotalMeshGroupCount;
	}

	for (ModelRenderPass& pass : passes)
	{
		if (pass.force_physics)
		{
			for (ModelRenderPass& pass : passes)
			{
				if (!pass.force_physics)
				{
					pass.disable_physics = true;
				}
			}
			break;
		}
	}

	DWORD dwNumFaces = 0;
	int nVertexCount = 0;
	for (ModelRenderPass& pass : passes)
	{
		if (pass.geoset >= 0 && pass.hasPhysics() && (pnMeshPhysicsGroup == 0 || ((*pnMeshPhysicsGroup) == pass.GetPhysicsGroup())))
		{
			dwNumFaces += pass.indexCount / 3;
		}
	}

	if (dwNumFaces == 0)
	{
		// in case, there is no physics faces in the mesh, return immediately.
		if (pNumVertices != 0)
			*pNumVertices = 0;
		if (pNumTriangles != 0)
			*pNumTriangles = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// read the vertex buffer
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumVx = 0;
	Vector3* verts = NULL;
	if (ppVerts != NULL)
	{
		dwNumVx = m_objNum.nVertices;
		verts = new Vector3[dwNumVx];
		auto pVertices = &(m_origVertices[0]);

		for (DWORD i = 0; i < dwNumVx; ++i)
		{
			verts[i] = pVertices->pos;
			pVertices++;
		}
		if (m_RenderMethod == SOFT_ANIM)
		{
			if (m_frame_number_vertices == 0)
				m_frame_number_vertices = new int[dwNumVx];
			memset(m_frame_number_vertices, 0, sizeof(int) * dwNumVx);
		}
	}
	if (m_objNum.nBones > 0 && !bones[0].calc)
		calcBones();

	//////////////////////////////////////////////////////////////////////////
	// read the index buffer
	//////////////////////////////////////////////////////////////////////////
	DWORD* indices = NULL;
	if (ppIndices)
	{
		indices = new DWORD[dwNumFaces * 3];
		int nD = 0; // destination indices index

		for (ModelRenderPass& pass : passes)
		{
			if (pass.geoset >= 0 && pass.hasPhysics() && (pnMeshPhysicsGroup == 0 || ((*pnMeshPhysicsGroup) == pass.GetPhysicsGroup())))
			{
				int nVertexOffset = pass.GetVertexStart(this);
				if (m_RenderMethod == SOFT_ANIM)
				{
					int nIndexOffset = pass.m_nIndexStart;
					for (int i = 0; i < pass.indexCount; ++i)
					{
						int a = m_indices[nIndexOffset + i] + nVertexOffset;
						if (m_frame_number_vertices[a] != 1)
						{
							m_frame_number_vertices[a] = 1;
							auto ov = m_origVertices + a;
							float weight = ov->weights[0] * (1 / 255.0f);
							Bone& bone = bones[ov->bones[0]];
							Vector3 v = (ov->pos * bone.mat) * weight;
							for (int b = 1; b < 4 && ov->weights[b]>0; b++)
							{
								weight = ov->weights[b] * (1 / 255.0f);
								Bone& bone = bones[ov->bones[b]];
								v += (ov->pos * bone.mat) * weight;
							}
							verts[a] = v;
						}
					}
				}

#ifdef INVERT_PHYSICS_FACE_WINDING
				int32* dest = (int32*)&(indices[nD]);
				int16* src;
				if (pass.indexStart == 0xffff)
					src = &(m_indices[pass.m_nIndexStart]);
				else
					src = &(m_indices[pass.indexStart]);
				int nFaceCount = pass.indexCount / 3;
				for (int i = 0; i < nFaceCount; ++i)
				{
					// change the triangle winding order
					*dest = *src + nVertexOffset; ++src;
					*(dest + 2) = *src + nVertexOffset; ++src;
					*(dest + 1) = *src + nVertexOffset; ++src;
					dest += 3;
				}
#else
				if (pass.indexStart == 0xffff)
				{
					for (int i = 0; i < pass.indexCount; ++i)
					{
						indices[nD + i] = m_indices[pass.m_nIndexStart + i] + nVertexOffset;
					}
				}
				else
				{
					for (int i = 0; i < pass.indexCount; ++i)
					{
						indices[nD + i] = m_indices[pass.indexStart + i] + nVertexOffset;
					}
				}

#endif
				nD += pass.indexCount;
			}
		}
	}
	// output result
	if (pNumVertices != 0) {
		*pNumVertices = dwNumVx;
	}
	if (ppVerts != 0) {
		*ppVerts = verts;
	}
	if (pNumTriangles != 0) {
		*pNumTriangles = dwNumFaces;
	}
	if (ppIndices != 0) {
		*ppIndices = indices;
	}
	return S_OK;
}

const char* CParaXModel::GetStrAnimIds()
{
	int nAnim = (int)GetObjectNum().nAnimations;
	thread_local static std::string strAnimIds;
	strAnimIds.clear();

	for (int i = 0; i < nAnim; i++) {
		strAnimIds = strAnimIds + std::to_string(anims[i].animID) + ";";
	}

	const char* _strAnimIds = strAnimIds.c_str();
	return _strAnimIds;
}

int CParaXModel::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass != NULL);
	pClass->AddField("TextureUsage", FieldType_String, (void*)0, (void*)DumpTextureUsage_s, NULL, NULL, bOverride);
	pClass->AddField("PolyCount", FieldType_Int, (void*)0, (void*)GetPolyCount_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsCount", FieldType_Int, (void*)0, (void*)GetPhysicsCount_s, NULL, NULL, bOverride);

	pClass->AddField("GeosetsCount", FieldType_Int, (void*)0, (void*)GetGeosetsCount_s, NULL, NULL, bOverride);
	pClass->AddField("RenderPassesCount", FieldType_Int, (void*)0, (void*)GetRenderPassesCount_s, NULL, NULL, bOverride);

	pClass->AddField("ObjectNum", FieldType_void_pointer, (void*)0, (void*)GetObjectNum_s, NULL, NULL, bOverride);
	pClass->AddField("Vertices", FieldType_void_pointer, (void*)0, (void*)GetVertices_s, NULL, NULL, bOverride);
	pClass->AddField("RenderPasses", FieldType_void_pointer, (void*)0, (void*)GetRenderPasses_s, NULL, NULL, bOverride);
	pClass->AddField("Geosets", FieldType_void_pointer, (void*)0, (void*)GetGeosets_s, NULL, NULL, bOverride);
	pClass->AddField("Indices", FieldType_void_pointer, (void*)0, (void*)GetIndices_s, NULL, NULL, bOverride);
	pClass->AddField("Animations", FieldType_void_pointer, (void*)0, (void*)GetAnimations_s, NULL, NULL, bOverride);
	pClass->AddField("SaveToDisk", FieldType_String, (void*)SaveToDisk_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("SaveToGltf", FieldType_String, (void*)SaveToGltf_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("strAnimIds", FieldType_String, (void*)0, (void*)GetStrAnimIds_s, NULL, NULL, bOverride);

	return S_OK;
}

void CParaXModel::SaveToDisk(const char* path)
{
	string filepath(path);
	RemoveUnusedAnimKeys();
	XFileCharModelExporter::Export(filepath, this);
}

void CParaXModel::RemoveUnusedAnimKeys()
{
	uint32 nBones = (uint32)GetObjectNum().nBones;
	for (uint32 i = 0; i < nBones; i++)
	{
		Bone& bone = bones[i];
		bone.RemoveUnusedAnimKeys();
	}
}

void CParaXModel::SaveToGltf(const char* path)
{
	GltfModel::ExportParaXModel(this, path);
	// glTFModelExporter::ParaXExportTo_glTF(this, path);
}
