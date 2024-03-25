//----------------------------------------------------------------------
// Class:	XFile Character Model Parser
// Authors:	LiXizhi
// Date:	2014.10.3
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"

#include "util/StringHelper.h"
#include "ParaWorldAsset.h"
#include "ParaXModel.h"
#include "ParaXBone.h"
#include "ParaVoxelModel.h"
#include "particle.h"
#include "XFileParsing.inl"
#include "XFileHelper.h"
#include "XFileCharModelParser.h"

using namespace ParaEngine;

ParaEngine::XFileCharModelParser::XFileCharModelParser(const char* pBuffer, int32 nSize)
	:XFileStaticModelParser(pBuffer, nSize), m_bHeaderLoaded(false), m_pRaw(NULL)
{
}

ParaEngine::XFileCharModelParser::~XFileCharModelParser()
{

}

CParaXModel* ParaEngine::XFileCharModelParser::ParseParaXModel()
{
	CParaXModel* pMesh = NULL;

	if (LoadParaX_Header()) {
		pMesh = LoadParaX_Body();
		LoadParaX_Finalize();
	}
	return pMesh;
}

bool ParaEngine::XFileCharModelParser::LoadParaX_Header()
{
	// if header already loaded, return 
	if (m_bHeaderLoaded)
		return true;
	m_pRoot = CreateEnumObject();

	if (!m_pRoot)
	{
		OUTPUT_LOG("warning: failed reading ParaX header. CreateEnumObject return D3DXFERR_BADTYPE\n");
		return false;
	}
	int nCount = m_pRoot->GetChildCount();

	XFileDataObjectPtr pData;
	for (int i = 0; i < nCount; i++)
	{
		pData = m_pRoot->GetChild(i);
		// Get the template type
		const std::string& Type = pData->GetType();

		if (Type == "ParaXHeader")
		{
			m_bHeaderLoaded = ReadParaXHeader(m_xheader, pData);
			if (!m_bHeaderLoaded)
			{
				m_xheader.type = (DWORD)PARAX_MODEL_INVALID;
				break;
			}
			pData.reset();
		}
		else if (Type == "ParaXBody")
			m_pParaXBody = pData;
		else if (Type == "XDWORDArray")
			m_pParaXRawData = pData;
		else if (Type == "ParaXRefSection")
			m_pParaXRef = pData;
		else if (Type == "D3DRMMesh")
			m_pD3DMesh = pData;
		else if (Type == "D3DRMFrame")
			m_pD3DRootFrame = pData;
		else
		{
			pData.reset();
			if (i == 0)
			{
				// this is not a ParaX file. since it does not have the ParaX header at the beginning.
				m_xheader.type = (DWORD)PARAX_MODEL_DX_STATIC;
			}
		}
	}
	return true;
}


bool ParaEngine::XFileCharModelParser::ReadParaXHeader2(CParaXModel& xmesh)
{
	if (m_xheader.nModelFormat & PARAX_FORMAT_EXTENDED_HEADER2)
	{
		ParaXHeaderDef2 header2;
		memcpy(&header2, GetRawData(m_xheader.nOffsetAdditionalHeader), sizeof(ParaXHeaderDef2));
		m_xheader.IsAnimated = header2.IsAnimated;
		xmesh.SetHeader(m_xheader);
		xmesh.m_vNeckYawAxis = header2.neck_yaw_axis;
		xmesh.m_vNeckPitchAxis = header2.neck_pitch_axis;
	}
	return true;
}

