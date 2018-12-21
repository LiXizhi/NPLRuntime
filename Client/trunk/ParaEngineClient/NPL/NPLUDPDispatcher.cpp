//-----------------------------------------------------------------------------
// Class:	NPLDispatcher
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLUDPDispatcher.h"

#include "NPLRuntime.h"
#include "NPLNetUDPServer.h"
#include "NPLNetServer.h"
#include "NPLHelper.h"
#include "EventsCenter.h"
#include "NPLRuntime.h"


namespace NPL {

	CNPLUDPDispatcher::CNPLUDPDispatcher(CNPLNetUDPServer& server, CNPLUDPRouteManager& manager)
		: m_server(server)
		, m_route_manager(manager)
		, m_bUseCompressionRoute(false)
		, m_nCompressionLevel(-1)
		, m_nCompressionThreshold(204800)
	{
		m_broadcast_route.reset(new CNPLUDPRoute(server, manager, *this));
	}

	CNPLUDPDispatcher::~CNPLUDPDispatcher()
	{

	}

	void CNPLUDPDispatcher::SetUseCompressionRoute(bool bCompress)
	{
		m_bUseCompressionRoute = bCompress;
	}

	bool CNPLUDPDispatcher::IsUseCompressionRoute()
	{
		return m_bUseCompressionRoute;
	}


	void CNPLUDPDispatcher::SetCompressionLevel(int nLevel)
	{
		m_nCompressionLevel = nLevel;
	}

	int CNPLUDPDispatcher::GetCompressionLevel()
	{
		return m_nCompressionLevel;
	}

	void CNPLUDPDispatcher::SetCompressionThreshold(int nThreshold)
	{
		m_nCompressionThreshold = nThreshold;
	}

	int CNPLUDPDispatcher::GetCompressionThreshold()
	{
		return m_nCompressionThreshold;
	}

	NPLUDPRoute_ptr CNPLUDPDispatcher::CreateGetNPLRouteByNID(const std::string& sNID)
	{
		ParaEngine::Lock lock_(m_mutex);

		// see if we have address for sNID. If so, we will actively establish a new route to it. 
		auto iter1 = m_server_address_map.find(sNID);
		if (iter1 != m_server_address_map.end())
		{
			auto pAddr = iter1->second;
			lock_.unlock();


			auto route = m_route_manager.GetRoute(pAddr->GetHash());
			if (route)
				return route;
			else
			{
				return m_server.CreateRoute(pAddr);
			}
		}

		return NPLUDPRoute_ptr();
	}


	void CNPLUDPDispatcher::Cleanup()
	{
		ParaEngine::Lock lock_(m_mutex);

		m_server_address_map.clear();
	}

	void CNPLUDPDispatcher::RenameAddress(NPLUDPAddress_ptr udp_address, const char* sNID)
	{
		ParaEngine::Lock lock_(m_mutex);

		if (sNID && udp_address->GetNID() != sNID)
		{
			auto old_it = m_server_address_map.find(udp_address->GetNID());
			if (old_it != m_server_address_map.end())
			{
				m_server_address_map.erase(old_it);
			}


			udp_address->SetNID(sNID);
			m_server_address_map[sNID] = udp_address;
		}
	}

	void CNPLUDPDispatcher::RemoveUDPAddress(NPLUDPAddress_ptr pAddress)
	{
		ParaEngine::Lock lock_(m_mutex);

		auto it = m_server_address_map.find(pAddress->GetNID());
		if (it == m_server_address_map.end())
		{
			m_server_address_map.erase(it);
		}
	}

	bool CNPLUDPDispatcher::AddNPLUDPAddress(NPLUDPAddress_ptr pAddress)
	{
		ParaEngine::Lock lock_(m_mutex);
		if (m_server_address_map.find(pAddress->GetNID()) == m_server_address_map.end())
		{
			m_server_address_map[pAddress->GetNID()] = pAddress;
			return true;
		}
		return false;
	}

	NPLUDPAddress_ptr CNPLUDPDispatcher::GetNPLUDPAddress(const string& sNID)
	{
		ParaEngine::Lock lock_(m_mutex);
		auto it = m_server_address_map.find(sNID);
		if (it != m_server_address_map.end())
			return it->second;
		else
			return nullptr;
	}



