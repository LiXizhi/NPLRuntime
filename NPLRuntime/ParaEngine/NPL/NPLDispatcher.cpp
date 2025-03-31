//-----------------------------------------------------------------------------
// Class:	NPLDispatcher
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLDispatcher.h"

#include "NPLRuntime.h"
#include "NPLNetServer.h"
#include "NPLHelper.h"
#include "StringHelper.h"
#include "EventsCenter.h"

#ifdef __EMSCRIPTEN__
#include "EmscriptenWebSocket.h"
#endif

#include <unordered_map>


NPL::CNPLDispatcher::CNPLDispatcher(CNPLNetServer* pServer)
	: m_pServer(pServer), m_bUseCompressionIncomingConnection(false), m_bUseCompressionOutgoingConnection(false),
	m_nCompressionLevel(-1), m_nCompressionThreshold(204800)
{
	PE_ASSERT(m_pServer != 0);
}

NPL::CNPLDispatcher::~CNPLDispatcher()
{

}

void NPL::CNPLDispatcher::SetUseCompressionIncomingConnection(bool bCompressIncoming)
{
	m_bUseCompressionIncomingConnection = bCompressIncoming;
}

bool NPL::CNPLDispatcher::IsUseCompressionIncomingConnection()
{
	return m_bUseCompressionIncomingConnection;
}

void NPL::CNPLDispatcher::SetUseCompressionOutgoingConnection(bool bCompressOutgoing)
{
	m_bUseCompressionOutgoingConnection = bCompressOutgoing;
}

bool NPL::CNPLDispatcher::IsUseCompressionOutgoingConnection()
{
	return m_bUseCompressionOutgoingConnection;
}

void NPL::CNPLDispatcher::SetCompressionLevel(int nLevel)
{
	m_nCompressionLevel = nLevel;
}

int NPL::CNPLDispatcher::GetCompressionLevel()
{
	return m_nCompressionLevel;
}

void NPL::CNPLDispatcher::SetCompressionThreshold(int nThreshold)
{
	m_nCompressionThreshold = nThreshold;
}

int NPL::CNPLDispatcher::GetCompressionThreshold()
{
	return m_nCompressionThreshold;
}

NPL::NPLConnection_ptr NPL::CNPLDispatcher::CreateGetNPLConnectionByNID(const string& sNID)
{
	ParaEngine::Lock lock_(m_mutex);

	// find in connected pool 
	ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(sNID);
	if (iter != m_active_connection_map.end())
	{
		return iter->second;
	}

	// find in pending pool 
	iter = m_pending_connection_map.find(sNID);
	if (iter != m_pending_connection_map.end())
	{
		return iter->second;
	}

	// see if we have address for sNID. If so, we will actively establish a new connection to it. 
	ServerAddressMap_Type::iterator iter1 = m_server_address_map.find(sNID);
	if (iter1 != m_server_address_map.end())
	{
		NPLRuntimeAddress_ptr pAddr = iter1->second;
		lock_.unlock();
		return CreateConnection(pAddr);
	}
	return NPLConnection_ptr();
}

NPL::NPLConnection_ptr NPL::CNPLDispatcher::CreateConnection(NPLRuntimeAddress_ptr pAddress)
{
	NPLConnection_ptr pConnection = m_pServer->CreateConnection(pAddress);
	if (pConnection)
	{
		ParaEngine::Lock lock_(m_mutex);

		// find in connected pool 
		ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(pAddress->GetNID());
		if (iter != m_active_connection_map.end())
		{
			return iter->second;
		}
		// find in pending pool 
		iter = m_pending_connection_map.find(pAddress->GetNID());
		if (iter != m_pending_connection_map.end())
		{
			return iter->second;
		}
		m_pending_connection_map[pAddress->GetNID()] = pConnection;
	}
	return pConnection;
}

