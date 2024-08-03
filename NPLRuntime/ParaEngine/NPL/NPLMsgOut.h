#pragma once

#include "NPLWriter.h"
#include "NPLMsgHeader.h"
#include "util/PoolBase.h"
#include "util/StringBuilder.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#endif
#include <boost/core/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>


namespace NPL
{
	/**
	* An outgoing message to be sent by a socket. Use CNPLMsgOut_gen to write content to the msg. 
	* this is output pending send queue item data struct. 
	* e.g. 
	NPLMsgOut_ptr msg_out = NPLMsgOut_ptr(new NPLMsgOut());
	CNPLMsgOut_gen writer(*msg_out);
	...
	const string& msg_str = msg_out.ToString();
	*/
	class NPLMsgOut : 
		public ParaEngine::PoolBase<NPLMsgOut>,
		public ParaEngine::intrusive_ptr_single_thread_base,
		private boost::noncopyable
	{
	public:
		virtual ~NPLMsgOut(){};

		/// The content to be sent in the reply.
		ParaEngine::StringBuilder m_msg;

		/** if message is empty */
		bool empty() {return m_msg.empty();}

		/** get the internal string buffer */
		ParaEngine::StringBuilder& GetBuffer() {return m_msg;};
	};

	/**
	* for generating raw NPL output message. It generates the messages like below. 
	<verbatim>
	A (g1)script/hello.lua NPL/1.0
	rts:r1
	User-Agent:NPL

	14:"hello world!"
	</verbatim>
	CNPLMsgOut_gen	writer;
	writer.AddFirstLine(filename);
	writer.AddHeaderPair("rts", "r1");
	writer.AddHeaderPair("User-Agent", "NPL");
	writer.AddMsgBody("msg={\"hello world!\"}");
	writer.AddBody("\"hello world!\"");
	const string& msg_str = writer.ToString();
	*/
	class CNPLMsgOut_gen : public CNPLBufWriter
	{
	public:
		/** whether the first line of the NPL protocol is in ansi code page. 
		because NPL protocol is compatible with HTTP protocol in ansi mode, some interception web cache servers may cache request even the port number is not 80, 
		so client side applications are encouraged to disable ansi mode. */
		static bool g_enable_ansi_mode;

		/** the internal buffer reserved size. */
		CNPLMsgOut_gen(NPLMsgOut& msg, int nReservedSize=-1):CNPLBufWriter(msg.GetBuffer(), nReservedSize){}
		
		void AddFirstLine(const char* method, const char* uri);
		/** @param file_id: if not -1, the filename will be sent as id */
		void AddFirstLine(const NPLFileName& file_name, int file_id=-1, const char* method = nullptr);

		/** add a header name value pair to the message. */
		void AddHeaderPair(const char* name, const char* value);
		/** add binary data to the message body. 
		* @param pData: the binary data
		* @param nSize: number of bytes in data. if -1, pData is considered as a string,and strlen() is used. 
		* @param nCompressionlevel: compression level, which is an integer in the range of -1 to 9. default to 0, which means no compression. 
		* We usually choose to compress using -1, when data is larger than a given threshold value. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		void AddBody(const char* pData, int nSize=-1, int nCompressionlevel = 0);

		
		/** Add a message string to the message body.
		* @param pMsg: if should be in the format "msg={some_data_here}"
		*	when transmitted, only "some_data_here" is added to the message body. 
		* @param nSize: number of bytes in msg. if -1, msg is considered as a string,and strlen() is used.  
		* @param nCompressionlevel: compression level, which is an integer in the range of -1 to 9. default to 0, which means no compression. 
		* We usually choose to compress using -1, when data is larger than a given threshold value. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		void AddMsgBody( const char* pMsg, int nSize=-1, int nCompressionlevel = 0);
	};
}



