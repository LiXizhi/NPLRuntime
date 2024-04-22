#include "ParaEngine.h"
#include "XFileCharModelExporter.h"
#include "ParaVoxelModel.h"
#include "./ParaXSerializer.h"
#include "Core/TextureEntity.h"
#include "util/StringHelper.h"
#include "ParaXBone.h"
#include "ZipWriter.h"
#include "particle.h"
#include <fstream>

using namespace ParaEngine;
vector<XFileTemplate_t> XFileCharModelExporter::m_vecTemplates = vector<XFileTemplate_t>();

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
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	auto path16 = ParaEngine::StringHelper::MultiByteToWideChar(filepath.c_str(), DEFAULT_FILE_ENCODING);
	ofstream file(path16, ios_base::binary);
#else
	ofstream file(filepath, ios_base::binary);
#endif

	if (file.is_open())
	{
		XFileCharModelExporter exporter(file, pMesh);
		exporter.ExportParaXModel(file);

		// compress the file to zip if file size is bigger than 10KB
		if (file.tellp() > 10240)
		{
			file.close();
			CZipWriter writer;
			std::string filename = CParaFile::GetWritablePath();
			filename = filename + "temp/xfile.zip";
			writer.InitNewZip(filename.c_str());
			writer.ZipAdd("data", filepath.c_str());
			if (writer.close() == 0) {
				OUTPUT_LOG("failed to compress the xfile to zip %s \n", filename.c_str());
				return false;
			}
			if (!CParaFile::CopyFile(filename.c_str(), filepath.c_str(), true)) {
				OUTPUT_LOG("failed to copy the zip file %s to %s \n", filename.c_str(), filepath.c_str());
				return false;
			}
		}
		return true;
	}
	return false;
}

