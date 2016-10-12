//-----------------------------------------------------------------------------
// Class:	StringHelper
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.3.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CyoEncode.h"
#include "CyoDecode.h"
#include "util/MD5.h"
#include "StringHelper.h"
#include <boost/thread/tss.hpp>
#include "ConvertUTF.h"

#ifdef USE_BOOST_REGULAR_EXPRESSION
#include "boost/regex.hpp"
#endif

using namespace ParaEngine;

/**
* a XOR key used in SimpleEncode and SimpleDecode methods
*/
const char g_simpleXOR_key[] = "Copyright@ParaEngine, LiXizhi";


const WCHAR* ParaEngine::StringHelper::MultiByteToWideChar(const char* name, unsigned int nCodePage)
{
	static boost::thread_specific_ptr< vector<WCHAR> > wsName_;
	if (!wsName_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		wsName_.reset(new vector<WCHAR>());
	}
	vector<WCHAR>& wsName = *wsName_;

#ifdef WIN32
	int nLength = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, NULL, 0);
	if (nLength > 0)
	{
		if (((int)wsName.size()) < nLength)
			wsName.resize(nLength);
		int nResult = ::MultiByteToWideChar(nCodePage, (nCodePage == CP_UTF8) ? 0 : MB_PRECOMPOSED, name, -1, &(wsName[0]), nLength);
		wsName[nLength - 1] = L'\0';
	}
	else
	{
		if (((int)wsName.size()) < 1)
			wsName.resize(1);
		wsName[0] = L'\0';
	}
	return &(wsName[0]);
#else
	size_t nLength = mbstowcs( 0, name, 0);
	if(nLength != (size_t)(-1))
	{
		if(wsName.size()<=nLength) 
			wsName.resize(nLength+1);
		size_t nResult= mbstowcs( &(wsName[0]), name, nLength+1);
	}
	else
	{
		if(((int)wsName.size())<1) 
			wsName.resize(1);
		wsName[0] = L'\0';
	}
	return &(wsName[0]);
#endif

}

const char* ParaEngine::StringHelper::WideCharToMultiByte(const WCHAR* name, unsigned int nCodePage)
{
	static boost::thread_specific_ptr< vector<char> > cName_;
	if (!cName_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		cName_.reset(new vector<char>());
	}
	vector<char>& cName = *cName_;

#ifdef WIN32
	int nLength = ::WideCharToMultiByte(nCodePage, 0, name, -1, NULL, NULL, NULL, NULL);
	if (nLength > 0)
	{
		if (((int)cName.size()) < nLength)
			cName.resize(nLength);
		int nResult = ::WideCharToMultiByte(nCodePage, 0, name, -1, &(cName[0]), nLength, NULL, NULL);
		cName[nLength - 1] = '\0';
	}
	else
	{
		if (((int)cName.size()) < 1)
			cName.resize(1);
		cName[0] = '\0';
	}
	return &(cName[0]);
#else
	size_t nLength=wcstombs( 0, name, 0);
	if(nLength != (size_t)(-1))
	{
		if(cName.size()<nLength) 
			cName.resize(nLength);
		wcstombs( &(cName[0]), name, nLength+1);
	}
	else
	{
		if(((int)cName.size())<1) 
			cName.resize(1);
		cName[0] = '\0';
	}
	return &(cName[0]);
#endif
}

