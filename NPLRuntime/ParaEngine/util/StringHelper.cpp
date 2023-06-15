//-----------------------------------------------------------------------------
// Class:	StringHelper
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.3.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "util/MD5.h"
#include "util/sha1.h"
#include "util/CyoEncode.h"
#include "util/CyoDecode.h"
#include "StringHelper.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#include <boost/locale/encoding_utf.hpp>
#include "ConvertUTF.h"

#include <unordered_map>

#ifdef USE_ICONV
	#include "iconv.h"
#endif

#ifdef USE_BOOST_REGULAR_EXPRESSION
#include "boost/regex.hpp"
#endif

using namespace ParaEngine;

/**
* a XOR key used in SimpleEncode and SimpleDecode methods
*/
const char g_simpleXOR_key[] = "Copyright@ParaEngine, LiXizhi";

StringHelper::_CodePageName StringHelper::defaultCPName;


const WCHAR* ParaEngine::StringHelper::MultiByteToWideChar(const char* name, unsigned int nCodePage, size_t* outLen)
{
	if (name == NULL)
		return NULL;

	thread_local static vector<WCHAR> wsName;

#ifdef WIN32
	int nLength = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, NULL, 0);
	if (nLength > 0)
	{
		if (((int)wsName.size()) < nLength)
			wsName.resize(nLength);
		int nResult = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, &(wsName[0]), nLength);
		wsName[nLength - 1] = L'\0';

		if (outLen)
			*outLen = nLength;
	}
	else
	{
		if (((int)wsName.size()) < 1)
			wsName.resize(1);
		wsName[0] = L'\0';

		if (outLen)
			*outLen = 0;
	}

	return &(wsName[0]);
#else
	size_t nLength = mbstowcs( 0, name, 0);
	if(nLength != (size_t)(-1))
	{
		if(wsName.size()<=nLength) 
			wsName.resize(nLength+1);
		size_t nResult= mbstowcs( &(wsName[0]), name, nLength+1);

		if (outLen)
			*outLen = nResult;
	}
	else
	{
		if(((int)wsName.size())<1) 
			wsName.resize(1);
		wsName[0] = L'\0';

		if (outLen)
			*outLen = 0;
	}
	return &(wsName[0]);
#endif

}

const char* ParaEngine::StringHelper::WideCharToMultiByte(const WCHAR* name, unsigned int nCodePage, size_t* outLen)
{
	if (name == NULL)
		return NULL;
	thread_local static vector<char> cName;
#ifdef WIN32
	int nLength = ::WideCharToMultiByte(nCodePage, 0, name, -1, NULL, NULL, NULL, NULL);
	if (nLength > 0)
	{
		if (((int)cName.size()) < nLength)
			cName.resize(nLength);
		int nResult = ::WideCharToMultiByte(nCodePage, 0, name, -1, &(cName[0]), nLength, NULL, NULL);
		cName[nLength - 1] = '\0';

		if (outLen)
			*outLen = nLength;
	}
	else
	{
		if (((int)cName.size()) < 1)
			cName.resize(1);
		cName[0] = '\0';

		if (outLen)
			*outLen = 0;
	}
	return &(cName[0]);
#else
	size_t nLength=wcstombs( 0, name, 0);
	if(nLength != (size_t)(-1))
	{
		if(cName.size()<nLength) 
			cName.resize(nLength);
		wcstombs( &(cName[0]), name, nLength+1);

		if (outLen)
			*outLen = nLength;
	}
	else
	{
		if(((int)cName.size())<1) 
			cName.resize(1);
		cName[0] = '\0';

		if (outLen)
			*outLen = 0;
	}
	return &(cName[0]);
#endif
}

int ParaEngine::StringHelper::WideCharToMultiByte(const WCHAR* name, char* szText, int nLength, unsigned int nCodePage /*= 0*/)
{
	if (name == NULL)
		return 0;
#ifdef WIN32
	if (nLength > 0)
	{
		int nResult = ::WideCharToMultiByte(nCodePage, 0, name, -1, szText, nLength, NULL, NULL);
		szText[nLength - 1] = '\0';
	}
	else
	{
		nLength = ::WideCharToMultiByte(nCodePage, 0, name, -1, NULL, NULL, NULL, NULL);
	}
	return nLength;
#else
	if (nLength > 0)
	{
		wcstombs(szText, name, nLength + 1);
	}
	else
	{
		nLength = wcstombs(0, name, 0);
	}
	return nLength;
#endif
}

const WCHAR* ParaEngine::StringHelper::AnsiToWideChar(const char* name)
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
	return MultiByteToWideChar(name, CP_ACP);
#else
	// this is not supported in linux, we will just assume input is in system default encoding, such as utf8
	return MultiByteToWideChar(name);
#endif
}

const char* ParaEngine::StringHelper::WideCharToAnsi(const WCHAR* name)
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
	return WideCharToMultiByte(name, CP_ACP);
#else
	// this is not supported in linux, we will just assume input is in system default encoding, such as utf8
	return WideCharToMultiByte(name);
