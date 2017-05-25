//#include "stdafx.h"
#include "ParaEngine.h"
#include "XFileExporter.h"
#include "ParaXModel.h"

#include <fstream>
#include <string>

using namespace ParaEngine;

XFileExporter::XFileExporter(ofstream& strm)
{
	WriteHeader(strm);
}


XFileExporter::~XFileExporter()
{
}

void XFileExporter::WriteHeader(ofstream& strm)
{
	XFileHeader_t header(true);
	//string flag("xof ");
	strm.write("xof ", 4);

	char num = 0;
	num = (char)(header.mMajorVersion / 10 + 48);
	strm.write((char*)&num, 1);
	num = (char)(header.mMajorVersion % 10 + 48);
	strm.write((char*)&num, 1);

	num = (char)(header.mMinorVersion / 10 + 48);
	strm.write((char*)&num, 1);
	num = (char)(header.mMinorVersion % 10 + 48);
	strm.write((char*)&num, 1);
	//strm.write((char*)header.mMinorVersion, 2);
	//char format[4] = header.mIsBinaryFormat ? "bin " : "txt ";
	if (header.mIsBinaryFormat)
	{
		strm.write("bin ", 4);
	}
	else
	{
		strm.write("txt ", 4);
	}

	// we work in bytes,but the x format specifies size in bits
	uint32 floatSize = header.mBinaryFloatSize * 8;

	num = (char)(floatSize / 1000 + 48);
	strm.write((char*)&num, 1);
	num = (char)(floatSize % 1000 / 100 + 48);
	strm.write((char*)&num, 1);
	num = (char)(floatSize % 100 / 10 + 48);
	strm.write((char*)&num, 1);
	num = (char)(floatSize % 10 + 48);
	strm.write((char*)&num, 1);
}

void XFileExporter::WriteName(ofstream& strm, const string& name)
{
	if (name.empty())
	{
		return;
	}
	WriteIntAndFloatArray(strm);

	WriteToken(strm, TOKEN_NAME);
	// 长度占4个字符
	WriteBinDWord(strm, (uint32)name.length());

	strm.write(name.c_str(), name.length());
}

void XFileExporter::WriteToken(ofstream& strm, XFileToken enToken)
{
	WriteIntAndFloatArray(strm);
	WriteBinWord(strm, (uint16)enToken);
}

// 把字符串token转换成一个uint16长度的数值,写入x文件，作为读取时的标示
bool XFileExporter::WriteToken(ofstream& strm, const string& token)
{
	static map<string, XFileToken> mapToken{
		/*{ "",TOKEN_NONE },
		{ "name",TOKEN_NAME },*/
		/*{ "int",TOKEN_INTEGER },*/
		/*{TOKEN_STRING = 2,
		{TOKEN_INTEGER = 3,
		{TOKEN_GUID = 5,
		{TOKEN_INTEGER_LIST = 6,
		{TOKEN_FLOAT_LIST = 7,*/
		{ "{",TOKEN_OBRACE },
		{ "}",TOKEN_CBRACE },
		{ "(",TOKEN_OPAREN },
		{ ")",TOKEN_CPAREN },
		{ "[",TOKEN_OBRACKET },
		{ "]",TOKEN_CBRACKET },
		{ "<",TOKEN_OANGLE },
		{ ">",TOKEN_CANGLE },
		{ ".",TOKEN_DOT },
		{ ",",TOKEN_COMMA },
		{ ";",TOKEN_SEMICOLON },
		//{ "template",TOKEN_TEMPLATE },
		{ "WORD",TOKEN_WORD },
		{ "DWORD",TOKEN_DWORD },
		{ "FLOAT",TOKEN_FLOAT },
		{ "DOUBLE",TOKEN_DOUBLE },
		{ "CHAR",TOKEN_CHAR },
		{ "UCHAR",TOKEN_UCHAR },
		{ "SWORD",TOKEN_SWORD },
		{ "SDWORD",TOKEN_SDWORD },
		//{ "void",TOKEN_VOID },
		{ "STRING",TOKEN_LPSTR },
		/*{ "unicode",TOKEN_UNICODE },
		{ "cstring",TOKEN_CSTRING },
		{ "array",TOKEN_ARRAY },*/
		/*{"",TOKEN_ERROR = 0xffff,*/
	};
	auto iter = mapToken.find(token);
	if (mapToken.end() != iter)
	{
		auto enToken = iter->second;
		WriteToken(strm, enToken);
		return true;
	}
	return false;
}