int ParaEngine::StringHelper::WideCharToMultiByte(const WCHAR* name, char* szText, int nLength, unsigned int nCodePage /*= 0*/)
{
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

const char* ParaEngine::StringHelper::UTF8ToAnsi(const char* name)
{
#ifdef WIN32
	return WideCharToMultiByte(MultiByteToWideChar(name, CP_UTF8), CP_ACP);
#else
	// this is not supported in linux, just forward input to output.
	return name;
#endif
}

const char* ParaEngine::StringHelper::AnsiToUTF8(const char* name)
{
#ifdef WIN32
	return WideCharToMultiByte(MultiByteToWideChar(name, CP_ACP), CP_UTF8);
#else
	// this is not supported in linux, just forward input to output
	return name;
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
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
	const int MaxCharacterNum = 1024;
	WCHAR wszText[MaxCharacterNum + 1];
	wszText[MaxCharacterNum] = 0;
	int nCharCount = ::MultiByteToWideChar(CP_UTF8, 0, szText, -1, wszText, MaxCharacterNum);
	if (nCharCount <= 0)
	{
		return "";
	}
	if (nTo < 0 || nTo >= nCharCount)
		nTo = nCharCount - 1;
	if (nFrom >= 0 && nTo < nCharCount && (nTo - nFrom) >= 0)
	{
		wszText[nTo + 1] = L'\0';
		char result[MaxCharacterNum * 2 + 2];
		int nResult = ::WideCharToMultiByte(CP_UTF8, 0, &(wszText[nFrom]), (nTo - nFrom) + 1, result, MaxCharacterNum * 2, NULL, NULL);
		if (nResult > 0)
		{
			result[nResult] = 0;
			return result;
		}
	}
#else
	std::u16string wideStr;
	if (UTF8ToUTF16(szText, wideStr) && (int)wideStr.size()>nFrom)
	{
		wideStr = wideStr.substr(nFrom, nTo - nFrom + 1);
		std::string str;
		if (UTF16ToUTF8(wideStr, str))
			return str;
	}
#endif
	return "";
}

const char* StringHelper::SimpleEncode(const char* source)
{
	static boost::thread_specific_ptr< std::string > g_code_;
	if (!g_code_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		g_code_.reset(new std::string());
	}
	else
	{
		g_code_->clear();
		g_code_->shrink_to_fit();
	}
	std::string& g_code = *g_code_;

	int nSize = (int)strlen(source);
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
			for (int i = 0; i < nSize; ++i, ++nKeyIndex)
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
			return g_code.c_str();
		}
	}
	else
	{
		OUTPUT_LOG("error: SimpleEncode max length exceeded: %d \n", nBufferSize);
	}
	return NULL;
}

const char* StringHelper::SimpleDecode(const char* source)
{
	static boost::thread_specific_ptr< std::string > g_code_;
	if (!g_code_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		g_code_.reset(new std::string());
	}
	else
	{
		g_code_->clear();
		g_code_->shrink_to_fit();
	}
	std::string& g_code = *g_code_;

	int nSize = (int)strlen(source);
	int nBufferSize = 0;
	try
	{
		nBufferSize = CyoDecode::Base64DecodeGetLength((unsigned long)nSize);
	}
	catch (...)
	{
		g_code = "";
		OUTPUT_LOG("invalid source length when calling ParaMisc.SimpleDecode()\n");
		return g_code.c_str();
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
				int filledsize = (int)CyoDecode::Base64Decode(buff, source, nSize);
				buff[filledsize] = '\0';

				int nKeySize = sizeof(g_simpleXOR_key);
				int nKeyIndex = 0;
				for (int i = 0; i < filledsize; ++i, ++nKeyIndex)
				{
					if (nKeyIndex >= nKeySize)
						nKeyIndex = 0;
					buff[i] = buff[i] ^ g_simpleXOR_key[nKeyIndex];
				}
				g_code = buff;
			}
			catch (...)
			{
				g_code = "";
				OUTPUT_LOG("unable to decode when calling ParaMisc.SimpleDecode()\n");
			}

			delete[] buff;
			return g_code.c_str();
		}
	}
	return NULL;
}

const char* StringHelper::EncodingConvert(const char* srcEncoding, const char* dstEncoding, const char* bytes)
{
	if (!bytes || bytes[0] == '\0')
		return CGlobals::GetString().c_str();

	static boost::thread_specific_ptr< vector<char> > g_result_;
	if (!g_result_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		g_result_.reset(new vector<char>());
	}
	vector<char>& g_result = *g_result_;

	int nResultSize = 0;
	g_result.resize(256);

	if ((strcmp(srcEncoding, "HTML") == 0) && (dstEncoding == NULL || dstEncoding[0] == '\0'))
	{
		// from  HTML special character in ANSI code page to default encoding in NPL. 
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
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

					char result[12];
					int nResult = ::WideCharToMultiByte(CP_ACP, 0, &(wszText[0]), 1, result, 12, NULL, NULL);
					if (nResult > 0)
					{
						for (int k = 0; k < nResult; ++k)
						{
							g_result[nResultSize++] = result[k];
						}
					}
				}
				bNewChar = false;
			}
		}