#endif
}

#ifdef USE_ICONV

static const std::string& code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen, bool appendZero = false)
{

	struct ZeroInfo
	{
		char charset_zero[4];
		unsigned int  zero_size;
	};

	struct iconvObject
	{
		iconvObject(iconv_t _cd) : cd(_cd) {};
		iconvObject() : cd((iconv_t)-1){};

		iconvObject& operator = (iconv_t _cd)
		{
			cd = _cd;

			return *this;
		}

		~iconvObject()
		{
			if (cd != (iconv_t)-1)
				iconv_close(cd);
		}
		iconv_t cd;
	};

	static std::unordered_map<std::string, ZeroInfo> ZeroCache;
	static ParaEngine::mutex m;

	thread_local static std::unordered_map<std::string, iconvObject> IconvCache;
	thread_local static std::string cName;
	thread_local static std::string sKey;

#if _LIBICONV_VERSION == 0x109 || _LIBICONV_VERSION == 0x010F
	typedef const char* iconv_input_type;
#else
	typedef char* iconv_input_type;
#endif

	do
	{
		size_t len, outlen;
		char* s;
		size_t ret;

		char* charset_zero;
		size_t zero_size;

		sKey = from_charset;
		sKey += to_charset;

		if (appendZero)
		{
			ParaEngine::Lock lock_(m);

			auto it = ZeroCache.find(sKey);

			if (it != ZeroCache.end())
			{
				charset_zero = it->second.charset_zero;
				zero_size = it->second.zero_size;
			}
			else
			{
				static const char zero = '\0';
				
				iconv_t z_cd = iconv_open(to_charset, "gb2312");
				if (z_cd == (iconv_t)-1)
				{
					break;
				}

				len = 2 + 2;
				outlen = len;

				ZeroCache[sKey] = ZeroInfo();
				ZeroInfo& info = ZeroCache[sKey];

				charset_zero = info.charset_zero;
				s = charset_zero;
				iconv_input_type pZero = (iconv_input_type)&zero;
				size_t zeorlen = 1;

				ret = iconv(z_cd, &pZero, &zeorlen, &s, &outlen);
				iconv_close(z_cd);
				if (ret == (size_t)-1)
				{
					break;
				}

				zero_size = len - outlen;
				info.zero_size = (unsigned int)zero_size;
			}
		}

		auto it = IconvCache.find(sKey);

		iconv_t cd;
		if (it == IconvCache.end())
		{
			cd = iconv_open(to_charset, from_charset);
			if (cd == (iconv_t)-1)
			{
				break;
			}

			IconvCache[sKey] = cd;
		}
		else
		{
			cd = it->second.cd;
		}

		len = inlen * 2 + 2;
		outlen = len;
		if (cName.size() < outlen)
			cName.resize(outlen);

		s = &cName[0];
		iconv_input_type pInbuf = (iconv_input_type)inbuf;

		ret = iconv(cd, &pInbuf, &inlen, &s, &outlen);

		if (ret == (size_t )-1)
		{
			break;
		}

		outlen = len - outlen;

		if (appendZero)
		{
			memcpy(&cName[outlen], charset_zero, zero_size);
			cName.resize(outlen + zero_size);
		}
		else
		{
			cName.resize(outlen);
		}

		return cName;

	} while (false);

	// OUTPUT_LOG("conversion from %s to %s not available", from_charset ? from_charset : "unknow", to_charset ? to_charset : "unknow");
	cName = std::string(inbuf, inlen);
	return cName;
}
#endif



const char* ParaEngine::StringHelper::UTF8ToAnsi(const char* name)
{
	if (name == NULL)
		return NULL;
#ifdef USE_ICONV
	size_t inlen = strlen(name);
	auto& s = code_convert("utf-8", defaultCPName.get().c_str(), name, inlen, true);
	return s.c_str();

#else
	#ifdef WIN32
		return WideCharToMultiByte(MultiByteToWideChar(name, CP_UTF8), CP_ACP);
	#else
		// this is not supported in linux, just forward input to output.
		return name;
	#endif
#endif
}

const char* ParaEngine::StringHelper::AnsiToUTF8(const char* name)
{
	if (name == NULL)
		return NULL;
#ifdef USE_ICONV
	size_t inlen = strlen(name);
	auto& s = code_convert(defaultCPName.get().c_str(), "utf-8", name, inlen, true);
	return s.c_str();

#else
	#ifdef WIN32
		return WideCharToMultiByte(MultiByteToWideChar(name, CP_ACP), CP_UTF8);
	#else
		// this is not supported in linux, just forward input to output
		return name;
	#endif
#endif
}

int StringHelper::GetUnicodeCharNum(const char* str)
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
	int nSize = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	if (nSize > 0)
		return nSize - 1; // the character number is minus one of the character buffer size
	else
	{
		OUTPUT_LOG("warning: StringHelper::GetUnicodeCharNum(%s) have error %d\n", str, GetLastError());
		return 0;
	}
