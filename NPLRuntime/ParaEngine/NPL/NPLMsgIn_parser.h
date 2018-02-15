#pragma once
#include "NPLMsgIn.h"
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

namespace NPL
{
	struct NPLMsgIn;

	/** Parser for incoming requests. 
	---++ NPL Message Format
	ALL NPL messages are created to loosely fit the HTTP message structure that the standard calls the generic message format. 
	I say loosely fit, because NPL messages are made as compact as possible, such that the minimum overhead for 
	sending a message is less than 15 bytes, which is negligible compared to the TCP/IP overhead. 

	*quick sample*

	<verbatim>
	A (g1)script/hello.lua NPL/1.0
	rts:r1
	User-Agent:NPL

	16:{"hello world!"}
	</verbatim>

	*a concise message*

	<verbatim>
	A (g1)12

	16:{"hello world!"}
	</verbatim>

	The NPL generic message format is as follows:

	<verbatim>
	<start-line>
	[<message-headers>]
	<empty-line>
	[<message-body>]
	</verbatim>
	You can see that this is pretty much the same as the format used for e-mail messages and for Usenet newsgroup messages too: headers, an empty line and then a message body. 
	All text lines are terminated with the "\n" CRLF control character (NOT the standard "\r\n"). 
	the empty line contains just those two characters and nothing else. The headers are always sent as regular text; 
	the body, however, may be either text or 8-bit binary information, depending on the nature of the data to be sent.

	---+++ start line
	The start line is a special text within the first line of that message that conveys the nature of the message. 
	It consists of the method to be applied to the NPL file, the identifier of the NPL file, and the protocol version in use.
	<verbatim>
	Method SP NPLFileName SP NPL-Version CRLF
	</verbatim>
	See some examples below 
	<verbatim>
	-- ACTivate a given file in a given runtime state on the target machine using NPL protocol 1.0. 
	ACT (runtime_name)script/hello.lua NPL/1.0
	-- this is a shortcut to above, where A means activate method; The file name is replaced by its Numeric identifier(N), which is dynamically known by both the sender and receiver; the NPL version is omitted. 
	A (g1)12
	-- This is like HTTP, but is NOT supported in current version.
	GET script/hello.lua NPL/1.0
	</verbatim>

	---++++ ACT method
	The ACT method activates a given file in a given runtime state on the target machine.

	---+++ message-headers
	Message-headers are just name value pairs and one pair on each line. There are many meaningful names that the NPL used to interprete the message. 
	For example, we can specify the message length and the type. 
	<verbatim>
	From:r1
	Length:100
	</verbatim>
	In most cases, message-length are omitted. And the message-body length is determined by parsing the message-body as NPL table string.

	---+++ message-body
	the message-body contains the length (number of bytes after ":" or ">") and the binary or text data.  
	If the character after number of bytes is ":", the message body is a non-compressed plain-text message
	If the character after number of bytes is ">", the message body is a compressed message and NPLCodec should be used to decode the message body. 
	Non-compressed:
	<verbatim>
	msg_length:{name=value, value, value, {name=value, value}}
	</verbatim>
	Compressed:
	<verbatim>
	msg_length>XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}
	</verbatim>
	Some examples are "0:", "16:{\"hello world!\"}"

	---+++ Example messages

	---++++ Example 1: a simple hello world

	<verbatim>
	NPL.activate("(r1)script/hello.lua", {"hello world!"});
	-- Output Message with NPL identifier
	A (r1)12 NPL/1.0

	16:{"hello world!"}

	-- Output Message without preinstalled  NPL identifier
	A (r1)script/hello.lua

	16:{"hello world!"}
	</verbatim>

	---++++ Example 2: a hello world with user-defined headers

	<verbatim>
	NPL.activate("(g1)script/hello.lua", {header={rts="r1",User-Agent="NPL"}, "hello world!"});
	-- Output Message 
	A (g1)script/hello.lua NPL/1.0
	rts:r1
	User-Agent:NPL

	16:{"hello world!"}
	</verbatim>
	*/
	class NPLMsgIn_parser
	{
	public:
		enum Consume_Result
		{
			c_res_indeterminate,
			c_res_code_body,
			c_res_true,
			c_res_false,
		};

		/// Construct ready to parse the NPLMsgIn method.
		NPLMsgIn_parser();

		/// Reset to initial parser state.
		void reset();

		/// Parse some data. The tribool return value is true when a complete NPLMsgIn
		/// has been parsed, false if the data is invalid, indeterminate when more
		/// data is required. The InputIterator return value indicates how much of the
		/// input has been consumed.
		template <typename InputIterator>
		boost::tuple<boost::tribool, InputIterator> parse(NPLMsgIn& req,
			InputIterator begin, InputIterator end)
		{
			Consume_Result result = c_res_indeterminate;
			while (begin != end && (result == c_res_indeterminate))
			{
				result = consume(req, *begin++);
			}
			if(begin != end && result == c_res_code_body)
			{
				// we can read to end or length instead of doing the lexical work one char at a time. 
				int nOldSize = req.m_code.size();
				int nByteCount = end-begin;
				if(req.m_nLength < (nOldSize+nByteCount))
				{
					nByteCount = req.m_nLength - nOldSize;
				}
				req.m_code.resize(nOldSize+nByteCount);
				memcpy(&(req.m_code[nOldSize]), begin, nByteCount);
				begin = begin + nByteCount;
				if (req.m_nLength ==(int)req.m_code.size()) 
				{
					result = c_res_true;
				}
			}
			if(result == c_res_true)
			{
				if(m_bCompressed)
					Decompress(req);
				reset();
				boost::tribool result_ = true;
				return boost::make_tuple(result_, begin);
			}
			else if(result == c_res_false)
			{
				boost::tribool result_ = false;
				return boost::make_tuple(result_, begin);
			}
			else
			{
				boost::tribool result_ = boost::indeterminate;
				return boost::make_tuple(result_, begin);
			}
		}

	private:
		/// Handle the next character of input.
		Consume_Result consume(NPLMsgIn& req, char input);

		/** decompress the message body*/
		void Decompress(NPLMsgIn& req);

		/** Decode the message body*/
		void Decode(NPLMsgIn& req);

		/// Check if a byte is an HTTP character.
		static bool is_char(int c);

		/// Check if a byte is an HTTP control character.
		static bool is_ctl(int c);

		/// Check if a byte is defined as an HTTP tspecial character.
		static bool is_tspecial(int c);

		/// Check if a byte is a digit.
		static bool is_digit(int c);

		/// The current state of the parser.
		enum state
		{
			method_start,
			method,
			uri,
			uri_rts_name,
			npl_version_n,
			npl_version_p,
			npl_version_l,
			npl_version_slash,
			npl_version_major,
			npl_version_minor,
			header_line_start,
			header_lws,
			header_name,
			header_value,
			code_length,
			code_body,
			uri_http,
		} state_;
		/** whether the message body is compressed. */
		bool m_bCompressed;
	};
}


