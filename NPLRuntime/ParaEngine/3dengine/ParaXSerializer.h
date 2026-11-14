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

		bool SaveParaXMesh(const string& filename, const CParaXModel& xmesh, SerializerOptions* pOptions = NULL);
	};

}