#else
	return getUTF8StringLength((const UTF8*)str);
#endif
}

std::string StringHelper::UniSubString(const char* szText, int nFrom, int nTo)
{
	if (nTo == 0)
		return "";
	nFrom--;
	nTo--;

	std::u16string wideStr;
	if (UTF8ToUTF16(szText, wideStr) && (int)wideStr.size()>nFrom)
	{
		wideStr = wideStr.substr(nFrom, nTo - nFrom + 1);
		std::string str;
		if (UTF16ToUTF8(wideStr, str))
			return str;
	}
	return "";
}

std::string StringHelper::SimpleEncode(const std::string& source)
{

	thread_local static std::string g_code;
	g_code.clear();
	g_code.shrink_to_fit();

	size_t nSize = source.size();
	int nBufferSize = CyoEncode::Base64EncodeGetLength((unsigned long)nSize);
	const int MAX_BINARY_DATA_SIZE = 102400000;
	if (nBufferSize < MAX_BINARY_DATA_SIZE)
	{
		char* buff = new char[nBufferSize + 10];
		char* buffSrc = new char[nSize];

		if (buff && buffSrc)
		{
			//////////////////////////////////////////////////////////////////////////
			// we will first encode the source using simple XOR with a given key, 
			// and then we encode it using Base64 so that it becomes plain text. 
			//////////////////////////////////////////////////////////////////////////
			int nKeySize = sizeof(g_simpleXOR_key);
			int nKeyIndex = 0;
			for (size_t i = 0; i < nSize; ++i, ++nKeyIndex)
			{
				if (nKeyIndex >= nKeySize)
					nKeyIndex = 0;
				buffSrc[i] = source[i] ^ g_simpleXOR_key[nKeyIndex];
			}

			int filledsize = (int)CyoEncode::Base64Encode(buff, buffSrc, nSize);
			buff[filledsize] = '\0';
			g_code = buff;
			delete[] buff;
			delete[] buffSrc;
			return g_code;
		}
	}
	else
	{
		OUTPUT_LOG("error: SimpleEncode max length exceeded: %d \n", nBufferSize);
	}
	return "";
}

std::string StringHelper::SimpleDecode(const std::string& source)
{
	thread_local static std::string g_code;
	g_code.clear();
	g_code.shrink_to_fit();

	size_t nSize = source.size();
	int nBufferSize = 0;
	try
	{
		nBufferSize = CyoDecode::Base64DecodeGetLength((unsigned long)nSize);
	}
	catch (...)
	{
		g_code = "";
		OUTPUT_LOG("invalid source length when calling ParaMisc.SimpleDecode()\n");
		return g_code;
	}

	const int MAX_BINARY_DATA_SIZE = 102400000;
	if (nBufferSize < MAX_BINARY_DATA_SIZE)
	{
		char* buff = new char[nBufferSize + 10];

		if (buff)
		{
			//////////////////////////////////////////////////////////////////////////
			// we will first decode the source using Base64 ,and then using simple XOR with a given key
			//////////////////////////////////////////////////////////////////////////
			try
			{
				int filledsize = (int)CyoDecode::Base64Decode(buff, (const unsigned char*)source.c_str(), nSize);
				buff[filledsize] = '\0';

				int nKeySize = sizeof(g_simpleXOR_key);
				int nKeyIndex = 0;
				for (int i = 0; i < filledsize; ++i, ++nKeyIndex)
				{
					if (nKeyIndex >= nKeySize)
						nKeyIndex = 0;
					buff[i] = buff[i] ^ g_simpleXOR_key[nKeyIndex];
				}
				//g_code = buff;
				g_code.clear();
				g_code.append(buff, filledsize);
			}
			catch (...)
			{
				g_code = "";
				OUTPUT_LOG("unable to decode when calling ParaMisc.SimpleDecode()\n");
			}

			delete[] buff;
			return g_code;
		}
	}
	return NULL;
}


