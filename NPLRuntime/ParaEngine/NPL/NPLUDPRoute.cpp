#include "ParaEngine.h"
#include <boost/bind.hpp>
#include "NPLDispatcher.h"
#include "NPLRuntime.h"
#include "NPLUDPRouteManager.h"
#include "NPLMsgIn_parser.h"
#include "NPLMsgOut.h"
#include "NPLNetUDPServer.h"
#include "NPLNetServer.h"
#include "NPLRuntime.h"

#include "NPLUDPRoute.h"
#include "json/json.h"
#include "NPLHelper.h"


/** @def the default maximum output message queue size. this is usually set to very big, such as 1024.
the send message function will fail (service not available), if the queue is full */
#define DEFAULT_NPL_OUTPUT_QUEUE_SIZE		1024

/** when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression.
For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0.
Default value is 200KB */
#define NPL_AUTO_COMPRESSION_THRESHOLD		2048000

namespace NPL {

	CNPLUDPRoute::CNPLUDPRoute(CNPLNetUDPServer& udp_server, CNPLUDPRouteManager& manager, CNPLUDPDispatcher& msg_dispatcher)
		: m_route_manager(manager)
		, m_udp_server(udp_server)
		, m_msg_dispatcher(msg_dispatcher)
		, m_totalBytesIn(0), m_totalBytesOut(0)
		, m_nCompressionLevel(0)
		, m_nCompressionThreshold(NPL_AUTO_COMPRESSION_THRESHOLD)
		, m_bEnableIdleTimeout(true)
		, m_nSendCount(0), m_nFinishedCount(0), m_bCloseAfterSend(false), m_nIdleTimeoutMS(0), m_nLastActiveTime(0)
	{
		// init common fields for input message. 
		m_input_msg.reset();
		m_input_msg.npl_version_major = NPL_VERSION_MAJOR;
		m_input_msg.npl_version_minor = NPL_VERSION_MINOR;
		m_input_msg.bUseConnection = false;
		m_input_msg.m_pRoute = this;
	}


	CNPLUDPRoute::~CNPLUDPRoute()
	{

	}

	void CNPLUDPRoute::EnableIdleTimeout(bool bEnable)
	{
		m_bEnableIdleTimeout = bEnable;
	}

	bool CNPLUDPRoute::IsIdleTimeoutEnabled()
	{
		return m_bEnableIdleTimeout;
	}

	void CNPLUDPRoute::SetIdleTimeoutPeriod(int nMilliseconds)
	{
		m_nIdleTimeoutMS = nMilliseconds;
	}

	int CNPLUDPRoute::GetIdleTimeoutPeriod()
	{
		return m_nIdleTimeoutMS;
	}

	const string& CNPLUDPRoute::GetNID() const
	{
		if (m_address)
			return m_address->GetNID();
		else
			return ParaEngine::CGlobals::GetString(0);
	}


	bool CNPLUDPRoute::SetNID(const char* sNID)
	{
		// update mapping. 
		if (sNID && m_address)
		{
			m_msg_dispatcher.RenameAddress(m_address, sNID);
		}
		return true;
	}

	void CNPLUDPRoute::SetNPLUDPAddress(NPLUDPAddress_ptr udp_address)
	{
		m_address = udp_address;
	}

	string CNPLUDPRoute::GetIP()
	{
		if (m_address)
			return m_address->GetHost();
		else
			return ParaEngine::CGlobals::GetString(0);
	}

	unsigned short CNPLUDPRoute::GetPort()
	{
		if (m_address)
			return m_address->GetPort();
		else
			return 0;
	}

	void CNPLUDPRoute::SetUseCompression(bool bUseCompression)
	{
		if (bUseCompression)
		{
			if (m_nCompressionLevel == 0)
				m_nCompressionLevel = -1;
		}
		else
		{
			m_nCompressionLevel = 0;
		}
	}

	bool CNPLUDPRoute::IsUseCompression()
	{
		return m_nCompressionLevel != 0;
	}

