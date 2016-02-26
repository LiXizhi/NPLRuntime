//-----------------------------------------------------------------------------
// Class:	ParaNetwork
// Authors:	Andy Wang, LiXizhi
// Emails:	mitnick_wang@hotmail.com
// Company: ParaEngine Dev Studio
// Date:	2006.9
// Desc: All are obsoleted
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaScriptingNetwork.h"
#include "NPLRuntime.h"
#include <string>
#include <map>

extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

using namespace ParaEngine;
using namespace std;
using namespace NPL;
using namespace luabind;

namespace ParaScripting
{
	void ParaNetwork::EnableNetwork( bool bEnable, const char* CenterName, const char* password )
	{
		if(!CGlobals::GetNetworkLayer())
			return;
		NPL::CNPLRuntime::GetInstance()->NPL_EnableNetwork(bEnable, CenterName, password);
	}

	bool ParaNetwork::IsNetworkLayerRunning()
	{
		return false;
	}

	void ParaNetwork::SetNerveCenterAddress( const char* pAddress )
	{
		APP_LOG("OBSOLETED API");
	}

	void ParaNetwork::SetNerveReceptorAddress( const char* pAddress )
	{
		APP_LOG("OBSOLETED API");
	}

#ifdef _OBSOLETED
	// ------------------- ParaNetwork -------------------

	bool ParaNetwork::IsNetworkLayerRunning()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->IsNetworkLayerRunning();

