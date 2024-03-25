//-----------------------------------------------------------------------------
// Class:	CParaXSerializer
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.1.5
// Note: serialization functions for ParaX model.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/particle.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/ParaVoxelModel.h"
#include "ParaXModel/XFileCharModelExporter.h"

#include "ParaXSerializer.h"
#ifdef USE_DIRECTX_RENDERER
// for ParaEngine x file template registration
#include <rmxfguid.h>
#include "mdxfile/ParaXFileGUID.h"
#include <algorithm>
/** define this to enable testing saving. See SaveParaXMesh() 
* text encoding is enforced when the macro is on. */
// #define TEST_NODE
#endif
#include "ParaXModel/XFileCharModelParser.h"

namespace ParaEngine
{
	extern float frand();
	SerializerOptions CParaXSerializer::g_pDefaultOption;

	void CParaXSerializer::ExportParaXMesh(const string& filePath, CParaXModel* pMesh)
	{
		XFileCharModelExporter::Export(filePath, pMesh);
	}
}

using namespace ParaEngine;

#ifdef USE_DIRECTX_RENDERER
//////////////////////////////////////////////////////////////////////////
/// ParaXParser															//
//////////////////////////////////////////////////////////////////////////
ParaXParser::ParaXParser(CParaFile& file, LPD3DXFILE	pDXFileParser)
: m_file(file), m_bHeaderLoaded(false), m_bIsValid(true), m_pDXEnum(NULL), m_pParaXBody(NULL), m_pParaXRawData(NULL),
m_pParaXRef(NULL), m_pD3DMesh(NULL), m_pD3DRootFrame(NULL)
{
	m_pDXFileParser = (pDXFileParser != 0) ? pDXFileParser : CGlobals::GetAssetManager()->GetParaXFileParser();
}

void ParaXParser::Finalize()
{
	// release objects
	SAFE_RELEASE(m_pParaXBody);
	SAFE_RELEASE(m_pParaXRawData);
	SAFE_RELEASE(m_pDXEnum);

	SAFE_RELEASE(m_pParaXRef);
	SAFE_RELEASE(m_pD3DMesh);
	SAFE_RELEASE(m_pD3DRootFrame);
}
#endif
//////////////////////////////////////////////////////////////////////////
/// CParaXSerializer													//
//////////////////////////////////////////////////////////////////////////

CParaXSerializer::CParaXSerializer(void)
:m_pRaw(NULL)
{
}

CParaXSerializer::~CParaXSerializer(void)
{
}

const std::string& CParaXSerializer::GetFilename() const
{
	return m_sFilename;
}

void CParaXSerializer::SetFilename(std::string val)
{
	m_sFilename = val;
}

void* CParaXSerializer::LoadParaXMesh(CParaFile &f)
{
	void* pMesh=NULL;
	
#if defined(USE_DIRECTX_RENDERER) && !defined(_DEBUG)
	{
		bool bIsStaticModel = false;
		ParaXParser p(f);
		if (LoadParaX_Header(p)) {
			pMesh = LoadParaX_Body(p);
			if (!pMesh && p.m_pD3DMesh)
			{
				bIsStaticModel = true;
			}
			LoadParaX_Finalize(p);
		}
		if(!bIsStaticModel)
			return pMesh;
	}
 #endif
	
	try
	{
		XFileCharModelParser p(f.getBuffer(), f.getSize());
		p.SetFilename(GetFilename());
		pMesh = p.ParseParaXModel();
	}
	catch (runtime_error* e)
	{
		OUTPUT_LOG("warn: LoadParaXMesh error:%s\n", e->what());
	}
	return pMesh;
}
#ifdef USE_DIRECTX_RENDERER
void* CParaXSerializer::LoadParaXMesh(CParaFile &f, ParaXParser& p)
{
	void* pMesh=NULL;
	if(LoadParaX_Header(p)){
		pMesh = LoadParaX_Body(p);
		LoadParaX_Finalize(p);
	}
	return pMesh;
}

void CParaXSerializer::LoadParaX_Finalize(ParaXParser& Parser)
{
	Parser.Finalize();
}

bool CParaXSerializer::LoadParaX_Header(ParaXParser& Parser)
{
	if(!Parser.m_bIsValid)
		return false;
	// if header already loaded, return 
	if(Parser.m_bHeaderLoaded)
		return true;
	if(Parser.GetFileParser() == NULL){
		Parser.m_bIsValid = false;
		return false;
	}
	D3DXF_FILELOADMEMORY memData;
	memData.dSize = (int)Parser.m_file.getSize();
	memData.lpMemory =  Parser.m_file.getBuffer();
	if(memData.lpMemory==0)
	{
		Parser.m_bIsValid = false;
		return false;
	}

	HRESULT res=Parser.GetFileParser()->CreateEnumObject(&memData, D3DXF_FILELOAD_FROMMEMORY, &Parser.m_pDXEnum);
	if(FAILED(res))
	{
		if(res == D3DXFERR_BADTYPE)
			OUTPUT_LOG("warning: failed reading ParaX header. CreateEnumObject return D3DXFERR_BADTYPE\n");
		else if(res == D3DXFERR_PARSEERROR)
			OUTPUT_LOG("warning: failed reading ParaX header. CreateEnumObject return D3DXFERR_PARSEERROR\n");
		Parser.m_bIsValid = false;
		return false;
	}

	LPFileData pData=NULL;
	SIZE_T nCount;
	Parser.m_pDXEnum->GetChildren(&nCount);
	for(int i = 0; i<(int)nCount;i++)
	{
		Parser.m_pDXEnum->GetChild(i, &pData);
		GUID Type;
		// Get the template type
		if(FAILED(pData->GetType(&Type)))
			break;
		if(Type == TID_ParaXHeader)
		{
			Parser.m_bHeaderLoaded = ReadParaXHeader(Parser.m_xheader, pData);
			if(!Parser.m_bHeaderLoaded)
			{
				Parser.m_xheader.type = (DWORD)PARAX_MODEL_INVALID;
				break;
			}
			SAFE_RELEASE(pData);
		}
		else if(Type == TID_ParaXBody)
			Parser.m_pParaXBody = pData;
		else if(Type == TID_XDWORDArray)
			Parser.m_pParaXRawData = pData;
		else if(Type == TID_ParaXRefSection)
			Parser.m_pParaXRef = pData;
		else if(Type == TID_D3DRMMesh)
			Parser.m_pD3DMesh = pData;
		else if(Type == TID_D3DRMFrame)
			Parser.m_pD3DRootFrame = pData;
		else
		{
			SAFE_RELEASE(pData);
			
			if(i==0)
			{
				// this is not a ParaX file. since it does not have the ParaX header at the beginning.
				Parser.m_xheader.type = (DWORD)PARAX_MODEL_DX_STATIC;
			}
		}
	}
	
	return true;
}

