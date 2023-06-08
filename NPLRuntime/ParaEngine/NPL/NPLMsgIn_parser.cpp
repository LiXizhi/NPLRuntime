//-----------------------------------------------------------------------------
// Class:	NPLMsgIn_parser(TODO)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.4
// Desc:  This file used be from boost asio HTTP sample's request_parser.cpp file. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLCommon.h"
#include "NPLMsgIn_parser.h"
#include "NPLCodec.h"
#include "NPLMsgIn.h"

namespace NPL
{
	NPLMsgIn_parser::NPLMsgIn_parser()
		: state_(method_start)
	{
	}

	void NPLMsgIn_parser::reset()
	{
		state_ = method_start;
	}

	NPLMsgIn_parser::Consume_Result NPLMsgIn_parser::consume(NPLMsgIn& req, char input)
	{
		switch (state_)
		{
		case method_start:
			if (input == ' ' || input == '\r' || input == '\n')
			{
				return c_res_indeterminate;
			}
			else
			{
				// allowing only ansi char and 0xff as method name
				if (((unsigned char)input) != 0xff && (input == '\0' || !is_char(input))) {
					// if the first byte is "\0" or non char we will switch to custom protocol by returning false.
					return c_res_false;
				}
				state_ = method;
				req.reset();
				req.method.push_back(input);
				return c_res_indeterminate;
			}
		case method:
			if (input == ' ')
			{
				// this fix a bug when http url contains the '(', it may be mis-interpreted. 
				state_ = req.IsNPLFileActivation() ? uri : uri_http;
				return c_res_indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return c_res_false;
			}
			else
			{
				req.method.push_back(input);
				return c_res_indeterminate;
			}
		case uri_http:
			if (input == ' ')
			{
				state_ = npl_version_n;
				return c_res_indeterminate;
			}
			else if (input == '\n' || input == '\r')
			{
				state_ = header_line_start; // version is omitted on first line. 
				return c_res_indeterminate;
			}
			else
			{
				req.m_n_filename = 0;
				req.m_filename.push_back(input);
				return c_res_indeterminate;
			}
		case uri:
			if (is_ctl(input))
			{
				req.npl_version_major = NPL_VERSION_MAJOR;
				req.npl_version_minor = NPL_VERSION_MINOR;
				state_ = header_line_start; // version is omitted on first line. 
				return c_res_indeterminate;
			}
			else if (input == '(')
			{
				state_ = uri_rts_name;
				return c_res_indeterminate;
			}
			else if (is_digit(input))
			{
				req.m_n_filename = req.m_n_filename * 10 + input - '0';
				req.m_filename.push_back(input);
				return c_res_indeterminate;
			}
			else if (input == ' ')
			{
				state_ = npl_version_n;
				return c_res_indeterminate;
			}
			else
			{
				req.m_n_filename = 0;
				req.m_filename.push_back(input);
				return c_res_indeterminate;
			}
		case uri_rts_name:
			if (input == ')')
			{
				state_ = uri;
				return c_res_indeterminate;
			}
			else if(is_ctl(input))
			{
				return c_res_false;
			}
			else
			{
				req.m_rts_name.push_back(input);
				return c_res_indeterminate;
			}
		case npl_version_n:
			if (input == '/')
			{
				req.npl_version_major = 0;
				req.npl_version_minor = 0;
				state_ = npl_version_major;
				return c_res_indeterminate;
			}
			else if (input == '\n')
			{
				state_ = header_line_start;
				return c_res_indeterminate;
			}
			else if (input == '\r')
				return c_res_indeterminate;
			else if (is_char(input))
			{
				// any character is accepted, such as "NPL", "HTTP"
				if(req.m_filename.empty() && req.m_n_filename > 0)
				{
					// for http response, this stores the third parameter "OK", "Error"
					req.m_filename.push_back(input);
				}
				return c_res_indeterminate;
			}
			else
			{
				return c_res_false;
			}
		case npl_version_major:
			if (is_digit(input))
			{
				req.npl_version_major = req.npl_version_major * 10 + input - '0';
				return c_res_indeterminate;
			}
			else if (input == '.')
			{
				state_ = npl_version_minor;
				return c_res_indeterminate;
			}
			else
			{
				return c_res_false;
			}
		case npl_version_minor:
			if (is_digit(input))
			{
				req.npl_version_minor = req.npl_version_minor * 10 + input - '0';
				return c_res_indeterminate;
			}
			else if (input == '\n')
			{
				state_ = header_line_start;
				return c_res_indeterminate;
			}
			else if (input == '\r')
				return c_res_indeterminate;
			else
			{
				return c_res_false;
			}
		case header_line_start:
			if (input == '\n')
			{
				// continue to read the code length
				state_ = code_length;

				if(req.method.size()>2 && req.method != "npl")
				{
					// only do check header if method has at least three characters. 
					int nCount = (int)req.headers.size();
					for (int i=0;i <nCount; ++i)
					{
						if(req.headers[i].name == "Content-Length")
						{
							int nLength = atoi(req.headers[i].value.c_str());
							if(nLength>0)
							{
								state_ = code_body;
								req.m_nLength = nLength;
								req.m_code.reserve(req.m_nLength+1);
								m_bCompressed = false;
								return c_res_code_body;
							}
							else
							{
								break;
							}
						}
					}
					m_bCompressed = false;
					reset();
					return c_res_true;
				}
				
				return c_res_indeterminate;
			}
			else if (input == '\r')
				return c_res_indeterminate;
			else if (!req.headers.empty() && (input == ' ' || input == '\t'))
			{
				state_ = header_lws;
				return c_res_indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return c_res_false;
			}
			else
			{
				req.headers.push_back(NPLMsgHeader());
				req.headers.back().name.push_back(input);
				state_ = header_name;
				return c_res_indeterminate;
			}
		case header_lws:
			if (input == '\n')
			{
				state_ = header_line_start;
				return c_res_indeterminate;
			}
			else if (input == '\r')
				return c_res_indeterminate;
			else if (input == ' ' || input == '\t')
			{
				return c_res_indeterminate;
			}
			else if (is_ctl(input))
			{
				return c_res_false;
			}
			else
			{
				state_ = header_value;
				req.headers.back().value.push_back(input);
				return c_res_indeterminate;
			}
		case header_name:
			if (input == ':')
			{
				state_ = header_value;
				return c_res_indeterminate;
			}
			else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
			{
				return c_res_false;
			}
			else
			{
				req.headers.back().name.push_back(input);
				return c_res_indeterminate;
			}
		case header_value:
			if (input == '\n')
			{
				state_ = header_line_start;
				return c_res_indeterminate;
			}
			else if (input == '\r')
				return c_res_indeterminate;
			else if (is_ctl(input))
			{
				return c_res_false;
			}
			else
			{
				string& value_ = req.headers.back().value;
				if( !(value_.empty() && (input == ' ' || input == '\t')) )
					value_.push_back(input);
				return c_res_indeterminate;
			}
		case code_length:
			if(is_digit(input))
			{
				req.m_nLength = req.m_nLength * 10 + input - '0';
				return c_res_indeterminate;
			}
			else if(input == ':' || input == '>')
			{
				state_ = code_body;
				m_bCompressed = (input == '>');
				if(req.m_nLength == 0)
				{
					reset();
					return c_res_true;
				}
				else
				{
					req.m_code.reserve(req.m_nLength+1);
					return c_res_code_body;
				}
			}
		case code_body:
			if(req.m_nLength>(int)req.m_code.size())
			{
				// I used  a special return value c_res_code_body to indicate that code body is met, 
				// so that we can read to end or length instead of doing the lexical work by char at a time. 
				req.m_code.push_back(input);
				if (req.m_nLength >(int)req.m_code.size()) 
					return c_res_code_body;
				else
				{
					reset();
					return c_res_true;
				}
			}
		default:
			return c_res_false;
		}
	}