NPL::NPLConnection_ptr NPL::CNPLDispatcher::GetNPLConnectionByNID(const string& sNID)
{
	ParaEngine::Lock lock_(m_mutex);
	ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(sNID);
	if (iter != m_active_connection_map.end())
	{
		return iter->second;
	}
	else
		return NPLConnection_ptr();
}

void NPL::CNPLDispatcher::AddNPLConnection(const string& sNID, NPL::NPLConnection_ptr pConnection)
{
	ParaEngine::Lock lock_(m_mutex);

	if (pConnection->IsConnected())
	{
		// add to active connection map
		ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(sNID);
		if (iter != m_active_connection_map.end())
		{
			if (iter->second != pConnection)
			{
				// this should rarely happen. 
				iter->second->stop();
			}
		}
		m_active_connection_map[sNID] = pConnection;

		// erase from pending connection map
		iter = m_pending_connection_map.find(sNID);
		if (iter != m_pending_connection_map.end())
		{
			m_pending_connection_map.erase(iter);
		}
	}
}

bool NPL::CNPLDispatcher::RemoveNPLConnection(NPLConnection_ptr pConnection)
{
	ParaEngine::Lock lock_(m_mutex);

	bool bFound = false;
	const string& nid = pConnection->GetNID();
	if (!nid.empty())
	{
		// remove both 
		ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(nid);
		if (iter != m_active_connection_map.end() && (iter->second == pConnection))
		{
			m_active_connection_map.erase(iter);
			bFound = true;
		}

		iter = m_pending_connection_map.find(nid);
		if (iter != m_pending_connection_map.end() && (iter->second == pConnection))
		{
			m_pending_connection_map.erase(iter);
			bFound = true;
		}
	}
	return bFound;
}

void NPL::CNPLDispatcher::Cleanup()
{
	ParaEngine::Lock lock_(m_mutex);

	m_active_connection_map.clear();
	m_server_address_map.clear();
}

NPL::NPLRuntimeAddress_ptr NPL::CNPLDispatcher::GetNPLRuntimeAddress(const string& sNID)
{
	ParaEngine::Lock lock_(m_mutex);
	ServerAddressMap_Type::iterator iter = m_server_address_map.find(sNID);
	if (iter != m_server_address_map.end())
	{
		return iter->second;
	}
	return NPLRuntimeAddress_ptr();
}

bool NPL::CNPLDispatcher::AddNPLRuntimeAddress(NPLRuntimeAddress_ptr pAddress)
{
	ParaEngine::Lock lock_(m_mutex);
	if (m_server_address_map.find(pAddress->GetNID()) == m_server_address_map.end())
	{
		m_server_address_map[pAddress->GetNID()] = pAddress;
		return true;
	}
	return false;
}

void NPL::CNPLDispatcher::AddPublicFile(const string& filename, int nID)
{
	ParaEngine::Lock lock_(m_mutex);
	if (!filename.empty() && nID != -1)
	{
		// only add if not already added. 
		if ((m_public_filemap.left.find(nID) == m_public_filemap.left.end()) &&
			(m_public_filemap.right.find(filename) == m_public_filemap.right.end()))
		{
			m_public_filemap.insert(StringBimap_Type::value_type(nID, filename));
		}
	}
	else if (filename.empty())
	{
		StringBimap_Type::left_map::iterator iter = m_public_filemap.left.find(nID);
		if (iter != m_public_filemap.left.end())
		{
			m_public_filemap.left.erase(iter);
		}
	}
}

void NPL::CNPLDispatcher::ClearPublicFiles()
{
	ParaEngine::Lock lock_(m_mutex);
	m_public_filemap.clear();
}

bool NPL::CNPLDispatcher::GetPubFileNameByID(string& filename, int nID)
{
	ParaEngine::Lock lock_(m_mutex);
	StringBimap_Type::left_map::iterator iter = m_public_filemap.left.find(nID);
	if (iter != m_public_filemap.left.end())
	{
		filename = iter->second.c_str();
		return true;
	}
	return false;
}

