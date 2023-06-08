#pragma once
#include "NPLMsgHeader.h"
namespace NPL 
{
	/**
	A NPL msg received from a socket.
	*/
	struct NPLMsgIn
	{
		/// in request this is "A", "GET"; in HTTP response, this is "HTTP/1.1"
		std::string method;
		/// NPL runtime state name
		std::string m_rts_name;
		/// NPL or Dll file name (uri); in http response this is "OK", "error".
		std::string m_filename; 
		int m_n_filename; // the filename number agreed upon dynamically by the sender and receiver. In http response this is the return code.
		int npl_version_major;
		int npl_version_minor;
		
		/// optional headers
		std::vector<NPLMsgHeader> headers;
		
		/// number of bytes in m_code
		int m_nLength;
		/// msg body
		std::string m_code;

		/** the connection object from which this message is received. */
		union {
			CNPLConnection * m_pConnection;
			CNPLUDPRoute * m_pRoute;
		};

		bool bUseConnection;

		NPLMsgIn() : bUseConnection(true), m_pConnection(nullptr) {};
	public:
		void reset()
		{
			method.clear();
			m_rts_name.clear();
			m_n_filename = 0;
			m_filename.clear();

			headers.clear();

			m_nLength = 0;
			m_code.clear();
		}

		bool IsNPLFileActivation()
		{
			return (method.size() > 0 && (((unsigned char)(method[0])) > 127 || method == "A" || method == "B"));
		}
	};

} // NPL
