#pragma once
#include "modelheaders.h"
#include "XFileStaticModelParser.h"
#include "animated.h"

namespace ParaEngine
{
	class CParaXModel;

	/** parsing character parax model: binary file only. */
	class XFileCharModelParser : public XFileStaticModelParser
	{
	public:
		/** Constructor. Creates a data structure out of the XFile given in the memory block.
		* @param pBuffer Null-terminated memory buffer containing the XFile
		*/
		XFileCharModelParser(const char* pBuffer, int32 nSize);
		~XFileCharModelParser();
	public:
		/** parse parax model */
		CParaXModel * ParseParaXModel();

		/** Load through a ParaXParser object.
		* enumerate all top-level nodes, and save ParaX header struct and nodes data pointers.
		*/
		bool LoadParaX_Header();
		/** processing the body data according to file type.
		* LoadParaX_Header() will be called automatically if not called before.
		* @see LoadParaX_Header() */
		CParaXModel* LoadParaX_Body();
		/** Finalize parser object. when the Parser will also be finalized in its destructor. */
		void LoadParaX_Finalize();

	public:
		/** get the raw data pointer at the specified offset.
		* return NULL if raw data is not available
		*/
		inline const char* GetRawData(int nOffset) const { return (m_pRaw != 0) ? (m_pRaw + nOffset) : NULL; };

	public:
		//** read the header to xheader. */
		bool ReadParaXHeader(ParaXHeaderDef& xheader, XFileDataObjectPtr pFileData);
		/** read all sub data*/
		bool ReadParaXHeader2(CParaXModel& xmesh);
		bool ReadXGlobalSequences(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXVertices(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXTextures(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXAttachments(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXColors(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXTransparency(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXViews(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXIndices0(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXGeosets(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXRenderPass(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXBones(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXTexAnims(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXParticleEmitters(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXRibbonEmitters(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXCameras(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXLights(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXAnimations(CParaXModel& xmesh, XFileDataObjectPtr pFileData);
		bool ReadXVoxels(CParaXModel& xmesh, XFileDataObjectPtr pFileData);


		/** read animation block to Animated<> struct
		@param b: animation block.
		@param anims: [out] animations
		*/
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<Vector3>& anims, int *gs);
		bool ReadAnimationBlock(const AnimationBlock* b, AnimatedShort& anims, int *gs);
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<float>& anims, int *gs);
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<Quaternion>& anims, int *gs);

		const std::string& GetFilename() const;
		void SetFilename(const std::string& val);
	protected:
		ParaXHeaderDef m_xheader;
		const char*		 m_pRaw;
		XFileDataObjectPtr m_pRoot;
		XFileDataObjectPtr m_pParaXBody;
		XFileDataObjectPtr m_pParaXRawData;
		XFileDataObjectPtr m_pParaXRef;
		XFileDataObjectPtr m_pD3DMesh;
		XFileDataObjectPtr m_pD3DRootFrame;
		std::string m_sFilename;
		bool	m_bHeaderLoaded;
	};
}