const std::string& StringHelper::EncodingConvert(const std::string& srcEncoding, const std::string& dstEncoding, const std::string& bytes)
{
	if (bytes.empty())
		return CGlobals::GetString();

	thread_local static std::string g_result;
	int nResultSize = 0;

	if (srcEncoding == "HTML" && dstEncoding.empty())
	{
		g_result.resize(256);

		// from  HTML special character in ANSI code page to default encoding in NPL. 
#define ISNUMBER(c)   (((c)>='0') && ((c)<='9'))
		// for HTML special characters &#XXXX; in ANSI code page, convert them to unicode and then encode to destination. 
		char c;
		bool bNewChar = false;
		unsigned int unicode = 0;

		for (int i = 0; (c = bytes[i]) != '\0'; ++i)
		{
			if (i >= (int)g_result.size())
				g_result.resize(g_result.size() * 2);

			char lookahead = bytes[i + 1];
			if (c == '#' && ISNUMBER(lookahead))
			{
				bNewChar = true;
				unicode = 0;
			}
			else if (bNewChar && ISNUMBER(c))
			{
				unicode = unicode * 10 + (int)(c - '0');
			}
			else
			{
				if ((c == '&' && lookahead == '#') ||
					(c == ';' && bNewChar))
				{
					// skip this character
				}
				else
				{
					g_result[nResultSize++] = c;
				}
				if (bNewChar && unicode > 0)
				{
					// convert this letter
					WCHAR wszText[3];
					wszText[0] = (WCHAR)unicode;
					wszText[1] = L'\0';

					size_t len = 0;

#ifdef WIN32
					auto result = WideCharToMultiByte(wszText, CP_ACP, &len);
#else
					auto result = WideCharToMultiByte(wszText, 0, &len);
#endif

					for (size_t i = 0; i < len; i++)
						g_result[nResultSize++] = result[i];

				}
				bNewChar = false;
			}
		}

		g_result[nResultSize] = '\0';
		g_result.resize(nResultSize);
		return g_result;
	}
	else if (dstEncoding == "HTML" && srcEncoding.empty())
	{
		g_result.resize(256);

		wstring w_srcText = AnsiToWideChar(bytes.c_str());
		int nSize = (int)(w_srcText.size());
		WCHAR wChar;
		int i = 0;
		for (int i = 0; i < nSize && (wChar = w_srcText[i]) != 0; i++)
		{
			if ((nResultSize + 10) > (int)g_result.size())
				g_result.resize(g_result.size() * 2);

			if (((int)wChar) < 127 && ((int)wChar) > 0)
			{
				g_result[nResultSize++] = (char)wChar;
			}
			else
			{
				g_result[nResultSize++] = '&';
				g_result[nResultSize++] = '#';
				char tmp[30];
				ParaEngine::StringHelper::fast_itoa((int)wChar, tmp, 30);
				for (int j = 0; tmp[j] != '\0'; j++)
				{
					g_result[nResultSize++] = tmp[j];
				}
				g_result[nResultSize++] = ';';
			}
		}

		g_result[nResultSize] = '\0';
		g_result.resize(nResultSize);
		return g_result;
	}
#ifdef USE_ICONV
	else
	{
		const string& src = srcEncoding.empty() ? defaultCPName.get() : srcEncoding;
		const string& dst = dstEncoding.empty() ? defaultCPName.get() : dstEncoding;

		if (src == dst)
			return bytes;
		else
			return code_convert(srcEncoding.empty() ? defaultCPName.get().c_str() : srcEncoding.c_str()
				, dstEncoding.empty() ? defaultCPName.get().c_str() : dstEncoding.c_str()
				, bytes.c_str()
				, bytes.size());
	}

#else	// USE_ICONV
	else if (srcEncoding == "utf-8" && dstEncoding.empty())
	{
		string destText = UTF8ToAnsi(bytes.c_str());
		int nSize = (int)destText.size();
		if (g_result.size() < destText.size())
			g_result.resize(destText.size() + 10);
		for (int i = 0; i < nSize; ++i)
		{
			g_result[i] = destText[i];
			nResultSize++;
		}

		g_result[nResultSize] = '\0';
		g_result.resize(nResultSize);
		return g_result;
	}
	else if (dstEncoding == "utf-8" && srcEncoding.empty())
	{
		string destText = AnsiToUTF8(bytes.c_str());
		int nSize = (int)destText.size();
		if (g_result.size() < destText.size())
			g_result.resize(destText.size() + 10);
		for (int i = 0; i < nSize; ++i)
		{
			g_result[i] = destText[i];
			nResultSize++;
		}

		g_result[nResultSize] = '\0';
		g_result.resize(nResultSize);
		return g_result;
	}
	else
	{
		g_result.clear();
		return g_result;
	}

#endif // USE_ICONV

}


void ParaEngine::StringHelper::DevideString(const string& input, string& str1, string&str2, char separator)
{
	str1.clear();
	str2.clear();
	size_t pos = input.find(separator);
	if (pos != string::npos) {
		str1.assign(input.c_str(), pos);
		str2.assign(input.c_str(), pos + 1, input.size() - pos - 1);
	}
	else
		str1 = input;
}

void ParaEngine::StringHelper::split(const std::string& src, const std::string& token, std::vector<std::string>& vect)
{
	size_t nend = 0;
	size_t nbegin = 0;
	size_t tokenSize = token.size();
	while (nend != std::string::npos)
	{
		nend = src.find(token, nbegin);
		if (nend == std::string::npos)
			vect.push_back(src.substr(nbegin, src.length() - nbegin));
		else
			vect.push_back(src.substr(nbegin, nend - nbegin));
		nbegin = nend + tokenSize;
	}
}

void ParaEngine::StringHelper::make_lower(string& str_)
{
	const char A = (char)'A';
	const char Z = (char)'Z';
	const char diff = (char)'a' - A;
	int nLen = (int)str_.size();
	char* str = &(str_[0]);
	for (int i = 0; i < nLen; ++i)
	{
		if (str[i] >= A && str[i] <= Z)
			str[i] += diff;
	}
}