	NPLReturnCode CNPLUDPDispatcher::DispatchMsg(NPLMsgIn& msg)
	{
		if (msg.bUseConnection)
			return NPL_Error;

		// this is a local activation
		auto pRuntime = ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState(msg.m_rts_name);
		if (pRuntime)
		{
			if (msg.method.size()>0 && (((byte)(msg.method[0])) > 127 /*== 0xff*/ || msg.method == "A"))
			{
				if (CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().CheckPubFile(msg.m_filename, msg.m_n_filename))
				{
					// TODO: we need to check the input
					// verify the msg.m_code is empty or a valid pure data table. 
					// Add headers, rts and connection(msg.m_pConnection) info to the msg.m_code.
					NPLMessage_ptr msg_(new NPLMessage());
					msg_->m_filename = msg.m_filename;
					int nSize = (int)(msg.m_code.size());
					msg_->m_code.reserve(nSize + 36);
					msg_->m_code.append("msg={");

					msg_->m_code.append(msg.m_code);

					if (nSize > 0 && msg.m_code[nSize - 1] != ',')
					{
						msg_->m_code.append(',');
					}

					msg_->m_code.append("isUDP=true,");

					// add nid or tid so that the runtime state can have access to the current connection object. 
					// we append last to msg, so that nid or tid in the input message is overridden by the real value. 
					//if (msg.m_pConnection->IsAuthenticated())
					if (m_server_address_map.find(msg.m_pRoute->GetNID()) != m_server_address_map.end())
					{
						msg_->m_code.append("nid=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pRoute->GetNID());
						msg_->m_code.append("}");
					}
					else
					{
						msg_->m_code.append("tid=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pRoute->GetNID());
						msg_->m_code.append(",nid=nil}");
					}
					return pRuntime->Activate_async(msg_);
				}
				else
				{
					//#ifdef PARAENGINE_CLIENT
					OUTPUT_LOG("error: NPL remote file access denied: %s(%d)\n", msg.m_filename.c_str(), msg.m_n_filename);
					//#endif
					return NPL_FileAccessDenied;
				}
			}
			else if (msg.method == "npl")
			{
				OUTPUT_LOG("NPL network command: %s %s\n", msg.m_filename.c_str(), msg.m_code.c_str());
				// npl low level command message
				if (msg.m_filename == "connect_overriden")
				{
					PostNetworkEvent(NPL_ConnectionDisconnected, msg.m_pRoute->GetNID().c_str(), "connection_overwrite");
				}
				return NPL_OK;
			}
			else
			{
				// http message
				std::string filename;
				int nHTTPCode = -10;
				if (CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().CheckPubFile(filename, nHTTPCode))
				{
					// we need to check the input
					NPLMessage_ptr msg_(new NPLMessage());
					msg_->m_filename = filename;

					int nSize = (int)(msg.m_code.size());
					msg_->m_code.reserve(nSize + 36);
					msg_->m_code.append("msg={");

					// add url or the "OK", "Error" string in response's first line
					msg_->m_code.append("url=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_filename);
					msg_->m_code.append(",");

					// add method: GET, POST,  HTTP/1.1, etc. 
					msg_->m_code.append("method=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.method);
					msg_->m_code.append(",");

					// rcode
					msg_->m_code.append("rcode=");
					msg_->m_code.append(msg.m_n_filename);
					msg_->m_code.append(",");


					// add headers
					int nCount = (int)msg.headers.size();
					for (int i = 0; i <nCount; ++i)
					{
						msg_->m_code.append("[");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.headers[i].name);
						msg_->m_code.append("]=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.headers[i].value);
						msg_->m_code.append(",");
					}

					// add body
					if (NPLHelper::CanEncodeStringInDoubleBrackets(msg.m_code.c_str(), (int)msg.m_code.size()))
					{
						msg_->m_code.append("body=[[");
						msg_->m_code.append(msg.m_code);
						msg_->m_code.append("]],");
					}
					else
					{
						msg_->m_code.append("body=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_code);
						msg_->m_code.append(",");
					}

					// add nid or tid so that the runtime state can have access to the current connection object. 
					// we append last to msg, so that nid or tid in the input message is overridden by the real value. 
					if (m_server_address_map.find(msg.m_pRoute->GetNID()) != m_server_address_map.end())
					{
						msg_->m_code.append("nid=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pRoute->GetNID());
						msg_->m_code.append("}");
					}
					else
					{
						msg_->m_code.append("tid=");
						NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pRoute->GetNID());
						msg_->m_code.append(",nid=nil}");
					}
					return pRuntime->Activate_async(msg_);
				}
				else
				{
					//#ifdef PARAENGINE_CLIENT
					OUTPUT_LOG("error: no HTTP defined in NPL. Message is dropped");
					//#endif
					return NPL_FileAccessDenied;
				}

			}

		}
		else
		{
			//#ifdef PARAENGINE_CLIENT
			OUTPUT_LOG("error: NPL runtime state %s does not exist \n", msg.m_rts_name.c_str());
			//#endif
			return NPL_RuntimeState_NotExist;
		}
		return NPL_Error;
	}

	int CNPLUDPDispatcher::PostNetworkEvent(NPLReturnCode nNPLNetworkCode, const char * sNid, const char* sMsg)
	{
		CNPLWriter writer;

		writer.WriteName("msg");
		writer.BeginTable();
		{
			writer.WriteName("code");
			writer.WriteValue((int)nNPLNetworkCode);

			writer.WriteName("nid");
			if (sNid != 0 && sNid[0] != '\0')
			{
				writer.WriteValue(sNid);
			}

			if (sMsg != 0 && sMsg[0] != '\0')
			{
				writer.WriteName("msg");
				writer.WriteValue(sMsg);
			}
		}
		writer.EndTable();
		writer.WriteParamDelimiter();

		ParaEngine::Event e(ParaEngine::EVENT_NETWORK, writer.ToString().c_str());
		return ParaEngine::CGlobals::GetEventsCenter()->PostEvent(e);
	}

	NPLReturnCode CNPLUDPDispatcher::Broadcast_Async(const NPLFileName& file_name, unsigned short port, const char * code /*= nullptr*/, int nLength/* = 0*/, int priority/* = 0*/)
	{
		m_broadcast_route->start_broadcast(port);
		return m_broadcast_route->SendMessage(file_name, code, nLength, priority);
	}

	NPLReturnCode CNPLUDPDispatcher::Activate_Async(const NPLFileName& file_name, const char * code /*= NULL*/, int nLength/*=0*/, int priority/*=0*/)
	{
		if (file_name.sNID.empty())
		{
			// this is a local activation
			NPLRuntimeState_ptr pRuntime = ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState(file_name.sRuntimeStateName);
			if (pRuntime)
			{
				return pRuntime->Activate_async(file_name.sRelativePath, code, nLength);
			}
		}
		else
		{
			// this is a remote activation
			auto pRoute = CreateGetNPLRouteByNID(file_name.sNID);
			if (pRoute)
			{
				// send via the connection. 
				return pRoute->SendMessage(file_name, code, nLength, priority);
			}
			
		}
		return NPL_Error;
	}

} // namespace NPL