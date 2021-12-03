//-----------------------------------------------------------------------------
// Class:	NPL Router Strategy
// Authors:	Gosling
// Emails:	gosling@paraengine.com
// Company: ParaEngine Corporation
// Date:	2009.7.13
// Desc: cross-platformed 
//-----------------------------------------------------------------------------
#ifndef WIN32
#include "NPLRouter.h"
#include "RouterMsgSend.h"

#include "NPLRouterStrategy.h"

#include <sys/time.h>
#include<list>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>



/** easy define :-) */
#define ROUTER_LOG(...) SERVICE_LOG1(logger, ## __VA_ARGS__)

using namespace std;

RouterMsgSend* RouterMsgSend::GetInstance()
{
	static RouterMsgSend singleton;
	return &singleton;
}

void  RouterMsgSend::AddSendMsg(char* sActivateFile,const char *output,void* pVoid)
{
	MsgInfo myMsgInfo;
	myMsgInfo.iTries = 5;
	myMsgInfo.output.assign(output);
	myMsgInfo.sActivateFile.assign(sActivateFile);
	myMsgInfo.pState = (NPL::INPLRuntimeState*)pVoid;

	ParaEngine::Lock  lock_(m_mutex);

	if(MsgList.empty())
	{
		iListCount = 0;
		MsgList.push_back(myMsgInfo);
		if( setitimer( ITIMER_REAL, &t, NULL) < 0 )
		{
			OUTPUT_LOG("setitimer err!\n");
		}
		signal( SIGALRM, AlarmHandler);
	}
	else
	{
		iListCount++;
		MsgList.push_back(myMsgInfo);
	}
}

void RouterMsgSend::SendTimeOutMsg()
{
	list<MsgInfo>::iterator iter;
	ParaEngine::Lock  lock_(m_mutex);
	int iRet;
	for(iter = MsgList.begin();iter != MsgList.end(); iter++)
	{
		iRet = ((*iter).pState)->activate((*iter).sActivateFile.c_str(), (*iter).output.c_str(), (*iter).output.size());
		if( iRet != 0)
		{
			OUTPUT_LOG("Resend failed,count=%d,%s,%s!\n",(*iter).iTries, (*iter).sActivateFile.c_str(), (*iter).output.c_str());
			(*iter).iTries--;
			if((*iter).iTries <= 0)
			{
				iListCount--;
			}
		}
		else
		{
			OUTPUT_LOG("Resend success,count=%d,%s,%s!\n",(*iter).iTries, (*iter).sActivateFile.c_str(), (*iter).output.c_str());
			(*iter).iTries = 0;
			iListCount--;
		}
	}
	MsgList.remove_if(is_zero<MsgInfo>());
	if(MsgList.empty())
	{
		//no alarm!
		struct itimerval t0;
		t0.it_value.tv_sec = 0;
		t0.it_value.tv_usec = 0;
		t0.it_interval.tv_sec = 0;
		t0.it_interval.tv_usec = 0;
		if( setitimer( ITIMER_REAL, &t0, NULL) < 0 )
		{
			OUTPUT_LOG("delete timer err!\n");
		}
	}
}

void AlarmHandler(int a)
{
	RouterMsgSend::GetInstance()->SendTimeOutMsg();
}

#endif