#else
		OUTPUT_LOG("error: EncodingConvert not supported in the platform\n");
#endif
	}
	else if ((strcmp(dstEncoding, "HTML") == 0) && (srcEncoding == NULL || srcEncoding[0] == '\0'))
	{
		wstring w_srcText = AnsiToWideChar(bytes);
		int nSize = (int)(w_srcText.size());
		WCHAR wChar;
		int i = 0;
		for (int i = 0; i<nSize && (wChar = w_srcText[i]) != 0; i++)
		{
			if ((nResultSize + 10)>(int)g_result.size())
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
	}
	else if ((strcmp(srcEncoding, "utf-8") == 0) && (dstEncoding == NULL || dstEncoding[0] == '\0'))
	{
		string destText = UTF8ToAnsi(bytes);
		int nSize = (int)destText.size();
		if (g_result.size() < destText.size())
			g_result.resize(destText.size() + 10);
		for (int i = 0; i < nSize; ++i)
		{
			g_result[i] = destText[i];
			nResultSize++;
		}
	}
	else if ((strcmp(dstEncoding, "utf-8") == 0) && (srcEncoding == NULL || srcEncoding[0] == '\0'))
	{
		string destText = AnsiToUTF8(bytes);
		int nSize = (int)destText.size();
		if (g_result.size() < destText.size())
			g_result.resize(destText.size() + 10);
		for (int i = 0; i < nSize; ++i)
		{
			g_result[i] = destText[i];
			nResultSize++;
		}
	}
	g_result[nResultSize++] = '\0';
	return (const char*)(&(g_result[0]));
}


bool ParaEngine::StringHelper::CopyTextToClipboard(const string& text)
{
#ifdef WIN32
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(char) * (text.size() + 1));
		if (hBlock)
		{
			char *szText = (char*)GlobalLock(hBlock);
			if (szText)
			{
				CopyMemory(szText, &(text[0]), text.size() * sizeof(char));
				szText[(int)text.size()] = '\0';  // Terminate it
				GlobalUnlock(hBlock);
			}
			SetClipboardData(CF_TEXT, hBlock);
		}
		CloseClipboard();
		// We must not free the object until CloseClipboard is called.
		if (hBlock)
			GlobalFree(hBlock);
		return true;
	}
#endif
	return false;
}

const char* ParaEngine::StringHelper::GetTextFromClipboard()
{
	static boost::thread_specific_ptr< std::string > g_str_;
	if (!g_str_.get()) {
		// first time called by this thread
		// construct test element to be used in all subsequent calls from this thread
		g_str_.reset(new std::string());
	}
	std::string& g_str = *g_str_;

	bool bSucceed = false;
#ifdef WIN32
	if (OpenClipboard(NULL))
	{
		HANDLE handle = GetClipboardData(CF_TEXT);
		if (handle)
		{
			// Convert the ANSI string to Unicode, then
			// insert to our buffer.
			char *szText = (char*)GlobalLock(handle);
			if (szText)
			{
				g_str = szText;
				bSucceed = true;
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
#endif
	if (!bSucceed)
		g_str.clear();

	return g_str.c_str();
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

string ParaEngine::StringHelper::md5(const char* source, bool bBinary)
{
	ParaEngine::MD5 md5_hash;
	md5_hash.feed(source);
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
	if (utf8.empty())
	{
		outUtf16.clear();
		return true;
	}

	bool ret = false;

	const size_t utf16Bytes = (utf8.length() + 1) * sizeof(char16_t);
	char16_t* utf16 = (char16_t*)malloc(utf16Bytes);
	memset(utf16, 0, utf16Bytes);

	char* utf16ptr = reinterpret_cast<char*>(utf16);
	const UTF8* error = nullptr;

	if (llvm::ConvertUTF8toWide(2, utf8, utf16ptr, error))
	{
		outUtf16 = utf16;
		ret = true;
	}

	free(utf16);

	return ret;
}

bool ParaEngine::StringHelper::UTF16ToUTF8(const std::u16string& utf16, std::string& outUtf8)
{
	if (utf16.empty())
	{
		outUtf8.clear();
		return true;
	}
	outUtf8.clear();
	return llvm::convertUTF16ToUTF8String(utf16, outUtf8);
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
