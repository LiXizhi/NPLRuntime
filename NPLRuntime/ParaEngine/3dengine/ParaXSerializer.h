#pragma once
#include "ParaXModel/ParaXModel.h"

#include <vector>

namespace ParaEngine
{
	// forward declare
	class CParaFile;

	/** Serialize options for input and output */
	struct SerializerOptions
	{
		/** binary or text file */
		bool m_bBinary : 1;
		bool m_bCompressed : 1;
		bool m_bHasAnimation : 1;
		bool m_bHasNorm : 1;
	public:
		SerializerOptions() :m_bBinary(false), m_bCompressed(false), m_bHasAnimation(true), m_bHasNorm(true) {};
		SerializerOptions(bool bBinary) :m_bBinary(bBinary), m_bCompressed(false), m_bHasAnimation(true), m_bHasNorm(true) {};
	};

#ifdef USE_DIRECTX_RENDERER
	typedef ID3DXFileSaveObject* LPFileSaveObject;
	typedef ID3DXFileSaveData* LPFileSaveData;
	typedef ID3DXFileData* LPFileData;
#endif
	struct CParaRawData
	{
	private:
		/** raw data referenced by offset.*/
		std::vector<unsigned char> m_RawData;
	public:
		CParaRawData() {}

		/** current size */
		unsigned int GetSize() { return (unsigned int)m_RawData.size(); }
		/** get the buffer from the beginning */
		const unsigned char* GetBuffer() { return &m_RawData.front(); }

