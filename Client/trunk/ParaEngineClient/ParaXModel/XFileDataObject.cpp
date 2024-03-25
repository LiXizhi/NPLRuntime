//----------------------------------------------------------------------
// Class:	XFile data object
// Authors:	LiXizhi
// Date:	2014.10.7
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "modelheaders.h"
#include "ModelRenderPass.h"
#include "XFileParser.h"
#include "XFileExporter.h"
#include "XFileDataObject.h"

using namespace ParaEngine;

ParaEngine::XFileDataObject::XFileDataObject()
{
}

ParaEngine::XFileDataObject::~XFileDataObject()
{
}

void ParaEngine::XFileDataObject::Init(XFileParser& parser, const std::string& objectName)
{
	m_sTemplateName = objectName;
	parser.readHeadOfDataObject(&m_sName);
	// for all types of data structures here. 
	if (objectName == "ParaXHeader")
	{
		if (!ReadParaXHeader(parser))
			OUTPUT_LOG("error loading vertices");
	}
	else if (objectName == "ParaXBody")
	{
		if (!ReadParaXBody(parser))
			OUTPUT_LOG("error loading vertices");
	}
	else if (objectName == "ParaXRefSection")
	{
		parser.ParseUnknownDataObject();
	}
	else if (objectName == "XDWORDArray")
	{
		if (!ReadXDWORDArray(parser))
			OUTPUT_LOG("error loading XDWORDArray");
	}
	else if (objectName == "XVertices")
	{
		if (!ReadXVertices(parser))
			OUTPUT_LOG("error loading vertices");
	}
	else if (objectName == "XVoxels")
	{
		if (!ReadXVertices(parser))
			OUTPUT_LOG("error loading voxels");
	}
	else if (objectName == "XTextures")
	{
		if (!ReadXTextures(parser))
			OUTPUT_LOG("error loading XTextures");
	}
	else if (objectName == "XAttachments")
	{
		if (!ReadXAttachments(parser))
			OUTPUT_LOG("error loading XAttachments");
	}
	else if (objectName == "XTransparency")
	{
		if (!ReadXTransparency(parser))
			OUTPUT_LOG("error loading XTransparency");
	}
	else if (objectName == "XViews")
	{
		if (!ReadXViews(parser))
			OUTPUT_LOG("error loading XViews");
	}
	else if (objectName == "XIndices0")
	{
		if (!ReadXIndices0(parser))
			OUTPUT_LOG("error loading XIndices0");
	}
	else if (objectName == "XGeosets")
	{
		if (!ReadXGeosets(parser))
			OUTPUT_LOG("error loading XGeosets");
	}
	else if (objectName == "XRenderPass")
	{
		if (!ReadXRenderPass(parser))
			OUTPUT_LOG("error loading XRenderPass");
	}
	else if (objectName == "XBones")
	{
		if (!ReadXBones(parser))
			OUTPUT_LOG("error loading XBones");
	}
	else if (objectName == "XTexAnims")
	{
		if (!ReadXTexAnims(parser))
			OUTPUT_LOG("error loading XTexAnims");
	}
	else if (objectName == "XParticleEmitters")
	{
		if (!ReadXParticleEmitters(parser))
			OUTPUT_LOG("error loading XParticleEmitters");
	}
	else if (objectName == "XRibbonEmitters")
	{
		if (!ReadXRibbonEmitters(parser))
			OUTPUT_LOG("error loading XRibbonEmitters");
	}
	else if (objectName == "XColors")
	{
		if (!ReadXColors(parser))
			OUTPUT_LOG("error loading XColors");
	}
	else if (objectName == "XCameras")
	{
		if (!ReadXCameras(parser))
			OUTPUT_LOG("error loading XCameras");
	}
	else if (objectName == "XLights")
	{
		if (!ReadXLights(parser))
			OUTPUT_LOG("error loading XLights");
	}
	else if (objectName == "XAnimations")
	{
		if (!ReadXAnimations(parser))
			OUTPUT_LOG("error loading XAnimations");
	}
	else if (objectName == "}")
	{

	}
	else
	{
		// unknown format
		// OUTPUT_DEBUG("Unknown data object in animation of .x file");
		parser.ParseUnknownDataObject();
	}
}

void ParaEngine::XFileDataObject::AddChild(ref_ptr<XFileDataObject> child)
{
	m_children.push_back(child);
}