void ParaEngine::StringHelper::make_upper(string& str_)
{
	const char a = (char)'a';
	const char z = (char)'z';
	const char diff = (char)'A' - a;

	int nLen = (int)str_.size();
	char* str = &(str_[0]);
	for (int i = 0; i < nLen; ++i)
	{
		if (str[i] >= a && str[i] <= z)
			str[i] += diff;
	}
}

bool ParaEngine::StringHelper::checkValidXMLChars(const std::string& data)
{
	if (data.empty())
		return true;

	std::string::const_iterator it = data.begin();
	for (; it != data.end()
		&& ((unsigned char)(*it) == 0x09
		|| (unsigned char)(*it) == 0x0a
		|| (unsigned char)(*it) == 0x0d
		|| ((unsigned char)(*it) >= 0x20
		&& (unsigned char)(*it) != 0xc0
		&& (unsigned char)(*it) != 0xc1
		&& (unsigned char)(*it) < 0xf5)); ++it)
		;
	return (it == data.end());
}

bool ParaEngine::StringHelper::removeInValidXMLChars(std::string& data)
{
	if (!checkValidXMLChars(data))
	{
		// input contains invalid xml chars. 
		int nSize = (int)(data.size());

		int src = 0;
		int dest = 0;
		// remove all invalid xml chars in the input string. 
		for (; src < nSize; ++src)
		{
			unsigned char src_char = (unsigned char)(data[src]);
			if (src_char == 0x09
				|| src_char == 0x0a
				|| src_char == 0x0d
				|| (src_char >= 0x20
				&& src_char != 0xc0
				&& src_char != 0xc1
				&& src_char < 0xf5))
			{
				data[dest] = src_char;
				dest++;
			}
		}
		data.resize(dest);

		return false;
	}
	else
	{
		return true;
	}
}

RECT* ParaEngine::StringHelper::GetImageAndRect(const string &str, string &imagefile, RECT * prect)
{
	string srect;
	size_t pos = str.find_first_of("#;");
	if (pos != string::npos) {
		imagefile.assign(str.c_str(), pos);
		srect.assign(str.c_str(), pos + 1, str.size() - pos - 1);
	}
	else{
		imagefile = str;
	}

	RECT rect;
	int width, height;
	if (!srect.empty() && sscanf(srect.c_str(), "%d %d %d %d", &rect.left, &rect.top, &width, &height) == 4)
	{
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

		if (prect)
			*prect = rect;
		else
		{
			static RECT g_rect;
			g_rect = rect;
			prect = &g_rect;
		}
		return prect;
	}
	return NULL;

}
bool ParaEngine::StringHelper::IsNumber(const char * str)
{
	const char *ptr = str;
	while ((*ptr)) {
		if ((*ptr) > '9' || (*ptr) < '0') {
			return false;
		}
		ptr++;
	}
	return true;
}

bool ParaEngine::StringHelper::IsLetter(const char* str)
{
	const char *ptr = str;
	while ((*ptr)) {
		if (!(((*ptr) >= 'a' && (*ptr) <= 'z') || ((*ptr) >= 'A' && (*ptr) <= 'Z'))) {
			return false;
		}
		ptr++;
	}
	return true;
}

int ParaEngine::StringHelper::StrToInt(const char *str)
{
	if (str != NULL){
		return atoi(str);
	}
	return 0;
}

double ParaEngine::StringHelper::StrToFloat(const char * str)
{
	return atof(str);
}

bool ParaEngine::StringHelper::RegularMatch(const char *input, const char *expression)
{
	if (input == NULL || expression == NULL) return false;
#ifdef USE_BOOST_REGULAR_EXPRESSION
	boost::regex strExp(expression, boost::regex::basic);
	string sinput=input;
	boost::match_results<std::string::const_iterator> result;
	if ((0 == boost::regex_match(sinput, result, strExp))) {
		return false;
	}
	if (result[0].matched==true) {
		return true;
	}
#endif
	return false;
}

string ParaEngine::StringHelper::md5(const std::string& source, bool bBinary)
{
	ParaEngine::MD5 md5_hash;
	md5_hash.feed((const unsigned char*)source.c_str(), source.size());
	if (!bBinary)
		return md5_hash.hex();
	else
		return md5_hash.binary();
}

string ParaEngine::StringHelper::ToCString(const char* input)
{
	string newstring;
	int a = 0, b = 0;
	while (input[a] != '\0') {
		if (input[a] == '\\') {
			newstring.append(input + b, a - b);
			newstring += "\\";
			b = a;
		}
		a++;
	}
	newstring.append(input + b, a - b);
	return newstring;
}

string ParaEngine::StringHelper::StrReplace(const char* inputstring, char srcchar, char destchar)
{
	string newstring;
	int a = 0, b = 0;
	while (inputstring[a] != '\0') {
		if (inputstring[a] == srcchar) {
			newstring.append(inputstring + b, a - b);
			newstring += destchar;
			b = a + 1;
		}
		a++;
	}
	newstring.append(inputstring + b, a - b);
	return newstring;
}

