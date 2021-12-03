//-----------------------------------------------------------------------------
// Class:	NPL Router Strategy
// Authors:	Gosling
// Emails:	gosling@paraengine.com
// Company: ParaEngine Corporation
// Date:	2009.7.13
// Desc: cross-platformed 
//-----------------------------------------------------------------------------
#include "NPLRouter.h"

#include "NPLRouterStrategy.h"
#include "RouterMsgSend.h"

#ifdef WIN32

#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define vsnwprintf _vsnwprintf
#endif

/** easy define :-) */
#define ROUTER_LOG(...) SERVICE_LOG1(logger, ## __VA_ARGS__)

using namespace std;
using namespace NPL;

int NPLRouterStrategy::GetDBServerNID(double dlUserNID)
{
	int iTableIndex =  (((int64)dlUserNID) % m_nUserTableCount);
	return m_MyDBRouterTables[iTableIndex].iNid;
}


void NPLRouterStrategy::LoadConfigInfo( const string& sMsg )
{
	ParaEngine::Mutex::ScopedLock  lock_(m_mutex);
	NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg.c_str());
	double dlNid,dlTableBegin,dlTableEnd,dlCount = (double)tabMsg["count"];
	int iTableBegin,iTableEnd,iCount=(int)dlCount;
	char sDataName[16];

	dlResponseNid = (double)tabMsg["response_nid"];
	strGameServerScript = (const std::string &)tabMsg["game_server_script"];
	strDBServerScript = (const std::string &)tabMsg["db_server_script"];
	//OUTPUT_LOG("iCount:%d,responseID:%d,%f\n", iCount,int(dlResponseNid),(double)tabMsg["response_nid"]);

	int nTotalTables = 0;
	for(int i=0;i<iCount;i++)
	{
		sprintf(sDataName,"data%d",i);

		dlTableBegin = (double)(tabMsg[(const char *)sDataName]["table_begin"]);
		iTableBegin = (int)dlTableBegin;
		
		dlTableEnd = (double)(tabMsg[(const char *)sDataName]["table_end"]);
		iTableEnd = (int)dlTableEnd;

		m_MyDBRouterTables.resize(iTableEnd+1);
		m_nUserTableCount = iTableEnd+1;

		//OUTPUT_LOG("%d,end:%d:dataname:%s\n", iCount, iTableEnd, sDataName);
		for(int iTableIndex=iTableBegin; iTableIndex<=iTableEnd; iTableIndex++)
		{
			dlNid = (tabMsg[(const char *)sDataName]["db_nid"]);
			m_MyDBRouterTables[iTableIndex].iNid = (int)dlNid;
			//OUTPUT_LOG("%d,end:%d:nid:%d\n", iTableIndex, iTableEnd, MyDBRouterTable[iTableIndex].iNid);
		}
	}
#ifndef WIN32
	logger = ParaEngine::CServiceLogger::GetLogger("router");
#endif
	//logger->WriteServeviceFormated("|sMsg=%s",sMsg.c_str());
}

NPLRouterStrategy* NPLRouterStrategy::GetInstance()
{
	static NPLRouterStrategy singleton;
	return &singleton;
}