bool ParaEngine::XFileDataObject::ReadXDWORDArray(XFileParser& parser)
{
	// read count
	int nCount = parser.ReadInt();
	ResizeBuffer(nCount * 4 + 4);
	uint32* pData = (uint32*)(GetBuffer() + 4);
	for (int a = 0; a < nCount; a++) {
		pData[a] = (uint32)parser.ReadInt();
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool ParaEngine::XFileDataObject::ReadParaXBody(XFileParser& parser)
{
	bool running = true;
	while (running)
	{
		// read name of next object
		std::string objectName = parser.GetNextToken();
		if (objectName.length() == 0 && parser.IsEndOfFile())
			break;
		if (objectName == "}")
		{
			break;
		}
		else if (objectName.empty())
		{
			OUTPUT_LOG("warning: empty object name in ParaX file\n");
		}
		else
		{
			XFileDataObjectPtr node(new XFileDataObject());
			node->Init(parser, objectName);
			AddChild(node);
		}
	}
	return true;
}

bool ParaEngine::XFileDataObject::ReadUnknownDataObject(XFileParser& parser)
{
	parser.ReadToEndOfDataObject();
	return true;
}

void ParaEngine::XFileDataObject::ReadAnimationBlock(AnimationBlock& unk, XFileParser& parser)
{
	unk.type = (int16)parser.ReadInt();
	unk.seq = (int16)parser.ReadInt();
	unk.nRanges = parser.ReadInt();
	unk.ofsRanges = parser.ReadInt();
	unk.nTimes = parser.ReadInt();
	unk.ofsTimes = parser.ReadInt();
	unk.nKeys = parser.ReadInt();
	unk.ofsKeys = parser.ReadInt();
}

int ParaEngine::XFileDataObject::GetChildCount()
{
	return (int)(m_children.size());
}

XFileDataObjectPtr ParaEngine::XFileDataObject::GetChild(int i)
{
	return m_children[i];
}

bool ParaEngine::XFileDataObject::IsReference()
{
	return false;
}

const std::string& ParaEngine::XFileDataObject::GetType()
{
	return m_sTemplateName;
}

const std::string& ParaEngine::XFileDataObject::GetName()
{
	return m_sName;
}

bool ParaEngine::XFileDataObject::Lock(DWORD* dwSize, const char** pBuffer)
{
	*dwSize = m_buffer.size();
	*pBuffer = GetBuffer();
	return true;
}

void ParaEngine::XFileDataObject::Unlock()
{
}

void ParaEngine::XFileDataObject::ResizeBuffer(uint32 nSize)
{
	m_buffer.resize(nSize);
}

bool XFileDataObject::ReadParaXHeader(XFileParser& parser)
{
	ResizeBuffer(sizeof(ParaXHeaderDef));
	ParaXHeaderDef* xheader = (ParaXHeaderDef*)GetBuffer();
	parser.ReadCharArray(xheader->id, 4);
	parser.ReadCharArray((char*)(xheader->version), 4);

	xheader->type = parser.ReadInt();
	xheader->IsAnimated = parser.ReadInt();
	xheader->minExtent = parser.ReadVector3();
	xheader->maxExtent = parser.ReadVector3();
	xheader->nModelFormat = parser.ReadInt();
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXGlobalSequences(XFileParser& parser)
{
	return ReadXDWORDArray(parser);
}

bool XFileDataObject::ReadXVertices(XFileParser& parser)
{
	ResizeBuffer(sizeof(XVerticesDef));
	XVerticesDef* pData = (XVerticesDef*)(GetBuffer());
	pData->nType = parser.ReadInt();
	pData->nVertexBytes = parser.ReadInt();
	pData->nVertices = parser.ReadInt();
	pData->ofsVertices = parser.ReadInt();
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXTextures(XFileParser& parser)
{
	DWORD nCount = parser.ReadInt();

	struct ModelTextureDef_ {
		uint32 type;
		uint32 flags;
		uint32 name;
	};

	ResizeBuffer((sizeof(ModelTextureDef_) + 255)*nCount + sizeof(DWORD));
	*((DWORD*)GetBuffer()) = nCount;
	ModelTextureDef_* pData = (ModelTextureDef_ *)(GetBuffer() + 4);

	for (DWORD i = 0; i < nCount; ++i)
	{
		ModelTextureDef_& data = *pData;
		data.type = parser.ReadInt();
		data.flags = parser.ReadInt();
		std::string name = parser.GetNextToken();
		data.name = name[0];
		if (!name.empty())
		{
			memcpy(&(data.name), name.c_str(), name.size() + 1);
		}
		pData = (ModelTextureDef_*)(((unsigned char*)pData) + 8 + name.size() + 1);
	}
	ResizeBuffer((int)(((char*)pData) - GetBuffer()));
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXAttachments(XFileParser& parser)
{
	int nAttachments = parser.ReadInt();
	int nAttachmentLookup = parser.ReadInt();
	ResizeBuffer(8 + sizeof(ModelAttachmentDef)*nAttachments + 4 * nAttachmentLookup);
	*((DWORD*)GetBuffer()) = nAttachments;
	*((DWORD*)(GetBuffer() + 4)) = nAttachmentLookup;
	ModelAttachmentDef *attachments = (ModelAttachmentDef *)(GetBuffer() + 8);
	DWORD * attLookup = (DWORD *)(GetBuffer() + 8 + sizeof(ModelAttachmentDef)*nAttachments);

	for (int i = 0; i < nAttachments; ++i) {
		ModelAttachmentDef& mad = attachments[i];
		mad.id = parser.ReadInt();
		mad.bone = parser.ReadInt();
		mad.pos = parser.ReadVector3();
		ReadAnimationBlock(mad.unk, parser);
	}
	// attachment lookups
	if (nAttachmentLookup > 0) {
		for (int i = 0; i < nAttachmentLookup; ++i)
		{
			attLookup[i] = parser.ReadInt();
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXColors(XFileParser& parser)
{
	int nColors = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelColorDef)*nColors);
	*((DWORD*)GetBuffer()) = nColors;
	if (nColors > 0)
	{ // at least one Bone
		ModelColorDef *colorDefs = (ModelColorDef*)(GetBuffer() + 4);
		for (int i = 0; i < nColors; ++i)
		{
			ReadAnimationBlock(colorDefs[i].color, parser);
			ReadAnimationBlock(colorDefs[i].opacity, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXTransparency(XFileParser& parser)
{
	int nTransparency = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelTransDef)*nTransparency);
	*((DWORD*)GetBuffer()) = nTransparency;
	if (nTransparency > 0)
	{ // at least one item
		ModelTransDef *transDefs = (ModelTransDef*)(GetBuffer() + 4);
		for (int i = 0; i < nTransparency; ++i)
		{
			ReadAnimationBlock(transDefs[i].trans, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXViews(XFileParser& parser)
{
	// no need to do anything, since there is only one view. all view 0 information are duplicated in other nodes.
	return ReadUnknownDataObject(parser);
}

bool XFileDataObject::ReadXIndices0(XFileParser& parser)
{
	ResizeBuffer(sizeof(Indice0Def));
	Indice0Def * pData = (Indice0Def*)GetBuffer();
	pData->nIndices = parser.ReadInt();
	pData->ofsIndices = parser.ReadInt();
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXGeosets(XFileParser& parser)
{
	int nGeosets = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelGeoset)*nGeosets);
	*((DWORD*)GetBuffer()) = nGeosets;
	if (nGeosets > 0)
	{ // at least one item
		ModelGeoset *pGeosets = (ModelGeoset*)(GetBuffer() + 4);
		for (int i = 0; i < nGeosets; ++i)
		{
			ModelGeoset& geoset = pGeosets[i];
			geoset.id = (uint16)parser.ReadInt();
			geoset.d2 = (uint16)parser.ReadInt();
			geoset.vstart = (uint16)parser.ReadInt();
			geoset.vcount = (uint16)parser.ReadInt();
			geoset.istart = (uint16)parser.ReadInt();
			geoset.icount = (uint16)parser.ReadInt();
			geoset.d3 = (uint16)parser.ReadInt();
			geoset.d4 = (uint16)parser.ReadInt();
			geoset.d5 = (uint16)parser.ReadInt();
			geoset.d6 = (uint16)parser.ReadInt();
			geoset.v = parser.ReadVector3();
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXRenderPass(XFileParser& parser)
{
	int nRenderPasses = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelRenderPass)*nRenderPasses);
	*((DWORD*)GetBuffer()) = nRenderPasses;
	if (nRenderPasses > 0)
	{ // at least one item
		ModelRenderPass *passes = (ModelRenderPass*)(GetBuffer() + 4);
		for (int i = 0; i < nRenderPasses; ++i)
		{
			ModelRenderPass& pass = passes[i];
			pass.indexStart = (uint16)parser.ReadInt();
			pass.indexCount = (uint16)parser.ReadInt();
			pass.vertexStart = (uint16)parser.ReadInt();
			pass.vertexEnd = (uint16)parser.ReadInt();
			pass.tex = parser.ReadInt();
			pass.m_fReserved0 = parser.ReadFloat();
			pass.texanim = (int16)parser.ReadInt();
			pass.color = (int16)parser.ReadInt();
			pass.opacity = (int16)parser.ReadInt();
			pass.blendmode = (int16)parser.ReadInt();
			pass.order = parser.ReadInt();
			pass.geoset = parser.ReadInt();
			*(((DWORD*)&(pass.geoset)) + 1) = parser.ReadInt();
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXBones(XFileParser& parser)
{
	int nBones = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelBoneDef)*nBones);
	*((DWORD*)GetBuffer()) = nBones;
	if (nBones > 0)
	{ // at least one item
		ModelBoneDef *mb = (ModelBoneDef*)(GetBuffer() + 4);
		for (int i = 0; i < nBones; ++i)
		{
			ModelBoneDef& bone = mb[i];
			bone.animid = parser.ReadInt();
			bone.flags = parser.ReadInt();
			bone.parent = (int16)parser.ReadInt();
			bone.boneid = (int16)parser.ReadInt();
			ReadAnimationBlock(bone.translation, parser);
			ReadAnimationBlock(bone.rotation, parser);
			ReadAnimationBlock(bone.scaling, parser);
			bone.pivot = parser.ReadVector3();
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXTexAnims(XFileParser& parser)
{
	int nTexAnims = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelTexAnimDef)*nTexAnims);
	*((DWORD*)GetBuffer()) = nTexAnims;
	if (nTexAnims > 0)
	{ // at least one item
		ModelTexAnimDef *texanims = (ModelTexAnimDef*)(GetBuffer() + 4);
		for (int i = 0; i < nTexAnims; ++i)
		{
			ModelTexAnimDef& anim = texanims[i];
			ReadAnimationBlock(anim.trans, parser);
			ReadAnimationBlock(anim.rot, parser);
			ReadAnimationBlock(anim.scale, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXParticleEmitters(XFileParser& parser)
{
	int nParticleEmitters = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelParticleEmitterDef)*nParticleEmitters);
	*((DWORD*)GetBuffer()) = nParticleEmitters;
	if (nParticleEmitters > 0)
	{ // at least one item
		ModelParticleEmitterDef *particleSystems = (ModelParticleEmitterDef*)(GetBuffer() + 4);
		for (int i = 0; i < nParticleEmitters; ++i)
		{
			ModelParticleEmitterDef& anim = particleSystems[i];

			anim.id = parser.ReadInt();
			anim.flags = parser.ReadInt();
			anim.pos = parser.ReadVector3();
			anim.bone = (int16)parser.ReadInt();
			anim.texture = (int16)parser.ReadInt();
			anim.nZero1 = parser.ReadInt();
			anim.ofsZero1 = parser.ReadInt();
			anim.nZero2 = parser.ReadInt();
			anim.ofsZero2 = parser.ReadInt();
			anim.blend = (int16)parser.ReadInt();
			anim.type = (int16)parser.ReadInt();
			anim.s1 = (int16)parser.ReadInt();
			anim.s2 = (int16)parser.ReadInt();
			anim.cols = (int16)parser.ReadInt();
			anim.rows = (int16)parser.ReadInt();
			for (int a = 0; a < 10; a++)
			{
				ReadAnimationBlock(anim.params[a], parser);
			}
			anim.p.mid = parser.ReadFloat();
			for (int i = 0; i < 3; ++i)
				anim.p.colors[i] = parser.ReadInt();
			for (int i = 0; i < 3; ++i)
				anim.p.sizes[i] = parser.ReadFloat();
			for (int i = 0; i < 10; ++i)
				anim.p.d[i] = (int16)parser.ReadInt();
			for (int i = 0; i < 3; ++i)
				anim.p.unk[i] = parser.ReadFloat();
			for (int i = 0; i < 3; ++i)
				anim.p.scales[i] = parser.ReadFloat();
			anim.p.slowdown = parser.ReadFloat();
			anim.p.rotation = parser.ReadFloat();
			for (int i = 0; i < 16; ++i)
				anim.p.f2[i] = parser.ReadFloat();
			ReadAnimationBlock(anim.unk, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXRibbonEmitters(XFileParser& parser)
{
	int nRibbonEmitters = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelRibbonEmitterDef)*nRibbonEmitters);
	*((DWORD*)GetBuffer()) = nRibbonEmitters;
	if (nRibbonEmitters > 0)
	{ // at least one item
		ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(GetBuffer() + 4);
		for (int i = 0; i < nRibbonEmitters; ++i)
		{
			ModelRibbonEmitterDef& anim = ribbons[i];

			anim.id = parser.ReadInt();
			anim.bone = parser.ReadInt();
			anim.pos = parser.ReadVector3();
			anim.nTextures = parser.ReadInt();
			anim.ofsTextures = parser.ReadInt();
			anim.nUnknown = parser.ReadInt();
			anim.ofsUnknown = parser.ReadInt();
			ReadAnimationBlock(anim.color, parser);
			ReadAnimationBlock(anim.opacity, parser);
			ReadAnimationBlock(anim.above, parser);
			ReadAnimationBlock(anim.below, parser);
			anim.res = parser.ReadFloat();
			anim.length = parser.ReadFloat();
			anim.unk = parser.ReadFloat();
			anim.s1 = (int16)parser.ReadInt();
			anim.s2 = (int16)parser.ReadInt();
			ReadAnimationBlock(anim.unk1, parser);
			ReadAnimationBlock(anim.unk2, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXCameras(XFileParser& parser)
{
	int nCameras = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelCameraDef)*nCameras);
	if (nCameras > 0)
	{ // at least one item
		ModelCameraDef *cameras = (ModelCameraDef*)(GetBuffer() + 4);
		for (int i = 0; i < nCameras; ++i)
		{
			ModelCameraDef& camera = cameras[i];
			camera.id = parser.ReadInt();
			camera.fov = parser.ReadFloat();
			camera.farclip = parser.ReadFloat();
			camera.nearclip = parser.ReadFloat();
			ReadAnimationBlock(camera.transPos, parser);
			camera.pos = parser.ReadVector3();
			ReadAnimationBlock(camera.transTarget, parser);
			camera.target = parser.ReadVector3();
			ReadAnimationBlock(camera.rot, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXLights(XFileParser& parser)
{
	int nLights = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelLightDef)*nLights);
	*((DWORD*)GetBuffer()) = nLights;
	if (nLights > 0)
	{ // at least one item
		ModelLightDef *lights = (ModelLightDef*)(GetBuffer() + 4);
		for (int i = 0; i < nLights; ++i)
		{
			ModelLightDef& light = lights[i];
			light.type = (int16)parser.ReadInt();
			light.bone = (int16)parser.ReadInt();
			light.pos = parser.ReadVector3();
			ReadAnimationBlock(light.ambColor, parser);
			ReadAnimationBlock(light.ambIntensity, parser);
			ReadAnimationBlock(light.color, parser);
			ReadAnimationBlock(light.intensity, parser);
			ReadAnimationBlock(light.attStart, parser);
			ReadAnimationBlock(light.attEnd, parser);
			ReadAnimationBlock(light.unk1, parser);
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

bool XFileDataObject::ReadXAnimations(XFileParser& parser)
{
	int nAnimations = parser.ReadInt();
	ResizeBuffer(4 + sizeof(ModelAnimation)*nAnimations);
	*((DWORD*)GetBuffer()) = nAnimations;
	if (nAnimations > 0)
	{ // at least one item
		ModelAnimation *anims = (ModelAnimation*)(GetBuffer() + 4);
		for (int i = 0; i < nAnimations; ++i)
		{
			ModelAnimation& anim = anims[i];

			anim.animID = parser.ReadInt();
			anim.timeStart = parser.ReadInt();
			anim.timeEnd = parser.ReadInt();
			anim.moveSpeed = parser.ReadFloat();
			anim.loopType = parser.ReadInt();
			anim.flags = parser.ReadInt();
			anim.d1 = parser.ReadInt();
			anim.d2 = parser.ReadInt();
			anim.playSpeed = parser.ReadInt();
			anim.boxA = parser.ReadVector3();
			anim.boxB = parser.ReadVector3();
			anim.rad = parser.ReadFloat();
			anim.s[0] = (int16)parser.ReadInt();
			anim.s[1] = (int16)parser.ReadInt();
		}
	}
	parser.ReadToEndOfDataObject();
	return true;
}

void XFileDataObject::Write(ofstream& strm, XFileExporter& exporter)
{
	if (m_sTemplateName.empty() && m_sName.empty())
	{
		WriteChildren(strm, exporter);
		return;
	}
	else
	{
		exporter.WriteName(strm, m_sTemplateName);
		exporter.WriteName(strm, m_sName);
		exporter.WriteToken(strm, "{");

		WriteInfo(strm, exporter);
		WriteChildren(strm, exporter);

		exporter.WriteToken(strm, "}");
	}
}

void ParaEngine::XFileDataObject::WriteChildren(ofstream& strm, XFileExporter& exporter)
{
	if (GetChildCount() > 0)
	{
		for (int i = 0; i < GetChildCount(); ++i)
		{
			auto pChildData = GetChild(i);
			pChildData->Write(strm, exporter);
		}
	}
}

void ParaEngine::XFileDataObject::WriteInfo(ofstream& strm, XFileExporter& exporter)
{
	if (m_buffer.empty())
	{
		return;
	}

	if (m_sTemplateName == "ParaXHeader")
	{
		WriteParaXHeader(strm, exporter);
	}
	else if (m_sTemplateName == "ParaXBody")
	{
		WriteParaXBody(strm, exporter);
	}
	else if (m_sTemplateName == "XDWORDArray")
	{
		WriteXDWORDArray(strm, exporter);
	}
	else if (m_sTemplateName == "XVertices")
	{
		WriteXVertices(strm, exporter);
	}
	else if (m_sTemplateName == "XVoxels")
	{
		WriteXVertices(strm, exporter);
	}
	else if (m_sTemplateName == "XTextures")
	{
		WriteXTextures(strm, exporter);
	}
	else if (m_sTemplateName == "XAttachments")
	{
		WriteXAttachments(strm, exporter);
	}
	else if (m_sTemplateName == "XTransparency")
	{
		WriteXTransparency(strm, exporter);
	}
	else if (m_sTemplateName == "XViews")
	{
		WriteXViews(strm, exporter);
	}
	else if (m_sTemplateName == "XIndices0")
	{
		WriteXIndices0(strm, exporter);
	}
	else if (m_sTemplateName == "XGeosets")
	{
		WriteXGeosets(strm, exporter);
	}
	else if (m_sTemplateName == "XRenderPass")
	{
		WriteXRenderPass(strm, exporter);
	}
	else if (m_sTemplateName == "XBones")
	{
		WriteXBones(strm, exporter);
	}
	else if (m_sTemplateName == "XTexAnims")
	{
		WriteXTexAnims(strm, exporter);
	}
	else if (m_sTemplateName == "XParticleEmitters")
	{
		WriteXParticleEmitters(strm, exporter);
	}
	else if (m_sTemplateName == "XRibbonEmitters")
	{
		WriteXRibbonEmitters(strm, exporter);
	}
	else if (m_sTemplateName == "XColors")
	{
		WriteXColors(strm, exporter);
	}
	else if (m_sTemplateName == "XCameras")
	{
		WriteXCameras(strm, exporter);
	}
	else if (m_sTemplateName == "XLights")
	{
		WriteXLights(strm, exporter);
	}
	else if (m_sTemplateName == "XAnimations")
	{
		WriteXAnimations(strm, exporter);
	}

	exporter.WriteIntAndFloatArray(strm);
}

void ParaEngine::XFileDataObject::WriteParaXHeader(ofstream& strm, XFileExporter& exporter)
{
	ParaXHeaderDef* xheader = (ParaXHeaderDef*)GetBuffer();

	exporter.WriteCharArray(strm, xheader->id, 4);
	exporter.WriteCharArray(strm, (char*)xheader->version, 4);
	exporter.WriteInt(strm, xheader->type);
	exporter.WriteInt(strm, xheader->IsAnimated);
	exporter.WriteVector3(strm, xheader->minExtent);
	exporter.WriteVector3(strm, xheader->maxExtent);
	exporter.WriteInt(strm, xheader->nModelFormat);
}

void ParaEngine::XFileDataObject::WriteParaXBody(ofstream& strm, XFileExporter& exporter)
{
	/*for (int i = 0;i<GetChildCount();++i)
	{
	auto child = GetChild(i);
	XFileDataObject writer(child);
	writer.Write(strm);
	}*/
}

void ParaEngine::XFileDataObject::WriteXDWORDArray(ofstream& strm, XFileExporter& exporter)
{
	int nCount = (GetSize() - 4) / 4;
	exporter.WriteInt(strm, nCount);

	uint32* pData = (uint32*)(GetBuffer() + 4);
	for (int i = 0; i < nCount; ++i)
	{
		exporter.WriteInt(strm, *pData);
		++pData;
	}
}

void ParaEngine::XFileDataObject::WriteXVertices(ofstream& strm, XFileExporter& exporter)
{
	XVerticesDef* pData = (XVerticesDef*)GetBuffer();
	exporter.WriteInt(strm, pData->nType);
	exporter.WriteInt(strm, pData->nVertexBytes);
	exporter.WriteInt(strm, pData->nVertices);
	exporter.WriteInt(strm, pData->ofsVertices);
}

void ParaEngine::XFileDataObject::WriteXTextures(ofstream& strm, XFileExporter& exporter)
{
	struct ModelTextureDef_ {
		uint32 type;
		uint32 flags;
		char name;
	};
	DWORD nCount = *((DWORD*)GetBuffer());
	exporter.WriteInt(strm, nCount);
	ModelTextureDef_* pData = (ModelTextureDef_ *)(GetBuffer() + 4);
	for (DWORD i = 0; i < nCount; ++i)
	{
		ModelTextureDef_& data = *pData;
		exporter.WriteInt(strm, data.type);
		exporter.WriteInt(strm, data.flags);
		string name(&data.name);
		exporter.WriteString(strm, name);
		pData = (ModelTextureDef_*)((char*)pData + 8 + name.size() + 1);
	}
}

void ParaEngine::XFileDataObject::WriteXAttachments(ofstream& strm, XFileExporter& exporter)
{
	int nAttachments = *((DWORD*)GetBuffer());
	int nAttachmentLookup = *((DWORD*)(GetBuffer() + 4));
	exporter.WriteInt(strm, nAttachments);
	exporter.WriteInt(strm, nAttachmentLookup);
	ModelAttachmentDef *attachments = (ModelAttachmentDef *)(GetBuffer() + 8);
	DWORD * attLookup = (DWORD *)(GetBuffer() + 8 + sizeof(ModelAttachmentDef)*nAttachments);
	for (int i = 0; i < nAttachments; ++i) {
		ModelAttachmentDef& mad = attachments[i];
		exporter.WriteInt(strm, mad.id);
		exporter.WriteInt(strm, mad.bone);
		exporter.WriteVector3(strm, mad.pos);
		WriteAnimationBlock(strm, exporter, mad.unk);
	}
	// attachment lookups
	if (nAttachmentLookup > 0) {
		for (int i = 0; i < nAttachmentLookup; ++i)
		{
			exporter.WriteInt(strm, attLookup[i]);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXTransparency(ofstream& strm, XFileExporter& exporter)
{
	int nTransparency = *((DWORD*)GetBuffer());
	if (nTransparency > 0)
	{ // at least one item
		exporter.WriteInt(strm, nTransparency);
		ModelTransDef *transDefs = (ModelTransDef*)(GetBuffer() + 4);
		for (int i = 0; i < nTransparency; ++i)
		{
			WriteAnimationBlock(strm, exporter, transDefs[i].trans);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXViews(ofstream& strm, XFileExporter& exporter)
{
	// no need to do anything, since there is only one view. all view 0 information are duplicated in other nodes.
	int nView = *((DWORD*)GetBuffer());
	if (nView > 0)
	{
		exporter.WriteInt(strm, nView);
		ModelView *pView = (ModelView*)(GetBuffer() + 4);
		for (int i = 0; i < nView; ++i)
		{
			exporter.WriteInt(strm, pView->nIndex);
			exporter.WriteInt(strm, pView->ofsIndex);
			exporter.WriteInt(strm, pView->nTris);
			exporter.WriteInt(strm, pView->ofsTris);
			exporter.WriteInt(strm, pView->nProps);
			exporter.WriteInt(strm, pView->ofsProps);
			exporter.WriteInt(strm, pView->nSub);
			exporter.WriteInt(strm, pView->ofsSub);
			exporter.WriteInt(strm, pView->nTex);
			exporter.WriteInt(strm, pView->ofsTex);
			exporter.WriteInt(strm, pView->lod);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXIndices0(ofstream& strm, XFileExporter& exporter)
{
	Indice0Def * pData = (Indice0Def*)GetBuffer();
	exporter.WriteInt(strm, pData->nIndices);
	exporter.WriteInt(strm, pData->ofsIndices);
}

void ParaEngine::XFileDataObject::WriteXGeosets(ofstream& strm, XFileExporter& exporter)
{
	int nGeosets = *((DWORD*)GetBuffer());
	if (nGeosets > 0)
	{ // at least one item
		exporter.WriteInt(strm, nGeosets);
		ModelGeoset *pGeosets = (ModelGeoset*)(GetBuffer() + 4);
		for (int i = 0; i < nGeosets; ++i)
		{
			ModelGeoset& geoset = pGeosets[i];
			exporter.WriteInt(strm, geoset.id);
			exporter.WriteInt(strm, geoset.d2);
			exporter.WriteInt(strm, geoset.vstart);
			exporter.WriteInt(strm, geoset.vcount);
			exporter.WriteInt(strm, geoset.istart);
			exporter.WriteInt(strm, geoset.icount);
			exporter.WriteInt(strm, geoset.d3);
			exporter.WriteInt(strm, geoset.d4);
			exporter.WriteInt(strm, geoset.d5);
			exporter.WriteInt(strm, geoset.d6);
			exporter.WriteVector3(strm, geoset.v);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXRenderPass(ofstream& strm, XFileExporter& exporter)
{
	int nRenderPasses = *((DWORD*)GetBuffer());
	if (nRenderPasses > 0)
	{ // at least one item
		exporter.WriteInt(strm, nRenderPasses);
		ModelRenderPass *passes = (ModelRenderPass*)(GetBuffer() + 4);
		for (int i = 0; i < nRenderPasses; ++i)
		{
			ModelRenderPass& pass = passes[i];
			exporter.WriteInt(strm, pass.indexStart);
			exporter.WriteInt(strm, pass.indexCount);
			exporter.WriteInt(strm, pass.vertexStart);
			exporter.WriteInt(strm, pass.vertexEnd);
			exporter.WriteInt(strm, pass.tex);
			exporter.WriteFloat(strm, pass.m_fReserved0);
			exporter.WriteInt(strm, pass.texanim);
			exporter.WriteInt(strm, pass.color);
			exporter.WriteInt(strm, pass.opacity);
			exporter.WriteInt(strm, pass.blendmode);
			exporter.WriteInt(strm, pass.order);
			exporter.WriteInt(strm, pass.geoset);
			exporter.WriteInt(strm, *(((DWORD*)&(pass.geoset)) + 1));
		}
	}
}

void ParaEngine::XFileDataObject::WriteXBones(ofstream& strm, XFileExporter& exporter)
{
	int nBones = *((DWORD*)GetBuffer());
	if (nBones > 0)
	{ // at least one item
		exporter.WriteInt(strm, nBones);
		ModelBoneDef *mb = (ModelBoneDef*)(GetBuffer() + 4);
		for (int i = 0; i < nBones; ++i)
		{
			ModelBoneDef& bone = mb[i];
			exporter.WriteInt(strm, bone.animid);
			exporter.WriteInt(strm, bone.flags);
			exporter.WriteShort(strm, bone.parent);
			exporter.WriteShort(strm, bone.boneid);
			WriteAnimationBlock(strm, exporter, bone.translation);
			WriteAnimationBlock(strm, exporter, bone.rotation);
			WriteAnimationBlock(strm, exporter, bone.scaling);
			exporter.WriteVector3(strm, bone.pivot);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXTexAnims(ofstream& strm, XFileExporter& exporter)
{
	int nTexAnims = *((DWORD*)GetBuffer());
	if (nTexAnims > 0)
	{ // at least one item
		exporter.WriteInt(strm, nTexAnims);
		ModelTexAnimDef *texanims = (ModelTexAnimDef*)(GetBuffer() + 4);
		for (int i = 0; i < nTexAnims; ++i)
		{
			ModelTexAnimDef& anim = texanims[i];
			WriteAnimationBlock(strm, exporter, anim.rot);
			WriteAnimationBlock(strm, exporter, anim.trans);
			WriteAnimationBlock(strm, exporter, anim.scale);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXParticleEmitters(ofstream& strm, XFileExporter& exporter)
{
	int nParticleEmitters = *((DWORD*)GetBuffer());
	if (nParticleEmitters > 0)
	{ // at least one item
		exporter.WriteInt(strm, nParticleEmitters);
		ModelParticleEmitterDef *particleSystems = (ModelParticleEmitterDef*)(GetBuffer() + 4);
		for (int i = 0; i < nParticleEmitters; ++i)
		{
			ModelParticleEmitterDef& anim = particleSystems[i];
			exporter.WriteInt(strm, anim.id);
			exporter.WriteInt(strm, anim.flags);
			exporter.WriteVector3(strm, anim.pos);
			exporter.WriteInt(strm, anim.bone);
			exporter.WriteInt(strm, anim.texture);
			exporter.WriteInt(strm, anim.nZero1);
			exporter.WriteInt(strm, anim.ofsZero1);
			exporter.WriteInt(strm, anim.nZero2);
			exporter.WriteInt(strm, anim.ofsZero2);
			exporter.WriteInt(strm, anim.blend);
			exporter.WriteInt(strm, anim.type);
			exporter.WriteInt(strm, anim.s1);
			exporter.WriteInt(strm, anim.s2);
			exporter.WriteInt(strm, anim.cols);
			exporter.WriteInt(strm, anim.rows);
			for (int a = 0; a < 10; a++)
			{
				WriteAnimationBlock(strm, exporter, anim.params[a]);
			}
			exporter.WriteFloat(strm, anim.p.mid);
			for (int i = 0; i < 3; ++i)
				exporter.WriteInt(strm, anim.p.colors[i]);
			for (int i = 0; i < 3; ++i)
				exporter.WriteFloat(strm, anim.p.sizes[i]);
			for (int i = 0; i < 10; ++i)
				exporter.WriteInt(strm, anim.p.d[i]);
			for (int i = 0; i < 3; ++i)
				exporter.WriteFloat(strm, anim.p.unk[i]);
			for (int i = 0; i < 3; ++i)
				exporter.WriteFloat(strm, anim.p.scales[i]);
			exporter.WriteFloat(strm, anim.p.slowdown);
			exporter.WriteFloat(strm, anim.p.rotation);
			for (int i = 0; i < 16; ++i)
				exporter.WriteFloat(strm, anim.p.f2[i]);
			WriteAnimationBlock(strm, exporter, anim.unk);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXRibbonEmitters(ofstream& strm, XFileExporter& exporter)
{
	int nRibbonEmitters = *((DWORD*)GetBuffer());
	if (nRibbonEmitters > 0)
	{ // at least one item
		exporter.WriteInt(strm, nRibbonEmitters);
		ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(GetBuffer() + 4);
		for (int i = 0; i < nRibbonEmitters; ++i)
		{
			ModelRibbonEmitterDef& anim = ribbons[i];
			exporter.WriteInt(strm, anim.id);
			exporter.WriteInt(strm, anim.bone);
			exporter.WriteVector3(strm, anim.pos);
			exporter.WriteInt(strm, anim.nTextures);
			exporter.WriteInt(strm, anim.ofsTextures);
			exporter.WriteInt(strm, anim.nUnknown);
			exporter.WriteInt(strm, anim.ofsUnknown);
			WriteAnimationBlock(strm, exporter, anim.color);
			WriteAnimationBlock(strm, exporter, anim.opacity);
			WriteAnimationBlock(strm, exporter, anim.above);
			WriteAnimationBlock(strm, exporter, anim.below);
			exporter.WriteFloat(strm, anim.res);
			exporter.WriteFloat(strm, anim.length);
			exporter.WriteFloat(strm, anim.unk);
			exporter.WriteInt(strm, anim.s1);
			exporter.WriteInt(strm, anim.s2);
			WriteAnimationBlock(strm, exporter, anim.unk1);
			WriteAnimationBlock(strm, exporter, anim.unk2);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXColors(ofstream& strm, XFileExporter& exporter)
{
	int nColors = *((DWORD*)GetBuffer());
	if (nColors > 0)
	{ // at least one Bone
		exporter.WriteInt(strm, nColors);
		ModelColorDef *colorDefs = (ModelColorDef*)(GetBuffer() + 4);
		for (int i = 0; i < nColors; ++i)
		{
			WriteAnimationBlock(strm, exporter, colorDefs[i].color);
			WriteAnimationBlock(strm, exporter, colorDefs[i].opacity);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXCameras(ofstream& strm, XFileExporter& exporter)
{
	int nCameras = (GetSize() - 4) / sizeof(ModelCameraDef);
	exporter.WriteInt(strm, nCameras);
	if (nCameras > 0)
	{ // at least one item
		exporter.WriteInt(strm, nCameras);
		ModelCameraDef *cameras = (ModelCameraDef*)(GetBuffer() + 4);
		for (int i = 0; i < nCameras; ++i)
		{
			ModelCameraDef& camera = cameras[i];
			exporter.WriteInt(strm, camera.id);
			exporter.WriteFloat(strm, camera.fov);
			exporter.WriteFloat(strm, camera.farclip);
			exporter.WriteFloat(strm, camera.nearclip);
			WriteAnimationBlock(strm, exporter, camera.transPos);
			exporter.WriteVector3(strm, camera.pos);
			WriteAnimationBlock(strm, exporter, camera.transTarget);
			exporter.WriteVector3(strm, camera.target);
			WriteAnimationBlock(strm, exporter, camera.rot);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXLights(ofstream& strm, XFileExporter& exporter)
{
	int nLights = *((DWORD*)GetBuffer());
	if (nLights > 0)
	{ // at least one item
		exporter.WriteInt(strm, nLights);
		ModelLightDef *lights = (ModelLightDef*)(GetBuffer() + 4);
		for (int i = 0; i < nLights; ++i)
		{
			ModelLightDef& light = lights[i];
			exporter.WriteInt(strm, light.type);
			exporter.WriteInt(strm, light.bone);
			exporter.WriteVector3(strm, light.pos);
			WriteAnimationBlock(strm, exporter, light.ambColor);
			WriteAnimationBlock(strm, exporter, light.ambIntensity);
			WriteAnimationBlock(strm, exporter, light.color);
			WriteAnimationBlock(strm, exporter, light.intensity);
			WriteAnimationBlock(strm, exporter, light.attStart);
			WriteAnimationBlock(strm, exporter, light.attEnd);
			WriteAnimationBlock(strm, exporter, light.unk1);
		}
	}
}

void ParaEngine::XFileDataObject::WriteXAnimations(ofstream& strm, XFileExporter& exporter)
{
	int nAnimations = *((DWORD*)GetBuffer());
	if (nAnimations > 0)
	{ // at least one item
		exporter.WriteInt(strm, nAnimations);
		ModelAnimation *anims = (ModelAnimation*)(GetBuffer() + 4);
		for (int i = 0; i < nAnimations; ++i)
		{
			ModelAnimation& anim = anims[i];

			exporter.WriteInt(strm, anim.animID);
			exporter.WriteInt(strm, anim.timeStart);
			exporter.WriteInt(strm, anim.timeEnd);
			exporter.WriteFloat(strm, anim.moveSpeed);
			exporter.WriteInt(strm, anim.loopType);
			exporter.WriteInt(strm, anim.flags);
			exporter.WriteInt(strm, anim.d1);
			exporter.WriteInt(strm, anim.d2);
			exporter.WriteInt(strm, anim.playSpeed);
			exporter.WriteVector3(strm, anim.boxA);
			exporter.WriteVector3(strm, anim.boxB);
			exporter.WriteFloat(strm, anim.rad);
			exporter.WriteInt(strm, anim.s[0]);
			exporter.WriteInt(strm, anim.s[1]);
		}
	}
}

void ParaEngine::XFileDataObject::WriteAnimationBlock(ofstream& strm, XFileExporter& exporter, AnimationBlock& unk)
{
	exporter.WriteShort(strm, unk.type);
	exporter.WriteShort(strm, unk.seq);
	exporter.WriteInt(strm, unk.nRanges);
	exporter.WriteInt(strm, unk.ofsRanges);
	exporter.WriteInt(strm, unk.nTimes);
	exporter.WriteInt(strm, unk.ofsTimes);
	exporter.WriteInt(strm, unk.nKeys);
	exporter.WriteInt(strm, unk.ofsKeys);
}

#pragma endregion