#pragma region printf
/* the printf region code is based on the tiny printf by
Copyright (C) 2004  Kustaa Nyholm
Modified by LiXizhi, so that %f will print %d if the value is %f
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
*/
typedef void(*putcf) (void*, char);

static void putchw(void* putp, putcf putf, int n, char z, char* bf)
{
	char fc = z ? '0' : ' ';
	char ch;
	char* p = bf;
	while (*p++ && n > 0)
		n--;
	while (n-- > 0)
		putf(putp, fc);
	while ((ch = *bf++))
		putf(putp, ch);
}

static int a2d(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else return -1;
}

static char a2i(char ch, const char** src, int base, int* nump)
{
	const char* p = *src;
	int num = 0;
	int digit;
	while ((digit = a2d(ch)) >= 0) {
		if (digit > base) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p;
	*nump = num;
	return ch;
}

void tfp_format(void* putp, putcf putf, const char *fmt, va_list va)
{
	char bf[30];
	char ch;

	while ((ch = *(fmt++))) {
		if (ch != '%')
			putf(putp, ch);
		else {
			char lz = 0;

			int w = 0;
			ch = *(fmt++);
			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}
			if (ch >= '0' && ch <= '9') {
				ch = a2i(ch, &fmt, 10, &w);
			}

			switch (ch) {
			case 0:
				goto abort;
			case 'd': case 'u': {
				// ignore unsigned int
				ParaEngine::StringHelper::fast_itoa(va_arg(va, int), bf, 30);
				putchw(putp, putf, w, lz, bf);
				break;
			}
			case 'x': case 'X':
				// ignore cases, always lower cased.
				ParaEngine::StringHelper::fast_itoa(va_arg(va, unsigned int), bf, 30, 16);
				putchw(putp, putf, w, lz, bf);
				break;
			case 'f':
			{
				double v = va_arg(va, double);
				ParaEngine::StringHelper::fast_dtoa(v, bf, 30, 5, 10);
				putchw(putp, putf, w, lz, bf);
				break;
			}
			case 'c':
				putf(putp, (char)(va_arg(va, int)));
				break;
			case 's':
				putchw(putp, putf, w, 0, va_arg(va, char*));
				break;
			case '%':
				putf(putp, ch);
			default:
				break;
			}
		}
	}
abort:;
}

inline void putcp(void* p, char c)
{
	*(*((char**)p))++ = c;
}

void ParaEngine::StringHelper::fast_sprintf(char* s, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	tfp_format(&s, putcp, fmt, va);
	putcp(&s, 0);
	va_end(va);
}
/** e.g. fast_snprintf(line, MAX_LINE, "%f ok", 10.f); */
void ParaEngine::StringHelper::fast_snprintf(char* s, int nMaxCount, const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	tfp_format(&s, putcp, fmt, va);
	putcp(&s, 0);
	va_end(va);
}

#pragma endregion printf

// modified from v0.4 of http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
// typically it is over 20% faster than MSVC itoa()
int ParaEngine::StringHelper::fast_itoa(int64 value, char* result, int buf_size, int base)
{
	// check that the base if valid
	if (base < 2 || base > 16) { *result = 0; return 0; }

	int nSize = 0;
	buf_size -= 2; // always save one byte for the minors sign char and '\0'. 

	char* ptr = result, *ptr1 = result, tmp_char;
	int64 tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "fedcba9876543210123456789abcdef"[15 + (tmp_value - value * base)];
		++nSize;
	} while (value && nSize < buf_size);

	// Apply negative sign
	if (tmp_value < 0)	{
		*ptr++ = '-';
		++nSize;
	}
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return nSize;
}

inline bool IsNAN(double x) {
	return x != x;
}
inline bool IsFiniteNumber(double x)
{
	return (x <= DBL_MAX && x >= -DBL_MAX);
}

