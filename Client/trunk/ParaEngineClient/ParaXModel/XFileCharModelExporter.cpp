#include "ParaEngine.h"
#include "XFileCharModelExporter.h"
#include "./ParaXSerializer.h"
#include "Core/TextureEntity.h"
#include "ParaXBone.h"
#include "particle.h"

#include <fstream>

using namespace ParaEngine;

XFileCharModelExporter::XFileCharModelExporter(ofstream& strm, CParaXModel* pMesh)
	: XFileExporter(strm)
	, m_pMesh(pMesh)
	, m_pRawData(new CParaRawData())
{
	WriteTemplates(strm);
}


XFileCharModelExporter::~XFileCharModelExporter()
{
	delete m_pRawData;
	m_pRawData = nullptr;

	m_pMesh = nullptr;
}

void XFileCharModelExporter::ExportParaXModel(ofstream& strm)
{
	auto node = Translate();
	node->Write(strm, *this);

	Release(node);
}

bool ParaEngine::XFileCharModelExporter::Export(const string& filepath, CParaXModel* pMesh)
{
	ofstream file(filepath, ios_base::binary);
	if (file.is_open())
	{
		XFileCharModelExporter exporter(file, pMesh);
		exporter.ExportParaXModel(file);
		return true;
	}
	return false;
}

ParaEngine::XFileDataObjectPtr ParaEngine::XFileCharModelExporter::Translate()
{
	XFileDataObjectPtr node(new XFileDataObject());
	XFileDataObjectPtr pParaXHeader(new XFileDataObject());
	if (WriteParaXHeader(pParaXHeader))
	{
		node->AddChild(pParaXHeader);
	}

	XFileDataObjectPtr pParaXBody(new XFileDataObject());
	if (WriteParaXBody(pParaXBody))
	{
		node->AddChild(pParaXBody);
	}

	XFileDataObjectPtr pXDWORDArray(new XFileDataObject());
	if (WriteParaXRawData(pXDWORDArray))
	{
		node->AddChild(pXDWORDArray);
	}
	return node;
}



#pragma region +[从CParaXModel读取数据到XFileDataObject对象]

void ParaEngine::XFileCharModelExporter::Release(XFileDataObjectPtr pData)
{
	if (pData->GetChildCount() > 0)
	{
		for (int i = 0; i < pData->GetChildCount(); ++i)
		{
			auto pChildData = pData->GetChild(i);
			Release(pChildData);
		}
	}
	pData.reset();
}


