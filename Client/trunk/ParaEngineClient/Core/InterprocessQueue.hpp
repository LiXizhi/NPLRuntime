#pragma once
// Author: LiXizhi
// Date: 2010.4.21
#include <boost/interprocess/ipc/message_queue.hpp>
#ifdef WIN32
	// I used to write my own implementation of message queue, since the default one used by boost under win32 has some securities issues that does not work under win7.
	// this allows us to use IPC even in low integrity level process in vista and win7. 
	#include "ipc_message_queue.hpp"
#endif
#include <boost/shared_ptr.hpp>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include "util/Mutex.hpp"

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

namespace ParaEngine
{
#pragma region Msg Header
	/** it represents an interprocess message */
	struct InterProcessMessage
	{
	public:
		InterProcessMessage():m_nMsgType(0), m_nParam1(0), m_nParam2(0){}
		/// the method name string. It must not contain space ' '. if empty, it will be assigned as "NPL", we usually use it for the version or packet category of the message. 
		std::string m_method;
		/// the sender message queue name, this is only assigned and used when receiving a message. 
		std::string m_from;
		/// the receiver message queue name, this is only assigned and used when sending a message. 
		std::string m_to;
		/// the destination file name as in NPL.activate(filename,  ...)
		std::string m_filename;
		/// the code (data) as in NPL.activate(..., scode), so in most cases it is a message table "msg={}"
		std::string m_code;
		/// this is a optional message type, which could mean how the m_code is interpreted. However its meaning can be application specific. 
		DWORD m_nMsgType;
		/// this is another field to ease sending certain messages. 
		DWORD m_nParam1;
		/// this is another field to ease sending certain messages. 
		DWORD m_nParam2;

		void reset()
		{
			m_method.clear();
			m_from.clear();
			m_to.clear();
			m_filename.clear();
			m_code.clear();
			m_nMsgType = 0;
			m_nParam1 = 0;
			m_nParam2 = 0;
		}
	};
	typedef boost::shared_ptr<InterProcessMessage> InterProcessMessagePtr;
#pragma endregion Msg Header

#pragma region Incomming msg
	/** for reading an input message. */
	struct InterProcessMessageIn
	{
		InterProcessMessageIn(){reset();};
		/// number of bytes in m_code
		std::string m_method;
		std::string m_codes[3];
		int m_code_lengths[3];
		/// this is a optional message type, which could mean how the m_code is interpreted. However its meaning can be application specific. 
		DWORD m_nMsgType;
		/// this is another field to ease sending certain messages. 
		DWORD m_nParam1;
		/// this is another field to ease sending certain messages. 
		DWORD m_nParam2;

		void reset(){
			m_method.clear();
			for (int i=0;i<3;++i)
			{
				m_code_lengths[i] = 0;
				m_codes[i].clear();
			}
			m_nMsgType = 0;
			m_nParam1 = 0;
			m_nParam2 = 0;
		};

		/** convert to message */
		void ToMessage(InterProcessMessage& msg)
		{
			msg.m_method = m_method;
			msg.m_nMsgType = m_nMsgType;
			msg.m_nParam1 = m_nParam1;
			msg.m_nParam2 = m_nParam2;
			msg.m_from = m_codes[0];
			msg.m_filename = m_codes[1];
			msg.m_code = m_codes[2];
		}
	};
	typedef boost::shared_ptr<InterProcessMessageIn> InterProcessMessageInPtr;


	/** for parsing incoming messages. 
	---++ NPL InterProcessMessage Format
	The NPL generic message format is as follows:

	<verbatim>
	m_method[' '(1byte)] // (method name string such as "NPL" followed by a space)
	m_nMsgType(4Bytes)
	m_nParam1(4Bytes)
	m_nParam2(4Bytes)
	m_from_length:[...m_from_length bytes] // m_from name
	m_filename_length:[...m_filename_length bytes]
	m_code_length:[...m_code_length bytes]
	</verbatim>

	The following is an example message, where [DWORD] means 4 bytes
	<verbatim>
	NPL [DWORD][DWORD][DWORD]11:MyQueueName8:test.lua11:msg={p1=10}
	</verbatim>
	*/
	class CInterProcessMessageIn_parser
	{
	public:
		enum Consume_Result
		{
			c_res_indeterminate,
			c_res_true,
			c_res_false,
			c_res_code_body1,
			c_res_code_body2,
			c_res_code_body3,
		};