	void CNPLUDPRoute::SetCompressionLevel(int nLevel)
	{
		m_nCompressionLevel = nLevel;
	}

	int CNPLUDPRoute::GetCompressionLevel()
	{
		return m_nCompressionLevel;
	}

	void CNPLUDPRoute::SetCompressionThreshold(int nThreshold)
	{
		m_nCompressionThreshold = nThreshold;
	}

	int CNPLUDPRoute::GetCompressionThreshold()
	{
		return m_nCompressionThreshold;
	}

	unsigned CNPLUDPRoute::GetLastActiveTime()
	{
		return m_nLastActiveTime;
	}

	void CNPLUDPRoute::TickSend()
	{
		m_nLastActiveTime = GetTickCount();
	}

	void CNPLUDPRoute::TickReceive()
	{
		m_nLastActiveTime = GetTickCount();
	}

	bool CNPLUDPRoute::HasUnsentData()
	{
		return m_nSendCount != m_nFinishedCount;
	}

	int CNPLUDPRoute::CheckIdleTimeout(unsigned int nCurTime)
	{
		if (!m_bEnableIdleTimeout || m_nIdleTimeoutMS == 0 || m_nLastActiveTime == 0)
			return 1;
		if ((m_nLastActiveTime + m_nIdleTimeoutMS) < nCurTime /*&& !HasUnsentData()*/)
		{
			// close the timed out route immediately
			if (GetLogLevel() > 0)
			{
				OUTPUT_LOG1("Route time out (%s/%d) with id (%s). \n",
							GetIP().c_str(), GetPort(), GetNID().c_str());
			}
			return 0;
		}
		return 1;
	}

	int CNPLUDPRoute::GetLogLevel()
	{
		return CNPLRuntime::GetInstance()->GetLogLevel();
	}

	void CNPLUDPRoute::start_send(const char* ip, unsigned short port)
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		boost::asio::ip::udp::endpoint broadcast_ep(boost::asio::ip::make_address_v4(ip), port);
		m_address.reset(new NPLUDPAddress(broadcast_ep, "send_to"));
#endif
		// update the start time and last send/receive time
		m_nStartTime = GetTickCount();
		m_nLastActiveTime = m_nStartTime;

		// set use compression. 
		bool bUseCompression = m_msg_dispatcher.IsUseCompressionRoute();
		SetUseCompression(bUseCompression);
		if (bUseCompression)
		{
			SetCompressionLevel(m_msg_dispatcher.GetCompressionLevel());
			SetCompressionThreshold(m_msg_dispatcher.GetCompressionThreshold());
		}
	}

	void CNPLUDPRoute::start_broadcast(unsigned short port)
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		boost::asio::ip::udp::endpoint broadcast_ep(boost::asio::ip::address_v4::broadcast(), port);
		m_address.reset(new NPLUDPAddress(broadcast_ep, "broadcast"));
