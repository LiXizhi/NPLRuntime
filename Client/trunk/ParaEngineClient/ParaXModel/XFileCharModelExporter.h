#pragma once
#include "XFileExporter.h"
#include "XFileDataObject.h"
#include "ParaXModel.h"
namespace ParaEngine
{
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
		GUID guid;
		vector<XFileTemplateMember_t> members;
		bool beExtend;
		XFileTemplate_t(string _name, GUID _guid, bool _beExtend = false) :name(_name), guid(_guid), beExtend(_beExtend) {}
		void Init(string _name, GUID _guid, bool _beExtend = false)
		{
			name = _name;
			guid = _guid;
			beExtend = _beExtend;
		}
		void clear()
		{
			name = "";
			guid = { 0x00000000, 0x0000, 0x0000,{ 0x00, 0x00,  0x0,  0x00,  0x00,  0x00,  0x00,  0x00 } };
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

		void InitTemplates();

		void WriteTemplates(ofstream& strm);

		string GUIDToString(GUID guid);
		void GUIDToBin(GUID guid,char* bin);

		void WriteTemplate(ofstream& strm, const XFileTemplate_t& stTem);
		void WriteGUID(ofstream& strm, GUID guid);
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
		vector<XFileTemplate_t> m_vecTemplates;
	};
}
