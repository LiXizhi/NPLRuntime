#pragma once
#include "XFileExporter.h"
#include "XFileDataObject.h"
#include "ParaXModel.h"
namespace ParaEngine
{
	struct UUID_t
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	};
#pragma region +[Template UUID define]
#define DECLARE_UUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		const UUID_t name \
				= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	/* {10000000-0000-0000-0000-123456789000} */
	DECLARE_UUID(UUID_ParaXHeader,
		0x10000000, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	/* ParaXBody contains array blocks */
	DECLARE_UUID(UUID_ParaXBody,
		0x20000000, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_ModelView,
		0x10000007, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XViews,
		0x20000005, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_ModelTextureDef,
		0x1000000d, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XTextures,
		0x20000002, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_AnimationBlock,
		0x10000003, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	/* {00000000-0000-0000-0000-123456789000} */
	DECLARE_UUID(UUID_ModelAttachmentDef,
		0x10000014, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XAttachments,
		0x20000003, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XVertices,
		0x20000001, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XVoxels,
		0x20000101, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XIndices0,
		0x20000006, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	/* {00000000-0000-0000-0000-123456789000} */
	DECLARE_UUID(UUID_ModelGeoset,
		0x10000008, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XGeosets,
		0x20000007, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XRenderPass,
		0x20000008, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	/* {00000000-0000-0000-0000-123456789000} */
	DECLARE_UUID(UUID_ModelBoneDef,
		0x10000004, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XBones,
		0x20000009, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	/* {00000000-0000-0000-0000-123456789000} */
	DECLARE_UUID(UUID_ModelAnimation,
		0x10000002, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XAnimations,
		0x2000000f, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

	DECLARE_UUID(UUID_XDWORDArray,
		0x20000010, 0x0000, 0x0000, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0x00);

#pragma endregion

	struct XFileTemplateMember_t
	{
		string name;
		string type;
		string count;
		XFileTemplateMember_t(string _name, string _type, string _count = "") :name(_name), type(_type), count(_count) {}
	};

	struct XFileTemplate_t
	{
		string name;
		UUID_t uuid;
		vector<XFileTemplateMember_t> members;
		bool beExtend;
		XFileTemplate_t(string _name, UUID_t _uuid, bool _beExtend = false) :name(_name), uuid(_uuid), beExtend(_beExtend) {}
		void Init(string _name, UUID_t _uuid, bool _beExtend = false)
		{
			name = _name;
			uuid = _uuid;
			beExtend = _beExtend;
		}
		void clear()
		{
			name = "";
			uuid = { 0x00000000, 0x0000, 0x0000,{ 0x00, 0x00,  0x0,  0x00,  0x00,  0x00,  0x00,  0x00 } };
			beExtend = false;
			members.clear();
		}
	};


	struct CParaRawData;
	class XFileCharModelExporter :
		public XFileExporter
	{
	public:
		XFileCharModelExporter(ofstream& strm, CParaXModel* pMesh);
		~XFileCharModelExporter();

		static bool Export(const string& filepath, CParaXModel* pMesh);

		void ExportParaXModel(ofstream& strm);
	private:

		static void InitTemplates();

		void WriteTemplates(ofstream& strm);

		void UUIDToBin(UUID_t guid, char* bin);

		void WriteTemplate(ofstream& strm, const XFileTemplate_t& stTem);
		void WriteUUID(ofstream& strm, UUID_t guid);
		void WriteTemplateMember(ofstream& strm, const XFileTemplateMember_t& memeber);

		XFileDataObjectPtr Translate();

		void Release(XFileDataObjectPtr pData);

		DWORD CountIsAnimatedValue();

		bool WriteParaXHeader(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteParaXBody(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteParaXRawData(XFileDataObjectPtr pData, const string& strName = "");

		bool WriteParaXBodyChild(XFileDataObjectPtr pData, const string& strTemplateName, const string& strName = "");

		bool WriteXGlobalSequences(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXVertices(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXVoxels(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXTextures(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXAttachments(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXColors(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXTransparency(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXViews(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXIndices0(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXGeosets(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXRenderPass(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXBones(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXTexAnims(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXParticleEmitters(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXRibbonEmitters(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXCameras(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXLights(XFileDataObjectPtr pData, const string& strName = "");
		bool WriteXAnimations(XFileDataObjectPtr pData, const string& strName = "");

		/** write animation block to raw data, and return an animation block struct with valid offset.
		@param b: [out] animation block.
		@param anims: animations
		@param gs: global sequence pointer
		*/
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<Vector3>& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const AnimatedShort& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<float>& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<Quaternion>& anims);
	private:
		CParaXModel* m_pMesh;
		CParaRawData* m_pRawData;
		static vector<XFileTemplate_t> m_vecTemplates;
	};
}