void NPLRouterStrategy::DoRouterStrategy(int nType, void* pVoid)
{
	if(nType == ParaEngine::PluginActType_STATE)
	{
		// Getting the runtime state (Thread) within which this file is activated. 

		NPL::INPLRuntimeState* pState = (NPL::INPLRuntimeState*)pVoid;
		const char* sMsg = pState->GetCurrentMsg();
		int nMsgLength = pState->GetCurrentMsgLength();

		//OUTPUT_LOG("activate input: %s\n", sMsg);

		NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
		const string& dest = tabMsg["dest"];

		//just get config info if opposite nid is 1000
		if(dest == "config")
		{
			LoadConfigInfo(sMsg);
			OUTPUT_LOG("=====  load config end and waiting for msg  =====\n");
			return;
		}

		NPLInterface::NPLObjectProxy msg;
		if(dest == "im")
			msg["action"] = tabMsg["action"];
		msg["ver"] = "1.0";
		msg["nid"] = dlResponseNid;
		msg["user_nid"] = (double)tabMsg["user_nid"];
		msg["game_nid"] = (double)tabMsg["game_nid"];
		msg["g_rts"] = tabMsg["g_rts"];
		msg["d_rts"] = tabMsg["d_rts"];
		msg["data_table"] = tabMsg["data_table"];
		std::string output;
		NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);
		
		char sActivateFile[256];
		if(dest == "db")
		{
			//go to db server
			int iRet=0;
			int iDBServerNid=0;
			double dlUserNid = 0;
			dlUserNid = (double)tabMsg["user_nid"];
			if(dlUserNid == 0)
			{
				iDBServerNid = GetDBServerNID(rand()%m_nUserTableCount);
			}
			else
			{
				iDBServerNid = GetDBServerNID(dlUserNid);
			}

			if( ((const std::string &)tabMsg["d_rts"]).length() == 0)
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"%d:%s",iDBServerNid,strDBServerScript.c_str());
			}
			else
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"(%s)%d:%s",((const std::string &)tabMsg["d_rts"]).c_str(),iDBServerNid,strDBServerScript.c_str());
			}
			if( (iRet = pState->activate(sActivateFile, output.c_str(), output.size())) != 0)
			{
#ifndef WIN32
				//OUTPUT_LOG("AddSendMsg (ignored):%s,%s\n",sActivateFile,output.c_str());
#endif
			}
			else
			{
				ROUTER_LOG("send|%s|%s\n",sActivateFile,sMsg);
			}

		}
		else if(dest == "game")
		{
			//from db server
			int iRet;
			double dlGameID = (double)tabMsg["game_nid"];
			if( ((const std::string &)tabMsg["g_rts"]).length() == 0)
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"%d:%s",(int)dlGameID,strGameServerScript.c_str());
			}
			else
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"(%s)%d:%s",((const std::string &)tabMsg["g_rts"]).c_str(),(int)dlGameID,strGameServerScript.c_str());
			}
			
			if( (iRet = pState->activate(sActivateFile, output.c_str(), output.size())) != 0)
			{
#ifndef WIN32
				//OUTPUT_LOG("AddSendMsg (ignored):%s,%s\n",sActivateFile,output.c_str());
#endif
			}
			else
			{
				ROUTER_LOG("send|%s|%s\n",sActivateFile,sMsg);
			}
		}
		else if(dest == "random")
		{
			//random to db server
			int iRet;
			snprintf(sActivateFile,sizeof(sActivateFile) -1,"%d:%s",GetDBServerNID(rand()%m_nUserTableCount),strDBServerScript.c_str());
			if( (iRet = pState->activate(sActivateFile, output.c_str(), output.size())) != 0)
			{
#ifndef WIN32
				// OUTPUT_LOG("AddSendMsg (ignored):%s,%s\n",sActivateFile,output.c_str());
#endif
			}
			else
			{
				ROUTER_LOG("send|%s|%s\n",sActivateFile,sMsg);
			}
		}
		else if(dest == "im")
		{
			//to im server
			int iRet;
			snprintf(sActivateFile,sizeof(sActivateFile) -1,"(1)IMServer1:IMServer.dll");
			if( (iRet = pState->activate(sActivateFile, output.c_str(), output.size())) != 0)
			{
#ifndef WIN32
				// OUTPUT_LOG("AddSendMsg (ignored):%s,%s\n",sActivateFile,output.c_str());
#endif
			}
			else
			{
				ROUTER_LOG("send|%s|%s\n",sActivateFile,sMsg);
			}
		}
		else
		{
			//err
			APP_LOG("no dest assigned, go to db server");
			//go to db server
			int iRet;

			if( ((const std::string &)tabMsg["d_rts"]).length() == 0)
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"%d:%s",GetDBServerNID((double)tabMsg["user_nid"]),strDBServerScript.c_str());
			}
			else
			{
				snprintf(sActivateFile,sizeof(sActivateFile) -1,"(%s)%d:%s",((const std::string &)tabMsg["d_rts"]).c_str(),GetDBServerNID((double)tabMsg["user_nid"]),strDBServerScript.c_str());
			}
			//OUTPUT_LOG("go to db server:%s\n", sActivateFile);
			if( (iRet = pState->activate(sActivateFile, output.c_str(), output.size())) != 0)
			{
#ifndef WIN32
				//RouterMsgSend::GetInstance()->AddSendMsg(sActivateFile,output.c_str(),pVoid);
				// OUTPUT_LOG("AddSendMsg (ignored):%s,%s\n",sActivateFile,output.c_str());
#endif
			}
			else
			{
				ROUTER_LOG("send|%s|%s\n",sActivateFile,sMsg);
			}
		}
	}
}
