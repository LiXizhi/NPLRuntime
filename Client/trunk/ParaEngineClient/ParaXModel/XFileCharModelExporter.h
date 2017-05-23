#pragma once
#include "XFileExporter.h"
#include "XFileDataObject.h"
#include "ParaXModel.h"
namespace ParaEngine
{
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
	};
}