/** by Xizhi, 2010.11.17
* use fast_itoa to compute fast_dtoa. Please note accuracy is not guaranteed. I.e.0 0.999 may be rounded to 1
* current only base 10 is supported.
*/
int ParaEngine::StringHelper::fast_dtoa(double num, char* result, int buf_size, int max_decimal/*=5*/, int base /*= 10*/)
{
	// #include <boost/math/special_functions/fpclassify.hpp> is a better way?
	if (!IsFiniteNumber(num))
	{
		// check for NAN, INF and -INF, and return "0".
		*result++ = '0';
		*result = '\0';
		return 1;
	}
	int64 integralPart = (int64)num;
	if (num == integralPart)
	{
		return fast_itoa(integralPart, result, buf_size, base);
	}

	int nDecimalFactor = 1;
	for (int i = 0; i < max_decimal; ++i)
	{
		nDecimalFactor *= base;
	}
	int64 allPart = (int64)(nDecimalFactor * num);
	// rounding off the last decimal.
	int64 nCheckNum = (int64)(nDecimalFactor * base * num);
	bool bRemoveOne = false;
	if (nCheckNum < 0)
	{
		int absQModB = (-nCheckNum) % base;
		if (absQModB >= (base >> 2))
			--allPart;
		if (-allPart < nDecimalFactor)
		{
			allPart -= nDecimalFactor;
			bRemoveOne = true;
		}
	}
	else
	{
		int absQModB = (nCheckNum) % base;
		if (absQModB >= (base >> 2))
			allPart++;
		if (allPart < nDecimalFactor)
		{
			allPart += nDecimalFactor;
			bRemoveOne = true;
		}
	}
	int nSize = fast_itoa(allPart, result, buf_size - 1, base);
	if (bRemoveOne)
	{
		char* sStart = result;
		if (*sStart == '-')
			sStart++;
		*sStart = '0';
	}
	char* sEnd = result + nSize - 1;
	// remove trailing zeros. 
	for (; max_decimal >= 1 && (*sEnd) == '0';)
	{
		*sEnd = '\0';
		--sEnd;
		--max_decimal;
	}
	if (max_decimal > 0)
	{
		// insert the decimal point
		nSize = (int)(sEnd - result) + 2;
		*(sEnd + 2) = '\0';
		for (int i = max_decimal; i >= 1; --i)
		{
			*(sEnd + 1) = *sEnd;
			--sEnd;
		}
		*(sEnd + 1) = '.';
		return nSize;
	}
	else
	{
		return (int)(sEnd - result) + 1;
	}
}

bool ParaEngine::StringHelper::UTF8ToUTF16(const std::string& utf8, std::u16string& outUtf16)
{
	
	using boost::locale::conv::utf_to_utf;

	std::wstring str = utf_to_utf<wchar_t>(utf8.c_str(), utf8.c_str() + utf8.size());

	outUtf16 = std::u16string(str.begin(), str.end());

	return true;
}

bool ParaEngine::StringHelper::UTF16ToUTF8(const std::u16string& utf16, std::string& outUtf8)
{
	using boost::locale::conv::utf_to_utf;
	outUtf8 = utf_to_utf<char>(utf16.c_str(), utf16.c_str() + utf16.size());
	return true;
}

bool ParaEngine::StringHelper::MatchWildcard(const std::string& str, const std::string& sWildcardPattern)
{
	int nDestCount = (int)sWildcardPattern.size();
	if (nDestCount == 0)
		return false;
	int j = 0;
	int i = 0;
	int nCount = (int)str.size();
	for (i = 0; i < nCount;)
	{
		char srcChar = str[i];
		char destChar = sWildcardPattern[j];
		if (destChar == srcChar)
		{
			++i;
			++j;
			if (j == nDestCount)
			{
				break;
			}
		}
		else if (destChar == '*')
		{
			if (srcChar == '/' || srcChar == '\\')
			{
				++j;
				if (j == nDestCount)
				{
					break;
				}
			}
			else if (srcChar == '.')
			{
				bool bLookAheadMatch = true;
				for (int nLookhead = 1; (j + nLookhead) < nDestCount && (i + nLookhead - 1) < nCount; ++nLookhead)
				{
					char srcChar = str[i + nLookhead - 1];
					char destChar = sWildcardPattern[j + nLookhead];
					if (destChar == '*')
						break;
					else if (srcChar != destChar){
						bLookAheadMatch = false;
						break;
					}
				}
				if (bLookAheadMatch)
				{
					if (j == (nDestCount - 1) && destChar == '*')
					{
						// if * is the last character, we will match and continue
						++i;
					}
					else
					{
						++j;
						if (j == nDestCount)
						{
							break;
						}
					}
				}
				else
				{
					++i;
					if (i == nCount && j == (nDestCount - 1))
					{ // a perfect match for * at the end
						++j;
						break;
					}
				}
			}
			else
			{
				++i;
				if (i == nCount && j == (nDestCount - 1))
				{ // a perfect match for * at the end
					++j;
					break;
				}
			}
		}
		else
		{
			if (destChar == '.')
			{
				++j;
				++i;
			}
			break;
		}
	}
	if (i == nCount && j == nDestCount)
		return true;
	else
		return false;
}

bool ParaEngine::StringHelper::StrEndsWith(const string& str, const string& sequence)
{
	int nOffset = (int)(str.size() - sequence.size());
	if (nOffset >= 0)
	{
		for (int i = (int)sequence.size() - 1; i >= 0; i--)
		{
			if (sequence[i] != '?' && sequence[i] != str[i + nOffset])
				return false;
		}
		return true;
	}
	return false;
}


bool ParaEngine::StringHelper::StrEndsWithWord(const string& str, const string& sequence)
{
	int nOffset = (int)(str.size() - sequence.size());
	if (nOffset >= 0)
	{
		// it must ends with a given word with no heading spaces
		if (nOffset > 0 && str[nOffset - 1] != ' ')
			return false;

		for (int i = (int)sequence.size() - 1; i >= 0; i--)
		{
			if (sequence[i] != '?' && sequence[i] != str[i + nOffset])
				return false;
		}
		return true;
	}
	return false;
}