		return false;
	}

	const char * ParaNetwork::GetLocalIP()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalIP();

		return NULL;
	}

	unsigned short ParaNetwork::GetLocalNerveCenterPort()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalNerveCenterPort();
 
		return 0;
	}

	unsigned short ParaNetwork::GetLocalNerveReceptorPort()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalNerveReceptorPort();

		return 0;
	}

	const char * ParaNetwork::GetLocalNerveCenterName()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalNerveCenterName();

		return NULL;
	}

	const char * ParaNetwork::GetLocalNerveReceptorAccountName()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalNerveReceptorAccountName();

		return NULL;
	}

	const char * ParaNetwork::GetLocalNerveReceptorConnectWorldName()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetLocalNerveReceptorConnectWorldName();

		return NULL;
	}

	const char * ParaNetwork::GetNerveCenterUserMapAsString()
	{
		// TODO: test

		// map string format: "<size> (#<userName>)*N !"
		static char NerveCenterUserMapString[MAP_STRING_BUFFER_LENGTH];
		const map < int, string > * NerveCenterUserMap;
		NerveCenterUserMap = CGlobals::GetNetworkLayer()->GetNerveCenterUserMap();
		string parser;
		char sizeBuffer[16];
		itoa(NerveCenterUserMap->size(), sizeBuffer, 10);
		parser.assign(sizeBuffer);
		map < int, string >::const_iterator iter;
		map < int, string >::const_iterator iterEnd = NerveCenterUserMap->end();
		for ( iter = NerveCenterUserMap->begin() ;  iter != iterEnd ; iter++ )
		{
			parser += "#";
			parser += iter->second.c_str();
		}
		parser += "!";
		strcpy(NerveCenterUserMapString, parser.c_str());
		return NerveCenterUserMapString;

		return NULL;
	}

	const char * ParaNetwork::GetNerveReceptorUserMapAsString()
	{
		// TODO: test

		// map string format: "<size> (#<userName>)*N !"
		static char NerveReceptorUserMapString[MAP_STRING_BUFFER_LENGTH];
		const map < int, string > * NerveReceptorUserMap;
		NerveReceptorUserMap = CGlobals::GetNetworkLayer()->GetNerveReceptorUserMap();
		string parser;
		char sizeBuffer[16];
		itoa(NerveReceptorUserMap->size(), sizeBuffer, 10);
		parser.assign(sizeBuffer);
		map < int, string >::const_iterator iter;
		map < int, string >::const_iterator iterEnd = NerveReceptorUserMap->end();
		for ( iter = NerveReceptorUserMap->begin() ;  iter != iterEnd ; iter++ )
		{
			parser += "#";
			parser += iter->second.c_str();
		}
		parser += "!";
		strcpy(NerveReceptorUserMapString, parser.c_str());
		return NerveReceptorUserMapString;

		return NULL;
	}

	ParaPacket ParaNetwork::CreateParaPacket()
	{

		NPLPacket * pack = new NPLPacket;
		memset(pack, 0, sizeof(NPLPacket));
		return ParaPacket(pack);

		return ParaPacket(0);
	}

	ParaPacket* ParaNetwork::CreateParaPacket_()
	{

		static ParaPacket pkt;
		NPLPacket * pack = new NPLPacket;
		memset(pack, 0, sizeof(NPLPacket));
		pkt.m_pObj = pack;
		return &pkt;

	}

	void ParaNetwork::DestroyParaPacket(ParaPacket& packet)
	{

		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->DeallocatePacket(packet.GetNPLPacket());

	}

	void ParaNetwork::PushPacket(ParaPacket& packet)
	{

		if(CGlobals::GetNetworkLayer())
		{
			NPLPacket * pPacket = packet.GetNPLPacket();
			if (pPacket)
			{
				CGlobals::GetNetworkLayer()->PushPacket(pPacket);
				packet.InvalidateMe();
			}
		}

	}

	ParaPacket ParaNetwork::PopPacket()
	{

		if(CGlobals::GetNetworkLayer())
			return ParaPacket(CGlobals::GetNetworkLayer()->PopPacket());
		return ParaPacket();
	}

	ParaPacket* ParaNetwork::PopPacket_()
	{

		static ParaPacket pkt;
		if(CGlobals::GetNetworkLayer())
			pkt.m_pObj = CGlobals::GetNetworkLayer()->PopPacket();
		return &pkt;
	}

	void ParaNetwork::EnableLog(bool bEnable)
	{

		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->EnableLog(bEnable);
	}
	
	bool ParaNetwork::IsLogEnabled()
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->IsLogEnabled();
		return false;
	}
	
	int ParaNetwork::AddNamespaceRecord_s(const char * name, const char * IP, unsigned short port)
	{

		return (int)AddNamespaceRecord(name, IP, port);
	}

	int64 ParaNetwork::AddNamespaceRecord(const char * name, const char * IP, const unsigned short port)
	{

		// first remove and then insert.
		if(CGlobals::GetNetworkLayer())
		{
			CGlobals::GetNetworkLayer()->GetNameSpaceBinding()->RemoveNamespaceRecord(name);
			return CGlobals::GetNetworkLayer()->GetNameSpaceBinding()->AddNamespaceRecord(name, IP, port);
		}
		return 0;
	}
	
	bool ParaNetwork::RemoveNamespaceRecord(const char* name)
	{

		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetNameSpaceBinding()->RemoveNamespaceRecord(name);
		return false;
	}

	int64 ParaNetwork::InsertAccountRecord(const char * name, const char * password)
	{

		AccountInfo info;
		info.AccountName = name;
		info.Password = password;
		info.IsOnline = false;
		return CGlobals::GetNetworkLayer()->GetAccountManager()->InsertAccountRecord(&info);
	}

	bool ParaNetwork::VerifyAccountRecord(const char * name, const char * password)
	{

		if(CGlobals::GetNetworkLayer())
		{
			AccountInfo info;
			info.AccountName = name;
			info.Password = password;
			return CGlobals::GetNetworkLayer()->GetAccountManager()->VerifyAccountRecord(&info);
		}
		return false;
	}

	void ParaNetwork::ConnectToCenter( const char * nameSpaceName )
	{
		if(CGlobals::GetNetworkLayer())
		{
			const char * worldName = CGlobals::GetNetworkLayer()->GetLocalNerveReceptorConnectWorldName();
			if (strlen(worldName) != 0)
			{
				OUTPUT_LOG("Already login to world: %s\r\n", worldName);
				return ;
			}

			NPLPacket * pPacket = new NPLPacket();
			pPacket->SetType(ID_NPL_LOGIN_PING);
			pPacket->SetDestinationName(NULL, true, nameSpaceName);
			CGlobals::GetNetworkLayer()->PushPacket(pPacket);
		}
	}

	void ParaNetwork::Restart()
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->Restart();
	}

	void ParaNetwork::CloseConnection( const char * name )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->CloseConnection(name);
	}

	void ParaNetwork::AddToBanList( const char *IP, unsigned int milliseconds )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->AddToBanList(IP, milliseconds);
	}

	void ParaNetwork::RemoveFromBanList( const char *IP )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->RemoveFromBanList(IP);
	}

	void ParaNetwork::ClearBanList( void )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->ClearBanList();
	}

	bool ParaNetwork::IsBanned( const char *IP )
	{
		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->IsBanned(IP);
		return false;
	}

	const char* ParaNetwork::GetUserIP( const char * name )
	{
		if(CGlobals::GetNetworkLayer())
		{
			NPLBinaryAddress address =  CGlobals::GetNetworkLayer()->GetUserIP(name);
			return address.GetIPAsString();
		}
		return NULL;
	}

	void ParaNetwork::SetMaximumIncomingConnections( unsigned short numberAllowed )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->SetMaximumIncomingConnections(numberAllowed);
	}

	unsigned short ParaNetwork::GetMaximumIncomingConnections( void )
	{
		if(CGlobals::GetNetworkLayer())
			return CGlobals::GetNetworkLayer()->GetMaximumIncomingConnections();
		return 0;
	}

	object ParaNetwork::GetConnectionList( const object& remoteSystems )
	{
		if(!CGlobals::GetNetworkLayer())
			return object(remoteSystems);
		
		if(type(remoteSystems) == LUA_TTABLE)
		{
			object out(remoteSystems);
			list<string> conn_list;
			unsigned short num = 0;
			if(CGlobals::GetNetworkLayer()->GetConnectionList(&conn_list, &num))
			{
				list<string>::const_iterator iter, iterEnd = conn_list.end();
				for (iter = conn_list.begin();iter!=iterEnd;++iter)
				{
					out[*iter] = GetUserIP((*iter).c_str());
				}
			}
			return out;
		}
		else if(type(remoteSystems) == LUA_TNUMBER)
		{
			unsigned short num = 0;
			if(CGlobals::GetNetworkLayer()->GetConnectionList(0, &num))
			{
				return object(remoteSystems.interpreter(), (int)num);
			}
		}
		return object(remoteSystems);
	}

	void ParaNetwork::SetTimeoutTime( unsigned int timeMS, const char* target )
	{
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->SetTimeoutTime(timeMS, target);
	}

	const char* ParaNetwork::GetInternalID( void )
	{
		if(CGlobals::GetNetworkLayer())
		{
			NPLBinaryAddress address =  CGlobals::GetNetworkLayer()->GetInternalID();
			return address.ToString();
		}
		return NULL;
	}

	const char* ParaNetwork::GetExternalID( const char* target )
	{
		NPLBinaryAddress targetAddress;
		if(CGlobals::GetNetworkLayer() && targetAddress.FromString(target))
		{
			NPLBinaryAddress address =  CGlobals::GetNetworkLayer()->GetExternalID(targetAddress);
			return address.GetIPAsString();
		}
		else
			return CGlobals::GetString(G_STR_EMPTY).c_str();
	}

	object ParaNetwork::GetStatistics( const char* target, const object& data )
	{
		if(!CGlobals::GetNetworkLayer())
			return object(data);
		// TODO: 
		if(type(data) == LUA_TTABLE)
		{
			NPLBinaryAddress targetAddress = CGlobals::GetNetworkLayer()->GetUserIP(target);
			if(targetAddress != NPLBinaryAddress::UNASSIGNED_NPLBinaryAddress)
			{
				const RakNetStatistics*  statistics =  CGlobals::GetNetworkLayer()->GetCenterStatistics(targetAddress);
				if(statistics)
				{
					data["acknowlegementBitsSent"] = statistics->acknowlegementBitsSent;
					data["acknowlegementsPending"] = statistics->acknowlegementsPending;
					data["acknowlegementsReceived"] = statistics->acknowlegementsReceived;

					data["acknowlegementsSent"] = statistics->acknowlegementsSent;
					data["bandwidthExceeded"] = statistics->bandwidthExceeded;
					data["bitsPerSecond"] = statistics->bitsPerSecond;
					data["bitsReceived"] = statistics->bitsReceived;
					data["bitsWithBadCRCReceived"] = statistics->bitsWithBadCRCReceived;
					data["connectionStartTime"] = statistics->connectionStartTime;

					data["duplicateAcknowlegementsReceived"] = statistics->duplicateAcknowlegementsReceived;
					data["duplicateMessagesReceived"] = statistics->duplicateMessagesReceived;
					data["encryptionBitsSent"] = statistics->encryptionBitsSent;
					data["internalOutputQueueSize"] = statistics->internalOutputQueueSize;
					data["invalidMessagesReceived"] = statistics->invalidMessagesReceived;
					data["messageDataBitsResent"] = statistics->messageDataBitsResent;
					data["messageDataBitsSent"] = statistics->messageDataBitsSent;
					data["messagesReceived"] = statistics->messagesReceived;
					data["messagesSent"] = statistics->messagesSent;
					data["messagesTotalBitsResent"] = statistics->messagesTotalBitsResent;
					data["sequencedMessagesInOrder"] = statistics->sequencedMessagesInOrder;
					data["sequencedMessagesOutOfOrder"] = statistics->sequencedMessagesOutOfOrder;
					data["orderedMessagesInOrder"] = statistics->orderedMessagesInOrder;
					data["orderedMessagesOutOfOrder"] = statistics->orderedMessagesOutOfOrder;

					data["packetsReceived"] = statistics->packetsReceived;
					data["packetsSent"] = statistics->packetsSent;
					data["totalBitsSent"] = statistics->totalBitsSent;
					data["totalSplits"] = statistics->totalSplits;
				}
			}
		}
		return object(data);
	}
	object ParaNetwork::GetReceptorStatistics( const char* target, const object& data )
	{
		if(!CGlobals::GetNetworkLayer())
			return object(data);
		// TODO: 
		if(type(data) == LUA_TTABLE)
		{
			NPLBinaryAddress targetAddress = CGlobals::GetNetworkLayer()->GetUserIPOfReceptor(target);
			if(targetAddress != NPLBinaryAddress::UNASSIGNED_NPLBinaryAddress)
			{
				const RakNetStatistics*  statistics =  CGlobals::GetNetworkLayer()->GetReceptorStatistics(targetAddress);
				if(statistics)
				{
					data["acknowlegementBitsSent"] = statistics->acknowlegementBitsSent;
					data["acknowlegementsPending"] = statistics->acknowlegementsPending;
					data["acknowlegementsReceived"] = statistics->acknowlegementsReceived;

					data["acknowlegementsSent"] = statistics->acknowlegementsSent;
					data["bandwidthExceeded"] = statistics->bandwidthExceeded;
					data["bitsPerSecond"] = statistics->bitsPerSecond;
					data["bitsReceived"] = statistics->bitsReceived;
					data["bitsWithBadCRCReceived"] = statistics->bitsWithBadCRCReceived;
					data["connectionStartTime"] = statistics->connectionStartTime;

					data["duplicateAcknowlegementsReceived"] = statistics->duplicateAcknowlegementsReceived;
					data["duplicateMessagesReceived"] = statistics->duplicateMessagesReceived;
					data["encryptionBitsSent"] = statistics->encryptionBitsSent;
					data["internalOutputQueueSize"] = statistics->internalOutputQueueSize;
					data["invalidMessagesReceived"] = statistics->invalidMessagesReceived;
					data["messageDataBitsResent"] = statistics->messageDataBitsResent;
					data["messageDataBitsSent"] = statistics->messageDataBitsSent;
					data["messagesReceived"] = statistics->messagesReceived;
					data["messagesSent"] = statistics->messagesSent;
					data["messagesTotalBitsResent"] = statistics->messagesTotalBitsResent;
					data["sequencedMessagesInOrder"] = statistics->sequencedMessagesInOrder;
					data["sequencedMessagesOutOfOrder"] = statistics->sequencedMessagesOutOfOrder;
					data["orderedMessagesInOrder"] = statistics->orderedMessagesInOrder;
					data["orderedMessagesOutOfOrder"] = statistics->orderedMessagesOutOfOrder;

					data["packetsReceived"] = statistics->packetsReceived;
					data["packetsSent"] = statistics->packetsSent;
					data["totalBitsSent"] = statistics->totalBitsSent;
					data["totalSplits"] = statistics->totalSplits;
				}
			}
		}
		return object(data);
	}
	const char* ParaNetwork::GetStatisticsAsString( const char* target, int verbosityLevel)
	{
		if(!CGlobals::GetNetworkLayer())
			return NULL;
		NPLBinaryAddress targetAddress = CGlobals::GetNetworkLayer()->GetUserIP(target);
		if(targetAddress != NPLBinaryAddress::UNASSIGNED_NPLBinaryAddress)
		{
			RakNetStatistics*  statistics =  CGlobals::GetNetworkLayer()->GetCenterStatistics(targetAddress);
			if(statistics)
			{
				static char g_statistics[2048];
				StatisticsToString( statistics, g_statistics, verbosityLevel );
				return g_statistics;
			}
		}
		return NULL;
	}

	const char* ParaNetwork::GetReceptorStatisticsAsString( const char* target, int verbosityLevel)
	{
		if(!CGlobals::GetNetworkLayer())
			return NULL;
		string sTarget;
		if(target == NULL || target[0]=='\0')
		{
			sTarget = "@";
			sTarget += CGlobals::GetNetworkLayer()->GetLocalNerveReceptorConnectWorldName();
			target = sTarget.c_str();
		}

		NPLBinaryAddress targetAddress = CGlobals::GetNetworkLayer()->GetUserIPOfReceptor(target);
		if(targetAddress != NPLBinaryAddress::UNASSIGNED_NPLBinaryAddress)
		{
			RakNetStatistics*  statistics =  CGlobals::GetNetworkLayer()->GetReceptorStatistics(targetAddress);
			if(statistics)
			{
				static char g_statistics[2048];
				StatisticsToString( statistics, g_statistics, verbosityLevel );
				return g_statistics;
			}
		}
		return NULL;
	}

	void ParaNetwork::SetNerveCenterAddress( const char* pAddress )
	{
		if(!CGlobals::GetNetworkLayer())
			return;
		NPLBinaryAddress address;
		address.FromString(pAddress);
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->SetNerveCenterAddress(&address);
	}

	void ParaNetwork::SetNerveReceptorAddress( const char* pAddress )
	{
		if(!CGlobals::GetNetworkLayer())
			return;
		NPLBinaryAddress address;
		address.FromString(pAddress);
		if(CGlobals::GetNetworkLayer())
			CGlobals::GetNetworkLayer()->SetNerveReceptorAddress(&address);
	}
	
	// ------------------- ParaPacket -------------------

	void ParaPacket::Release()
	{
		if (m_pObj)
		{
			m_pObj->Release();
			m_pObj = NULL;
		}
	}

	void ParaPacket::ReleasePacketData()
	{
		if (m_pObj)
		{
			m_pObj->Release();
			m_pObj = NULL;
		}
	}
	int ParaPacket::GetType() const
	{
		if(m_pObj)
			return (int)m_pObj->type;
		else
			return 0;
	}
	void ParaPacket::SetType( int type )
	{
		if(m_pObj)
			m_pObj->type = (NPL_PACKET_TYPE)type;
	}
	string ParaPacket::GetDestName() const
	{
		string name = "";
		if(m_pObj)
		{
			name.assign(m_pObj->destName);
			return name;
		}
		else
		{
			return "";
		}
	}
	void ParaPacket::SetDestName( const char * destName)
	{
		if(m_pObj)
		{
			strcpy(m_pObj->destName, destName);
		}
	}
	string ParaPacket::GetSrcName() const
	{
		string name = "";
		if(m_pObj)
		{
			name.assign(m_pObj->srcName);
			return name;
		}
		else
		{
			return "";
		}
	}
	void ParaPacket::SetSrcName( const char * srcName)
	{
		if(m_pObj)
		{
			strcpy(m_pObj->srcName, srcName);
		}
	}
	unsigned long ParaPacket::GetSize() const
	{
		if(m_pObj)
			return m_pObj->size;
		else
			return 0;
	}
	string ParaPacket::GetData() const
	{
		string name = "";
		if(m_pObj)
		{
			name.assign(m_pObj->data);
			return name;
		}
		else
			return "";
	}
	void ParaPacket::SetDataAndSize( const char * data, unsigned long size)
	{
		// TODO: check memory allocation logic
		if(m_pObj)
		{
			if( (data == NULL) && (size == 0) )
			{
				// data and size both 0
				if (m_pObj->data)
				{
					delete [] m_pObj->data;
					m_pObj->data = NULL;
				}
				m_pObj->size = 0;
				return;
			}

			if (m_pObj->data)
			{
				delete [] m_pObj->data;
			}
			char * packetData =  new char[size];
			memcpy(packetData, data, size);
			m_pObj->data = packetData;
			m_pObj->size = size;
		}
	}

	void ParaPacket::InvalidateMe()
	{
		m_pObj = NULL;
	}
	//void ParaNetwork::SendPacketFromString(const char * packetString)
	//{
	//	// TODO: test
	//	// map string format: "<type>#<destName>#<srcName>#<size>#<data>"
	//	NPLPacket * pPacket = new NPLPacket;
	//	size_t beginTag = 0;
	//	size_t endTag = 0;

	//	string parserString;
	//	string strType;
	//	string strDestName;
	//	string strSrcName;
	//	string strSize;
	//	parserString.assign(packetString);
	//	endTag = parserString.find_first_of('#', beginTag) + 1;
	//	strType.assign(parserString, beginTag, endTag - beginTag - 1);
	//	beginTag = endTag;
	//	endTag = parserString.find_first_of('#', beginTag) + 1;
	//	strDestName.assign(parserString, beginTag, endTag - beginTag - 1);
	//	beginTag = endTag;
	//	endTag = parserString.find_first_of('#', beginTag) + 1;
	//	strSrcName.assign(parserString, beginTag, endTag - beginTag - 1);
	//	beginTag = endTag;
	//	endTag = parserString.find_first_of('#', beginTag) + 1;
	//	strSize.assign(parserString, beginTag, endTag - beginTag - 1);

	//	char * data = new char[strSize];

	//	pPacket->type = atoi(strType.c_str());
	//	strcpy(pPacket->destName, strDestName.c_str());
	//	strcpy(pPacket->srcName, strSrcName.c_str());
	//	pPacket->size = atoi(strSize.c_str());
	//	memcpy(data, packetString + endTag, pPacket->size);
	//	pPacket->data = data;
	//}
#endif
}