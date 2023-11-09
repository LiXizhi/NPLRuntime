//-----------------------------------------------------------------------------
// Class:	NPLMsgOut (TODO)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.4
// Desc:  This file used be from boost asio HTTP sample's reply.cpp file. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLCommon.h"
#include "NPLMsgOut.h"
#include "NPLCodec.h"
#include "util/StringHelper.h"
#include <boost/lexical_cast.hpp>

#ifdef _DEBUG
	/** @def define this to include version string NPL/1.0 on the first line of each NPL message. */
	#define NPL_PROTOCOL_USE_VERSION
#endif

namespace NPL{
	bool CNPLMsgOut_gen::g_enable_ansi_mode = true;

	namespace status_strings {
		const std::string ok =
			"NPL/1.0 200 OK\r\n";
		const std::string created =
			"NPL/1.0 201 Created\r\n";
		const std::string accepted =
			"NPL/1.0 202 Accepted\r\n";
		const std::string no_content =
			"NPL/1.0 204 No Content\r\n";
		const std::string multiple_choices =
			"NPL/1.0 300 Multiple Choices\r\n";
		const std::string moved_permanently =
			"NPL/1.0 301 Moved Permanently\r\n";
		const std::string moved_temporarily =
			"NPL/1.0 302 Moved Temporarily\r\n";
		const std::string not_modified =
			"NPL/1.0 304 Not Modified\r\n";
		const std::string bad_request =
			"NPL/1.0 400 Bad Request\r\n";
		const std::string unauthorized =
			"NPL/1.0 401 Unauthorized\r\n";
		const std::string forbidden =
			"NPL/1.0 403 Forbidden\r\n";
		const std::string not_found =
			"NPL/1.0 404 Not Found\r\n";
		const std::string internal_server_error =
			"NPL/1.0 500 Internal Server Error\r\n";
		const std::string not_implemented =
			"NPL/1.0 501 Not Implemented\r\n";
		const std::string bad_gateway =
			"NPL/1.0 502 Bad Gateway\r\n";
		const std::string service_unavailable =
			"NPL/1.0 503 Service Unavailable\r\n";
	} // namespace status_strings

	namespace misc_strings {
		const char name_value_separator[] = { ':', ' ' };
		const char crlf[] = { '\r', '\n' };
	} // namespace misc_strings


	//////////////////////////////////////////////////////////////////////////
	//
	// NPL output message generator. 
	//
	//////////////////////////////////////////////////////////////////////////

	namespace const_strings {
		const std::string npl_version = "NPL/1.0";
		const std::string name_value_separator = ":";
		const std::string crlf = "\n";
	}

	void CNPLMsgOut_gen::AddFirstLine( const char* method, const char* uri)
	{
		Append(method);
		if(uri != NULL)
		{
			Append(" ");
			Append(uri);
		}
#ifdef NPL_PROTOCOL_USE_VERSION
		Append(" ");
		Append(const_strings::npl_version);
#endif
		Append(const_strings::crlf);
	}

	void CNPLMsgOut_gen::AddFirstLine( const NPLFileName& file_name, int file_id, const char* method)
	{
		if (method)
		{
			Append(method, 2);
		}
		else
		{
			if (g_enable_ansi_mode)
			{
				Append("A ");
			}
			else
			{
				// 0xff is the binary version for NPL append
				unsigned char method_[] = { 0xff, ' ' };
				Append((const char*)method_, 2);
			}
		}

		if(!file_name.sRuntimeStateName.empty())
		{
			Append("(");
			Append(file_name.sRuntimeStateName);
			Append(")");
		}
		if(file_id > 0 && file_id < 65535)
		{
			char buff[16];
			int nLen = ParaEngine::StringHelper::fast_itoa(file_id, buff, 16);
			Append(buff, nLen);
		}
		else if(!file_name.sRelativePath.empty())
		{
			Append(file_name.sRelativePath);
		}
#ifdef NPL_PROTOCOL_USE_VERSION
		Append(" ");
		Append(const_strings::npl_version);
#endif
		Append(const_strings::crlf);
	}

	void CNPLMsgOut_gen::AddHeaderPair( const char* name, const char* value )
	{
		Append(name);
		Append(const_strings::name_value_separator);
		Append(value);
		Append(const_strings::crlf);
	}

	void CNPLMsgOut_gen::AddBody( const char* pData, int nSize, int nCompressionlevel)
	{
		PE_ASSERT(pData!=0);
		Append(const_strings::crlf);
		if(nSize<0)
			nSize = strlen(pData);

		char tmp[16];
		if(nCompressionlevel == 0)
		{
			// for non-compressed messages. "%d:"
			snprintf(tmp, 15, "%d:", nSize);
			Append(tmp);

			if(nSize>0)
				Append(pData, nSize);
		}
		else
		{
			// for compressed messages. "%d>"
			std::string output;
			if(NPLCodec::Compress(output, pData, nSize, nCompressionlevel) == 1)
			{
				int nCompressedSize = (int)(output.size());
				snprintf(tmp, 15, "%d>", nCompressedSize);
				Append(tmp);
				Append(output.c_str(), nCompressedSize);
			}
			else
			{
				OUTPUT_LOG("warning: AddBody Compress error\n");
				Append("0:");
			}
		}
	}

	void CNPLMsgOut_gen::AddMsgBody( const char* pData, int nSize, int nCompressionlevel)
	{
		PE_ASSERT(pData!=0);
		if(nSize<=0)
			nSize = strlen(pData);

		if(nSize > 5 &&
			pData[0] == 'm' && pData[1] == 's' && pData[2] == 'g' && pData[3] == '='  && pData[4] == '{' && 
			pData[nSize-1] == '}')
		{
			AddBody(pData+5, nSize-6, nCompressionlevel);
		}
		else
		{
			OUTPUT_LOG("warning: AddMsgBody input can only be \"msg={}\"\n");
			Append("0:");
		}
	}

} 