		/** Add specified data to raw data
		@param nCount: number of objects.
		@return: offset of the beginning of data added. */
		unsigned int AddRawData(const DWORD* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		/** @param nCount: number of objects. */
		unsigned int AddRawData(const short* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		/** @param nCount: number of objects. */
		unsigned int AddRawData(const Vector2* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		/** @param nCount: number of objects. */
		unsigned int AddRawData(const Vector3* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		/** @param nCount: number of objects. */
		unsigned int AddRawData(const Vector4* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		/** @param nCount: number of objects. */
		unsigned int AddRawData(const Matrix4* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};

		/** @param nCount: number of objects. */
		unsigned int AddRawData(const char* pData, int nCount) {
			unsigned int nOffset = GetSize();
			if (nCount > 0)
			{
				unsigned int nSize = nCount * sizeof(*pData);
				m_RawData.resize(nOffset + nSize);
				memcpy(&m_RawData[nOffset], pData, nSize);
			}
			return nOffset;
		};
		unsigned int AddRawData(const std::string& str) {
			return AddRawData(str.c_str(), (int)str.size() + 1);
		};
	};
#ifdef USE_DIRECTX_RENDERER
	/** in case, one want to selectively load from file. this struct keeps intermediary data.*/
	struct ParaXParser
	{
		const char* m_pBuffer;
		int32 m_nBufferSize;

		bool	m_bIsValid;
		bool	m_bHeaderLoaded;

		ID3DXFileEnumObject* m_pDXEnum;
		ParaXHeaderDef m_xheader;
		LPFileData m_pParaXBody;
		LPFileData m_pParaXRawData;
		LPFileData m_pParaXRef;
		LPFileData m_pD3DMesh;
		LPFileData m_pD3DRootFrame;

		// x file template parsing
		LPD3DXFILE	m_pDXFileParser;

	public:
		/** a blank parser from a file. object with this constructor is usually passed to LoadParaX_Begin()
		* @param pDXFileParser: the x file template parser to use. if NULL, the default one is used.
		*/
		ParaXParser(CParaFile& file, LPD3DXFILE	pDXFileParser = NULL);
		ParaXParser(const char* pBuffer, int32 nSize);

		~ParaXParser() { Finalize(); };

		inline LPD3DXFILE GetFileParser() { return m_pDXFileParser; }
		/** release parser. */
		void Finalize();
	};
#endif
	/**
	* this class is used for importing and exporting ParaX model from or to memory files.
	* currently, it supports ParaX and M2 file importing and ParaX file exporting.
	*/
	class CParaXSerializer
	{
	public:
		CParaXSerializer(void);
		~CParaXSerializer(void);

		/** default option*/
		static SerializerOptions g_pDefaultOption;

	private:
		CParaRawData m_rawdata;
		const unsigned char* m_pRaw;
		std::string m_sFilename;
	public:
		/** get the raw data pointer at the specified offset.
		* return NULL if raw data is not available
		*/
		inline const unsigned char* GetRawData(int nOffset) const { return (m_pRaw != 0) ? (m_pRaw + nOffset) : NULL; };
		/**
		* load an existing mesh at one time. To selectively parse file, use LoadParaX_XXX() functions.
		* @param f: file to be saved to.
		* @return: return a pointer to the loaded mesh object. The mesh object is allocated on heap
		* using new operator, one needs to delete it manually. If the function failed, NULL is returned
		*/
		void* LoadParaXMesh(CParaFile& f);

		const std::string& GetFilename() const;
		void SetFilename(std::string val);

		static void ExportParaXMesh(const string& filePath, CParaXModel* pMesh);

#ifdef USE_DIRECTX_RENDERER
		/** ParaX parser */
		void* LoadParaXMesh(CParaFile& f, ParaXParser& Parser);

		/** Load through a ParaXParser object.
		* enumerate all top-level nodes, and save ParaX header struct and nodes data pointers.
		* e.g.
		ParaXParser p(CParaFile("a.txt"));
		if(LoadParaX_Header(p)){
			pMesh = LoadParaX_Body(p);
			LoadParaX_Finalize(p);
		}
		*/
		bool LoadParaX_Header(ParaXParser& Parser);
		/** processing the body data according to file type.
		* LoadParaX_Header() will be called automatically if not called before.
		* @see LoadParaX_Header() */
		void* LoadParaX_Body(ParaXParser& Parser);
		/** Finalize parser object. when the Parser will also be finalized in its destructor. */
		void LoadParaX_Finalize(ParaXParser& Parser);

		/**
		* save an existing mesh to file.
		* @param filename: file to be saved to.
		* @param xmesh: ParaX mesh object to export.
		* @param pOptions: options for serialization @see SerializerOptions.
		* @return: return true if succeeds.
		*/
		bool SaveParaXMesh(const string& filename, const CParaXModel& xmesh, SerializerOptions* pOptions = NULL);

	private:
		/** write animation block to raw data, and return an animation block struct with valid offset.
		@param b: [out] animation block.
		@param anims: animations
		@param gs: global sequence pointer
		*/
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<Vector3>& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const AnimatedShort& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<float>& anims);
		bool WriteAnimationBlock(AnimationBlock* b, const Animated<Quaternion>& anims);

		/** read animation block to Animated<> struct
		@param b: animation block.
		@param anims: [out] animations
		*/
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<Vector3>& anims, int* gs);
		bool ReadAnimationBlock(const AnimationBlock* b, AnimatedShort& anims, int* gs);
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<float>& anims, int* gs);
		bool ReadAnimationBlock(const AnimationBlock* b, Animated<Quaternion>& anims, int* gs);

		/** the file save object */
		bool WriteParaXHeader(const CParaXModel& xmesh, LPFileSaveObject pFileData);
		//bool WriteParaXHeader(const CParaXModel& xmesh, LPFileSaveData pFileData);

		bool WriteXGlobalSequences(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXVertices(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXTextures(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXAttachments(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXColors(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXTransparency(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXViews(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXIndices0(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXGeosets(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXRenderPass(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXBones(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXTexAnims(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXParticleEmitters(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXRibbonEmitters(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXCameras(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXLights(const CParaXModel& xmesh, LPFileSaveData pFileData);
		bool WriteXAnimations(const CParaXModel& xmesh, LPFileSaveData pFileData);

		/** this function must be called until all Animation Block has been written. So it is usually the
		* last file node to be written to file.*/
		bool WriteXRawBytes(const CParaXModel& xmesh, LPFileSaveObject pFileData);

		/** read the header to xheader. */
		bool ReadParaXHeader(ParaXHeaderDef& xheader, LPFileData pFileData);

		/** read all sub data*/
		bool ReadXGlobalSequences(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXVertices(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXVoxels(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXTextures(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXAttachments(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXColors(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXTransparency(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXViews(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXIndices0(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXGeosets(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXRenderPass(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXBones(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXTexAnims(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXParticleEmitters(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXRibbonEmitters(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXCameras(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXLights(CParaXModel& xmesh, LPFileData pFileData);
		bool ReadXAnimations(CParaXModel& xmesh, LPFileData pFileData);
#endif
	};

}