bool CParaXSerializer::WriteXRawBytes(const CParaXModel& xmesh, LPFileSaveObject pFileData)
{
	int nCount = m_rawdata.GetSize();
	// save as DWORD, so 
	int nDWORD = nCount/4+1;
	if(nDWORD>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+nDWORD*4;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nDWORD;
		unsigned char *bytes = (unsigned char*)(pData+4);
		memcpy(bytes, m_rawdata.GetBuffer(), nCount);

		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XDWORDArray, "ParaXRawData", NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

bool CParaXSerializer::WriteAnimationBlock(AnimationBlock* b, const Animated<Vector3>& anims)
{
	if(b==0) return false;
	memset(b,0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if(anims.ranges.size()==1 && anims.seq==-1 && anims.type!=0)
	{
		b->nRanges=0;
		b->ofsRanges=0;
	}
	else
		b->nRanges=(uint32)anims.ranges.size();
	if(b->nRanges>0)
		b->ofsRanges = m_rawdata.AddRawData( (const Vector2*)(&anims.ranges[0]), b->nRanges);
	if(b->nTimes>0)
		b->ofsTimes = m_rawdata.AddRawData( (const DWORD*)(&anims.times[0]), b->nTimes);
	if(b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<Vector3> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		b->ofsKeys = m_rawdata.AddRawData( (const Vector3*)(&data[0]), b->nKeys*3);
	}
	else
		b->ofsKeys = m_rawdata.AddRawData( (const Vector3*)(&anims.data[0]), b->nKeys);
	return true;
}

bool CParaXSerializer::WriteAnimationBlock(AnimationBlock* b, const AnimatedShort& anims)
{
	if(b==0) return false;
	memset(b,0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if(anims.ranges.size()==1 && anims.seq==-1 && anims.type!=0)
	{
		b->nRanges=0;
		b->ofsRanges=0;
	}
	else
		b->nRanges=(uint32)anims.ranges.size();
	if(b->nRanges>0)
		b->ofsRanges = m_rawdata.AddRawData( (const Vector2*)(&anims.ranges[0]), b->nRanges);
	if(b->nTimes>0)
		b->ofsTimes = m_rawdata.AddRawData( (const DWORD*)(&anims.times[0]), b->nTimes);
	if(b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<float> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		b->ofsKeys = m_rawdata.AddRawData( (const DWORD*)(&data[0]), b->nKeys*3);
	}
	else 
		b->ofsKeys = m_rawdata.AddRawData( (const DWORD*)(&anims.data[0]), b->nKeys);
	return true;
}
bool CParaXSerializer::WriteAnimationBlock(AnimationBlock* b, const Animated<float>& anims)
{
	if(b==0) return false;
	memset(b,0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if(anims.ranges.size()==1 && anims.seq==-1 && anims.type!=0)
	{
		b->nRanges=0;
		b->ofsRanges=0;
	}
	else
		b->nRanges=(uint32)anims.ranges.size();
	if(b->nRanges>0)
		b->ofsRanges = m_rawdata.AddRawData( (const Vector2*)(&anims.ranges[0]), b->nRanges);
	if(b->nTimes>0)
		b->ofsTimes = m_rawdata.AddRawData( (const DWORD*)(&anims.times[0]), b->nTimes);
	if(b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<float> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		b->ofsKeys = m_rawdata.AddRawData( (const DWORD*)(&data[0]), b->nKeys*3);
	}
	else
		b->ofsKeys = m_rawdata.AddRawData( (const DWORD*)(&anims.data[0]), b->nKeys);
	return true;
}
bool CParaXSerializer::WriteAnimationBlock(AnimationBlock* b, const Animated<Quaternion>& anims)
{
	if(b==0) return false;
	memset(b,0, sizeof(AnimationBlock));
	b->type = anims.type;
	b->seq = anims.seq;
	b->nTimes = b->nKeys = (uint32)anims.times.size();
	if(anims.ranges.size()==1 && anims.seq==-1 && anims.type!=0)
	{
		b->nRanges=0;
		b->ofsRanges=0;
	}
	else
		b->nRanges=(uint32)anims.ranges.size();
	if(b->nRanges>0)
		b->ofsRanges = m_rawdata.AddRawData( (const Vector2*)(&anims.ranges[0]), b->nRanges);
	if(b->nTimes>0)
		b->ofsTimes = m_rawdata.AddRawData( (const DWORD*)(&anims.times[0]), b->nTimes);
	if(b->type == INTERPOLATION_HERMITE)
	{
		// not tested
		vector<Quaternion> data;
		data.insert(data.end(), anims.data.begin(), anims.data.end());
		data.insert(data.end(), anims.in.begin(), anims.in.end());
		data.insert(data.end(), anims.out.begin(), anims.out.end());
		b->ofsKeys = m_rawdata.AddRawData( (const Vector4*)(&data[0]), b->nKeys*3);
	}
	else 
		b->ofsKeys = m_rawdata.AddRawData( (const Vector4*)(&anims.data[0]), b->nKeys);
	return true;
}

#define DEFINE_ReadAnimationBlock(DECL_TYPE, TYPE) \
bool CParaXSerializer::ReadAnimationBlock(const AnimationBlock* b, DECL_TYPE& anims,int *gs)\
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

bool CParaXSerializer::SaveParaXMesh(const string& filename, const CParaXModel& xmesh, SerializerOptions* pOptions)
{
	/// Save animation file ParaEngine X file format
	LPFileSaveObject pDXSaveObj = NULL;
	ID3DXFileData       *pDXData = NULL;

	LPD3DXFILE pDXFile = CGlobals::GetAssetManager()->GetParaXFileParser();

	if(pDXFile == NULL)
		return false;
	if(pOptions==NULL)
		pOptions = &g_pDefaultOption;

#ifdef TEST_NODE
	pOptions->m_bBinary = false;
#endif

	D3DXF_FILEFORMAT dwFileFormat = pOptions->m_bBinary? D3DXF_FILEFORMAT_BINARY:D3DXF_FILEFORMAT_TEXT;
	dwFileFormat |= pOptions->m_bCompressed? D3DXF_FILEFORMAT_COMPRESSED:0;

	if(FAILED(pDXFile->CreateSaveObject(filename.c_str(), D3DXF_FILESAVE_TOFILE, dwFileFormat, &pDXSaveObj)))
	{
		OUTPUT_LOG("failed saving ParaX file %s", filename.c_str());
		return false;
	}

	/// top level: ParaXHeader node
	if(!WriteParaXHeader(xmesh, pDXSaveObj))
		return false;

	/// top level: ParaXBody node
	LPFileSaveData pFileData=0;
	if(SUCCEEDED(pDXSaveObj->AddDataObject(TID_ParaXBody, NULL, NULL, 0, NULL, &pFileData)))
	{

#ifndef TEST_NODE
		/// global sequences must be serialized first.
		if(!(WriteXGlobalSequences(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X global sequences");
		/// the following can be serialized in any order.
		if(!(WriteXViews(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X views");
		if(!(WriteXTextures(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X textures");
		if(!(WriteXAttachments(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X attachments");
		if(!(WriteXColors(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X colors");
		if(!(WriteXTransparency(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X transparency");
		if(!(WriteXVertices(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X vertices");
		if(!(WriteXIndices0(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X indices0");
		if(!(WriteXGeosets(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X geosets for 0 view");
		if(!(WriteXRenderPass(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X render passes for 0 view");
		if(!(WriteXBones(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X bones");
		if(!(WriteXCameras(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X cameras");
		if(!(WriteXAnimations(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X animations");
#endif
		
		// TODO: the following nodes are not tested
		if(!(WriteXTexAnims(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X texture animations");
		if(!(WriteXParticleEmitters(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X particle systems");
		if(!(WriteXRibbonEmitters(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X ribbon emitters");
		if(!(WriteXLights(xmesh,pFileData)))
			OUTPUT_LOG("failed exporting X lights");

		SAFE_RELEASE(pFileData);
	}
	else
		return false;

#ifndef TEST_NODE
	/// raw bytes must be serialized last.
	if(!(WriteXRawBytes(xmesh,pDXSaveObj)))
		OUTPUT_LOG("failed exporting raw bytes");
#endif

	// save to disk, 
	if(FAILED(pDXSaveObj->Save()))
	{
		OUTPUT_LOG("failed saving ParaX file %s", filename.c_str());
		return false;
	}
	
	SAFE_RELEASE(pDXSaveObj);

	return true;
}

bool CParaXSerializer::WriteParaXHeader(const CParaXModel& xmesh, LPFileSaveObject pFileData)
{
	ParaXHeaderDef xheader;
	memcpy(&xheader, &xmesh.GetHeader(), sizeof(ParaXHeaderDef));
	
	LPFileSaveData pDataNode;
	if(FAILED(pFileData->AddDataObject(TID_ParaXHeader, NULL, NULL, sizeof(ParaXHeaderDef), &xheader, &pDataNode)))
		return false;
	SAFE_RELEASE(pDataNode);
	return true;
}

bool CParaXSerializer::ReadParaXHeader(ParaXHeaderDef& xheader, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		memcpy(&xheader, pBuffer, sizeof(ParaXHeaderDef));
		pFileData->Unlock();
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXGlobalSequences(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nGlobalSequences = xmesh.GetObjectNum().nGlobalSequences;
	if(nGlobalSequences>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+nGlobalSequences*sizeof(DWORD);
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nGlobalSequences;
		unsigned char *bytes = (unsigned char*)(pData+4);
		memcpy(bytes, xmesh.globalSequences, nSize-4);

		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XDWORDArray, "XGlobalSequences", NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXGlobalSequences(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		xmesh.m_objNum.nGlobalSequences = *(DWORD*)(pBuffer);
		xmesh.globalSequences = new int[xmesh.m_objNum.nGlobalSequences];
		if(xmesh.globalSequences)
			memcpy(xmesh.globalSequences, pBuffer+4, xmesh.m_objNum.nGlobalSequences*sizeof(DWORD));
		else
			return false;
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXVertices(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	if((xmesh.m_RenderMethod == CParaXModel::SOFT_ANIM || xmesh.m_RenderMethod == CParaXModel::NO_ANIM) && xmesh.m_origVertices)
	{
		XVerticesDef data;
		data.nType = 0; 
		data.nVertexBytes = sizeof(ModelVertex);
		data.nVertices = xmesh.GetObjectNum().nVertices;
		data.ofsVertices = m_rawdata.AddRawData((const DWORD*)xmesh.m_origVertices, data.nVertices*data.nVertexBytes/4);
		
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XVertices, NULL, NULL, sizeof(XVerticesDef), &data, &pDataNode)))
			return false;
		SAFE_RELEASE(pDataNode);
	}
	else
	{
		// TODO: from vertex buffer
	}
	return true;
}
bool CParaXSerializer::ReadXVertices(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		XVerticesDef* pXVert = (XVerticesDef*)pBuffer;
		xmesh.initVertices( pXVert->nVertices, (ModelVertex*)(GetRawData(pXVert->ofsVertices)));
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::ReadXVoxels(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char* pBuffer = NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		XVerticesDef* pXVert = (XVerticesDef*)pBuffer;
		auto pVoxelMesh = xmesh.CreateGetVoxelModel();
		if (pVoxelMesh)
		{
			pVoxelMesh->Load((const char*)GetRawData(pXVert->ofsVertices), pXVert->nVertices);
		}
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXTextures(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nTextures = xmesh.GetObjectNum().nTextures;
	if(nTextures)
	{
		int nSize = 500*nTextures;
		unsigned char* pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = nTextures;
		nSize = 4;
		struct ModelTextureDef_
		{
			uint32 type;
			uint32 flags;
			char sName;
		};
		for (int i=0; i<nTextures; i++) {
			ModelTextureDef_* pTexDef = (ModelTextureDef_*)(pData+nSize);
			
			pTexDef->flags = 0; // unused
			if (xmesh.textures[i].get() == 0) {
				pTexDef->type = xmesh.specialTextures[i];
				pTexDef->sName = '\0';
				nSize += 8+1;
			} else {
				pTexDef->type = 0;
				int nameLen = (uint32)xmesh.textures[i]->GetKey().size();
				strcpy(&pTexDef->sName, xmesh.textures[i]->GetKey().c_str());
				nSize += 8 + nameLen + 1;
			}
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XTextures, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

bool CParaXSerializer::ReadXTextures(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nTextures = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nTextures = nTextures;
		struct ModelTextureDef_
		{
			uint32 type;
			uint32 nOffsetEmbeddedTexture;
			char sName;
		};
		if(nTextures>0)
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
				string sFilename(((const char*)pTex) + 8); // for safety.
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

bool CParaXSerializer::WriteXAttachments(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nAttachments = xmesh.GetObjectNum().nAttachments;
	int nAttachmentLookup = xmesh.GetObjectNum().nAttachLookup;
	if(nAttachments>0 || nAttachmentLookup>0 )
	{
		unsigned char* pData = 0;
		int nSize = 8+sizeof(ModelAttachmentDef)*nAttachments+4*nAttachmentLookup;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nAttachments;
		*(((DWORD*)(pData))+1) = (DWORD)nAttachmentLookup;
		ModelAttachmentDef *attachments = (ModelAttachmentDef *)(pData+8);
		int32 * attLookup = (int32 *)(pData+8+sizeof(ModelAttachmentDef)*nAttachments);
		for (int i=0; i<nAttachments; i++) {
			const ModelAttachment& att = xmesh.m_atts[i];
			attachments[i].id = att.id;
			attachments[i].bone = att.bone;
			attachments[i].pos = att.pos;
			// TODO: this member is not used.
			memset(&attachments[i].unk, 0 , sizeof(AnimationBlock));
		}
		if(nAttachmentLookup>0)
			memcpy(attLookup, xmesh.m_attLookup, 4*nAttachmentLookup);

		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XAttachments, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}


bool CParaXSerializer::ReadXAttachments(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nAttachments = *(DWORD*)(pBuffer);
		int nAttachmentLookup = *(((DWORD*)(pBuffer))+1);
		xmesh.m_objNum.nAttachments = nAttachments;
		xmesh.m_objNum.nAttachLookup = nAttachmentLookup;
		
		ModelAttachmentDef *attachments = (ModelAttachmentDef *)(pBuffer+8);
		int32 * attLookup = (int32 *)(pBuffer+8+sizeof(ModelAttachmentDef)*nAttachments);

		// attachments
		xmesh.m_atts.reserve(nAttachments);
		for (int i=0; i<nAttachments; ++i) {
			ModelAttachment att;
			const ModelAttachmentDef& mad = attachments[i];
			att.pos = mad.pos;
			att.bone = mad.bone;
			att.id = mad.id;
			xmesh.m_atts.push_back(att);
		}
		// attachment lookups
		if(nAttachmentLookup>0){
			PE_ASSERT(nAttachmentLookup<=40);
			memcpy(xmesh.m_attLookup, attLookup, 4*nAttachmentLookup);
		}
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXColors(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nColors = xmesh.GetObjectNum().nColors;
	if(nColors>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelColorDef)*nColors;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nColors;
		ModelColorDef *colorDefs = (ModelColorDef*)(pData+4);
		for (int i=0; i<nColors; i++) {
			const ModelColor& att = xmesh.colors[i];
			WriteAnimationBlock(&colorDefs[i].color, att.color);
			WriteAnimationBlock(&colorDefs[i].opacity, att.opacity);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XColors, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXColors(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if (SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nColors = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nColors = nColors;
		if(nColors>0)
		{ // at least one Bone
			ModelColorDef *colorDefs = (ModelColorDef*)(pBuffer+4);
			xmesh.colors = new ModelColor[nColors];
			for (int i=0;i<nColors;++i) 
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

bool CParaXSerializer::WriteXTransparency(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nTransparency = xmesh.GetObjectNum().nTransparency;
	if(nTransparency>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelTransDef)*nTransparency;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nTransparency;
		ModelTransDef *transDefs = (ModelTransDef*)(pData+4);
		for (int i=0; i<nTransparency; i++) {
			const ModelTransparency& transp = xmesh.transparency[i];
			WriteAnimationBlock(&transDefs[i].trans, transp.trans);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XTransparency, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

bool CParaXSerializer::ReadXTransparency(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nTransparency = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nTransparency = nTransparency;
		if(nTransparency>0)
		{ // at least one item
			ModelTransDef *transDefs = (ModelTransDef*)(pBuffer+4);
			xmesh.transparency = new ModelTransparency[nTransparency];
			for (int i=0;i<nTransparency;++i) 
			{
				ReadAnimationBlock(&transDefs[i].trans, xmesh.transparency[i].trans, xmesh.globalSequences);
			}
		}
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXViews(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	// for each view: 
	// there is only one view
	int nView = 1;
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelView)*nView;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nView;
		ModelView* view = (ModelView*)(pData+4);
		for (int i=0; i<nView; i++) {
			memset(&view[i], 0 , sizeof(ModelView));
			view[i].nTris = (uint32)xmesh.GetObjectNum().nIndices; // TODO:rename nTris to nIndices
			view[i].nSub = (uint32)xmesh.geosets.size();
			view[i].nTex = (uint32)xmesh.passes.size(); // TODO: rename nTex to render pass
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XViews, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXViews(CParaXModel& xmesh, LPFileData pFileData)
{
	// no need to do anything, since there is only one view. all view 0 information are duplicated in other nodes.
	return true;
}

bool CParaXSerializer::WriteXIndices0(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	if((xmesh.m_RenderMethod == CParaXModel::SOFT_ANIM || xmesh.m_RenderMethod == CParaXModel::NO_ANIM) && xmesh.m_indices)
	{
		int nIndices = (int)xmesh.GetObjectNum().nIndices;
		{
			
			Indice0Def indices0;
			indices0.nIndices = nIndices;
			indices0.ofsIndices = m_rawdata.AddRawData((const short*)xmesh.m_indices, nIndices);
			LPFileSaveData pDataNode;
			if(FAILED(pFileData->AddDataObject(TID_XIndices0, NULL, NULL, sizeof(Indice0Def), &indices0, &pDataNode)))
				return false;
			SAFE_RELEASE(pDataNode);
		}
	}
	else 
	{
		//TODO: read from index buffer.
	}
	return true;
}
bool CParaXSerializer::ReadXIndices0(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		Indice0Def* pIndices = (Indice0Def*)pBuffer;
		xmesh.initIndices( pIndices->nIndices, (uint16*)(GetRawData(pIndices->ofsIndices)));
	}
	else
		return false;
	return true;
}

bool CParaXSerializer::WriteXGeosets(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nGeosets = (int)xmesh.geosets.size();
	if(nGeosets>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelGeoset)*nGeosets;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nGeosets;
		ModelGeoset *geosets = (ModelGeoset*)(pData+4);
		memcpy(geosets, &xmesh.geosets[0], nSize-4);
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XGeosets, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXGeosets(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nGeosets = *(DWORD*)(pBuffer);
		ModelGeoset* pGeosets = (ModelGeoset*)(pBuffer+4);
		xmesh.showGeosets = new bool[nGeosets];
		for(int i=0;i<nGeosets;++i)
			xmesh.showGeosets[i] = true;

		xmesh.geosets.resize(nGeosets);
		if (nGeosets > 0) {
			memcpy(&xmesh.geosets[0], pGeosets, sizeof(ModelGeoset)*nGeosets);
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

bool CParaXSerializer::WriteXRenderPass(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nRenderPasses = (int)xmesh.passes.size();
	if(nRenderPasses>0 )
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelRenderPass)*nRenderPasses;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nRenderPasses;
		ModelRenderPass *opsDefs = (ModelRenderPass*)(pData+4);
		memcpy(opsDefs, &xmesh.passes[0], nSize-4);
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XRenderPass, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXRenderPass(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nRenderPasses = *(DWORD*)(pBuffer);
		ModelRenderPass* passes = (ModelRenderPass*)(pBuffer+4);
		xmesh.passes.resize(nRenderPasses);
		if(nRenderPasses>0)
		{
			memcpy(&xmesh.passes[0],passes, sizeof(ModelRenderPass)*nRenderPasses);

			// for opaque faces, always enable culling.
			for (int i=0;i<nRenderPasses;++i)
			{
				ModelRenderPass& pass = xmesh.passes[i];
				if(pass.blendmode == BM_OPAQUE)
				{
					pass.cull = true;
				}
				/** fix 2009.1.14 by LiXizhi, use 32 bits index offset */
				if(pass.indexStart != 0xffff)
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

bool CParaXSerializer::WriteXBones(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nBones = xmesh.GetObjectNum().nBones;
	if(nBones>0 && xmesh.animBones)
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelBoneDef)*nBones;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		memset(pData, 0, nSize);
		*(DWORD*)(pData) = (DWORD)nBones;
		ModelBoneDef *bones = (ModelBoneDef*)(pData+4);
		for (int i=0; i<nBones; i++) {
			const Bone& bone = xmesh.bones[i];
			bones[i].parent = bone.parent;
			bones[i].pivot = bone.pivot;
			bones[i].flags = bone.flags;
			bones[i].boneid = bone.nBoneID;
			WriteAnimationBlock(&bones[i].translation, bone.trans);
			WriteAnimationBlock(&bones[i].rotation, bone.rot);
			WriteAnimationBlock(&bones[i].scaling, bone.scale);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XBones, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXBones(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nBones = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nBones = nBones;
		if(nBones>0)
		{ // at least one Bone
			xmesh.bones = new Bone[nBones];
			ModelBoneDef *mb = (ModelBoneDef*)(pBuffer+4);
			for (int i=0;i<nBones;++i) 
			{
				Bone& bone = xmesh.bones[i];
				const ModelBoneDef&b = mb[i];
				bone.parent = b.parent;
				bone.flags = b.flags;
				bone.nIndex = i;
				
				if(b.boneid>0 && b.boneid<MAX_KNOWN_BONE_NODE)
				{
					xmesh.m_boneLookup[b.boneid] = i;
					bone.nBoneID = b.boneid;
				}
				if ((bone.flags & 0x80000000) != 0)
				{
					bone.flags = bone.flags & (~0x80000000);
					if (b.nBoneName != 0)
						bone.SetName((const char*)GetRawData(b.nBoneName));

					if (bone.IsOffsetMatrixBone()) {
						bone.matOffset = *((const Matrix4*)GetRawData(b.nOffsetMatrix));
						bone.bUsePivot = false;
					}

					bone.pivot = *((const Vector3*)GetRawData(b.nOffsetPivot));
					if (bone.IsStaticTransform())
						bone.matTransform = *((const Matrix4*)GetRawData(b.ofsStaticMatrix));
				}
				else
				{
					bone.pivot = b.pivot;
				}
				
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

bool CParaXSerializer::WriteXTexAnims(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nTexAnims = xmesh.GetObjectNum().nTexAnims;
	if(nTexAnims>0 && xmesh.animTextures)
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelTexAnimDef)*nTexAnims;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nTexAnims;
		ModelTexAnimDef *texanims = (ModelTexAnimDef*)(pData+4);
		for (int i=0; i<nTexAnims; i++) {
			const TextureAnim& TexAnim = xmesh.texanims[i];
			WriteAnimationBlock(&texanims[i].trans, TexAnim.trans);
			WriteAnimationBlock(&texanims[i].rot, TexAnim.rot);
			WriteAnimationBlock(&texanims[i].scale, TexAnim.scale);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XTexAnims, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}
bool CParaXSerializer::ReadXTexAnims(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nTexAnims = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nTexAnims = nTexAnims;
		if(nTexAnims>0)
		{ // at least one Bone
			ModelTexAnimDef *texanims = (ModelTexAnimDef*)(pBuffer+4);
			xmesh.texanims = new TextureAnim[nTexAnims];
			for (int i=0;i<nTexAnims;++i) 
			{
				TextureAnim& TexAnim = xmesh.texanims[i];
				const ModelTexAnimDef &texanim = texanims[i];
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

bool CParaXSerializer::WriteXParticleEmitters(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nParticleEmitters = xmesh.GetObjectNum().nParticleEmitters;
	if(nParticleEmitters>0)
	{		
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelParticleEmitterDef)*nParticleEmitters;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		memset(pData, 0, nSize);
		*(DWORD*)(pData) = (DWORD)nParticleEmitters;
		ModelParticleEmitterDef *particleSystems = (ModelParticleEmitterDef*)(pData+4);
		for (int i=0; i<nParticleEmitters; i++) {
			const ParticleSystem& particleSystem = xmesh.particleSystems[i];
			
			for (int k=0; k<3; k++) {
				particleSystems[i].p.colors[k] = (DWORD)LinearColor((float*)&particleSystem.colors[k]) ;
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
			particleSystems[i].type = particleSystem.emitter ? particleSystem.emitter->GetEmitterType():ParticleEmitter::TYPE_NONE;

			if(!particleSystem.billboard)
				particleSystems[i].flags |= 4096;
			else
				particleSystems[i].flags = 0;
			
			for(uint32 m=0;m<xmesh.GetObjectNum().nBones;++m){
				if((xmesh.bones+m) == particleSystem.parent){
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
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XParticleEmitters, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

//TODO: untested
bool CParaXSerializer::ReadXParticleEmitters(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nParticleEmitters = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nParticleEmitters = nParticleEmitters;
		if(nParticleEmitters>0)
		{ // at least one item
			ModelParticleEmitterDef *particleSystems = (ModelParticleEmitterDef*)(pBuffer+4);
			xmesh.particleSystems = new ParticleSystem[nParticleEmitters];
			for (int i=0;i<nParticleEmitters;++i) 
			{
				ParticleSystem& ps = xmesh.particleSystems[i];
				const ModelParticleEmitterDef & PSDef =  particleSystems[i];
				ps.model = &xmesh;

				if(xmesh.rotatePartice2SpeedVector)
					ps.rotate2SpeedDirection = true;

				for (size_t i=0; i<3; i++) {
					ps.colors[i] = reinterpret_cast<const Vector4&>(LinearColor(Color(PSDef.p.colors[i])));
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

bool CParaXSerializer::WriteXRibbonEmitters(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nRibbonEmitters = xmesh.GetObjectNum().nRibbonEmitters;
	if(nRibbonEmitters>0)
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelRibbonEmitterDef)*nRibbonEmitters;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		memset(pData, 0, nSize);
		*(DWORD*)(pData) = (DWORD)nRibbonEmitters;
		ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(pData+4);
		for (int i=0; i<nRibbonEmitters; i++) {
			const RibbonEmitter& ribbon = xmesh.ribbons[i];

			ribbons[i].pos = ribbon.pos;
			ribbons[i].res = (float)ribbon.numsegs;
			ribbons[i].length = ribbon.seglen;

			for(uint32 m=0;m<xmesh.GetObjectNum().nBones;++m){
				if((xmesh.bones+m) == ribbon.parent){
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
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XRibbonEmitters, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

//TODO: untested
bool CParaXSerializer::ReadXRibbonEmitters(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nRibbonEmitters = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nRibbonEmitters = nRibbonEmitters;
		if(nRibbonEmitters>0)
		{ // at least one item
			ModelRibbonEmitterDef *ribbons = (ModelRibbonEmitterDef*)(pBuffer+4);
			xmesh.ribbons = new RibbonEmitter[nRibbonEmitters];

			for (int i=0;i<nRibbonEmitters;++i) 
			{
				RibbonEmitter& emitter = xmesh.ribbons[i];
				const ModelRibbonEmitterDef & emitterDef =  ribbons[i];
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

bool CParaXSerializer::WriteXCameras(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nCameras = xmesh.GetObjectNum().nCameras;
	if(nCameras>0)
	{
		// we export just one camera
		nCameras = 1;
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelCameraDef)*nCameras;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		memset(pData, 0, nSize);
		*(DWORD*)(pData) = (DWORD)nCameras;
		ModelCameraDef *cameras = (ModelCameraDef*)(pData+4);
		for (int i=0; i<nCameras; i++) {
			const ModelCamera& camera = xmesh.cam; // just one camera
			cameras[i].nearclip = camera.nearclip;
			cameras[i].farclip = camera.farclip;
			cameras[i].fov = camera.fov;
			cameras[i].pos = camera.pos;
			cameras[i].target = camera.target;
			cameras[i].nearclip = camera.nearclip;
			WriteAnimationBlock(&cameras[i].transPos, camera.tPos);
			WriteAnimationBlock(&cameras[i].transTarget, camera.tTarget);
			WriteAnimationBlock(&cameras[i].rot, camera.rot);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XCameras, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

bool CParaXSerializer::ReadXCameras(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nCameras = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nCameras = nCameras;
		if(nCameras>0)
		{ // at least one item
			ModelCameraDef *cameras = (ModelCameraDef*)(pBuffer+4);
			int i=0;
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

bool CParaXSerializer::WriteXLights(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nLights = xmesh.GetObjectNum().nLights;
	if(nLights>0)
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelLightDef)*nLights;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nLights;
		ModelLightDef *lights = (ModelLightDef*)(pData+4);
		for (int i=0; i<nLights; i++) {
			const ModelLight& light = xmesh.lights[i]; 
			lights[i].pos = light.pos;
			lights[i].type = light.type;
			lights[i].bone = light.parent;
			WriteAnimationBlock(&lights[i].ambColor, light.ambColor);
			WriteAnimationBlock(&lights[i].ambIntensity, light.ambIntensity);
			WriteAnimationBlock(&lights[i].color, light.diffColor);
			WriteAnimationBlock(&lights[i].intensity, light.diffIntensity);
		}
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XLights, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

// not tested.
bool CParaXSerializer::ReadXLights(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		int nLights = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nLights = nLights;
		if(nLights>0)
		{ // at least one item
			ModelLightDef *lights = (ModelLightDef*)(pBuffer+4);
			xmesh.lights = new ModelLight[nLights];
			for (int i=0;i<nLights;++i) 
			{
				ModelLight& light = xmesh.lights[i]; 
				const ModelLightDef & lightDef = lights[i];

				light.pos = lightDef.pos;
				light.type = lightDef.type;
				light.parent = lightDef.bone;
				light.tdir = light.dir = Vector3(0,1,0);

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

bool CParaXSerializer::WriteXAnimations(const CParaXModel& xmesh, LPFileSaveData pFileData)
{
	int nAnimations = (int)xmesh.GetObjectNum().nAnimations;
	if(nAnimations>0 && xmesh.anims && xmesh.animated)
	{
		unsigned char* pData = 0;
		int nSize = 4+sizeof(ModelAnimation)*nAnimations;
		pData = new unsigned char[nSize];
		if(pData==0)
			return false;
		*(DWORD*)(pData) = (DWORD)nAnimations;
		ModelAnimation *anims = (ModelAnimation*)(pData+4);
		memcpy(anims, xmesh.anims, nSize-4);
		LPFileSaveData pDataNode;
		if(FAILED(pFileData->AddDataObject(TID_XAnimations, NULL, NULL, nSize, pData, &pDataNode)))
		{
			delete [] pData;
			return false;
		}
		SAFE_RELEASE(pDataNode);

		delete [] pData;
	}
	return true;
}

bool CParaXSerializer::ReadXAnimations(CParaXModel& xmesh, LPFileData pFileData)
{
	SIZE_T       dwSize;
	const char       *pBuffer=NULL;
	// Get the template data
	if(SUCCEEDED(pFileData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
	{
		uint32 nAnimations = *(DWORD*)(pBuffer);
		xmesh.m_objNum.nAnimations = nAnimations;

		ModelAnimation *anims = (ModelAnimation *)(pBuffer+4);
		xmesh.anims = new ModelAnimation[nAnimations];
		if(xmesh.anims){
			memcpy(xmesh.anims, anims, sizeof(ModelAnimation)*nAnimations);
		}
	}
	else
		return false;
	return true;
}

void* CParaXSerializer::LoadParaX_Body(ParaXParser& Parser)
{
	// load header if not done so yet.
	if(!Parser.m_bHeaderLoaded)
	{
		if(!LoadParaX_Header(Parser))
			return false;
	}
	CParaXModel* pMesh = NULL;
	
	if(Parser.m_pParaXBody)
	{
		SIZE_T			dwSize;
		const unsigned char      *pBuffer=NULL;
		m_pRaw = NULL;
		// Lock the raw unsigned char data if any
		if(Parser.m_pParaXRawData && SUCCEEDED(Parser.m_pParaXRawData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
			m_pRaw = pBuffer+4;

		if(Parser.m_xheader.type == PARAX_MODEL_ANIMATED || Parser.m_xheader.type == PARAX_MODEL_BMAX)
		{
			pMesh = new CParaXModel(Parser.m_xheader);
			
			if (Parser.m_xheader.nModelFormat & PARAX_FORMAT_EXTENDED_HEADER2)
			{
				ParaXHeaderDef2 header2;
				memcpy(&header2, GetRawData(Parser.m_xheader.nOffsetAdditionalHeader), sizeof(ParaXHeaderDef2));
				Parser.m_xheader.IsAnimated = header2.IsAnimated;
				pMesh->SetHeader(Parser.m_xheader);
				pMesh->m_vNeckYawAxis = header2.neck_yaw_axis;
				pMesh->m_vNeckPitchAxis = header2.neck_pitch_axis;
			}

			// Scan for data nodes inside the ParaXBody
			SIZE_T nCount;
			Parser.m_pParaXBody->GetChildren(&nCount);
			for(int i = 0; i<(int)nCount;i++)
			{
				LPFileData pSubData=NULL;
				Parser.m_pParaXBody->GetChild(i, &pSubData);
				if(!pSubData->IsReference())
				{
					GUID Type;
					// Get the template type
					if(SUCCEEDED(pSubData->GetType(&Type)))
					{
						if(Type == TID_XDWORDArray)	{//XGlobalSequences
							SIZE_T nSize=100;
							char szName[100+1];
							// Get the frame name (if any)
							if(SUCCEEDED(pSubData->GetName(szName, &nSize))){
								if( strcmp("XGlobalSequences",szName)==0 )
									ReadXGlobalSequences(*pMesh, pSubData);
							}
						}
						else if(Type == TID_XVertices)	{//XVertices
							if(!ReadXVertices(*pMesh, pSubData))
								OUTPUT_LOG("error loading vertices");
						}
						else if (Type == TID_XVoxels) {
							if (!ReadXVoxels(*pMesh, pSubData))
								OUTPUT_LOG("error loading vertices");
						}
						else if(Type == TID_XTextures)	{//XTextures
							if(!ReadXTextures(*pMesh, pSubData))
								OUTPUT_LOG("error loading XTextures");
						}
						else if(Type == TID_XAttachments)	{//XAttachments
							if(!ReadXAttachments(*pMesh, pSubData))
								OUTPUT_LOG("error loading XAttachments");
						}
						else if(Type == TID_XTransparency)	{//XTransparency
							if(!ReadXTransparency(*pMesh, pSubData))
								OUTPUT_LOG("error loading XTransparency");
						}
						else if(Type == TID_XViews)	{//XViews
							if(!ReadXViews(*pMesh, pSubData))
								OUTPUT_LOG("error loading XViews");
						}
						else if(Type == TID_XIndices0)	{//XIndices0
							if(!ReadXIndices0(*pMesh, pSubData))
								OUTPUT_LOG("error loading XIndices0");
						}
						else if(Type == TID_XGeosets)	{//XGeosets
							if(!ReadXGeosets(*pMesh, pSubData))
								OUTPUT_LOG("error loading XGeosets");
						}
						else if(Type == TID_XRenderPass)	{//XRenderPass
							if(!ReadXRenderPass(*pMesh, pSubData))
								OUTPUT_LOG("error loading XRenderPass");
						}
						else if(Type == TID_XBones)	{//XBones
							if(!ReadXBones(*pMesh, pSubData))
								OUTPUT_LOG("error loading XBones");
						}
						else if(Type == TID_XTexAnims)	{//XTexAnims
							if(!ReadXTexAnims(*pMesh, pSubData))
								OUTPUT_LOG("error loading XTexAnims");
						}
						else if(Type == TID_XParticleEmitters)	{//XParticleEmitters
							if(!ReadXParticleEmitters(*pMesh, pSubData))
								OUTPUT_LOG("error loading XParticleEmitters");
						}
						else if(Type == TID_XRibbonEmitters)	{//XRibbonEmitters
							if(!ReadXRibbonEmitters(*pMesh, pSubData))
								OUTPUT_LOG("error loading XRibbonEmitters");
						}
						else if(Type == TID_XColors)	{//XColors
							if(!ReadXColors(*pMesh, pSubData))
								OUTPUT_LOG("error loading XColors");
						}
						else if(Type == TID_XCameras)	{//XCameras
							if(!ReadXCameras(*pMesh, pSubData))
								OUTPUT_LOG("error loading XCameras");
						}
						else if(Type == TID_XLights)	{//XLights
							if(!ReadXLights(*pMesh, pSubData))
								OUTPUT_LOG("error loading XLights");
						}
						else if(Type == TID_XAnimations)	{//XAnimations
							if(!ReadXAnimations(*pMesh, pSubData))
								OUTPUT_LOG("error loading XAnimations");
						}
					}
				}
				SAFE_RELEASE(pSubData);
			}

			if(pMesh->m_RenderMethod == CParaXModel::NO_ANIM)
			{
				pMesh->calcBones();
			}

			// optimize to see if any pass contains rigid body. For rigid body we will render without skinning, thus saving lots of CPU cycles. 
			// TODO: move this to ParaX Exporter instead. 
			{
				uint16 * indices = pMesh->m_indices;
				int nRenderPasses = (int)pMesh->passes.size();
				for(int j=0; j < nRenderPasses; ++j)
				{
					ModelRenderPass& p = pMesh->passes[j];
					int nLockedNum = p.indexCount / 3;
					if(nLockedNum>0 && !(p.is_rigid_body))
					{
						bool bIsRigidBody = true;
						int nVertexOffset = p.GetVertexStart(pMesh);
						ModelVertex * origVertices = pMesh->m_origVertices;
						ModelVertex * ov = NULL;
						uint8 nLastBoneIndex = origVertices[indices[p.m_nIndexStart] + nVertexOffset].bones[0];
						
						int nIndexOffset = p.m_nIndexStart;
						for(int i=0;i<nLockedNum && bIsRigidBody;++i)
						{
							int nVB = 3*i;
							for(int k=0; k<3; ++k, ++nVB)
							{
								uint16 a = indices[nIndexOffset + nVB] + nVertexOffset;
								ov = origVertices + a;
								// weighted vertex
								if( ov->weights[1] != 0 || ov->bones[0] != nLastBoneIndex)
								{
									bIsRigidBody = false;
									break;
								}
							}
						}
						if(bIsRigidBody)
						{
							p.is_rigid_body = bIsRigidBody;
						}
					}
				}
			}
		}
		else if(Parser.m_xheader.type == PARAX_MODEL_DX_STATIC)
		{
			// TODO: for original dx model file.
		}
		

		// unlock raw unsigned char data
		if(m_pRaw!=NULL)
		{
			m_pRaw = NULL;
			Parser.m_pParaXRawData->Unlock();
		}
	}
	return pMesh;
}
#endif