void XFileExporter::WriteBinWord(ofstream& strm, uint16 nWord)
{
	char c = nWord & 0xff;
	strm.write(&c, 1);
	c = nWord >> 8;
	strm.write(&c, 1);
}
void XFileExporter::WriteBinDWord(ofstream& strm, uint32 nDWord)
{
	char c = nDWord & 0xff;
	strm.write(&c, 1);
	c = (nDWord >> 8) & 0xff;
	strm.write(&c, 1);
	c = (nDWord >> 16) & 0xff;
	strm.write(&c, 1);
	c = (nDWord >> 24) & 0xff;
	strm.write(&c, 1);
}

void XFileExporter::WriteString(ofstream& strm, const string& str)
{

	if (str.empty())
	{
		return;
	}
	WriteIntAndFloatArray(strm);

	WriteToken(strm, TOKEN_STRING);
	// 长度占4个字符
	WriteBinDWord(strm, (uint32)str.length());

	strm.write(str.c_str(), str.length());
	WriteToken(strm, TOKEN_SEMICOLON);
	//strm.write(";", 1);
}

void ParaEngine::XFileExporter::WriteCharArray(ofstream& strm, char* cInput, int nCount)
{
	//WriteBinWord(strm, 0x06);// 标示是数组格式
	//WriteBinDWord(strm, nCount);
	for (int i = 0; i<nCount; ++i)
	{
		int nChar = (int)cInput[i];
		WriteInt(strm, nChar);
	}
}

void ParaEngine::XFileExporter::WriteShort(ofstream& strm, uint16 nInt)
{
	uint32 value = nInt & 0x0000ffff;
	WriteInt(strm, value);
}

void ParaEngine::XFileExporter::WriteInt(ofstream& strm, uint32 nInt)
{
	WriteFloatArray(strm);
	m_vecInt.push_back(nInt);
	//WriteBinDWord(strm, nInt);
}

void ParaEngine::XFileExporter::WriteIntArray(ofstream& strm)
{
	if (m_vecInt.empty())
	{
		return;
	}
	/*else if (1 == m_vecInt.size())
	{
		WriteBinWord(strm, 0x03);
		WriteBinDWord(strm, m_vecInt[0]);
	}
	else
	{
		WriteBinWord(strm, 0x06);
		WriteBinDWord(strm, m_vecInt.size());
		for (auto nInt : m_vecInt)
		{
			WriteBinDWord(strm, nInt);
		}
	}*/
	WriteBinWord(strm, 0x06);
	WriteBinDWord(strm, m_vecInt.size());
	for (auto nInt : m_vecInt)
	{
		WriteBinDWord(strm, nInt);
	}
	m_vecInt.clear();
}

void ParaEngine::XFileExporter::WriteFloat(ofstream& strm, float f)
{
	WriteIntArray(strm);
	m_vecFloat.push_back(f);
	//strm.write((char*)&f, sizeof(float));
}

void ParaEngine::XFileExporter::WriteFloatArray(ofstream& strm)
{
	if (m_vecFloat.empty())
	{
		return;
	}
	/*else if (1 == m_vecFloat.size())
	{
		WriteBinWord(strm, 0x42);
		strm.write((char*)&m_vecFloat[0], sizeof(float));
	}
	else
	{
		WriteBinWord(strm, 0x07);
		WriteBinDWord(strm, m_vecFloat.size());
		for (auto nFloat : m_vecFloat)
		{
			strm.write((char*)&nFloat, sizeof(float));
		}
	}*/
	WriteBinWord(strm, 0x07);
	WriteBinDWord(strm, m_vecFloat.size());
	for (auto nFloat : m_vecFloat)
	{
		strm.write((char*)&nFloat, sizeof(float));
	}
	m_vecFloat.clear();
}

void ParaEngine::XFileExporter::WriteIntAndFloatArray(ofstream& strm)
{
	WriteIntArray(strm);
	WriteFloatArray(strm);
}

void ParaEngine::XFileExporter::WriteVector2(ofstream& strm, Vector2 vec2)
{
	WriteFloat(strm, vec2.x);
	WriteFloat(strm, vec2.y);
}

void ParaEngine::XFileExporter::WriteVector3(ofstream& strm, Vector3 vec3)
{
	WriteFloat(strm, vec3.x);
	WriteFloat(strm, vec3.y);
	WriteFloat(strm, vec3.z);
}

void ParaEngine::XFileExporter::WriteRGB(ofstream& strm, Vector3 color)
{
	WriteFloat(strm, color.x);
	WriteFloat(strm, color.y);
	WriteFloat(strm, color.z);
}

void ParaEngine::XFileExporter::WriteRGBA(ofstream& strm, LinearColor color)
{
	WriteFloat(strm, color.r);
	WriteFloat(strm, color.g);
	WriteFloat(strm, color.b);
	WriteFloat(strm, color.a);
}

#pragma endregion