		/// Construct ready to parse the InterProcessMessageIn method.
		CInterProcessMessageIn_parser() : state_(method_start){};

		/// Reset to initial parser state.
		void reset(){state_ = method_start;};

		/// Parse some data. The tribool return value is true when a complete InterProcessMessageIn
		/// has been parsed, false if the data is invalid, indeterminate when more
		/// data is required. The InputIterator return value indicates how much of the
		/// input has been consumed.
		template <typename InputIterator>
		boost::tuple<boost::tribool, InputIterator> parse(InterProcessMessageIn& req,
			InputIterator begin, InputIterator end)
		{
			Consume_Result result = c_res_indeterminate;
			while (begin != end && (result == c_res_indeterminate))
			{
				result = consume(req, *begin++);
			}
			if(begin != end && result >= c_res_code_body1)
			{
				// we can read to end or length instead of doing the lexical work one char at a time. 
				int nIndex = result - c_res_code_body1;
				int nOldSize = req.m_codes[nIndex].size();
				int nByteCount = end-begin;
				if(req.m_code_lengths[nIndex] < (nOldSize+nByteCount))
				{
					nByteCount = req.m_code_lengths[nIndex] - nOldSize;
				}
				req.m_codes[nIndex].resize(nOldSize+nByteCount);
				memcpy(&(req.m_codes[nIndex][nOldSize]), begin, nByteCount);
				begin = begin + nByteCount;

				if (req.m_code_lengths[nIndex]==(int)req.m_codes[nIndex].size()) 
				{
					if(nIndex == (code_body1-code_length1-1)){
						result = c_res_true;
					}
					else{
						state_ = (state)(code_length1 + nIndex + 1);
						result = c_res_indeterminate;
					}
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
		Consume_Result consume(InterProcessMessageIn& req, char input)
		{
			switch (state_)
			{
			case method_start:
				if (!is_char(input))
				{
					return c_res_false;
				}
				else
				{
					state_ = method;
					req.reset();
					req.m_method.push_back(input);
					return c_res_indeterminate;
				}
			case method:
				if (input == ' ')
				{
					state_ = msg_type_1;
					return c_res_indeterminate;
				}
				else if (!is_char(input) || is_ctl(input))
				{
					return c_res_false;
				}
				else
				{
					req.m_method.push_back(input);
					return c_res_indeterminate;
				}
			case msg_type_1:
			case msg_type_2:
			case msg_type_3:
			case msg_type_4:
				{
					state_ = (state)(state_+1);
					req.m_nMsgType = ((req.m_nMsgType)<<8) + ((unsigned char)input);
					return c_res_indeterminate;
				}
			case msg_param1_1:
			case msg_param1_2:
			case msg_param1_3:
			case msg_param1_4:
				{
					state_ = (state)(state_+1);
					req.m_nParam1 = ((req.m_nParam1)<<8) + ((unsigned char)input);
					return c_res_indeterminate;
				}
			case msg_param2_1:
			case msg_param2_2:
			case msg_param2_3:
			case msg_param2_4:
				{
					state_ = (state)(state_+1);
					req.m_nParam2 = ((req.m_nParam2)<<8) + ((unsigned char)input);
					return c_res_indeterminate;
				}
			case code_length1:
			case code_length2:
			case code_length3:
				{
					int nIndex = (state_ - code_length1);
					if(is_digit(input))
					{
						req.m_code_lengths[nIndex] = req.m_code_lengths[nIndex] * 10 + input - '0';
						return c_res_indeterminate;
					}
					else if(input == ':' || input == '>')
					{
						m_bCompressed = (input == '>');
						if(req.m_code_lengths[nIndex]== 0)
						{
							state_ = (state)(state_+1);
							return (state_ == code_body1) ? c_res_true : c_res_indeterminate;
						}
						else
						{
							state_ = (state)(state_ + (code_body1-code_length1)); // change to body
							req.m_codes[nIndex].reserve(req.m_code_lengths[nIndex]+1);
							return (Consume_Result)(c_res_code_body1+nIndex);
						}
					}
				}
			case code_body1:
			case code_body2:
			case code_body3:
				{
					int nIndex = (state_ - code_body1);
					if(req.m_code_lengths[nIndex]>(int)req.m_codes[nIndex].size())
					{
						// I used  a special return value c_res_code_body to indicate that code body is met, 
						// so that we can read to end or length instead of doing the lexical work by char at a time. 
						req.m_codes[nIndex].push_back(input);
						if (req.m_code_lengths[nIndex] > (int)req.m_codes[nIndex].size())
						{
							return (Consume_Result)(c_res_code_body1+nIndex);
						}
						else
						{
							if(nIndex == (code_body1-code_length1-1)){
								return c_res_true;
							}
							else{
								state_ = (state)(code_length1 + nIndex + 1);
								return c_res_indeterminate;
							}
						}
					}
				}
			default:
				return c_res_false;
			}
			return c_res_false;
		}

		/** decompress the message body*/
		void Decompress(InterProcessMessageIn& req) {};

		/** Decode the message body*/
		void Decode(InterProcessMessageIn& req) {};

		/// Check if a byte is an HTTP character.
		static bool is_char(int c){
			return c >= 0 && c <= 127;
		}

		/// Check if a byte is an HTTP control character.
		static bool is_ctl(int c){
			return c >= 0 && c <= 31 || c == 127;
		}

		/// Check if a byte is a digit.
		static bool is_digit(int c){
			return c >= '0' && c <= '9';
		}

		/// The current state of the parser.
		enum state
		{
			method_start,
			method,
			msg_type_1,
			msg_type_2,
			msg_type_3,
			msg_type_4,
			msg_param1_1,
			msg_param1_2,
			msg_param1_3,
			msg_param1_4,
			msg_param2_1,
			msg_param2_2,
			msg_param2_3,
			msg_param2_4,
			code_length1,
			code_length2,
			code_length3,
			code_body1,
			code_body2,
			code_body3,
		} state_;

		/** whether the message body is compressed. */
		bool m_bCompressed;
	};
#pragma endregion Incomming msg

#pragma region Out Msg
	/** generate an output message to be send via network. */
	class  CInterProcessMessageOut_gen
	{
	public:
		typedef std::string BufferType_t;

		/** call this function to generate a new message. call GetBuffer to and GetBufferSize to retrieve the message. 
		* @return the buffer size is returned. 
		*/
		int GenerateMsg(const InterProcessMessage& msg)
		{
			reset();
			m_buffer.append(msg.m_method.empty() ? "NPL" : msg.m_method);
			m_buffer.append(" ");
			append(msg.m_nMsgType);
			append(msg.m_nParam1);
			append(msg.m_nParam2);
			append(msg.m_from);
			append(msg.m_filename);
			append(msg.m_code);
			return GetBufferSize();
		}

		const char* GetBuffer() {return m_buffer.c_str();};
		int GetBufferSize(){return (int)(m_buffer.size());};
	public:
		void reset() 
		{
			m_buffer.clear();
		}

		void append(const std::string & str)
		{
			append(str.c_str(), (int)(str.size()));
		}
		void append(const char* pData, int nLength)
		{
			assert(pData!=0);
			if(nLength<=0)
				nLength = (int)strlen(pData);
			char tmp[16];
			snprintf(tmp, 15, "%d:", nLength);
			m_buffer.append(tmp);
			if(nLength>0)
				m_buffer.append(pData, nLength);
		}
		void append(DWORD dwValue)
		{
			unsigned char value[4];

			for (int i=0;i<4;++i)
			{
				value[i] = (dwValue >> ((3-i)*8)) & 0xff;
			}
			m_buffer.append((const char*)value, 4);
		}
	private:
		BufferType_t m_buffer;
	};

#pragma endregion Out Msg

#pragma region InterprocessQueue
	/** how the message queue is used. */
	enum IPQueueUsageEnum
	{
		IPQU_create_only = 0,
		IPQU_open_only,
		IPQU_open_or_create,
		IPQU_open_read_only, // not supported
		IPQU_open_copy_on_write, // not supported
	};

	/** possible return code. */
	enum IPQueueReturnCodeEnum
	{
		IPRC_OK = 0,
		IPRC_FAILED,
		IPRC_QUEUE_IS_FULL,
		IPRC_QUEUE_IS_EMPTY,
		IPRC_QUEUE_NOT_FOUND,
		IPRC_TIMEDOUT,
	};

	/** It is for sending and receiving InterProcessMessage. 
	* internally we use shared memory to create the message queue. Each message queue have a globally unique string name and a fixed max size
	* Two processes can create CInterprocessQueue using the same name and send and receive messages via it. 
	* Usually one process is producer and the other is consumer. To achieve bi-directional communications, we can use two queues. 

	---++ Queue Life Time
	Since the message queue is a global object, it is not removed even the queue object is deleted. 
	One must call Remove() method explicitly to remove a queue. Therefore to create a new empty queue, 
	one usually needs to call Clear() to ensure that queue is emptied. 

	---++ Example Usage
	Message queue can be used between process or between threads of the same process, etc. 

	<verbatim>
	{
	// process 1: writer
	CInterprocessQueue ipQueueWriter("InstanceName", IPQU_open_or_create);
	// since we are the owner, clear all messages.
	ipQueueWriter.Clear(); 
	InterProcessMessage msg_out;

	for (int i=0;i<10;++i)
	{
	msg_out.m_method = "NPLv1";
	msg_out.m_nMsgType = 100;
	msg_out.m_nParam1 = 200;
	msg_out.m_nParam2 = 300;
	msg_out.m_filename = "test.lua";
	msg_out.m_from = "FromName";
	msg_out.m_code = "msg={param=10}";
	if(ipQueueWriter.try_send(msg_out, 1) == IPRC_OK){
	OutputDebugString("Test Case i: msg sent\n");
	}
	}
	}

	{
	// process 2: reader
	CInterprocessQueue ipQueueReader("InstanceName", IPQU_open_or_create);
	InterProcessMessage msg_in;
	unsigned int nPriority = 0;
	while(ipQueueReader.try_receive(msg_in, nPriority) == IPRC_OK)
	{
	assert(msg_in.m_method == "NPLv1");
	assert(msg_in.m_nMsgType == 100);
	assert(msg_in.m_nParam1 == 200);
	assert(msg_in.m_nParam2 == 300);
	assert(msg_in.m_from == "FromName");
	assert(msg_in.m_filename == "test.lua");
	assert(msg_in.m_code == "msg={param=10}");
	assert(nPriority == 1);
	OutputDebugString("Test Case i passed\n");
	}
	}

	</verbatim>
	*/
	template <int MAX_QUEUE_SIZE = 2000, int MAX_PACKET_SIZE = 256, typename MessageQueueType = boost::interprocess::message_queue>
	class CInterprocessQueueT
	{
	public:
		typedef boost::shared_ptr< typename MessageQueueType > message_queue_t;
		typedef boost::array<char, MAX_PACKET_SIZE> Buffer_Type;

		/** Since the message queue is a global object, it is not removed even the queue object is deleted. 
		* One must call Remove() method explicitly to remove a queue. Therefore to create a new empty queue, 
		* one usually needs to call Clear() to ensure that queue is emptied. 
		* @param	sQueueName		Name of the queue. 
		* @param	usage			The usage. The most common usage is perhaps IPQU_open_or_create
		*/
		CInterprocessQueueT(const char* sQueueName, IPQueueUsageEnum usage = IPQU_open_or_create)
			:m_sQueueName(sQueueName), m_usage(usage), m_queue_size(MAX_QUEUE_SIZE), m_max_packet_size(MAX_PACKET_SIZE)
		{
			using namespace boost::interprocess;
			try
			{
				switch(m_usage)
				{
				case IPQU_create_only:
					{
						//Erase previous message queue
						typename MessageQueueType::remove(m_sQueueName.c_str());
						//Create a message_queue.
						m_msg_queue.reset(new (typename MessageQueueType)(create_only //only create
							,m_sQueueName.c_str()				//name
							,m_queue_size              //max message number
							,m_max_packet_size              //max message size
							));

						break;
					}
				case IPQU_open_only:
					{
						//Opens a message_queue. If the queue, does not exist throws an exception.
						m_msg_queue.reset(new (typename MessageQueueType)(open_only,m_sQueueName.c_str()));
						break;
					}
				case IPQU_open_or_create:
					{
						//Create a message_queue.
						m_msg_queue.reset(new (typename MessageQueueType)(open_or_create 
							,m_sQueueName.c_str()				//name
							,m_queue_size              //max message number
							,m_max_packet_size         //max message size
							));
						break;
					}
				default:
					break;
				}
			}
			catch (...)
			{
				m_msg_queue.reset();
			}
		};
		~CInterprocessQueueT()
		{
		};

	public:

		/** if this is valid. */
		bool IsValid()
		{
			if(m_msg_queue)
				return true;
			else
				return false;
		}

		void Cleanup()
		{
			// in case of error, just clear the queue and reset parser. 
			Clear();
			m_out_gen.reset();
			m_input_msg.reset();
			m_parser.reset();
		}


		/** remove all messages. */
		bool Remove()
		{
			try
			{
				typename MessageQueueType::remove(m_sQueueName.c_str());
			}
			catch ( ... )
			{
				return false;
			}
			return true;
		}

		/** clear all messages. This function is usually called by the owner of the queue to empty the queue. */
		void Clear()
		{
			if(m_msg_queue)
			{
				ParaEngine::Mutex::ScopedLock lock_(m_mutex);
				try
				{
					std::size_t bytes_transferred = 0;
					unsigned int nPriority = 0;
					while (m_msg_queue->try_receive(m_buffer.c_array(), m_buffer.size(), bytes_transferred, nPriority) || m_msg_queue->get_num_msg()>0)
					{
					}
					m_out_gen.reset();
					m_input_msg.reset();
					m_parser.reset();
				}
				catch (...)
				{
				}
			}
		}

		/** send a message and block if queue is full until message is sent out. 
		*/
		IPQueueReturnCodeEnum send(const InterProcessMessage& msg, unsigned int nPriority = 0)
		{
			if(m_msg_queue)
			{
				ParaEngine::Mutex::ScopedLock lock_(m_mutex);

				if( m_out_gen.GenerateMsg(msg) > 0)
				{
					int nSize = m_out_gen.GetBufferSize();
					const char* pBuffer = m_out_gen.GetBuffer();

					int nMaxSize = (int)(m_msg_queue->get_max_msg_size());
					if(nMaxSize == 0)
						return IPRC_QUEUE_NOT_FOUND;

					assert(nMaxSize == m_max_packet_size);

					try
					{
						while(nSize > 0)
						{
							if(nSize > nMaxSize)
							{
								m_msg_queue->send(pBuffer, nMaxSize, nPriority);
								nSize -= nMaxSize;
								pBuffer += nMaxSize;
							}
							else
							{
								m_msg_queue->send(pBuffer, nSize, nPriority);
								nSize = 0;
							}
						}
					}
					catch (...)
					{
						Cleanup();
						return IPRC_FAILED;
					}
				}
			}
			else
			{
				return IPRC_QUEUE_NOT_FOUND;
			}
			return IPRC_OK;
		}

		/** only send if queue is not full. non-blocking. Internally it check available queue size and send via the blocking send() method. */
		IPQueueReturnCodeEnum try_send(const InterProcessMessage& msg, unsigned int nPriority = 0)
		{
			if(m_msg_queue)
			{
				ParaEngine::Mutex::ScopedLock lock_(m_mutex);

				if( m_out_gen.GenerateMsg(msg) > 0)
				{
					int nSize = m_out_gen.GetBufferSize();
					const char* pBuffer = m_out_gen.GetBuffer();

					try
					{
						int nMaxSize = (int)(m_msg_queue->get_max_msg_size());
						if(nMaxSize == 0)
							return IPRC_QUEUE_NOT_FOUND;

						assert(nMaxSize == m_max_packet_size);

						int nMaxMessageLength = (int)(m_msg_queue->get_max_msg() -m_msg_queue->get_num_msg())*nMaxSize;
						if(nMaxMessageLength < nSize)
							return IPRC_QUEUE_IS_FULL;

						while(nSize > 0)
						{
							if(nSize > nMaxSize)
							{
								m_msg_queue->send(pBuffer, nMaxSize, nPriority);
								nSize -= nMaxSize;
								pBuffer += nMaxSize;
							}
							else
							{
								m_msg_queue->send(pBuffer, nSize, nPriority);
								nSize = 0;
							}
						}
					}
					catch (...)
					{
						Cleanup();
						return IPRC_FAILED;
					}
				}
			}
			else
			{
				return IPRC_QUEUE_NOT_FOUND;
			}
			return IPRC_OK;
		}

		/** blocking call to force receive a message.
		* @return IPRC_OK if a message is received. or IPRC_FAILED 
		*/
		IPQueueReturnCodeEnum receive(InterProcessMessage& msg, unsigned int & nPriority)
		{
			// m_in_parser
			if(m_msg_queue)
			{
				try
				{
					while (true)
					{
						std::size_t bytes_transferred = 0;
						m_msg_queue->receive(m_buffer.c_array(), m_buffer.size(), bytes_transferred, nPriority);

						boost::tribool result = true;
						typename Buffer_Type::iterator curIt = m_buffer.begin(); 
						typename Buffer_Type::iterator curEnd = m_buffer.begin() + bytes_transferred; 

						while (curIt!=curEnd)
						{
							boost::tie(result, curIt) = m_parser.parse(m_input_msg, curIt, curEnd);
							if (result)
							{
								// a complete message is read to m_input_msg.
								m_input_msg.ToMessage(msg);
								return IPRC_OK;
							}
							else if (!result)
							{
								m_input_msg.reset();
								m_parser.reset();
								break;
							}
						}

						if (!result)
						{
							// message parsing failed. the message format is not supported. 
							// This is a stream error, we shall close the connection
							Cleanup();
							return IPRC_FAILED;
						}
						else
						{
							// the message has not been received completely, needs to receive more data. 
						}
					}
				}
				catch (...)
				{
					Cleanup();
					return IPRC_FAILED;
				}
			}
			else
			{
				return IPRC_QUEUE_NOT_FOUND;
			}
			return IPRC_OK;
		}

		/**  non-blocking call
		* @return IPRC_OK if a message is received. or IPRC_QUEUE_IS_EMPTY is no complete message is read. or IPRC_FAILED 
		*/
		IPQueueReturnCodeEnum try_receive(InterProcessMessage& msg, unsigned int & nPriority)
		{
			// m_in_parser
			if(m_msg_queue)
			{
				std::size_t bytes_transferred = 0;
				try
				{
					while (m_msg_queue->try_receive(m_buffer.c_array(), m_buffer.size(), bytes_transferred, nPriority))
					{
						boost::tribool result = true;
						typename Buffer_Type::iterator curIt = m_buffer.begin(); 
						typename Buffer_Type::iterator curEnd = m_buffer.begin() + bytes_transferred; 

						while (curIt!=curEnd)
						{
							boost::tie(result, curIt) = m_parser.parse(m_input_msg, curIt, curEnd);
							if (result)
							{
								// a complete message is read to m_input_msg.
								m_input_msg.ToMessage(msg);
								return IPRC_OK;
							}
							else if (!result)
							{
								m_input_msg.reset();
								m_parser.reset();
								break;
							}
						}

						if (!result)
						{
							// message parsing failed. the message format is not supported. 
							// This is a stream error, we shall close the connection
							Cleanup();
							return IPRC_FAILED;
						}
						else
						{
							// the message has not been received completely, needs to receive more data. 
						}
					}
				}
				catch (...)
				{
					Cleanup();
					return IPRC_FAILED;
				}
				return IPRC_QUEUE_IS_EMPTY;
			}
			else
			{
				return IPRC_QUEUE_NOT_FOUND;
			}
			return IPRC_OK;
		}

		const std::string& GetName() {return m_sQueueName;}
	protected:
		std::string m_sQueueName;
		int m_queue_size;
		int m_max_packet_size;
		IPQueueUsageEnum m_usage;

		message_queue_t m_msg_queue;
		CInterProcessMessageOut_gen m_out_gen;
		InterProcessMessageIn m_input_msg;
		CInterProcessMessageIn_parser m_parser;

		/// Buffer for incoming data.
		Buffer_Type m_buffer;

		ParaEngine::Mutex m_mutex;
	};

#ifdef WIN32
	typedef CInterprocessQueueT<2000, 256, ParaEngine::interprocess::message_queue_win32> CInterprocessQueue;
#else
	typedef CInterprocessQueueT<2000, 256, boost::interprocess::message_queue>	CInterprocessQueueFile;
#endif

	typedef CInterprocessQueueT<2000, 256, boost::interprocess::message_queue>	CInterprocessQueueFileEmu;
#pragma endregion InterprocessQueue
}