CParaXModel* ParaEngine::XFileCharModelParser::LoadParaX_Body()
{
	// load header if not done so yet.
	if (!m_bHeaderLoaded)
	{
		if (!LoadParaX_Header())
			return NULL;
	}
	CParaXModel* pMesh = NULL;

	if (m_pParaXBody)
	{
		DWORD			dwSize;
		const char* pBuffer = NULL;
		m_pRaw = NULL;
		// Lock the raw byte data if any
		if (m_pParaXRawData && (m_pParaXRawData->Lock(&dwSize, &pBuffer)))
			m_pRaw = pBuffer + 4;

		if (m_xheader.type == PARAX_MODEL_ANIMATED || m_xheader.type == PARAX_MODEL_BMAX)
		{
			pMesh = new CParaXModel(m_xheader);
			ReadParaXHeader2(*pMesh);

			// Scan for data nodes inside the ParaXBody
			int nCount = m_pParaXBody->GetChildCount();
			for (int i = 0; i < nCount; i++)
			{
				XFileDataObjectPtr pSubData = m_pParaXBody->GetChild(i);
				if (!pSubData->IsReference())
				{
					const std::string& Type = pSubData->GetType();
					// Get the template type
					if (Type == "XDWORDArray") {//XGlobalSequences
						// Get the frame name (if any)
						if (pSubData->GetName() == "XGlobalSequences") {
							ReadXGlobalSequences(*pMesh, pSubData);
						}
					}
					else if (Type == "XVertices") {//XVertices
						if (!ReadXVertices(*pMesh, pSubData))
							OUTPUT_LOG("error loading vertices");
					}
					else if (Type == "XTextures") {//XTextures
						if (!ReadXTextures(*pMesh, pSubData))
							OUTPUT_LOG("error loading XTextures");
					}
					else if (Type == "XAttachments") {//XAttachments
						if (!ReadXAttachments(*pMesh, pSubData))
							OUTPUT_LOG("error loading XAttachments");
					}
					else if (Type == "XTransparency") {//XTransparency
						if (!ReadXTransparency(*pMesh, pSubData))
							OUTPUT_LOG("error loading XTransparency");
					}
					else if (Type == "XViews") {//XViews
						if (!ReadXViews(*pMesh, pSubData))
							OUTPUT_LOG("error loading XViews");
					}
					else if (Type == "XIndices0") {//XIndices0
						if (!ReadXIndices0(*pMesh, pSubData))
							OUTPUT_LOG("error loading XIndices0");
					}
					else if (Type == "XGeosets") {//XGeosets
						if (!ReadXGeosets(*pMesh, pSubData))
							OUTPUT_LOG("error loading XGeosets");
					}
					else if (Type == "XRenderPass") {//XRenderPass
						if (!ReadXRenderPass(*pMesh, pSubData))
							OUTPUT_LOG("error loading XRenderPass");
					}
					else if (Type == "XBones") {//XBones
						if (!ReadXBones(*pMesh, pSubData))
							OUTPUT_LOG("error loading XBones");
					}
					else if (Type == "XTexAnims") {//XTexAnims
						if (!ReadXTexAnims(*pMesh, pSubData))
							OUTPUT_LOG("error loading XTexAnims");
					}
					else if (Type == "XParticleEmitters") {//XParticleEmitters
						if (!ReadXParticleEmitters(*pMesh, pSubData))
							OUTPUT_LOG("error loading XParticleEmitters");
					}
					else if (Type == "XRibbonEmitters") {//XRibbonEmitters
						if (!ReadXRibbonEmitters(*pMesh, pSubData))
							OUTPUT_LOG("error loading XRibbonEmitters");
					}
					else if (Type == "XColors") {//XColors
						if (!ReadXColors(*pMesh, pSubData))
							OUTPUT_LOG("error loading XColors");
					}
					else if (Type == "XCameras") {//XCameras
						if (!ReadXCameras(*pMesh, pSubData))
							OUTPUT_LOG("error loading XCameras");
					}
					else if (Type == "XLights") {//XLights
						if (!ReadXLights(*pMesh, pSubData))
							OUTPUT_LOG("error loading XLights");
					}
					else if (Type == "XAnimations") {//XAnimations
						if (!ReadXAnimations(*pMesh, pSubData))
							OUTPUT_LOG("error loading XAnimations");
					}
					else if (Type == "XVoxels") {
						if (!ReadXVoxels(*pMesh, pSubData))
							OUTPUT_LOG("error loading XAnimations");
					}
				}
				pSubData.reset();
			}

			if (pMesh->m_RenderMethod == CParaXModel::NO_ANIM)
			{
				pMesh->calcBones();
			}

			// optimize to see if any pass contains rigid body. For rigid body we will render without skinning, thus saving lots of CPU cycles. 
			// TODO: move this to ParaX Exporter instead. 
			{
				uint16* indices = pMesh->m_indices;
				int nRenderPasses = (int)pMesh->passes.size();
				for (int j = 0; j < nRenderPasses; ++j)
				{
					ModelRenderPass& p = pMesh->passes[j];
					int nLockedNum = p.indexCount / 3;
					if (nLockedNum > 0 && !(p.is_rigid_body))
					{
						bool bIsRigidBody = true;
						int nVertexOffset = p.GetVertexStart(pMesh);
						ModelVertex* origVertices = pMesh->m_origVertices;
						ModelVertex* ov = NULL;
						uint8 nLastBoneIndex = origVertices[indices[p.m_nIndexStart] + nVertexOffset].bones[0];

						int nIndexOffset = p.m_nIndexStart;
						for (int i = 0; i < nLockedNum && bIsRigidBody; ++i)
						{
							int nVB = 3 * i;
							for (int k = 0; k < 3; ++k, ++nVB)
							{
								uint16 a = indices[nIndexOffset + nVB] + nVertexOffset;
								ov = origVertices + a;
								// weighted vertex
								if (ov->weights[0] != 0xff || ov->bones[0] != nLastBoneIndex)
								{
									bIsRigidBody = false;
									break;
								}
							}
						}
						if (bIsRigidBody)
						{
							p.is_rigid_body = bIsRigidBody;
						}
					}
				}
			}
		}
		else if (m_xheader.type == PARAX_MODEL_DX_STATIC)
		{
			// TODO: for original dx model file.
		}


		// unlock raw byte data
		if (m_pRaw != NULL)
		{
			m_pRaw = NULL;
			m_pParaXRawData->Unlock();
		}
	}
	else if (mScene != 0)
	{
		memcpy(&mScene->m_header, &m_xheader, sizeof(ParaXHeaderDef));
		pMesh = LoadParaXModelFromScene(mScene);
		SAFE_DELETE(mScene);
	}
	return pMesh;
}