void ParaEngine::XFileCharModelExporter::InitTemplates()
{
	if (XFileCharModelExporter::m_vecTemplates.empty())
	{
		auto& vec = XFileCharModelExporter::m_vecTemplates;

		UUID_t guid = { 0x3d82ab5e, 0x62da, 0x11cf,{ 0xab, 0x39, 0x00, 0x20, 0xaf, 0x71, 0xe4, 0x33 } };
		XFileTemplate_t stTemplate("Vector", guid);
		stTemplate.members.push_back(XFileTemplateMember_t("x", "FLOAT"));
		stTemplate.members.push_back(XFileTemplateMember_t("y", "FLOAT"));
		stTemplate.members.push_back(XFileTemplateMember_t("z", "FLOAT"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ParaXHeader", UUID_ParaXHeader);
		stTemplate.members.push_back(XFileTemplateMember_t("id", "CHAR", "4"));
		stTemplate.members.push_back(XFileTemplateMember_t("version", "UCHAR", "4"));
		stTemplate.members.push_back(XFileTemplateMember_t("type", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("AnimationBitwise", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("minExtent", "Vector"));
		stTemplate.members.push_back(XFileTemplateMember_t("maxExtent", "Vector"));
		stTemplate.members.push_back(XFileTemplateMember_t("nReserved", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ParaXBody", UUID_ParaXBody, true);
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelView", UUID_ModelView);
		stTemplate.members.push_back(XFileTemplateMember_t("nIndex", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsIndex", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nTris", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsTris", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nProps", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsProps", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nSub", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsSub", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nTex", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsTex", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("lod", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XViews", UUID_XViews);
		stTemplate.members.push_back(XFileTemplateMember_t("nView", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("views", "ModelView", "nView"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelTextureDef", UUID_ModelTextureDef);
		stTemplate.members.push_back(XFileTemplateMember_t("type", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("flags", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("name", "STRING"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XTextures", UUID_XTextures);
		stTemplate.members.push_back(XFileTemplateMember_t("nTextures", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("textures", "ModelTextureDef", "nTextures"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("AnimationBlock", UUID_AnimationBlock);
		stTemplate.members.push_back(XFileTemplateMember_t("type", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("seq", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nRanges", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsRanges", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nTimes", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsTimes", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nKeys", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsKeys", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelAttachmentDef", UUID_ModelAttachmentDef);
		stTemplate.members.push_back(XFileTemplateMember_t("id", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("bone", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("pos", "Vector"));
		stTemplate.members.push_back(XFileTemplateMember_t("unk", "AnimationBlock"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XAttachments", UUID_XAttachments);
		stTemplate.members.push_back(XFileTemplateMember_t("nAttachments", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nAttachLookup", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("attachments", "ModelAttachmentDef", "nAttachments"));
		stTemplate.members.push_back(XFileTemplateMember_t("attLookup", "DWORD", "nAttachLookup"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XVertices", UUID_XVertices);
		stTemplate.members.push_back(XFileTemplateMember_t("nType", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nVertexBytes", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nVertices", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsVertices", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XVoxels", UUID_XVoxels);
		stTemplate.members.push_back(XFileTemplateMember_t("nType", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nVertexBytes", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("nVertices", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsVertices", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XIndices0", UUID_XIndices0);
		stTemplate.members.push_back(XFileTemplateMember_t("nIndices", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("ofsIndices", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelGeoset", UUID_ModelGeoset);
		stTemplate.members.push_back(XFileTemplateMember_t("id", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d2", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("vstart", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("vcount", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("istart", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("icount", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d3", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d4", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d5", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d6", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("v", "Vector"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XGeosets", UUID_XGeosets);
		stTemplate.members.push_back(XFileTemplateMember_t("nGeosets", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("geosets", "ModelGeoset", "nGeosets"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		guid = { 0x10000015, 0x0000, 0x0000,{ 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00 } };
		stTemplate.Init("ModelRenderPass", guid);
		stTemplate.members.push_back(XFileTemplateMember_t("indexStart", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("indexCount", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("vertexStart", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("vertexEnd", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("tex", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("p", "FLOAT"));
		stTemplate.members.push_back(XFileTemplateMember_t("texanim", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("color", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("opacity", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("blendmode", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("order", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("geoset", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("renderstateBitWise", "DWORD"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XRenderPass", UUID_XRenderPass);
		stTemplate.members.push_back(XFileTemplateMember_t("nPasses", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("passes", "ModelRenderPass", "nPasses"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelBoneDef", UUID_ModelBoneDef);
		stTemplate.members.push_back(XFileTemplateMember_t("animid", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("flags", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("parent", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("geoid", "WORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("translation", "AnimationBlock"));
		stTemplate.members.push_back(XFileTemplateMember_t("rotation", "AnimationBlock"));
		stTemplate.members.push_back(XFileTemplateMember_t("scaling", "AnimationBlock"));
		stTemplate.members.push_back(XFileTemplateMember_t("pivot", "Vector"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XBones", UUID_XBones);
		stTemplate.members.push_back(XFileTemplateMember_t("nBones", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("bones", "ModelBoneDef", "nBones"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("ModelAnimation", UUID_ModelAnimation);
		stTemplate.members.push_back(XFileTemplateMember_t("animID", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("timeStart", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("timeEnd", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("moveSpeed", "FLOAT"));
		stTemplate.members.push_back(XFileTemplateMember_t("loopType", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("flags", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d1", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("d2", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("playSpeed", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("boxA", "Vector"));
		stTemplate.members.push_back(XFileTemplateMember_t("boxB", "Vector"));
		stTemplate.members.push_back(XFileTemplateMember_t("rad", "FLOAT"));
		stTemplate.members.push_back(XFileTemplateMember_t("s", "WORD", "2"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XAnimations", UUID_XAnimations);
		stTemplate.members.push_back(XFileTemplateMember_t("nAnimations", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("anims", "ModelAnimation", "nAnimations"));
		vec.push_back(stTemplate);

		stTemplate.clear();
		stTemplate.Init("XDWORDArray", UUID_XDWORDArray);
		stTemplate.members.push_back(XFileTemplateMember_t("nCount", "DWORD"));
		stTemplate.members.push_back(XFileTemplateMember_t("dwData", "DWORD", "nCount"));
		vec.push_back(stTemplate);
	}
}

void ParaEngine::XFileCharModelExporter::UUIDToBin(UUID_t guid, char* bin)
{
	for (int i = 0; i < 4; ++i)
	{
		*bin = (guid.Data1 >> (8 * i)) & 0xff;
		++bin;
	}
	for (int i = 0; i < 2; ++i)
	{
		*bin = (guid.Data2 >> (8 * i)) & 0xff;
		++bin;
	}
	for (int i = 0; i < 2; ++i)
	{
		*bin = (guid.Data3 >> (8 * i)) & 0xff;
		++bin;
	}
	memcpy(bin, guid.Data4, 8);
}

void ParaEngine::XFileCharModelExporter::WriteUUID(ofstream& strm, UUID_t uuid)
{
	WriteIntAndFloatArray(strm);

	WriteToken(strm, TOKEN_GUID);
	char binUUID[16] = { 0 };
	UUIDToBin(uuid, binUUID);
	strm.write(binUUID, 16);
}

void ParaEngine::XFileCharModelExporter::WriteTemplates(ofstream& strm)
{
	XFileCharModelExporter::InitTemplates();
	for (XFileTemplate_t& tem : XFileCharModelExporter::m_vecTemplates)
	{
		WriteTemplate(strm, tem);
	}
}

void ParaEngine::XFileCharModelExporter::WriteTemplateMember(ofstream& strm, const XFileTemplateMember_t& memeber)
{
	if (!memeber.count.empty())
	{
		WriteToken(strm, TOKEN_ARRAY);
	}
	if (!WriteToken(strm, memeber.type))
	{
		WriteName(strm, memeber.type);
	}
	WriteName(strm, memeber.name);
	if (!memeber.count.empty())
	{
		WriteToken(strm, TOKEN_OBRACKET);
		int count = atoi(memeber.count.c_str());
		if (0 == count)
		{
			WriteName(strm, memeber.count);
		}
		else
		{
			WriteBinWord(strm, 0x03);
			WriteBinDWord(strm, count);
		}
		WriteToken(strm, TOKEN_CBRACKET);
	}
	WriteToken(strm, TOKEN_SEMICOLON);
}

void ParaEngine::XFileCharModelExporter::WriteTemplate(ofstream& strm, const XFileTemplate_t& stTem)
{
	WriteToken(strm, TOKEN_TEMPLATE);
	WriteName(strm, stTem.name);
	WriteToken(strm, TOKEN_OBRACE);

	WriteUUID(strm, stTem.uuid);

	auto& members = stTem.members;

	for (auto iter = members.begin(); iter != members.end(); ++iter)
	{
		WriteTemplateMember(strm, *iter);
	}

	if (stTem.beExtend)
	{
		WriteToken(strm, TOKEN_OBRACKET);
		WriteToken(strm, TOKEN_DOT);
		WriteToken(strm, TOKEN_DOT);
		WriteToken(strm, TOKEN_DOT);
		WriteToken(strm, TOKEN_CBRACKET);
	}

	WriteToken(strm, TOKEN_CBRACE);
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

	stHead->nModelFormat |= PARAX_FORMAT_EXTENDED_HEADER2;
	ParaXHeaderDef2 header2;
	memset(&header2, 0, sizeof(ParaXHeaderDef2));
	header2.IsAnimated = stHead->IsAnimated;
	header2.neck_yaw_axis = m_pMesh->m_vNeckYawAxis;
	header2.neck_pitch_axis = m_pMesh->m_vNeckPitchAxis;
	stHead->nOffsetAdditionalHeader = m_pRawData->AddRawData((const char*)(&header2), sizeof(ParaXHeaderDef2));

	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXBody(XFileDataObjectPtr pData, const string& strName)
{
	pData->m_sTemplateName = "ParaXBody";
	pData->m_sName = strName;

	static vector<string> vecChildNames{ "XViews","XTextures","XAttachments","XVertices", "XVoxels", "XIndices0" ,"XGeosets" ,"XRenderPass" ,"XBones" ,"XAnimations" ,
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
		DWORD nCount = m_pRawData->GetSize() / sizeof(int32);
		pData->ResizeBuffer(4 + m_pRawData->GetSize() + 4);
		*(DWORD*)pData->GetBuffer() = nCount;
		memcpy(pData->GetBuffer() + 4, m_pRawData->GetBuffer(), m_pRawData->GetSize());
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteParaXBodyChild(XFileDataObjectPtr pData, const string& strTemplateName, const string& strName /*= ""*/)
{
	if (strTemplateName == "XDWORDArray") {

	}
	else if (strTemplateName == "XVertices") {
		WriteXVertices(pData, strName);
	}
	else if (strTemplateName == "XTextures") {
		WriteXTextures(pData, strName);
	}
	else if (strTemplateName == "XAttachments") {
		WriteXAttachments(pData, strName);
	}
	else if (strTemplateName == "XTransparency") {
		WriteXTransparency(pData, strName);
	}
	else if (strTemplateName == "XViews") {
		WriteXViews(pData, strName);
	}
	else if (strTemplateName == "XIndices0") {
		WriteXIndices0(pData, strName);
	}
	else if (strTemplateName == "XGeosets") {
		WriteXGeosets(pData, strName);
	}
	else if (strTemplateName == "XRenderPass") {
		WriteXRenderPass(pData, strName);
	}
	else if (strTemplateName == "XBones") {
		WriteXBones(pData, strName);
	}
	else if (strTemplateName == "XTexAnims") {
		WriteXTexAnims(pData, strName);
	}
	else if (strTemplateName == "XParticleEmitters") {
		WriteXParticleEmitters(pData, strName);
	}
	else if (strTemplateName == "XRibbonEmitters") {
		WriteXRibbonEmitters(pData, strName);
	}
	else if (strTemplateName == "XColors") {
		WriteXColors(pData, strName);
	}
	else if (strTemplateName == "XCameras") {
		WriteXCameras(pData, strName);
	}
	else if (strTemplateName == "XLights")
	{
		WriteXLights(pData, strName);
	}
	else if (strTemplateName == "XAnimations")
	{
		WriteXAnimations(pData, strName);
	}
	else if (strTemplateName == "XVoxels")
	{
		WriteXVoxels(pData, strName);
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
		unsigned char* bytes = (unsigned char*)(pData->GetBuffer() + 4);
		memcpy(pData->GetBuffer() + 4, m_pMesh->globalSequences, nSize - 4);
	}
	return true;
}

bool ParaEngine::XFileCharModelExporter::WriteXVertices(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	pData->m_sTemplateName = "XVertices";
	pData->m_sName = strName;

	int nSize = sizeof(XVerticesDef);
	pData->ResizeBuffer(nSize);

	int nVertices = m_pMesh->m_objNum.nVertices;

	XVerticesDef data;
	data.nType = 0;
	data.nVertexBytes = sizeof(ModelVertex);
	data.nVertices = nVertices;

	data.ofsVertices = m_pRawData->AddRawData((const DWORD*)m_pMesh->m_origVertices, data.nVertices * data.nVertexBytes / 4);

	memcpy(pData->GetBuffer(), &data, nSize);
	return true;
}


bool ParaEngine::XFileCharModelExporter::WriteXVoxels(XFileDataObjectPtr pData, const string& strName)
{
	if (m_pMesh->m_pVoxelModel)
	{
		std::vector<char> output;
		if (m_pMesh->m_pVoxelModel->Save(output))
		{
			pData->m_sTemplateName = "XVoxels";
			pData->m_sName = strName;

			int nSize = sizeof(XVerticesDef);
			pData->ResizeBuffer(nSize);

			XVerticesDef data;
			data.nType = 1; // 1 for XVoxels
			data.nVertexBytes = 1;
			data.nVertices = (DWORD)output.size(); // byte count
			data.ofsVertices = m_pRawData->AddRawData((const char*)(&output[0]), data.nVertices);
			memcpy(pData->GetBuffer(), &data, nSize);
			return true;
		}
	}
	return false;
}

bool ParaEngine::XFileCharModelExporter::WriteXTextures(XFileDataObjectPtr pData, const string& strName /*= ""*/)
{
	struct ModelTextureDef_ {
		uint32 type;
		uint32 nOffsetEmbeddedTexture;
		char sName;
	};
	int nTextures = m_pMesh->m_objNum.nTextures;
	if (nTextures > 0)
	{
		pData->m_sTemplateName = "XTextures";
		pData->m_sName = strName;

		// 255 is the reserved texture file length
		pData->ResizeBuffer((sizeof(ModelTextureDef_) + 255) * nTextures + 4);

		*(int32*)pData->GetBuffer() = nTextures;

		ModelTextureDef_* pBuffer = (ModelTextureDef_*)(pData->GetBuffer() + 4);

		for (int i = 0; i < nTextures; ++i)
		{
			pBuffer->nOffsetEmbeddedTexture = 0;

			if (m_pMesh->textures[i].get() == nullptr) {
				if (m_pMesh->specialTextures[i] < CParaXModel::MAX_MODEL_TEXTURES)
					pBuffer->type = m_pMesh->specialTextures[i];
				else
					pBuffer->type = 0;
				//pBuffer->sName = '\0';
				//pBuffer = (ModelTextureDef_*)((char*)pBuffer + 8 + 1);

				string name = ""; // "Textures/whitedot.png";
				memcpy(&(pBuffer->sName), name.c_str(), name.size() + 1);
				pBuffer = (ModelTextureDef_*)((char*)pBuffer + 8 + name.size() + 1);
			}
			else {
				if (m_pMesh->specialTextures[i] < CParaXModel::MAX_MODEL_TEXTURES)
					pBuffer->type = m_pMesh->specialTextures[i];
				else
					pBuffer->type = 0;
				string name = m_pMesh->textures[i]->GetKey();
				if (m_pMesh->textures[i]->GetRawData())
				{
					DWORD data[1] = { (DWORD)(m_pMesh->textures[i]->GetRawDataSize()) };
					m_pRawData->AddRawData(data, 1);
					pBuffer->nOffsetEmbeddedTexture = m_pRawData->AddRawData((const char*)m_pMesh->textures[i]->GetRawData(), m_pMesh->textures[i]->GetRawDataSize());
					// shall we use a shorter name for embedded texture
					name = CParaFile::GetFileName(name);
				}
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

		int nSize = 8 + sizeof(ModelAttachmentDef) * nAttachments + 4 * nAttachmentLookup;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nAttachments;
		*(((DWORD*)(pData->GetBuffer())) + 1) = (DWORD)nAttachmentLookup;
		ModelAttachmentDef* attachments = (ModelAttachmentDef*)(pData->GetBuffer() + 8);
		int32* attLookup = (int32*)(pData->GetBuffer() + 8 + sizeof(ModelAttachmentDef) * nAttachments);
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

		int nSize = 4 + sizeof(ModelColorDef) * nColors;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nColors;
		ModelColorDef* colorDefs = (ModelColorDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelTransDef) * nTransparency;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nTransparency;
		ModelTransDef* transDefs = (ModelTransDef*)(pData->GetBuffer() + 4);
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
	pData->m_sTemplateName = "XViews";
	pData->m_sName = strName;

	int nView = 1;
	{
		int nSize = 4 + sizeof(ModelView) * nView;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nView;
		ModelView* view = (ModelView*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nView; i++) {
			memset(&view[i], 0, sizeof(ModelView));
			view[i].nTris = (uint32)m_pMesh->GetObjectNum().nIndices; // TODO:rename nTris to nIndices
			view[i].nSub = (uint32)m_pMesh->geosets.size();
			view[i].nTex = (uint32)m_pMesh->passes.size(); // TODO: rename nTex to render pass
		}
	}

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

		int nSize = 4 + sizeof(ModelGeoset) * nGeosets;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nGeosets;
		ModelGeoset* geosets = (ModelGeoset*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelRenderPass) * nRenderPasses;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRenderPasses;
		ModelRenderPass* opsDefs = (ModelRenderPass*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelBoneDef) * nBones;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nBones;
		ModelBoneDef* bones = (ModelBoneDef*)(pData->GetBuffer() + 4);
		for (int i = 0; i < nBones; i++) {
			const Bone& bone = m_pMesh->bones[i];
			bones[i].parent = bone.parent;
			// we will encode string
			bones[i].flags = bone.flags | 0x80000000;
			bones[i].boneid = bone.nBoneID;

			bones[i].nBoneName = (!bone.GetName().empty()) ? m_pRawData->AddRawData(bone.GetName()) : 0;

			if (bone.IsOffsetMatrixBone())
				bones[i].nOffsetMatrix = m_pRawData->AddRawData(&bone.matOffset, 1);
			else
				bones[i].nOffsetMatrix = 0;
			bones[i].nOffsetPivot = m_pRawData->AddRawData(&bone.pivot, 1);

			if (bone.IsStaticTransform())
			{
				bones[i].ofsStaticMatrix = m_pRawData->AddRawData(&bone.matTransform, 1);
			}
			else
			{
				WriteAnimationBlock(&bones[i].translation, bone.trans);
				WriteAnimationBlock(&bones[i].rotation, bone.rot);
				WriteAnimationBlock(&bones[i].scaling, bone.scale);
			}
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

		int nSize = 4 + sizeof(ModelTexAnimDef) * nTexAnims;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nTexAnims;
		ModelTexAnimDef* texanims = (ModelTexAnimDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelParticleEmitterDef) * nParticleEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nParticleEmitters;
		ModelParticleEmitterDef* particleSystems = (ModelParticleEmitterDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelRibbonEmitterDef) * nRibbonEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRibbonEmitters;
		ModelRibbonEmitterDef* ribbons = (ModelRibbonEmitterDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelRibbonEmitterDef) * nRibbonEmitters;
		pData->ResizeBuffer(nSize);
		memset(pData->GetBuffer(), 0, nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nRibbonEmitters;
		ModelRibbonEmitterDef* ribbons = (ModelRibbonEmitterDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelLightDef) * nLights;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nLights;
		ModelLightDef* lights = (ModelLightDef*)(pData->GetBuffer() + 4);
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

		int nSize = 4 + sizeof(ModelAnimation) * nAnimations;
		pData->ResizeBuffer(nSize);
		*(DWORD*)(pData->GetBuffer()) = (DWORD)nAnimations;
		ModelAnimation* anims = (ModelAnimation*)(pData->GetBuffer() + 4);
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
