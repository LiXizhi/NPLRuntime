#pragma once

namespace ParaEngine
{
	class XFileParser;
	class XFileExporter;
	struct AnimationBlock;

	/** data object in binary format */
	class XFileDataObject : public CRefCounted
	{
	public:
		friend class XFileCharModelExporter;

		typedef std::vector< ref_ptr<XFileDataObject> > XFileDataObject_list_type;

		std::string m_sTemplateName;
		std::string m_sName;
		std::vector<char> m_buffer;
		XFileDataObject_list_type m_children;
	public:
		XFileDataObject();
		~XFileDataObject();
		void Init(XFileParser& parser, const std::string& objectname);

		void AddChild(ref_ptr<XFileDataObject> child);
		int GetChildCount();
		ref_ptr<XFileDataObject> GetChild(int i);
		const std::string& GetType();
		const std::string& GetName();
		bool IsReference();
		/** get buffer for reading */
		bool Lock(DWORD* dwSize, const char** pBuffer);
		void Unlock();
	protected:
		inline char* GetBuffer() { return (char*)(&m_buffer[0]); };
		void ResizeBuffer(uint32 nSize);

		/** read all sub data*/
		bool ReadParaXBody(XFileParser& parser);
		bool ReadParaXHeader(XFileParser& parser);
		bool ReadXDWORDArray(XFileParser& parser);

		bool ReadXGlobalSequences(XFileParser& parser);
		bool ReadParaXRawBuffer(XFileParser& parser);
		bool ReadXVertices(XFileParser& parser);
		bool ReadXTextures(XFileParser& parser);
		bool ReadXAttachments(XFileParser& parser);
		bool ReadXColors(XFileParser& parser);
		bool ReadXTransparency(XFileParser& parser);
		bool ReadXViews(XFileParser& parser);
		bool ReadXIndices0(XFileParser& parser);
		bool ReadXGeosets(XFileParser& parser);
		bool ReadXRenderPass(XFileParser& parser);
		bool ReadXBones(XFileParser& parser);
		bool ReadXTexAnims(XFileParser& parser);
		bool ReadXParticleEmitters(XFileParser& parser);
		bool ReadXRibbonEmitters(XFileParser& parser);
		bool ReadXCameras(XFileParser& parser);
		bool ReadXLights(XFileParser& parser);
		bool ReadXAnimations(XFileParser& parser);
		
		bool ReadUnknownDataObject(XFileParser& parser);
		void ReadAnimationBlock(AnimationBlock& unk, XFileParser& parser);
	public:
		void Write(ofstream& strm, XFileExporter& exporter);
	protected:
		inline size_t GetSize() { return m_buffer.size(); };

		void WriteChildren(ofstream& strm,XFileExporter& exporter);
		void WriteInfo(ofstream& strm,XFileExporter& exporter);

		void WriteParaXHeader(ofstream& strm,XFileExporter& exporter);
		void WriteParaXBody(ofstream& strm,XFileExporter& exporter);
		void WriteXDWORDArray(ofstream& strm,XFileExporter& exporter);
		void WriteXVertices(ofstream& strm,XFileExporter& exporter);
		void WriteXTextures(ofstream& strm,XFileExporter& exporter);
		void WriteXAttachments(ofstream& strm,XFileExporter& exporter);
		void WriteXTransparency(ofstream& strm,XFileExporter& exporter);
		void WriteXViews(ofstream& strm,XFileExporter& exporter);
		void WriteXIndices0(ofstream& strm,XFileExporter& exporter);
		void WriteXGeosets(ofstream& strm,XFileExporter& exporter);
		void WriteXRenderPass(ofstream& strm,XFileExporter& exporter);
		void WriteXBones(ofstream& strm,XFileExporter& exporter);
		void WriteXTexAnims(ofstream& strm,XFileExporter& exporter);
		void WriteXParticleEmitters(ofstream& strm,XFileExporter& exporter);
		void WriteXRibbonEmitters(ofstream& strm,XFileExporter& exporter);
		void WriteXColors(ofstream& strm,XFileExporter& exporter);
		void WriteXCameras(ofstream& strm,XFileExporter& exporter);
		void WriteXLights(ofstream& strm,XFileExporter& exporter);
		void WriteXAnimations(ofstream& strm,XFileExporter& exporter);
		void WriteAnimationBlock(ofstream& strm,XFileExporter& exporter, AnimationBlock& unk);
	};
	typedef ref_ptr<XFileDataObject> XFileDataObjectPtr;
}