const std::string& ParaEngine::XFileCharModelParser::GetFilename() const
{
	return m_sFilename;
}

void ParaEngine::XFileCharModelParser::SetFilename(const std::string& val)
{
	m_sFilename = val;
}

void ParaEngine::XFileCharModelParser::LoadParaX_Finalize()
{
	m_pParaXBody.reset();
	m_pParaXRawData.reset();
	m_pParaXRef.reset();
	m_pD3DMesh.reset();
	m_pD3DRootFrame.reset();
	m_pRoot.reset();
}


#define DEFINE_ReadAnimationBlock(DECL_TYPE, TYPE) \
bool XFileCharModelParser::ReadAnimationBlock(const AnimationBlock* b, DECL_TYPE& anims,int *gs)\
{\
	anims.globals = gs;\
	anims.type = b->type;\
	anims.seq = b->seq;\
	if (anims.seq>=0) {\
		PE_ASSERT(gs);\
			}\
	anims.used = (anims.type != INTERPOLATION_NONE) || (anims.seq != -1);\
\
	/* ranges*/\
	if (b->nRanges > 0) {\
		uint32 *pranges = (uint32*)(GetRawData(b->ofsRanges));\
		anims.ranges.resize(b->nRanges);\
		memcpy(&anims.ranges[0], pranges, b->nRanges*8);\
			} else if (anims.type!=0 && anims.seq==-1) {\
		AnimRange r;\
		r.first = 0;\
		r.second = b->nKeys - 1;\
		anims.ranges.push_back(r);\
			}\
\
	/* times*/\
	PE_ASSERT(b->nTimes == b->nKeys);\
	uint32 *ptimes = (uint32*)GetRawData(b->ofsTimes);\
	if(b->nTimes>0){\
		anims.times.resize(b->nTimes);\
		memcpy(&anims.times[0], ptimes, b->nTimes*4);\
			}\
\
	/* key frames*/\
	TYPE *keys = (TYPE*)GetRawData(b->ofsKeys);\
	if(b->nKeys>0)\
			{\
		switch (anims.type) {\
		case INTERPOLATION_NONE:\
		case INTERPOLATION_LINEAR:\
		case INTERPOLATION_LINEAR_CROSSFRAME:\
			anims.data.resize(b->nKeys);\
			memcpy(&anims.data[0], keys, b->nKeys*sizeof(TYPE));\
			break;\
		case INTERPOLATION_HERMITE:\
			/* NOT tested*/\
			int nSize = b->nKeys*sizeof(TYPE);\
			anims.data.resize(b->nKeys);\
			memcpy(&anims.data[0], keys, nSize);\
			keys+=b->nKeys;\
			anims.in.resize(b->nKeys);\
			memcpy(&anims.in[0], keys, nSize);\
			keys+=b->nKeys;\
			anims.out.resize(b->nKeys);\
			memcpy(&anims.out[0], keys, nSize);\
			break;\
		}\
			}\
	if (anims.data.size()==0) \
		anims.data.push_back(TYPE());\
	return true;\
}