int NPL::CNPLDispatcher::GetIDByPubFileName(const string& filename)
{
	if (!filename.empty())
	{
		ParaEngine::Lock lock_(m_mutex);
		StringBimap_Type::right_map::iterator iter = m_public_filemap.right.find(filename);
		if (iter != m_public_filemap.right.end())
		{
			return iter->second;
		}
	}
	return -1;
}

bool NPL::CNPLDispatcher::CheckPubFile(string& filename, int& nID)
{
	ParaEngine::Lock lock_(m_mutex);
	if (nID != 0)
	{
		StringBimap_Type::left_map::iterator iter = m_public_filemap.left.find(nID);
		if (iter != m_public_filemap.left.end())
		{
			if (filename != iter->second)
			{
				filename = iter->second;
			}
			return true;
		}
	}
	else if (!filename.empty())
	{
		StringBimap_Type::right_map::iterator iter = m_public_filemap.right.find(filename);
		if (iter != m_public_filemap.right.end())
		{
			nID = iter->second;
			return true;
		}
	}
	return false;
}

NPL::NPLReturnCode NPL::CNPLDispatcher::Activate_Async(const NPLFileName& file_name, const char * code /*= NULL*/, int nLength/*=0*/, int priority/*=0*/)
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
#ifdef __EMSCRIPTEN__
		auto nid = file_name.sNID;
		auto websocket = EmscriptenWebSocket::CreateGetWebSocket(file_name.sNID);
		if (websocket->IsConnected())
		{
			NPLMsgOut_ptr msg_out(new NPLMsgOut());
			CNPLMsgOut_gen writer(*msg_out);
			int file_id = GetIDByPubFileName(file_name.sRelativePath);
			if (nLength < 0) nLength = strlen(code);
			writer.AddFirstLine(file_name, file_id);
			writer.AddMsgBody(code, nLength, (nLength <= m_nCompressionThreshold ? 0 : m_nCompressionLevel));
			return websocket->Send(msg_out->GetBuffer().ToString()) ? NPL_OK : NPL_Error;
		}
		else
		{
			auto it = m_server_address_map.find(file_name.sNID);
			if (it != m_server_address_map.end())
			{
				auto ip = it->second->GetHost();
				auto port = it->second->GetPort();
				websocket->Connect("wss://" + ip + ":" + port + "/nplwebsocket");
				if (websocket->GetConnection() == nullptr)
				{
					auto connection = std::make_shared<CNPLConnection>();
					connection->SetNID(file_name.sNID.c_str());
					websocket->SetConnection(connection);
				} 
			}

			static thread_local NPLMsgIn s_input_msg;
			static thread_local NPLMsgIn_parser s_parser;
			static thread_local std::unordered_map<std::string, std::string> s_last_msg;
			auto on_msg = [this, websocket, nid](const std::string & msg) {
				auto& last_msg = s_last_msg[nid];
				auto last_msg_size = last_msg.size();
				last_msg.resize(last_msg.size() + msg.size());
				memcpy(last_msg.data() + last_msg_size, msg.data(), msg.size());

				boost::tribool result = true;
				auto curIt = last_msg.data();
				auto curEnd = curIt + last_msg.size();
				s_parser.reset();
				s_input_msg.reset();
				while (curIt != curEnd)
				{
					boost::tie(result, curIt) = s_parser.parse(s_input_msg, curIt, curEnd);
					if (result)
					{
						// std::cout << "receive nplwebsocket message: " << std::endl;
						// std::cout << s_input_msg.m_filename << std::endl;
						// std::cout << s_input_msg.m_n_filename << std::endl;
						// std::cout << s_input_msg.m_code << std::endl;
						s_input_msg.m_pConnection = websocket->GetConnection();
						this->DispatchMsg(s_input_msg);
						s_input_msg.m_pConnection = nullptr;
						last_msg.clear();
					}
					else
					{
						OUTPUT_LOG("parse nplwebsocket message failed!!!");
						static std::string s_temp;
						s_temp.resize(curEnd - curIt);
						memcpy(s_temp.data(), curIt, s_temp.size());
						last_msg.resize(s_temp.size());
						memcpy(last_msg.data(), s_temp.data(), s_temp.size());
						break;
					}
				}
			};

			auto on_close = [this, websocket, nid] () {
				s_last_msg[nid].clear();
				PostNetworkEvent(NPL_ConnectionDisconnected, websocket->GetConnection()->GetNID().c_str(), "websocket close");
			};
			websocket->SetOnReceive(on_msg);
			websocket->SetOnClose(on_close);
		}