	bool NPLMsgIn_parser::is_char(int c)
	{
		return c >= 0 && c <= 127;
	}

	bool NPLMsgIn_parser::is_ctl(int c)
	{
		return (c >= 0 && c <= 31) || c == 127;
	}

	bool NPLMsgIn_parser::is_tspecial(int c)
	{
		switch (c)
		{
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
		}
	}

	bool NPLMsgIn_parser::is_digit(int c)
	{
		return c >= '0' && c <= '9';
	}

	void NPLMsgIn_parser::Decode(NPLMsgIn& req)
	{
		int nSize = (int)(req.m_code.size());
		if(nSize > 0)
		{
			char* pSrc = &(req.m_code[0]);
			// perform decoding in place and use size as public key. 
			NPLCodec::Decode(pSrc, pSrc, nSize, nSize);
		}
	}

	void NPLMsgIn_parser::Decompress(NPLMsgIn& req)
	{
		int nSize = (int)(req.m_code.size());
		if(nSize > 0)
		{
			string src = req.m_code;
			req.m_code.clear();
			// perform decoding in place and use size as public key. 
			if(NPLCodec::Decompress(req.m_code, src.c_str(), (int)(src.size()) ) != 1)
			{
				// error occured during decompression. 
				req.m_code.clear();
			}
		}
	}
} // NPL