DWORD ParaEngine::XFileCharModelExporter::CountIsAnimatedValue()
{
	DWORD value = (m_pMesh->rotatePartice2SpeedVector ? 1 << 5 : 0) |
		(m_pMesh->animGeometry ? 1 << 0 : 0) |
		(m_pMesh->animTextures ? 1 << 1 : 0) |
		(m_pMesh->animBones ? 1 << 2 : 0) |
		(m_pMesh->animTexRGB ? 1 << 4 : 0);
	//value = value >0 ?value:
	if (0 == value && m_pMesh->animated)
	{
		value |= (1 << 3);
	}
	else
	{
		value |= (m_pMesh->m_header.IsAnimated & (1 << 3));
	}
	return value;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXHeader(XFileDataObjectPtr pData, const string& strName)
{
	pData->m_sTemplateName = "ParaXHeader";
	pData->m_sName = strName;

	pData->ResizeBuffer(sizeof(ParaXHeaderDef));
	//
	memcpy(pData->GetBuffer(), &(m_pMesh->GetHeader()), sizeof(ParaXHeaderDef));
	ParaXHeaderDef* stHead = (ParaXHeaderDef*)pData->GetBuffer();
	stHead->IsAnimated = CountIsAnimatedValue();
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXBody(XFileDataObjectPtr pData, const string& strName)
{
	pData->m_sTemplateName = "ParaXBody";
	pData->m_sName = strName;

	static vector<string> vecChildNames{ "XViews","XTextures","XAttachments","XVertices" ,"XIndices0" ,"XGeosets" ,"XRenderPass" ,"XBones" ,"XAnimations" ,
		"XTransparency" ,"XTexAnims","XParticleEmitters","XRibbonEmitters","XColors","XCameras","XLights" };
	for (string strChildName : vecChildNames)
	{
		XFileDataObjectPtr pChildData(new XFileDataObject());
		WriteParaXBodyChild(pChildData, strChildName);
		pData->AddChild(pChildData);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXRawData(XFileDataObjectPtr pData, const string& strName)
{
	pData->m_sTemplateName = "XDWORDArray";
	pData->m_sName = "ParaXRawData";

	DWORD nSize = m_pRawData->GetSize();
	if (nSize > 0)
	{
		DWORD nCount = m_pRawData->GetSize() / sizeof(int);
		pData->ResizeBuffer(4 + m_pRawData->GetSize() + 4);
		*(DWORD*)pData->GetBuffer() = nCount;
		memcpy(pData->GetBuffer() + 4, m_pRawData->GetBuffer(), m_pRawData->GetSize());
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXBodyChild(XFileDataObjectPtr pData, const string& strTemplateName, const string& strName /*= ""*/)
{
	if (strTemplateName == "XDWORDArray") {
		// 文件解析时候没有"XGlobalSequences"节点的相关处理逻辑，故此处也不做处理
		////XGlobalSequences
		//// Get the frame name (if any)
		//if (pSubData->GetName() == "XGlobalSequences") {
		//	ReadXGlobalSequences(*pMesh, pSubData);
		//}
	}
	else if (strTemplateName == "XVertices") {//XVertices
		WriteXVertices(pData, strName);
		/*if (!ReadXVertices(*pMesh, pSubData))
		OUTPUT_LOG("error loading vertices");*/
	}
	else if (strTemplateName == "XTextures") {//XTextures
		WriteXTextures(pData, strName);
		/*if (!ReadXTextures(*pMesh, pSubData))
		OUTPUT_LOG("error loading XTextures");*/
	}
	else if (strTemplateName == "XAttachments") {//XAttachments
		WriteXAttachments(pData, strName);
		/*if (!ReadXAttachments(*pMesh, pSubData))
		OUTPUT_LOG("error loading XAttachments");*/
	}
	else if (strTemplateName == "XTransparency") {//XTransparency
		WriteXTransparency(pData, strName);
		/*if (!ReadXTransparency(*pMesh, pSubData))
		OUTPUT_LOG("error loading XTransparency");*/
	}
	else if (strTemplateName == "XViews") {//XViews
		WriteXViews(pData, strName);
		/*if (!ReadXViews(*pMesh, pSubData))
		OUTPUT_LOG("error loading XViews");*/
	}
	else if (strTemplateName == "XIndices0") {//XIndices0
		WriteXIndices0(pData, strName);
		/*if (!ReadXIndices0(*pMesh, pSubData))
		OUTPUT_LOG("error loading XIndices0");*/
	}
	else if (strTemplateName == "XGeosets") {//XGeosets
		WriteXGeosets(pData, strName);
		/*if (!ReadXGeosets(*pMesh, pSubData))
		OUTPUT_LOG("error loading XGeosets");*/
	}
	else if (strTemplateName == "XRenderPass") {//XRenderPass
		WriteXRenderPass(pData, strName);
		/*if (!ReadXRenderPass(*pMesh, pSubData))
		OUTPUT_LOG("error loading XRenderPass");*/
	}
	else if (strTemplateName == "XBones") {//XBones
		WriteXBones(pData, strName);
		/*if (!ReadXBones(*pMesh, pSubData))
		OUTPUT_LOG("error loading XBones");*/
	}
	else if (strTemplateName == "XTexAnims") {//XTexAnims
		WriteXTexAnims(pData, strName);
		/*if (!ReadXTexAnims(*pMesh, pSubData))
		OUTPUT_LOG("error loading XTexAnims");*/
	}
	else if (strTemplateName == "XParticleEmitters") {//XParticleEmitters
		WriteXParticleEmitters(pData, strName);
		/*if (!ReadXParticleEmitters(*pMesh, pSubData))
		OUTPUT_LOG("error loading XParticleEmitters");*/
	}
	else if (strTemplateName == "XRibbonEmitters") {//XRibbonEmitters
		WriteXRibbonEmitters(pData, strName);
		/*if (!ReadXRibbonEmitters(*pMesh, pSubData))
		OUTPUT_LOG("error loading XRibbonEmitters");*/
	}
	else if (strTemplateName == "XColors") {//XColors
		WriteXColors(pData, strName);
		/*if (!ReadXColors(*pMesh, pSubData))
		OUTPUT_LOG("error loading XColors");*/
	}
	else if (strTemplateName == "XCameras") {//XCameras
		WriteXCameras(pData, strName);
		/*if (!ReadXCameras(*pMesh, pSubData))
		OUTPUT_LOG("error loading XCameras");*/
	}
	else if (strTemplateName == "XLights")
	{
		//XLights
		WriteXLights(pData, strName);
		/*if (!ReadXLights(*pMesh, pSubData))
		OUTPUT_LOG("error loading XLights");*/
	}
	else if (strTemplateName == "XAnimations")
	{
		//XAnimations
		WriteXAnimations(pData, strName);
		/*if (!ReadXAnimations(*pMesh, pSubData))
		OUTPUT_LOG("error loading XAnimations");*/
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXGlobalSequences(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nGlobalSequences = m_pMesh->GetObjectNum().nGlobalSequences;
	if (nGlobalSequences > 0)
	{
		pData->m_sTemplateName = "XDWORDArray";
		pData->m_sName = "XGlobalSequences";

		int nSize = 4 + nGlobalSequences * sizeof(DWORD);
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nGlobalSequences;
		byte *bytes = (byte*)(pData->GetBuffer() + 4);
		memcpy(pData->GetBuffer() + 4, m_pMesh->globalSequences, nSize - 4);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXVertices(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	pData->m_sTemplateName = "XVertices";
	pData->m_sName = strName;

	int nSize =sizeof(XVerticesDef);
	pData->ResizeBuffer(nSize);

	int nVertices = m_pMesh->m_objNum.nVertices;

	XVerticesDef data;
	data.nType = 0;
	data.nVertexBytes = sizeof(ModelVertex);
	data.nVertices = nVertices;

	// m_pRaw信息填充
	data.ofsVertices = m_pRawData->AddRawData((const DWORD*)m_pMesh->m_origVertices, data.nVertices*data.nVertexBytes / 4);

	memcpy(pData->GetBuffer(), &data, nSize);
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXTextures(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	struct ModelTextureDef_ {
		uint32 type;
		uint32 flags;
		char sName;
	};
	int nTextures = m_pMesh->m_objNum.nTextures;
	if (nTextures > 0)
	{
		pData->m_sTemplateName = "XTextures";
		pData->m_sName = strName;

		// 255是每个texture路径预留长度,导入完成后按实际长度动态修改
		pData->ResizeBuffer((sizeof(ModelTextureDef_) + 255)*nTextures + sizeof(int));

		*(int*)pData->GetBuffer() = nTextures;

		ModelTextureDef_* pBuffer = (ModelTextureDef_*)(pData->GetBuffer() + 4);

		for (int i = 0; i < nTextures; ++i)
		{
			pBuffer->flags = 0; // unused
			if (m_pMesh->textures[i].get() == nullptr) {
				pBuffer->type = m_pMesh->specialTextures[i];
				pBuffer->sName = '\0';
				pBuffer = (ModelTextureDef_*)((char*)pBuffer + 8 + 1);
			}
			else {
				pBuffer->type = m_pMesh->specialTextures[i];
				string name = m_pMesh->textures[i]->GetKey();
				memcpy(&(pBuffer->sName), name.c_str(), name.size() + 1);
				pBuffer = (ModelTextureDef_*)((char*)pBuffer + 8 + name.size() + 1);
			}
		}
		pData->ResizeBuffer((int)((char*)pBuffer - pData->GetBuffer()));

	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXAttachments(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nAttachments = m_pMesh->GetObjectNum().nAttachments;
	int nAttachmentLookup = m_pMesh->GetObjectNum().nAttachLookup;
	if (nAttachments > 0 || nAttachmentLookup > 0)
	{
		pData->m_sTemplateName = "XAttachments";
		pData->m_sName = strName;

		int nSize = 8 + sizeof(ModelAttachmentDef)*nAttachments + 4 * nAttachmentLookup;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nAttachments;
		*(((DWORD*)(pData->GetBuffer())) + 1) = (DWORD)nAttachmentLookup;
		ModelAttachmentDef *attachments = (ModelAttachmentDef *)(pData->GetBuffer() + 8);
		int32 * attLookup = (int32 *)(pData->GetBuffer() + 8 + sizeof(ModelAttachmentDef)*nAttachments);
		for (int i = 0; i < nAttachments; i++) {
			const ModelAttachment& att = m_pMesh->m_atts[i];
			attachments[i].id = att.id;
			attachments[i].bone = att.bone;
			attachments[i].pos = att.pos;
			// TODO: this member is not used.
			memset(&attachments[i].unk, 0, sizeof(AnimationBlock));
		}
		if (nAttachmentLookup > 0)
			memcpy(attLookup, m_pMesh->m_attLookup, 4 * nAttachmentLookup);
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXColors(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nColors = m_pMesh->GetObjectNum().nColors;
	if (nColors > 0)
	{
		pData->m_sTemplateName = "XColors";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelColorDef)*nColors;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nColors;
		ModelColorDef *colorDefs = (ModelColorDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nColors; i++) {
			const ModelColor& att = m_pMesh->colors[i];
			WriteAnimationBlock(&colorDefs[i].color, att.color);
			WriteAnimationBlock(&colorDefs[i].opacity, att.opacity);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXTransparency(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nTransparency = m_pMesh->GetObjectNum().nTransparency;
	if (nTransparency > 0)
	{
		pData->m_sTemplateName = "XTransparency";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelTransDef)*nTransparency;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nTransparency;
		ModelTransDef *transDefs = (ModelTransDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nTransparency; i++) {
			const ModelTransparency& transp = m_pMesh->transparency[i];
			WriteAnimationBlock(&transDefs[i].trans, transp.trans);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXViews(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	// no need to do anything, since there is only one view. all view 0 information are duplicated in other nodes.
	//pData->m_sTemplateName = "XViews";
	//pData->m_sName = strName;
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXIndices0(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	if (m_pMesh->m_indices)
	{
		int nIndices = (int)m_pMesh->GetObjectNum().nIndices;
		{
			pData->m_sTemplateName = "XIndices0";
			pData->m_sName = strName;

			Indice0Def indices0;
			indices0.nIndices = nIndices;
			indices0.ofsIndices = m_pRawData->AddRawData((const short*)m_pMesh->m_indices, nIndices);

			int nSize = sizeof(Indice0Def);
			pData->ResizeBuffer(nSize);

			memcpy(pData->GetBuffer(), &indices0, nSize);
		}
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXGeosets(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nGeosets = (int)m_pMesh->geosets.size();
	if (nGeosets > 0)
	{
		pData->m_sTemplateName = "XGeosets";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelGeoset)*nGeosets;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nGeosets;
		ModelGeoset *geosets = (ModelGeoset*)(pData->GetBuffer() + 4);
		memcpy(geosets, &m_pMesh->geosets[0], nSize - 4);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXRenderPass(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nRenderPasses = (int)m_pMesh->passes.size();
	if (nRenderPasses > 0)
	{
		pData->m_sTemplateName = "XRenderPass";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelRenderPass)*nRenderPasses;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRenderPasses;
		ModelRenderPass *opsDefs = (ModelRenderPass*)(pData->GetBuffer() + 4);
		memcpy(opsDefs, &m_pMesh->passes[0], nSize - 4);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXBones(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nBones = m_pMesh->GetObjectNum().nBones;
	if (nBones > 0 && m_pMesh->animBones)
	{
		pData->m_sTemplateName = "XBones";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelBoneDef)*nBones;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nBones;
		ModelBoneDef *bones = (ModelBoneDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nBones; i++) {
			const Bone& bone = m_pMesh->bones[i];
			bones[i].parent = bone.parent;
			bones[i].pivot = bone.pivot;
			bones[i].flags = bone.flags;
			bones[i].boneid = bone.nBoneID;
			WriteAnimationBlock(&bones[i].translation, bone.trans);
			WriteAnimationBlock(&bones[i].rotation, bone.rot);
			WriteAnimationBlock(&bones[i].scaling, bone.scale);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXTexAnims(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nTexAnims = m_pMesh->GetObjectNum().nTexAnims;
	if (nTexAnims > 0 && m_pMesh->animTextures)
	{
		pData->m_sTemplateName = "XTexAnims";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelTexAnimDef)*nTexAnims;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nTexAnims;
		ModelTexAnimDef *texanims = (ModelTexAnimDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nTexAnims; i++) {
			const TextureAnim& TexAnim = m_pMesh->texanims[i];
			WriteAnimationBlock(&texanims[i].trans, TexAnim.trans);
			WriteAnimationBlock(&texanims[i].rot, TexAnim.rot);
			WriteAnimationBlock(&texanims[i].scale, TexAnim.scale);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXParticleEmitters(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nParticleEmitters = m_pMesh->GetObjectNum().nParticleEmitters;
	if (nParticleEmitters > 0)
	{
		pData->m_sTemplateName = "XParticleEmitters";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelParticleEmitterDef)*nParticleEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nParticleEmitters;
		ModelParticleEmitterDef *particleSystems = (ModelParticleEmitterDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nParticleEmitters; i++) {
			const ParticleSystem& particleSystem = m_pMesh->particleSystems[i];

			for (int k = 0; k < 3; k++) {
				particleSystems[i].p.colors[k] = (DWORD)LinearColor((float*)&particleSystem.colors[k]);
				particleSystems[i].p.sizes[k] = particleSystem.sizes[k];
			}
			particleSystems[i].p.mid = particleSystem.mid;
			particleSystems[i].p.slowdown = particleSystem.slowdown;
			particleSystems[i].p.rotation = particleSystem.rotation;
			particleSystems[i].pos = particleSystem.pos;
			particleSystems[i].blend = particleSystem.blend;
			particleSystems[i].rows = particleSystem.rows;
			particleSystems[i].cols = particleSystem.cols;
			particleSystems[i].s1 = particleSystem.type;
			particleSystems[i].type = particleSystem.emitter ? particleSystem.emitter->GetEmitterType() : ParticleEmitter::TYPE_NONE;

			if (!particleSystem.billboard)
				particleSystems[i].flags |= 4096;
			else
				particleSystems[i].flags = 0;

			for (uint32 m = 0; m < m_pMesh->GetObjectNum().nBones; ++m) {
				if ((m_pMesh->bones + m) == particleSystem.parent) {
					particleSystems[i].bone = (int16)m;
					break;
				}
			}
			particleSystems[i].texture = (int16)particleSystem.m_texture_index;

			WriteAnimationBlock(&particleSystems[i].params[0], particleSystem.speed);
			WriteAnimationBlock(&particleSystems[i].params[1], particleSystem.variation);
			WriteAnimationBlock(&particleSystems[i].params[2], particleSystem.spread);
			WriteAnimationBlock(&particleSystems[i].params[3], particleSystem.lat);
			WriteAnimationBlock(&particleSystems[i].params[4], particleSystem.gravity);
			WriteAnimationBlock(&particleSystems[i].params[5], particleSystem.lifespan);
			WriteAnimationBlock(&particleSystems[i].params[6], particleSystem.rate);
			WriteAnimationBlock(&particleSystems[i].params[7], particleSystem.areal);
			WriteAnimationBlock(&particleSystems[i].params[8], particleSystem.areaw);
			WriteAnimationBlock(&particleSystems[i].params[9], particleSystem.grav2);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXRibbonEmitters(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nRibbonEmitters = m_pMesh->GetObjectNum().nRibbonEmitters;
	if (nRibbonEmitters > 0)
	{
		pData->m_sTemplateName = "XRibbonEmitters";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelRibbonEmitterDef)*nRibbonEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRibbonEmitters;
		ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nRibbonEmitters; i++) {
			const RibbonEmitter& ribbon = m_pMesh->ribbons[i];

			ribbons[i].pos = ribbon.pos;
			ribbons[i].res = (float)ribbon.numsegs;
			ribbons[i].length = ribbon.seglen;

			for (uint32 m = 0; m < m_pMesh->GetObjectNum().nBones; ++m) {
				if ((m_pMesh->bones + m) == ribbon.parent) {
					ribbons[i].bone = m;
					break;
				}
			}
			ribbons[i].ofsTextures = ribbon.m_texture_index;

			WriteAnimationBlock(&ribbons[i].color, ribbon.color);
			WriteAnimationBlock(&ribbons[i].opacity, ribbon.opacity);
			WriteAnimationBlock(&ribbons[i].above, ribbon.above);
			WriteAnimationBlock(&ribbons[i].below, ribbon.below);
		}
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXCameras(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nRibbonEmitters = m_pMesh->GetObjectNum().nRibbonEmitters;
	if (nRibbonEmitters > 0)
	{
		pData->m_sTemplateName = "XCameras";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelRibbonEmitterDef)*nRibbonEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRibbonEmitters;
		ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nRibbonEmitters; i++) {
			const RibbonEmitter& ribbon = m_pMesh->ribbons[i];

			ribbons[i].pos = ribbon.pos;
			ribbons[i].res = (float)ribbon.numsegs;
			ribbons[i].length = ribbon.seglen;

			for (uint32 m = 0; m < m_pMesh->GetObjectNum().nBones; ++m) {
				if ((m_pMesh->bones + m) == ribbon.parent) {
					ribbons[i].bone = m;
					break;
				}
			}
			ribbons[i].ofsTextures = ribbon.m_texture_index;

			WriteAnimationBlock(&ribbons[i].color, ribbon.color);
			WriteAnimationBlock(&ribbons[i].opacity, ribbon.opacity);
			WriteAnimationBlock(&ribbons[i].above, ribbon.above);
			WriteAnimationBlock(&ribbons[i].below, ribbon.below);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXLights(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nLights = m_pMesh->GetObjectNum().nLights;
	if (nLights > 0)
	{
		pData->m_sTemplateName = "XLights";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelLightDef)*nLights;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nLights;
		ModelLightDef *lights = (ModelLightDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nLights; i++) {
			const ModelLight& light = m_pMesh->lights[i];
			lights[i].pos = light.pos;
			lights[i].type = light.type;
			lights[i].bone = light.parent;
			WriteAnimationBlock(&lights[i].ambColor, light.ambColor);
			WriteAnimationBlock(&lights[i].ambIntensity, light.ambIntensity);
			WriteAnimationBlock(&lights[i].color, light.diffColor);
			WriteAnimationBlock(&lights[i].intensity, light.diffIntensity);
		}
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXAnimations(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	int nAnimations = (int)m_pMesh->GetObjectNum().nAnimations;
	if (nAnimations > 0 && m_pMesh->anims && m_pMesh->animated)
	{
		pData->m_sTemplateName = "XAnimations";
		pData->m_sName = strName;

		int nSize = 4 + sizeof(ModelAnimation)*nAnimations;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nAnimations;
		ModelAnimation *anims = (ModelAnimation*)(pData->GetBuffer() + 4);
		memcpy(anims, m_pMesh->anims, nSize - 4);
	}

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteAnimationBlock(AnimationBlock* b, const Animated<Vector3>& anims)
{
	if (b == 0) return false;
	memset(b, 0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if (anims.ranges.size() == 1 && anims.seq == -1 && anims.type != 0)
	{
		b->nRanges = 0;
		b->ofsRanges = 0;
	}
	else
		b->nRanges = (uint32)anims.ranges.size();
	if (b->nRanges > 0)
		b->ofsRanges = m_pRawData->AddRawData((const Vector2*)(&anims.ranges[0]), b->nRanges);
	if (b->nTimes > 0)
		b->ofsTimes = m_pRawData->AddRawData((const DWORD*)(&anims.times[0]), b->nTimes);
	if (b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<Vector3> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		if (!data.empty())
			b->ofsKeys = m_pRawData->AddRawData((const Vector3*)(&data[0]), b->nKeys * 3);
	}
	else if (!anims.data.empty())
	{
		b->ofsKeys = m_pRawData->AddRawData((const Vector3*)(&anims.data[0]), b->nKeys);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteAnimationBlock(AnimationBlock* b, const AnimatedShort& anims)
{
	if (b == 0) return false;
	memset(b, 0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if (anims.ranges.size() == 1 && anims.seq == -1 && anims.type != 0)
	{
		b->nRanges = 0;
		b->ofsRanges = 0;
	}
	else
		b->nRanges = (uint32)anims.ranges.size();
	if (b->nRanges > 0)
		b->ofsRanges = m_pRawData->AddRawData((const Vector2*)(&anims.ranges[0]), b->nRanges);
	if (b->nTimes > 0)
		b->ofsTimes = m_pRawData->AddRawData((const DWORD*)(&anims.times[0]), b->nTimes);
	if (b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<float> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		if (!data.empty())
			b->ofsKeys = m_pRawData->AddRawData((const DWORD*)(&data[0]), b->nKeys * 3);
	}
	else if (!anims.data.empty())
		b->ofsKeys = m_pRawData->AddRawData((const DWORD*)(&anims.data[0]), b->nKeys);
	return true;
}
bool ParaEngine::XFileCharModelExporter::WriteAnimationBlock(AnimationBlock* b, const Animated<float>& anims)
{
	if (b == 0) return false;
	memset(b, 0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if (anims.ranges.size() == 1 && anims.seq == -1 && anims.type != 0)
	{
		b->nRanges = 0;
		b->ofsRanges = 0;
	}
	else
		b->nRanges = (uint32)anims.ranges.size();
	if (b->nRanges > 0)
		b->ofsRanges = m_pRawData->AddRawData((const Vector2*)(&anims.ranges[0]), b->nRanges);
	if (b->nTimes > 0)
		b->ofsTimes = m_pRawData->AddRawData((const DWORD*)(&anims.times[0]), b->nTimes);
	if (b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<float> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		if (!data.empty())
			b->ofsKeys = m_pRawData->AddRawData((const DWORD*)(&data[0]), b->nKeys * 3);
	}
	else if (!anims.data.empty())
		b->ofsKeys = m_pRawData->AddRawData((const DWORD*)(&anims.data[0]), b->nKeys);
	return true;
}
bool ParaEngine::XFileCharModelExporter::WriteAnimationBlock(AnimationBlock* b, const Animated<Quaternion>& anims)
{
	if (b == 0) return false;
	memset(b, 0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if (anims.ranges.size() == 1 && anims.seq == -1 && anims.type != 0)
	{
		b->nRanges = 0;
		b->ofsRanges = 0;
	}
	else
		b->nRanges = (uint32)anims.ranges.size();
	if (b->nRanges > 0)
		b->ofsRanges = m_pRawData->AddRawData((const Vector2*)(&anims.ranges[0]), b->nRanges);
	if (b->nTimes > 0)
		b->ofsTimes = m_pRawData->AddRawData((const DWORD*)(&anims.times[0]), b->nTimes);
	if (b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<Quaternion> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		if (!data.empty())
			b->ofsKeys = m_pRawData->AddRawData((const Vector4*)(&data[0]), b->nKeys * 3);
	}
	else if (!anims.data.empty())
		b->ofsKeys = m_pRawData->AddRawData((const Vector4*)(&anims.data[0]), b->nKeys);
	return true;
}

#pragma endregion