#else 
		// this is a remote activation
		NPLConnection_ptr pConnection = CreateGetNPLConnectionByNID(file_name.sNID);
		if (pConnection)
		{
			// send via the connection. 
			return pConnection->SendMessage(file_name, code, nLength, priority);
		}
#endif
	}
	return NPL_Error;
}

int NPL::CNPLDispatcher::PostNetworkEvent(NPLReturnCode nNPLNetworkCode, const char * sNid, const char* sMsg)
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

NPL::NPLReturnCode NPL::CNPLDispatcher::DispatchMsg(NPLMsgIn& msg)
{
	if (!msg.bUseConnection)
		return NPL_Error;

	// this is a local activation
	NPLRuntimeState_ptr pRuntime = ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState(msg.m_rts_name);
	if (pRuntime)
	{
		if (msg.method.size() > 0 && (((byte)(msg.method[0])) > 127 /*== 0xff*/ || msg.method == "A"))
		{
			if (CheckPubFile(msg.m_filename, msg.m_n_filename))
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
				// add nid or tid so that the runtime state can have access to the current connection object. 
				// we append last to msg, so that nid or tid in the input message is overridden by the real value. 
				if (msg.m_pConnection->IsAuthenticated())
				{
					msg_->m_code.append("nid=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pConnection->GetNID());
					msg_->m_code.append("}");
				}
				else
				{
					msg_->m_code.append("tid=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pConnection->GetNID());
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
				PostNetworkEvent(NPL_ConnectionDisconnected, msg.m_pConnection->GetNID().c_str(), "connection_overwrite");
			}
			return NPL_OK;
		}
		else
		{
			// handle  websocket connection
			if (msg.m_n_filename == 0 && msg.m_filename == "/nplwebsocket")
			{
				std::unordered_map<std::string, std::string> headers;
				auto size = msg.headers.size();
				for (int i = 0; i < size; ++i)
				{
					headers.insert_or_assign(msg.headers[i].name, msg.headers[i].value);
				}
				if (headers["Connection"] == "Upgrade" && headers["Upgrade"] == "websocket" && headers["Sec-WebSocket-Version"] == "13" && headers["Sec-WebSocket-Key"] != "")
				{
					auto sec_websocket_key = headers["Sec-WebSocket-Key"];
					auto src_websocket_accept = sec_websocket_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
					src_websocket_accept = ParaEngine::StringHelper::base64(ParaEngine::StringHelper::sha1(src_websocket_accept, true));
					std::ostringstream oss;
					oss << "HTTP/1.1 101 Switching Protocols\r\n";
					oss << "Upgrade: websocket\r\n";
					oss << "Connection: Upgrade\r\n";
					oss << "Sec-WebSocket-Accept: " << src_websocket_accept << "\r\n";
					oss << "\r\n";
					auto text = oss.str();
					NPL_SetProtocol(msg.m_pConnection->GetNID().c_str(), CNPLConnection::ProtocolType::WEBSOCKET);
					// NPL_accept(msg.m_pConnection->GetNID().c_str(), "nplwebsocket");
					msg.m_pConnection->SendMessage(NPLFileName((msg.m_pConnection->GetNID() + ":http").c_str()), text.c_str(), text.size());
					msg.m_pConnection->SetNplWebSocket(true);
					return NPL_OK;
				}                                                    
			}

			// http message
			std::string filename;
			int nHTTPCode = -10;
			if (CheckPubFile(filename, nHTTPCode))
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
				for (int i = 0; i < nCount; ++i)
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
				if (msg.m_pConnection->IsAuthenticated())
				{
					msg_->m_code.append("nid=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pConnection->GetNID());
					msg_->m_code.append("}");
				}
				else
				{
					msg_->m_code.append("tid=");
					NPLHelper::EncodeStringInQuotation(msg_->m_code, msg_->m_code.size(), msg.m_pConnection->GetNID());
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

void NPL::CNPLDispatcher::NPL_accept(const char* sTID, const char* sNID /*= NULL*/)
{
	if (sTID != 0 && strcmp(sTID, sNID) != 0)
	{
		ParaEngine::Lock lock_(m_mutex);

		ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(sTID);
		if (iter != m_active_connection_map.end())
		{
			NPLConnection_ptr pConnection = iter->second;

			if (pConnection && pConnection->IsConnected())
			{
				pConnection->SetAuthenticated(true);
				RenameConnectionImp(pConnection, sNID);
			}
		}
	}
}

void NPL::CNPLDispatcher::NPL_SetProtocol(const char* nid, CNPLConnection::ProtocolType protocolType)
{
	NPLConnection_ptr conn = GetNPLConnectionByNID(nid);
	if (conn)
	{
		conn->SetProtocol(protocolType);
	}
}

void NPL::CNPLDispatcher::RenameConnection(NPLConnection_ptr pConnection, const char* sNID)
{
	ParaEngine::Lock lock_(m_mutex);
	RenameConnectionImp(pConnection, sNID);
}

// this is one atomic function, do not call directly. 
void NPL::CNPLDispatcher::RenameConnectionImp(NPLConnection_ptr pConnection, const char* sNID)
{
	// only accept if it connected. 
	if (pConnection && pConnection->IsConnected())
	{
		if (sNID != 0 && pConnection->GetNID() != sNID)
		{
			// Now accept it. 

			// remove sTID old connection
			ActiveConnectionMap_Type::iterator iter = m_active_connection_map.find(pConnection->GetNID());
			if (iter != m_active_connection_map.end())
			{
				m_active_connection_map.erase(iter);
			}

			iter = m_pending_connection_map.find(pConnection->GetNID());
			if (iter != m_pending_connection_map.end())
			{
				// PE_ASSERT(iter->second == pConnection);
				m_pending_connection_map.erase(iter);
			}

			// remove old sNID connection
			iter = m_active_connection_map.find(sNID);
			if (iter != m_active_connection_map.end())
			{
				if (iter->second != pConnection)
				{
					// this should rarely happen. 
#ifdef WIN32
					// in win32 server mode, we will not close old connection, 
					// since we may assign the same server, port name to the different nid such as in an All-In-One server. 
					OUTPUT_LOG("warning: connection %s is ignored and overridden by a new one\n", sNID);
#else
					iter->second->stop(true, 1);
					OUTPUT_LOG("warning: connection %s is stopped and overridden by a new one\n", sNID);
#endif

				}
			}

			// erase from pending connection map
			iter = m_pending_connection_map.find(sNID);
			if (iter != m_pending_connection_map.end())
			{
				if (iter->second != pConnection)
				{
					// this should rarely happen. 
					iter->second->stop(true, 1);
					OUTPUT_LOG("warning: connection %s is stopped and overridden by a new one\n", sNID);
				}
				m_pending_connection_map.erase(iter);
			}

			// add new active connection. 
			// modify the address nid
			pConnection->m_address->SetNID(sNID);
			// finally update the naming map. 
			m_active_connection_map[sNID] = pConnection;
		}
	}
}