#endif
		// update the start time and last send/receive time
		m_nStartTime = GetTickCount();
		m_nLastActiveTime = m_nStartTime;

		// set use compression. 
		bool bUseCompression = m_msg_dispatcher.IsUseCompressionRoute();
		SetUseCompression(bUseCompression);
		if (bUseCompression)
		{
			SetCompressionLevel(m_msg_dispatcher.GetCompressionLevel());
			SetCompressionThreshold(m_msg_dispatcher.GetCompressionThreshold());
		}
	}

	void CNPLUDPRoute::start()
	{
		// update the start time and last send/receive time
		m_nStartTime = GetTickCount();
		m_nLastActiveTime = m_nStartTime;

		if (m_address)
		{
			// set use compression. 
			bool bUseCompression = m_msg_dispatcher.IsUseCompressionRoute();
			SetUseCompression(bUseCompression);
			if (bUseCompression)
			{
				SetCompressionLevel(m_msg_dispatcher.GetCompressionLevel());
				SetCompressionThreshold(m_msg_dispatcher.GetCompressionThreshold());
			}
		}
		else
		{
			PE_ASSERT(false);
		}
	}

	bool CNPLUDPRoute::handleMessageIn()
	{
		bool bRes = true;

		// update the last send/receive time
		TickReceive();

		// dispatch the message

		if (m_input_msg.npl_version_major == NPL_VERSION_MAJOR /*&& m_input_msg.npl_version_minor==NPL_VERSION_MINOR*/)
		{
			// TODO: some more check on method, uri, headers, etc, before dispatching it.  
			NPLReturnCode nResult = m_msg_dispatcher.DispatchMsg(m_input_msg);

			if (nResult != NPL_OK)
			{
				if (GetLogLevel() > 0)
				{
					if (nResult == NPL_QueueIsFull)
					{
						OUTPUT_LOG("NPL dispatcher error because NPL_QueueIsFull incoming msg from nid:%s to thread %s\n", GetNID().c_str(), m_input_msg.m_rts_name.c_str());
					}
					else if (nResult == NPL_RuntimeState_NotExist)
					{
						OUTPUT_LOG("NPL dispatcher error because incoming nid:%s NPL_RuntimeState_NotExist\n", GetNID().c_str());
					}
					else
					{
						OUTPUT_LOG("NPL dispatcher error because incoming nid:%s NPLReturnCode %d.\n", GetNID().c_str(), nResult);
					}
				}
			}

		}
		else
		{
			if (GetLogLevel() > 0) {
				OUTPUT_LOG("NPL protocol version is not supported. nid %s \n", GetNID().c_str());
			}
			bRes = false;
		}
		return bRes;
	}

	bool CNPLUDPRoute::handleReceivedData(const char* buff, size_t bytes_transferred)
	{
		boost::tribool result = true;
		auto curIt = buff;
		auto curEnd = buff + bytes_transferred;

		while (curIt != curEnd)
		{
			boost::tie(result, curIt) = m_parser.parse(m_input_msg, curIt, curEnd);
			if (result)
			{
				// a complete message is read to m_input_msg.
				handleMessageIn();
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
			// This is a stream error, we shall close the route
			if (GetLogLevel() > 0) {
				OUTPUT_LOG("warning: message parsing failed when received data. we will close route. nid %s \n", GetNID().c_str());
			}
			m_route_manager.stop(shared_from_this());
			return false;
		}

		return true;
	}

	unsigned long long CNPLUDPRoute::getHash()
	{
		if (m_address)
		{
			return m_address->GetHash();
		}
		else
			return 0;
		
	}

	void CNPLUDPRoute::CloseAfterSend()
	{
		m_bCloseAfterSend = true;
	}

#ifndef EMSCRIPTEN_SINGLE_THREAD
	void CNPLUDPRoute::handle_send(const boost::system::error_code& error, size_t bytes_transferred, const char* buff, size_t buff_size)
	{
		if (!error)
		{
			// update the last send/receive time
			TickSend();

			m_nFinishedCount++;


			if (m_bCloseAfterSend)
				stop(true);
		}
		else
		{
			if (GetLogLevel() > 0) {
				OUTPUT_LOG("warning: handle_send operation aborted. nid %s  msg : \n", GetNID().c_str(), error.message().c_str());
			}
		}
	}
#endif

	void CNPLUDPRoute::stop(bool bRemoveConnection)
	{
		if (bRemoveConnection)
		{
			m_route_manager.stop(shared_from_this());
		}
		else
		{
			// Post a call to the stop function so that stop() is safe to call from any thread.
#ifndef EMSCRIPTEN_SINGLE_THREAD
			m_udp_server.GetIoService().post(boost::bind(&CNPLUDPRoute::handle_stop, shared_from_this()));
#endif	
		}
	}


	void CNPLUDPRoute::handle_stop()
	{
		// also erase from dispatcher
		if (m_address)
			m_msg_dispatcher.RemoveUDPAddress(m_address);


		if (GetLogLevel() > 0)
		{
			OUTPUT_LOG1("route closed (%s/%d) with id (%s). \n",
				GetIP().c_str(), GetPort(), GetNID().c_str());
		}
	}


	NPLReturnCode CNPLUDPRoute::SendMessage(const NPLFileName& file_name, const char * code /*= nullptr*/, int nLength /*= 0*/, int priority/* = 0*/)
	{
		NPLMsgOut_ptr msg_out(new NPLMsgOut());
		CNPLMsgOut_gen writer(*msg_out);

#ifdef NPL_DYNAMIC_FILE_ID
		// TODO: we should use this->m_filename_id_map instead of the local dispatcher. 
		int file_id = -1;
#else
		// we expect all remote NPL runtime's public file list mapping to be identical 
		int file_id = CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetIDByPubFileName(file_name.sRelativePath);
#endif
		//if (file_name.sRelativePath == "http")
		//{
		//	// for HTTP request/response message 
		//	if (nLength < 0)
		//		nLength = strlen(code);
		//	writer.Append(code, nLength);
		//}
		//else if (file_name.sRelativePath == "tcp")
		//{
		//	// for TCP request/response message 
		//	if (m_protocolType != TCP_CUSTOM)
		//	{
		//		SetProtocol(TCP_CUSTOM);
		//	}
		//	if (nLength < 0)
		//		nLength = strlen(code);
		//	writer.Append(code, nLength);
		//}
		//else if (file_name.sRelativePath == "websocket")
		//{
		//	if (m_protocolType == WEBSOCKET)
		//	{
		//		m_websocket_out_data.clear();
		//		m_websocket_writer.generate(code, nLength, m_websocket_out_data);
		//		writer.Append((char*)&m_websocket_out_data[0], m_websocket_out_data.size());
		//	}
		//	else
		//	{
		//		OUTPUT_LOG("NPL can't send websocket message with a wrong protocol,The connection nid is %s, current protocol is %d. \n", GetNID().c_str(), m_protocolType);
		//		return NPL_WrongProtocol;
		//	}
		//}
		//else
		if (code[0] == '\0')
		{
			writer.AddFirstLine(file_name, file_id, "B ");
			nLength--;
			writer.AddBody(code + 1, nLength, (nLength <= m_nCompressionThreshold ? 0 : m_nCompressionLevel));
		}
		else
		{
			// for NPL message 
			writer.AddFirstLine(file_name, file_id);
			//writer.AddHeaderPair(name,value);
			//writer.AddHeaderPair(name2,value2);

			if (nLength < 0)
				nLength = strlen(code);
			writer.AddMsgBody(code, nLength, (nLength <= m_nCompressionThreshold ? 0 : m_nCompressionLevel));
		}

		return SendMessage(msg_out);
	}


	NPLReturnCode CNPLUDPRoute::SendMessage(const NPLMessage& msg)
	{
		// TODO: 
		return NPL_Error;
	}

	NPLReturnCode CNPLUDPRoute::SendMessage(const char* sCommandName, const char* sCommandData)
	{
		NPLMsgOut_ptr msg_out(new NPLMsgOut());
		CNPLMsgOut_gen writer(*msg_out);

		writer.AddFirstLine("npl", sCommandName);

		char msg[256];
		snprintf(msg, 255, "msg={data=\"%s\"}", sCommandData);
		size_t nLength = strlen(msg);
		writer.AddMsgBody(msg, nLength, (nLength <= m_nCompressionThreshold ? 0 : m_nCompressionLevel));

		return SendMessage(msg_out);
	}


	NPLReturnCode CNPLUDPRoute::SendMessage(NPLMsgOut_ptr& msg)
	{
		// NOTE: m_state is not protected by a lock. This is ok, since its value is always one of the enumeration type. 
		if (msg->empty())
			return NPL_OK;

	
		int nLength = (int)msg->GetBuffer().size();
		m_nSendCount++;
#ifndef EMSCRIPTEN_SINGLE_THREAD
		m_udp_server.SendTo(msg->GetBuffer().c_str(), msg->GetBuffer().size(), shared_from_this());
#endif
		m_totalBytesOut += nLength;
		return NPL_OK;
	}


} // namespace NPL