#pragma once
#include "XFileParsing.inl"
namespace ParaEngine
{
	struct XFileHeader_t
	{
		uint32 mMajorVersion;
		uint32 mMinorVersion;
		uint32 mBinaryFloatSize;
		bool mIsBinaryFormat;
		XFileHeader_t() :mMajorVersion(0), mMinorVersion(0), mBinaryFloatSize(0), mIsBinaryFormat(false) {}
		XFileHeader_t(bool beBinary) :mMajorVersion(3), mMinorVersion(3), mBinaryFloatSize(4), mIsBinaryFormat(beBinary) {}

	};

	class CParaXModel;

	class XFileExporter
	{
	public:
		friend class XFileDataObject;

		XFileExporter(ofstream& strm);
		~XFileExporter();

		//binary format functions:
	protected:

		void WriteHeader(ofstream& strm);
		void WriteBinWord(ofstream& strm, uint16 nWord);
		void WriteBinDWord(ofstream& strm, uint32 nDWord);
		void WriteString(ofstream& strm, const string& str);
		void WriteName(ofstream& strm, const string& name);
		bool WriteToken(ofstream& strm, const string& token);
		void WriteToken(ofstream& strm, XFileToken enToken);

		void WriteCharArray(ofstream& strm, char* cInput, int nCount);
		void WriteInt(ofstream& strm, uint32 nInt);
		void WriteShort(ofstream& strm, uint16 nInt);
		void WriteIntArray(ofstream& strm);
		void WriteFloat(ofstream& strm, float f);
		void WriteFloatArray(ofstream& strm);
		void WriteIntAndFloatArray(ofstream& strm);

		void WriteVector2(ofstream& strm, Vector2 vec2);
		void WriteVector3(ofstream& strm, Vector3 vec3);
		void WriteRGB(ofstream& strm, Vector3 color);
		void WriteRGBA(ofstream& strm, LinearColor color);

	protected:
		vector<uint32> m_vecInt;
		vector<float> m_vecFloat;
	};
}


