//-----------------------------------------------------------------------------
// Class:	http utility
// Authors:	LiXizhi ported from http://www.koders.com/zeitgeist/cpp/ (appears to be libjingle project)
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2008.12.14
// Desc: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "HttpUtility.h"
#include "util/MD5.h"

using namespace ParaEngine;

static int HexPairValue(const char * code) {
	int value = 0;
	const char * pch = code;
	for (;;) {
		int digit = *pch++;
		if (digit >= '0' && digit <= '9') {
			value += digit - '0';
		}
		else if (digit >= 'A' && digit <= 'F') {
			value += digit - 'A' + 10;
		}
		else if (digit >= 'a' && digit <= 'f') {
			value += digit - 'a' + 10;
		}
		else {
			return -1;
		}
		if (pch == code + 2)
			return value;
		value <<= 4;
	}
}

int UrlDecode(const char *source, char *dest)
{
	char * start = dest;

	while (*source) {
		switch (*source) {
	case '+':
		*(dest++) = ' ';
		break;
	case '%':
		if (source[1] && source[2]) {
			int value = HexPairValue(source + 1);
			if (value >= 0) {
				*(dest++) = value;
				source += 2;
			}
			else {
				*dest++ = '?';
			}
		}
		else {
			*dest++ = '?';
		}
		break;
	default:
		*dest++ = *source;
		}
		source++;
	}

	*dest = 0;
	return (int)(dest - start);
}  

int UrlEncode(const char *source, char *dest, unsigned int max)  
{
	static const char *digits = "0123456789ABCDEF";
	unsigned char ch;
	unsigned int len = 0;
	char *start = dest;

	while (len < max - 4 && *source)
	{
		ch = (unsigned char)*source;
		if (*source == ' ') {
			*dest++ = '+';
		}
		else if (isalnum(ch) || strchr("-_.!~*'()", ch)) {
			*dest++ = *source;
		}
		else {
			*dest++ = '%';
			*dest++ = digits[(ch >> 4) & 0x0F];
			*dest++ = digits[       ch & 0x0F];
		}  
		source++;
	}
	*dest = 0;
	return (int)(start - dest);
}

/** encode the input byte array into string presentation that only contains hex letters. 
* @param output: the output is always twice as big as the source. 
* @param source: input buffer pointer
* @param nSrcSize: input buffer size. 
*/
void SimpleEncode(string& output, const unsigned char *source, int nSrcSize)  
{
	output.resize(nSrcSize*2);
	static const char *digits = "0123456789ABCDEF";
	unsigned char ch;
	
	for (int i=0,j=0;i<nSrcSize;++i, j=j+2)
	{
		ch = source[i];
		output[j] = digits[(ch >> 4) & 0x0F];
		output[j+1] = digits[       ch & 0x0F];
	}
}

std::string CHttpUtility::UrlDecode(const std::string & encoded) {
	const char * sz_encoded = encoded.c_str();
	size_t needed_length = encoded.length();
	for (const char * pch = sz_encoded; *pch; pch++) {
		if (*pch == '%')
			needed_length += 2;
	}
	needed_length += 10;
	char stackalloc[64];
	char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
		(char *)malloc(needed_length) : stackalloc;
	::UrlDecode(encoded.c_str(), buf);
	std::string result(buf);
	if (buf != stackalloc) {
		free(buf);
	}
	return result;
}


std::string CHttpUtility::UrlEncode( const char* sz_decoded, int nSize )
{
	size_t needed_length = nSize * 3 + 3;
	char stackalloc[64];
	char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
		(char *)malloc(needed_length) : stackalloc;
	::UrlEncode(sz_decoded, buf, (int)needed_length);
	std::string result(buf);
	if (buf != stackalloc) {
		free(buf);
	}
	return result;
}

std::string CHttpUtility::UrlEncode(const std::string & decoded) 
{
	size_t needed_length = decoded.length() * 3 + 3;
	char stackalloc[64];
	char * buf = needed_length > sizeof(stackalloc)/sizeof(*stackalloc) ?
		(char *)malloc(needed_length) : stackalloc;
	::UrlEncode(decoded.c_str(), buf, (int)needed_length);
	std::string result(buf);
	if (buf != stackalloc) {
		free(buf);
	}
	return result;
}
////////////////////////////////////////////////////////////////////////////
//
// url building class
//
//////////////////////////////////////////////////////////////////////////

const char* ParaEngine::CUrlBuilder::GetParam( const string& name )
{
	vector<CNameValuePair>::iterator itCur, itEnd = m_params.end();
	for (itCur=m_params.begin(); itCur!=itEnd; ++itCur)
	{
		if(itCur->m_name == name)
		{
			return itCur->m_name.c_str();
		}
	}
	return NULL;
}

const string& ParaEngine::CUrlBuilder::ToString()
{
	if(m_bNeedRebuild)
	{
		RebuildUrl();
	}
	return m_url;
}

void ParaEngine::CUrlBuilder::SetBaseURL(const string & baseurl)
{
	m_bNeedRebuild = true;
	m_baseurl = baseurl;
};

void ParaEngine::CUrlBuilder::RebuildUrl()
{
	m_bNeedRebuild = false;
	m_url = m_baseurl;
	if(!m_params.empty())
	{
		m_url += "?";
		bool bFirst = true;
		vector<CNameValuePair>::iterator itCur, itEnd = m_params.end();
		for (itCur=m_params.begin(); itCur!=itEnd; ++itCur)
		{
			if(!(itCur->m_name.empty()))
			{
				if(!bFirst)
					m_url += "&";
				else
					bFirst = false;

				m_url += CHttpUtility::UrlEncode(itCur->m_name);
				m_url += "=";
				m_url += CHttpUtility::UrlEncode(itCur->m_value);
			}
		}
	}
}

void ParaEngine::CUrlBuilder::AppendParam( const string& name, const string& value )
{
	m_bNeedRebuild = true;
	m_params.push_back(CNameValuePair(name, value));
}

void ParaEngine::CUrlBuilder::InsertParam( int nIndex, const char* name, const char* value )
{
	m_bNeedRebuild = true;
	if((int)m_params.size() <= nIndex)
		m_params.resize(nIndex+1);
	if(name)
		m_params[nIndex].m_name = name;
	if(value)
		m_params[nIndex].m_value = value;
}

bool ParaEngine::CUrlBuilder::HasParams()
{
	return !(m_params.empty());
}

string ParaEngine::CHttpUtility::HashStringMD5( const char* input )
{
	ParaEngine::MD5 md5_hash;
	md5_hash.feed(input);
	return md5_hash.hex();
}

void ParaEngine::CHttpUtility::HashStringMD5(std::string& out, const char* input)
{
	ParaEngine::MD5 md5_hash;
	md5_hash.feed(input);
	out = md5_hash.hex();
}