DEFINE_ReadAnimationBlock(Animated<Vector3>, Vector3);
DEFINE_ReadAnimationBlock(AnimatedShort, float);
DEFINE_ReadAnimationBlock(Animated<float>, float);
DEFINE_ReadAnimationBlock(Animated<Quaternion>, Quaternion);


bool XFileCharModelParser::ReadParaXHeader(ParaXHeaderDef& xheader, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		memcpy(&xheader, pBuffer, sizeof(ParaXHeaderDef));
		pFileData->Unlock();
	}
	else
		return false;
	return true;
}
bool XFileCharModelParser::ReadXGlobalSequences(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		DWORD _nGlobalSequences;
		memcpy(&_nGlobalSequences, pBuffer, sizeof(DWORD));
		xmesh.m_objNum.nGlobalSequences = _nGlobalSequences;
		xmesh.globalSequences = new int[xmesh.m_objNum.nGlobalSequences];
		if (xmesh.globalSequences)
			memcpy(xmesh.globalSequences, pBuffer + 4, xmesh.m_objNum.nGlobalSequences * sizeof(DWORD));
		else
			return false;
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXVertices(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		XVerticesDef* pXVert = (XVerticesDef*)pBuffer;
		xmesh.initVertices(pXVert->nVertices, (ModelVertex*)(GetRawData(pXVert->ofsVertices)));
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXTextures(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data

	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nTextures = *(DWORD*)(pBuffer);
		DWORD _nTextures;
		memcpy(&_nTextures, pBuffer, sizeof(DWORD));
		int nTextures = _nTextures;

		xmesh.m_objNum.nTextures = nTextures;


#pragma pack(push) 
#pragma pack(1)
		struct ModelTextureDef_
		{
			uint32 type;
			uint32 nOffsetEmbeddedTexture;
			char sName[1];
		};
#pragma pack(pop)

		if (nTextures > 0)
		{ // at least one texture

			typedef TextureEntity* LPTextureEntity;
			xmesh.textures = new asset_ptr<TextureEntity>[nTextures];
			ModelTextureDef_* pTex = (ModelTextureDef_*)(pBuffer + 4);

			for (int i = 0; i < nTextures; ++i)
			{
				ModelTextureDef_ texInfo;
				memcpy(&texInfo, pTex, sizeof(ModelTextureDef_));

				if (texInfo.type != 0)
				{
					if (texInfo.type < CParaXModel::MAX_MODEL_TEXTURES)
					{
						xmesh.specialTextures[i] = texInfo.type;
						xmesh.useReplaceTextures[texInfo.type] = true;
					}
					else
					{
						xmesh.specialTextures[i] = -1;
						xmesh.useReplaceTextures[i] = false;
					}
				}
				else
				{
					xmesh.specialTextures[i] = -1;
					xmesh.useReplaceTextures[i] = false;
				}
				//string sFilename(((const char*)pTex) + 8); // for safety.
				string sFilename = pTex->sName;

				if (!sFilename.empty())
				{
					// 2006.9.11 by LXZ: we will save the default replaceable texture in m_textures, if it exists. 
					// So that we do not need to supply the name elsewhere in order to display a model with replaceable textures.
					if (texInfo.nOffsetEmbeddedTexture > 0)
					{
						// TODO: for embedded textures, shall we use a different key name adding the file name.
						std::string sFilename_ = GetFilename() + "/" + CParaFile::GetFileName(sFilename);
						xmesh.textures[i] = CGlobals::GetAssetManager()->LoadTexture("", sFilename_.c_str(), TextureEntity::StaticTexture);
						DWORD nSize = 0;
						memcpy(&nSize, GetRawData(texInfo.nOffsetEmbeddedTexture - sizeof(DWORD)), sizeof(DWORD));
						if (nSize > 0)
						{
							char* bufferCpy = new char[nSize];
							memcpy(bufferCpy, GetRawData(texInfo.nOffsetEmbeddedTexture), nSize);
							xmesh.textures[i]->SetRawData(bufferCpy, nSize);
						}
					}
					else
					{
						xmesh.textures[i] = CGlobals::GetAssetManager()->LoadTexture("", sFilename.c_str(), TextureEntity::StaticTexture);
					}

					pTex = (ModelTextureDef_*)(((unsigned char*)pTex) + 8 + sFilename.size() + 1);
				}
				else
				{
					pTex = (ModelTextureDef_*)(((unsigned char*)pTex) + 8 + 1);
					xmesh.textures[i].reset();
				}
			}
		}
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXAttachments(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nAttachments = *(DWORD*)(pBuffer);
		//int nAttachmentLookup = *(((DWORD*)(pBuffer)) + 1);
		DWORD tmp;
		memcpy(&tmp, pBuffer, sizeof(DWORD));
		int nAttachments = tmp;

		memcpy(&tmp, pBuffer + sizeof(DWORD), sizeof(DWORD));

		int nAttachmentLookup = tmp;

		xmesh.m_objNum.nAttachments = nAttachments;
		xmesh.m_objNum.nAttachLookup = nAttachmentLookup;

		ModelAttachmentDef* attachments = (ModelAttachmentDef*)(pBuffer + 8);
		int32* attLookup = (int32*)(pBuffer + 8 + sizeof(ModelAttachmentDef) * nAttachments);

		// attachments
		xmesh.m_atts.reserve(nAttachments);
		for (int i = 0; i < nAttachments; ++i) {
			ModelAttachment att;
			ModelAttachmentDef mad;
			memcpy(&mad, attachments + i, sizeof(ModelAttachmentDef));
			att.pos = mad.pos;
			att.bone = mad.bone;
			att.id = mad.id;
			xmesh.m_atts.push_back(att);
		}
		// attachment lookups
		if (nAttachmentLookup > 0) {
			PE_ASSERT(nAttachmentLookup <= 40);
			memcpy(xmesh.m_attLookup, attLookup, 4 * nAttachmentLookup);
		}
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXColors(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nColors = *(DWORD*)(pBuffer);
		DWORD _nColors;
		memcpy(&_nColors, pBuffer, sizeof(DWORD));
		int nColors = _nColors;

		xmesh.m_objNum.nColors = nColors;
		if (nColors > 0)
		{ // at least one Bone
			ModelColorDef* colorDefs = (ModelColorDef*)(pBuffer + 4);
			xmesh.colors = new ModelColor[nColors];
			for (int i = 0; i < nColors; ++i)
			{
				ModelColor& att = xmesh.colors[i];
				ReadAnimationBlock(&colorDefs[i].color, att.color, xmesh.globalSequences);
				ReadAnimationBlock(&colorDefs[i].opacity, att.opacity, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXTransparency(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nTransparency = *(DWORD*)(pBuffer);
		DWORD _nTransparency;
		memcpy(&_nTransparency, pBuffer, sizeof(DWORD));
		int nTransparency = _nTransparency;

		xmesh.m_objNum.nTransparency = nTransparency;
		if (nTransparency > 0)
		{ // at least one item
			ModelTransDef* transDefs = (ModelTransDef*)(pBuffer + 4);
			xmesh.transparency = new ModelTransparency[nTransparency];
			for (int i = 0; i < nTransparency; ++i)
			{
				ModelTransDef def;
				memcpy(&def, transDefs + i, sizeof(ModelTransDef));
				ReadAnimationBlock(&(def.trans), xmesh.transparency[i].trans, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXViews(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	// no need to do anything, since there is only one view. all view 0 information are duplicated in other nodes.
	return true;
}

bool XFileCharModelParser::ReadXIndices0(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		Indice0Def* pIndices = (Indice0Def*)pBuffer;
		xmesh.initIndices(pIndices->nIndices, (uint16*)(GetRawData(pIndices->ofsIndices)));
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXGeosets(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nGeosets = *(DWORD*)(pBuffer);
		DWORD _nGeosets;
		memcpy(&_nGeosets, pBuffer, sizeof(DWORD));
		int nGeosets = _nGeosets;

		ModelGeoset* pGeosets = (ModelGeoset*)(pBuffer + 4);
		xmesh.showGeosets = new bool[nGeosets];
		for (int i = 0; i < nGeosets; ++i)
			xmesh.showGeosets[i] = true;

		xmesh.geosets.resize(nGeosets);
		if (nGeosets > 0)
		{
			memcpy(&xmesh.geosets[0], pGeosets, sizeof(ModelGeoset) * nGeosets);
			if (xmesh.CheckMinVersion(1, 0, 0, 1))
			{
				/* since Intel is little endian.
				for (int i = 0; i < nGeosets; ++i)
				{
				ModelGeoset& geoset = xmesh.geosets[i];
				geoset.SetVertexStart((DWORD)geoset.d3 + ((DWORD)(geoset.d4) << 16));
				}*/
			}
			else
			{
				// disable vertex start for all parax file version before 1.0.0.1, since we only support uint16 indices. 
				for (int i = 0; i < nGeosets; ++i)
				{
					xmesh.geosets[i].SetVertexStart(0);
				}
			}
		}
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXRenderPass(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nRenderPasses = *(DWORD*)(pBuffer);
		DWORD _nRenderPasses;
		memcpy(&_nRenderPasses, pBuffer, sizeof(DWORD));

		int nRenderPasses = _nRenderPasses;

		ModelRenderPass* passes = (ModelRenderPass*)(pBuffer + 4);
		xmesh.passes.resize(nRenderPasses);
		if (nRenderPasses > 0)
		{
			memcpy(&xmesh.passes[0], passes, sizeof(ModelRenderPass) * nRenderPasses);

			// for opaque faces, always enable culling.
			for (int i = 0; i < nRenderPasses; ++i)
			{
				ModelRenderPass& pass = xmesh.passes[i];
				if (pass.blendmode == BM_OPAQUE)
				{
					pass.cull = true;
				}
				/** fix 2009.1.14 by LiXizhi, use 32 bits index offset */
				if (pass.indexStart != 0xffff)
				{
					pass.m_nIndexStart = pass.indexStart;
				}
			}

			// transparent parts come later
			std::stable_sort(xmesh.passes.begin(), xmesh.passes.end());
		}
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXBones(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nBones = *(DWORD*)(pBuffer);
		DWORD _nBones;
		memcpy(&_nBones, pBuffer, sizeof(DWORD));
		int nBones = _nBones;

		xmesh.m_objNum.nBones = nBones;
		if (nBones > 0)
		{ // at least one Bone
			xmesh.bones = new Bone[nBones];

			ModelBoneDef* mb = (ModelBoneDef*)(pBuffer + 4);



			for (int i = 0; i < nBones; ++i)
			{
				Bone& bone = xmesh.bones[i];
				const ModelBoneDef& b = mb[i];
				bone.parent = b.parent;
				bone.flags = b.flags;
				if ((bone.flags & 0x80000000) != 0)
				{
					bone.flags = bone.flags & (~0x80000000);
					if (b.nBoneName != 0)
						bone.SetName((const char*)GetRawData(b.nBoneName));

					if (bone.IsOffsetMatrixBone()) {
						//bone.matOffset = *((const Matrix4*)GetRawData(b.nOffsetMatrix));
						memcpy(&bone.matOffset, GetRawData(b.nOffsetMatrix), sizeof(Matrix4));
						bone.bUsePivot = false;
					}
					//bone.pivot = *((const Vector3*)GetRawData(b.nOffsetPivot));
					memcpy(&bone.pivot, GetRawData(b.nOffsetPivot), sizeof(Vector3));
					if (bone.IsStaticTransform())
					{
						//bone.matTransform = *((const Matrix4*)GetRawData(b.ofsStaticMatrix));
						memcpy(&bone.matTransform, GetRawData(b.ofsStaticMatrix), sizeof(Matrix4));
					}
				}
				else
				{
					bone.pivot = b.pivot;
				}
				bone.nIndex = i;

				if (b.boneid > 0 && b.boneid < MAX_KNOWN_BONE_NODE)
				{
					xmesh.m_boneLookup[b.boneid] = i;
					//bone.nBoneID = b.boneid;
				}
				bone.nBoneID = b.boneid;

				if (!bone.IsStaticTransform())
				{
					ReadAnimationBlock(&b.translation, bone.trans, xmesh.globalSequences);
					ReadAnimationBlock(&b.rotation, bone.rot, xmesh.globalSequences);
					ReadAnimationBlock(&b.scaling, bone.scale, xmesh.globalSequences);
					bone.RemoveRedundentKeys();
				}
			}
		}
	}
	else
		return false;
	return true;
}

bool XFileCharModelParser::ReadXTexAnims(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nTexAnims = *(DWORD*)(pBuffer);
		DWORD _nTexAnims;
		memcpy(&_nTexAnims, pBuffer, sizeof(DWORD));
		int nTexAnims = _nTexAnims;

		xmesh.m_objNum.nTexAnims = nTexAnims;
		if (nTexAnims > 0)
		{ // at least one Bone
			ModelTexAnimDef* texanims = (ModelTexAnimDef*)(pBuffer + 4);
			xmesh.texanims = new TextureAnim[nTexAnims];
			for (int i = 0; i < nTexAnims; ++i)
			{
				TextureAnim& TexAnim = xmesh.texanims[i];
				const ModelTexAnimDef& texanim = texanims[i];
				ReadAnimationBlock(&texanim.trans, TexAnim.trans, xmesh.globalSequences);
				ReadAnimationBlock(&texanim.rot, TexAnim.rot, xmesh.globalSequences);
				ReadAnimationBlock(&texanim.scale, TexAnim.scale, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}

//TODO: untested
bool XFileCharModelParser::ReadXParticleEmitters(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nParticleEmitters = *(DWORD*)(pBuffer);
		DWORD _nParticleEmitters;
		memcpy(&_nParticleEmitters, pBuffer, sizeof(DWORD));
		int nParticleEmitters = _nParticleEmitters;


		xmesh.m_objNum.nParticleEmitters = nParticleEmitters;
		if (nParticleEmitters > 0)
		{ // at least one item
			ModelParticleEmitterDef* particleSystems = (ModelParticleEmitterDef*)(pBuffer + 4);
			xmesh.particleSystems = new ParticleSystem[nParticleEmitters];
			for (int i = 0; i < nParticleEmitters; ++i)
			{
				ParticleSystem& ps = xmesh.particleSystems[i];
				const ModelParticleEmitterDef& PSDef = particleSystems[i];
				ps.model = &xmesh;

				if (xmesh.rotatePartice2SpeedVector)
					ps.rotate2SpeedDirection = true;

				for (size_t i = 0; i < 3; i++) {
					LinearColor c(Color(PSDef.p.colors[i]));
					ps.colors[i] = reinterpret_cast<const Vector4&>(c);
					ps.sizes[i] = PSDef.p.sizes[i];// * PSDef.p.scales[i];
				}
				ps.mid = PSDef.p.mid;
				ps.slowdown = PSDef.p.slowdown;
				ps.rotation = PSDef.p.rotation;
				ps.pos = PSDef.pos;
				ps.blend = PSDef.blend;
				ps.rows = PSDef.rows;
				ps.cols = PSDef.cols;
				ps.type = PSDef.s1;
				ps.order = 0; // triangle winding order is fixed for all types.
				ps.SetTextureRowsCols(ps.rows, ps.cols);

				ps.parent = xmesh.bones + PSDef.bone;
				ps.m_texture_index = PSDef.texture;

				ReadAnimationBlock(&PSDef.params[0], ps.speed, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[1], ps.variation, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[2], ps.spread, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[3], ps.lat, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[4], ps.gravity, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[5], ps.lifespan, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[6], ps.rate, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[7], ps.areal, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[8], ps.areaw, xmesh.globalSequences);
				ReadAnimationBlock(&PSDef.params[9], ps.grav2, xmesh.globalSequences);

				switch (PSDef.type) {
				case ParticleEmitter::TYPE_PLANE_PARTICLE_EMITTER:
					ps.emitter = new PlaneParticleEmitter(&ps);
					break;
				case ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER:
					ps.emitter = new SphereParticleEmitter(&ps);
					break;
				default:
					OUTPUT_LOG("warning: particle system contains unknown emitter type\r\n");
					break;
				}
				ps.billboard = !(PSDef.flags & 4096);
				ps.tofs = frand();
			}
		}
	}
	else
		return false;
	return true;
}


//TODO: untested
bool XFileCharModelParser::ReadXRibbonEmitters(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nRibbonEmitters = *(DWORD*)(pBuffer);
		DWORD _nRibbonEmitters;
		memcpy(&_nRibbonEmitters, pBuffer, sizeof(DWORD));
		int nRibbonEmitters = _nRibbonEmitters;

		xmesh.m_objNum.nRibbonEmitters = nRibbonEmitters;
		if (nRibbonEmitters > 0)
		{ // at least one item
			ModelRibbonEmitterDef* ribbons = (ModelRibbonEmitterDef*)(pBuffer + 4);
			xmesh.ribbons = new RibbonEmitter[nRibbonEmitters];

			for (int i = 0; i < nRibbonEmitters; ++i)
			{
				RibbonEmitter& emitter = xmesh.ribbons[i];
				const ModelRibbonEmitterDef& emitterDef = ribbons[i];
				emitter.model = &xmesh;

				emitter.pos = emitterDef.pos;
				emitter.numsegs = (int)emitterDef.res;
				emitter.seglen = emitterDef.length;

				emitter.parent = xmesh.bones + emitterDef.bone;
				emitter.m_texture_index = emitterDef.ofsTextures;

				ReadAnimationBlock(&emitterDef.color, emitter.color, xmesh.globalSequences);
				ReadAnimationBlock(&emitterDef.opacity, emitter.opacity, xmesh.globalSequences);
				ReadAnimationBlock(&emitterDef.above, emitter.above, xmesh.globalSequences);
				ReadAnimationBlock(&emitterDef.below, emitter.below, xmesh.globalSequences);

				//TODO: create first segment
				//RibbonSegment rs;
				//rs.pos = tpos;
				//rs.len = 0;
			}
		}
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXCameras(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nCameras = *(DWORD*)(pBuffer);
		DWORD _nCameras;
		memcpy(&_nCameras, pBuffer, sizeof(DWORD));
		int nCameras = _nCameras;

		xmesh.m_objNum.nCameras = nCameras;
		if (nCameras > 0)
		{ // at least one item
			ModelCameraDef* cameras = (ModelCameraDef*)(pBuffer + 4);
			int i = 0;
			{
				ModelCamera& camera = xmesh.cam; // just one camera
				xmesh.hasCamera = true;

				camera.m_bIsValid = true;
				camera.nearclip = cameras[i].nearclip;
				camera.farclip = cameras[i].farclip;
				camera.fov = cameras[i].fov;
				camera.pos = cameras[i].pos;
				camera.target = cameras[i].target;
				camera.nearclip = cameras[i].nearclip;
				ReadAnimationBlock(&cameras[i].transPos, camera.tPos, xmesh.globalSequences);
				ReadAnimationBlock(&cameras[i].transTarget, camera.tTarget, xmesh.globalSequences);
				ReadAnimationBlock(&cameras[i].rot, camera.rot, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}


// not tested.
bool XFileCharModelParser::ReadXLights(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//int nLights = *(DWORD*)(pBuffer);
		DWORD _nLights;
		memcpy(&_nLights, pBuffer, sizeof(DWORD));
		int nLights = _nLights;

		xmesh.m_objNum.nLights = nLights;
		if (nLights > 0)
		{ // at least one item
			ModelLightDef* lights = (ModelLightDef*)(pBuffer + 4);
			xmesh.lights = new ModelLight[nLights];
			for (int i = 0; i < nLights; ++i)
			{
				ModelLight& light = xmesh.lights[i];
				const ModelLightDef& lightDef = lights[i];

				light.pos = lightDef.pos;
				light.type = lightDef.type;
				light.parent = lightDef.bone;
				light.tdir = light.dir = Vector3(0, 1, 0);

				ReadAnimationBlock(&lightDef.ambColor, light.ambColor, xmesh.globalSequences);
				ReadAnimationBlock(&lightDef.ambIntensity, light.ambIntensity, xmesh.globalSequences);
				ReadAnimationBlock(&lightDef.color, light.diffColor, xmesh.globalSequences);
				ReadAnimationBlock(&lightDef.intensity, light.diffIntensity, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}


bool XFileCharModelParser::ReadXAnimations(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		//uint32 nAnimations = *(DWORD*)(pBuffer);
		DWORD _nAnimations;
		memcpy(&_nAnimations, pBuffer, sizeof(DWORD));
		uint32 nAnimations = _nAnimations;

		xmesh.m_objNum.nAnimations = nAnimations;

		ModelAnimation* anims = (ModelAnimation*)(pBuffer + 4);
		xmesh.anims = new ModelAnimation[nAnimations];
		if (xmesh.anims) {
			memcpy(xmesh.anims, anims, sizeof(ModelAnimation) * nAnimations);
		}
	}
	else
		return false;
	return true;
}

bool ParaEngine::XFileCharModelParser::ReadXVoxels(CParaXModel& xmesh, XFileDataObjectPtr pFileData)
{
	DWORD       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if ((pFileData->Lock(&dwSize, (&pBuffer))))
	{
		XVerticesDef* pXVert = (XVerticesDef*)pBuffer;
		auto nByteCount = pXVert->nVertices;
		auto pData = GetRawData(pXVert->ofsVertices);
		auto pVoxelMesh = xmesh.CreateGetVoxelModel();
		if (pVoxelMesh) 
		{
			pVoxelMesh->Load(pData, nByteCount);
		}
	}
	else
		return false;
	return true;
}