bool ParaEngine::StringHelper::UTF8ToUTF16_Safe(const std::string& utf8, std::u16string& outUtf16)
{
	if (UTF8ToUTF16(utf8, outUtf16))
		return true;
	else
	{
		std::string utf8_safe = utf8;
		int nSize = (int)utf8.size();
		for (int i=0; i < nSize; ++i)
		{
			char c = utf8_safe[i];
			if (c < 0)
				utf8_safe[i] = '?';
		}
		if (! UTF8ToUTF16(utf8_safe, outUtf16))
		{
			OUTPUT_LOG("error: UTF8ToUTF16 for string %s\n", utf8.c_str());
		}
		return false;
	}
}


void ParaEngine::StringHelper::TrimUTF16VectorFromIndex(std::vector<char16_t>& str, int index)
{
	int size = static_cast<int>(str.size());
	if (index >= size || index < 0)
		return;
	str.erase(str.begin() + index, str.begin() + size);
}

bool ParaEngine::StringHelper::IsUnicodeSpace(char16_t ch)
{
	return  (ch >= 0x0009 && ch <= 0x000D) || ch == 0x0020 || ch == 0x0085 || ch == 0x00A0 || ch == 0x1680
		|| (ch >= 0x2000 && ch <= 0x200A) || ch == 0x2028 || ch == 0x2029 || ch == 0x202F
		|| ch == 0x205F || ch == 0x3000;
}

bool ParaEngine::StringHelper::IsCJKUnicode(char16_t ch)
{
	return (ch >= 0x4E00 && ch <= 0x9FBF)   // CJK Unified Ideographs
		|| (ch >= 0x2E80 && ch <= 0x2FDF)   // CJK Radicals Supplement & Kangxi Radicals
		|| (ch >= 0x2FF0 && ch <= 0x30FF)   // Ideographic Description Characters, CJK Symbols and Punctuation & Japanese
		|| (ch >= 0x3100 && ch <= 0x31BF)   // Korean
		|| (ch >= 0xAC00 && ch <= 0xD7AF)   // Hangul Syllables
		|| (ch >= 0xF900 && ch <= 0xFAFF)   // CJK Compatibility Ideographs
		|| (ch >= 0xFE30 && ch <= 0xFE4F)   // CJK Compatibility Forms
		|| (ch >= 0x31C0 && ch <= 0x4DFF);  // Other extensions
}

void ParaEngine::StringHelper::TrimUTF16Vector(std::vector<char16_t>& str)
{
	int len = static_cast<int>(str.size());

	if (len <= 0)
		return;

	int last_index = len - 1;

	// Only start trimming if the last character is whitespace..
	if (IsUnicodeSpace(str[last_index]))
	{
		for (int i = last_index - 1; i >= 0; --i)
		{
			if (IsUnicodeSpace(str[i]))
				last_index = i;
			else
				break;
		}

		TrimUTF16VectorFromIndex(str, last_index);
	}
}

unsigned int ParaEngine::StringHelper::GetIndexOfLastNotChar16(const std::vector<char16_t>& str, char16_t c)
{
	int len = static_cast<int>(str.size());

	int i = len - 1;
	for (; i >= 0; --i)
		if (str[i] != c) return i;

	return i;
}

std::string ParaEngine::StringHelper::sha1(const std::string& source, bool bBinary)
{
	SHA1Context sha;
	int err = SHA1Reset(&sha);
	uint8_t hash[SHA1HashSize];

	if (!err)
		err = SHA1Input(&sha, (const unsigned char *)source.c_str(), source.size());

	if (!err)
		err = SHA1Result(&sha, hash);


	if (!err)
	{
		if (bBinary)
		{
			return string((char*)hash, SHA1HashSize);
		}
		else
		{
			char hashString[SHA1HashSize * 2 + 1];
			SHA1ConvertMessageToString(hash, hashString);
			return hashString;
		}
	}
	else
	{
		return "";
	}
}


std::string ParaEngine::StringHelper::base64(const std::string& source)
{
	auto size = source.size();
	if (size > 0)
	{
		int nBufferSize = CyoEncode::Base64EncodeGetLength(size);
		std::string outBase64Buffers;
		outBase64Buffers.resize(nBufferSize);
		int nSize = CyoEncode::Base64Encode(&(outBase64Buffers[0]), source.c_str(), size);
		if (nSize >= 0 && nSize!= nBufferSize)
			outBase64Buffers.resize(nSize);
		return outBase64Buffers;
	}
	else
	{
		return "";
	}
	
}

std::string ParaEngine::StringHelper::unbase64(const std::string& source)
{
	auto size = source.size();
	if (size > 0)
	{
		int nBufferSize = CyoDecode::Base64DecodeGetLength(size);
		std::string out;
		out.resize(nBufferSize);
		int nSize = CyoDecode::Base64Decode(&(out[0]), source.c_str(), size);
		if (nSize >= 0 && nSize != nBufferSize)
			out.resize(nSize);
		return out;
	}
	else
	{
		return "";
	}
}
