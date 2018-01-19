//-----------------------------------------------------------------------------
// Class:	NPLWriter
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.10.23
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLHelper.h"
#include "util/StringBuilder.h"
#include "util/StringHelper.h"
#include "NPLWriter.h"

#ifdef PARAENGINE_CLIENT
	#include "memdebug.h"
#endif

extern "C"
{
#include "lua.h"
}

//////////////////////////////////////////////////////////////////////////
//
// NPL writer
//
//////////////////////////////////////////////////////////////////////////

template <typename StringBufferType>
NPL::CNPLWriterT<StringBufferType>::CNPLWriterT(int nReservedSize) 
	: m_sCode(m_buf), m_bBeginAssignment(false),m_nTableLevel(0)
{
	if(nReservedSize>0)
		m_sCode.reserve(nReservedSize);
}

template <typename StringBufferType>
NPL::CNPLWriterT<StringBufferType>::CNPLWriterT( StringBufferType& buff_ , int nReservedSize )
: m_sCode(buff_), m_bBeginAssignment(false),m_nTableLevel(0)
{
	if(nReservedSize>0)
		m_sCode.reserve(nReservedSize);
}

template <typename StringBufferType>
NPL::CNPLWriterT<StringBufferType>::~CNPLWriterT()
{
}


template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::Reset( int nReservedSize /*= -1*/ )
{
	m_sCode.clear();
	if(nReservedSize>0)
		m_sCode.reserve(nReservedSize);
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteName( const char* name, bool bUseBrackets /*= false*/ )
{
	if(name)
	{
		m_bBeginAssignment = true;
		if(!bUseBrackets)
		{
			m_sCode += name;
		}
		else
		{
			m_sCode += "[\"";
			m_sCode += name;
			m_sCode += "\"]";
		}
	}
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteValue( const char* value, bool bInQuotation/*=true*/ )
{
	if(value == NULL)
	{
		return WriteNil();
	}
	if(m_bBeginAssignment)
	{
		m_sCode += "=";
	}
	if(bInQuotation)
	{
		NPLHelper::EncodeStringInQuotation(m_sCode, (int)m_sCode.size(), value);
	}
	else
	{
		m_sCode += value;
	}
	if(m_nTableLevel>0)
		m_sCode += ",";
	m_bBeginAssignment = false;
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteValue( const char* buffer, int nSize, bool bInQuotation/*=true*/ )
{
	if(buffer == NULL)
	{
		return WriteNil();
	}
	if(m_bBeginAssignment)
	{
		m_sCode += "=";
	}
	if(bInQuotation)
	{
		NPLHelper::EncodeStringInQuotation(m_sCode, (int)m_sCode.size(), buffer, nSize);
	}
	else
	{
		size_t nOldSize = m_sCode.size();
		m_sCode.resize(nOldSize+nSize);
		memcpy((void*)(m_sCode.c_str()+nOldSize), buffer, nSize);
	}
	if(m_nTableLevel>0)
		m_sCode += ",";
	m_bBeginAssignment = false;
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteValue( double value )
{
	char buff[40];
	int nLen = ParaEngine::StringHelper::fast_dtoa(value, buff, 40, 5); // similar to "%.5f" but without trailing zeros. 
	WriteValue(buff, nLen, false);
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteValue( const string& sStr, bool bInQuotation/*=true*/ )
{
	WriteValue(sStr.c_str(), (int)sStr.size(), bInQuotation);
}
template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::WriteNil()
{
	WriteValue("nil", false);
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::BeginTable()
{
	m_sCode += m_bBeginAssignment ? "={" : "{";
	m_nTableLevel++;
	m_bBeginAssignment = false;
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::EndTable()
{
	m_sCode += "}";
	if((--m_nTableLevel)>0)
		m_sCode += ",";
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::Append( const char* text )
{
	if(text)
		m_sCode += text;
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::Append(const char* pData, int nSize)
{
	m_sCode.append(pData, nSize);
}

template <typename StringBufferType>
void NPL::CNPLWriterT<StringBufferType>::Append( const string& text )
{
	m_sCode += text;
}

template <typename StringBufferType>
char* NPL::CNPLWriterT<StringBufferType>::AddMemBlock( int nSize )
{
	if(nSize>0)
	{
		m_sCode.resize(m_sCode.size() + nSize);
		return &(m_sCode[(int)(m_sCode.size() - nSize)]);
	}
	else
		return NULL;
}

template <typename StringBufferType>
const StringBufferType& NPL::CNPLWriterT<StringBufferType>::GetNilMessage()
{
	static const StringBufferType g_str = "msg=nil;";
	return g_str;
}

template <typename StringBufferType>
const StringBufferType& NPL::CNPLWriterT<StringBufferType>::GetEmptyMessage()
{
	static const StringBufferType g_str = "msg={};";
	return g_str;
}

namespace NPL
{
	// instantiate class so that no link errors when seperating template implementation to hpp file. 
	template class CNPLWriterT< ParaEngine::StringBuilder >; 
	// instantiate class so that no link errors when seperating template implementation to hpp file. 
	template class CNPLWriterT< std::string >; 
}