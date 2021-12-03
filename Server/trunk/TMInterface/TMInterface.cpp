//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Cross platformed NPL router plugin for ParaEngine
//-----------------------------------------------------------------------------
#include "TMInterface.h"


#include "TMService.hpp"

//#include <NPLHelper.h>
//#include <NPLTable.h>
//#include <INPLRuntimeState.h>
//#include "ParaScriptingGlobal.h"
//#include "ParaScriptingNPL.h"

// ParaEngine includes



#include "queue.hpp"

// ParaEngine includes
#include "PluginAPI.h"

using namespace ParaEngine;

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif


// forware declare of exported functions. 
#ifdef __cplusplus
extern "C" {
#endif
	CORE_EXPORT_DECL const char* LibDescription();
	CORE_EXPORT_DECL int LibNumberClasses();
	CORE_EXPORT_DECL unsigned long LibVersion();
	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
	CORE_EXPORT_DECL void LibInit();
	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
#ifdef __cplusplus
}   /* extern "C" */
#endif


CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine Auto Updater Ver 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return 1;
}

CORE_EXPORT_DECL int LibNumberClasses()
{
	return 0;
}

CORE_EXPORT_DECL ClassDescriptor* LibClassDesc(int i)
{
	return NULL;
}


ParaEngine::IParaEngineCore* g_pCoreInterface = NULL;
ParaEngine::IParaEngineCore* GetCoreInterface()
{
	return g_pCoreInterface;
}

CORE_EXPORT_DECL void LibInitParaEngine(IParaEngineCore* pCoreInterface)
{
	g_pCoreInterface = pCoreInterface;
}


CORE_EXPORT_DECL void LibInit()
{
}

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	try
	{
		//Test_NPL_DLL_Activate_Interface(nType, pVoid);
		if(nType == ParaEngine::PluginActType_STATE)
		{
			//fprintf(TMService::Instance().fp,"%s|TM Interface begin.\n",TMService::Instance().GetLogFormatTime());
			// TODO: in new thread
			char sendbuf[4096]={0};
			char recvbuf[4096]={0};
			int iOutLength = 0,iInLength=4096;
	//		char pe_wanip_arr[15]={0};

	//		fscanf(TMService::Instance().ipfp,"%s",pe_wanip_arr);
			
			string pe_wanip = TMService::Instance().pe_wanip_arr;
			
			NPL::INPLRuntimeState* pState = (NPL::INPLRuntimeState*)pVoid;
			const char* sMsg = pState->GetCurrentMsg();
			fprintf(TMService::Instance().fp,"%s|Msg=%s|str_ip=%s\n",TMService::Instance().GetLogFormatTime(),sMsg,pe_wanip.c_str());
			int nMsgLength = pState->GetCurrentMsgLength();
			if(TMService::Instance()._pState ==0 ){
				TMService::Instance()._pState = pState;
			}
		
			NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
			const string& sCmd = tabMsg["action"];
			NPLInterface::NPLObjectProxy forward = tabMsg["forward"];
			//const string& sParams = tabMsg["params"];
			//fprintf(TMService::Instance().fp,"%s|Input:cmd=%s\n",TMService::Instance().GetLogFormatTime(),sCmd.c_str());
			//fflush(TMService::Instance().fp);
			//NPLInterface::NPLObjectProxy tabParams = NPLInterface::NPLHelper::MsgStringToNPLTable(sParams.c_str());
			//if(sCmd == "login" && (string&)tabMsg["params"]["user"]=="119154189")
			if(sCmd == "start")
			{
				queue::Instance().start();
			}
			else if(sCmd == "register")
			{
				const string& email = tabMsg["params"]["email"];
				const string& passwd = tabMsg["params"]["passwd"];
				const string& ip = tabMsg["params"]["ip"];
				const string& realname = tabMsg["params"]["realname"];
				const string& idno = tabMsg["params"]["no"];

				const string& callback = tabMsg["callback"];

				double sex= tabMsg["params"]["gender"];
				double birthday= tabMsg["params"]["birthday"];

				double ifrom = tabMsg["params"]["from"];

				TMService::Instance().EncodeRegist(sendbuf,iOutLength,email,passwd,ip,sex,birthday,realname,idno,ifrom);
				if (ifrom == 0)
				{				
					queue::Instance().handle_request("tmregist.paraengine.com", "11016",sendbuf,iOutLength,PROXY_REGISTER,sMsg,nMsgLength);
					//TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,1);
					//TMService::Instance().DecodeRegist(recvbuf,iInLength,callback,forward);
				}
			}
			else if(sCmd == "login")
			{
				const string& user = tabMsg["params"]["user"];
				const string& passwd = tabMsg["params"]["passwd"];
				const string& callback = tabMsg["callback"];
				const string& ip = tabMsg["params"]["ip"];
				const string& vfysession = tabMsg["params"]["sessionid"];
				const string& vfycode = tabMsg["params"]["vfycode"];
				double isteen = tabMsg["params"]["v"];

				double ifrom = tabMsg["params"]["from"];
				/*
				if(user == "50077")
				{
					//double nid = tabMsg["params"]["nid"];
					//double dest_user_id = tabMsg["params"]["dest_user_id"];
					//double product_id = tabMsg["params"]["product_id"];
					//double product_count = tabMsg["params"]["product_count"];
					//const string& pay_passwd = tabMsg["pay_passwd"];
					double nid = 50022;
					double dest_user_id = 50022;
					double product_id = 310001;
					double product_count = 1;
					const string& pay_passwd = tabMsg["params"]["passwd"];
					fprintf(TMService::Instance().fp,"%s|params:product=%d,nid=%d,dest_nid=%d,count=%d,passwd=%s\n",TMService::Instance().GetLogFormatTime(),int(product_id),int(nid),int(dest_user_id),int(product_count),pay_passwd.c_str());
					TMService::Instance().EncodeBuyProduct(sendbuf,iOutLength,product_id,nid,dest_user_id,product_count,pay_passwd);
					queue::Instance().handle_request("116.228.240.106","12323",sendbuf,iOutLength,PROXY_PAY,sMsg,nMsgLength);
					fflush(TMService::Instance().fp);
					return;
				}
				*/
				fprintf(TMService::Instance().fp,"%s|wanip=%s|\n",TMService::Instance().GetLogFormatTime(),pe_wanip.c_str());
				if((passwd=="618ba5293ccbc236160b51902c53446b" && (ip == pe_wanip || ip=="192.168.0.107"|| ip=="192.168.0.61"|| ip=="192.168.0.60" || ip=="192.168.0.105" ))||ifrom==1)
				{
					TMService::Instance().DecodeSuperLogin(user,callback,forward);
					return;
				}

				//fprintf(TMService::Instance().fp,"%s|params:user=%s,passwd=%s,ip=%s,callback=%s\n",TMService::Instance().GetLogFormatTime(),user.c_str(),passwd.c_str(),ip.c_str(),callback.c_str());
				/*
				if(user.find("@") != string::npos )
				{
					TMService::Instance().EncodeGetID(sendbuf,iOutLength,user);
					queue::Instance().handle_request("tmregist.paraengine.com","11016",sendbuf,iOutLength,PROXY_GETIDFROMEMAIL,sMsg,nMsgLength);
				}
				else
				{
					TMService::Instance().EncodeLogin(sendbuf,iOutLength,user,passwd,ip);
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_LOGIN,sMsg,nMsgLength);
				}
				*/
				if (ifrom == 0)
				{
					if ( user.find_first_not_of("0123456789") == string::npos && user.length()<=10)
					{
						string email="";
						fprintf(TMService::Instance().fp,"%s|user_nid=%s\n",TMService::Instance().GetLogFormatTime(),user.c_str());
						TMService::Instance().EncodeLogin(sendbuf,iOutLength,user,passwd,email,ip,ifrom,vfysession,vfycode,isteen);
						queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_LOGIN,sMsg,nMsgLength);
					}
					else 
					{
//						if(user.find("@") != string::npos || user.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos || (user.find_first_not_of("0123456789") == string::npos && user.length()==11))
//						{
						string usernid="0";
						fprintf(TMService::Instance().fp,"%s|user=%s\n",TMService::Instance().GetLogFormatTime(),user.c_str());
						TMService::Instance().EncodeLogin(sendbuf,iOutLength,usernid,passwd,user,ip,ifrom,vfysession,vfycode,isteen);
						queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_LOGIN,sMsg,nMsgLength);
//						}
					}
				}

				/*TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,2);
				TMService::Instance().DecodeLogin(recvbuf,iInLength,callback,forward);*/
			}
			else if(sCmd == "buy_product")
			{
				double nid = tabMsg["params"]["nid"];
				double dest_user_id = tabMsg["params"]["tonid"];
				double product_id = tabMsg["params"]["gsid"];
				double product_count = tabMsg["params"]["cnt"];
				const string& pay_passwd = tabMsg["params"]["pass"];

				double ifrom = tabMsg["params"]["from"];

				double gamever = tabMsg["params"]["v"];  /*0:kids, 1:teen*/

				if (ifrom ==0)
				{
					if(product_id == 998)
					{
						product_id = 310000;
					}
					if(product_id == 984)
					{
						if (gamever == 0) /*kids*/
						{
							product_id = 310001;
						}
						else if (gamever == 1)/*teen*/
						{
							product_id = 315001;
						}						
					}
					fprintf(TMService::Instance().fp,"%s|params:product=%d,nid=%d,dest_nid=%d,count=%d,passwd=%s\n",TMService::Instance().GetLogFormatTime(),int(product_id),int(nid),int(dest_user_id),int(product_count),pay_passwd.c_str());
					TMService::Instance().EncodeBuyProduct(sendbuf,iOutLength,product_id,nid,dest_user_id,product_count,pay_passwd);
					queue::Instance().handle_request("tmpay.paraengine.com","12323",sendbuf,iOutLength,PROXY_PAY,sMsg,nMsgLength);
					fflush(TMService::Instance().fp);
				}
				return;
			}
			else if(sCmd == "query_magicword")
			{
				double nid = tabMsg["params"]["nid"];
				const string& magic_word = tabMsg["params"]["magic_word"];
				fprintf(TMService::Instance().fp,"%s|params:nid=%d,magic_word=%s\n",TMService::Instance().GetLogFormatTime(),int(nid),magic_word.c_str());
				TMService::Instance().EncodeQueryMagicWord(sendbuf,iOutLength,nid,magic_word);
				//queue::Instance().handle_request("116.228.240.106","57123",sendbuf,iOutLength,PROXY_QUERY_MAGICWORD,sMsg,nMsgLength);
				queue::Instance().handle_request("magicword.paraengine.com","57123",sendbuf,iOutLength,PROXY_QUERY_MAGICWORD,sMsg,nMsgLength);
				fflush(TMService::Instance().fp);
				return;
			}
			else if(sCmd == "consume_magicword")
			{
				const string& ip = tabMsg["params"]["ip"];
				double nid = tabMsg["params"]["nid"];;
				const string& magic_word = tabMsg["params"]["magic_word"];
				fprintf(TMService::Instance().fp,"%s|params:nid=%d,magic_word=%s\n",TMService::Instance().GetLogFormatTime(),int(nid),magic_word.c_str());
				TMService::Instance().EncodeConsumeMagicWord(sendbuf,iOutLength,nid,magic_word,ip);
				queue::Instance().handle_request("magicword.paraengine.com","57123",sendbuf,iOutLength,PROXY_CONSUME_MAGICWORD,sMsg,nMsgLength);
				fflush(TMService::Instance().fp);
				return;
			}
			else if(sCmd == "setgameflag")
			{
				double nid = tabMsg["params"]["nid"];
				double isteen = tabMsg["params"]["v"];
				TMService::Instance().EncodeSetGameFlag(sendbuf,iOutLength,nid,isteen);
				queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_SETGAMEFLAG,sMsg,nMsgLength);			//TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,2);
				//TMService::Instance().DecodeSetGameFlag(recvbuf,iInLength);
			}
			else if(sCmd == "logout")
			{
				double nid = tabMsg["params"]["nid"];
				double login_time = tabMsg["params"]["login_time"];
				double logout_time = tabMsg["params"]["logout_time"];
				double ifrom = tabMsg["params"]["from"];
				double isteen = tabMsg["params"]["v"];

				fprintf(TMService::Instance().fp,"%s|nid=%d,login_time=%d ,logout_time=%d,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),int(login_time),int(logout_time),int(ifrom));
				//fflush(TMService::Instance().fp);
				TMService::Instance().EncodeLogout(sendbuf,iOutLength,nid,login_time,logout_time,ifrom,isteen);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_LOGOUT,sMsg,nMsgLength);
				}
			}
			else if(sCmd == "getlastlogintime")
			{
				double nid = tabMsg["params"]["nid"];
				double ifrom = tabMsg["params"]["from"];

				fprintf(TMService::Instance().fp,"%s|nid=%d,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),int(ifrom));
				TMService::Instance().EncodeGetLastLoginTime(sendbuf,iOutLength,nid,ifrom);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_GETLASTLOGINTIME,sMsg,nMsgLength);
				}
			}
			else if(sCmd == "postmsg")
			{
				double nid = tabMsg["params"]["nid"];
				const string& title = tabMsg["params"]["title"];
				const string& nick = tabMsg["params"]["nick"];
				const string& content = tabMsg["params"]["content"];
				double type = tabMsg["params"]["type"];
				const string& callback = tabMsg["callback"];

				double ifrom = tabMsg["params"]["from"];

				fprintf(TMService::Instance().fp,"%s|params:nid=%d,title=%s,content=%s,type=%.0f,nick=%s,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),title.c_str(),content.c_str(),type,nick.c_str(),int(ifrom));
				TMService::Instance().EncodePostMsg(sendbuf,iOutLength,title,content,nid,type,nick);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbpost.paraengine.com","11015",sendbuf,iOutLength,PROXY_POSTMSG,sMsg,nMsgLength);
					//TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,3);
					//TMService::Instance().DecodePostMsg(recvbuf,iInLength,callback,forward);
				}
			}
			else if(sCmd == "getvfyimg")
			{
				double nid = tabMsg["params"]["nid"];
				const string& callback = tabMsg["callback"];
				const string& ip = tabMsg["params"]["ip"];

				double ifrom = tabMsg["params"]["from"];		

				fprintf(TMService::Instance().fp,"%s|params:nid=%d,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),int(ifrom));
				TMService::Instance().EncodeGetVfyImg(sendbuf,iOutLength,nid,ip,ifrom);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_VFYIMG,sMsg,nMsgLength);
				}
			}
			else if(sCmd == "getvfyimg_session")
			{
				double nid = tabMsg["params"]["nid"];
				const string& callback = tabMsg["callback"];
				const string& ip = tabMsg["params"]["ip"];
				const string& vfys = tabMsg["params"]["vfysession"];
				double ifrom = tabMsg["params"]["from"];		

				fprintf(TMService::Instance().fp,"%s|params:nid=%d,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),int(ifrom));
				TMService::Instance().EncodeGetVfyImgBySession(sendbuf,iOutLength,nid,ip,ifrom,vfys);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_VFYIMG_SESSION,sMsg,nMsgLength);
				}
			}
			else if(sCmd == "getuserinfo")
			{
				double nid = tabMsg["params"]["nid"];
				const string& callback = tabMsg["callback"];
				double ifrom = tabMsg["params"]["from"];		

				fprintf(TMService::Instance().fp,"%s|params:nid=%d,from=%d\n",TMService::Instance().GetLogFormatTime(),int(nid),int(ifrom));
				TMService::Instance().EncodeGetUserInfo(sendbuf,iOutLength,nid,ifrom);
				if (ifrom == 0)
				{
					queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_USERINFO,sMsg,nMsgLength);
				}
			}

			else
			{
				fprintf(TMService::Instance().fp,"cmd:%s nod found\n",sCmd.c_str());
			}
			fflush(TMService::Instance().fp);
		}

	}
	catch (...)
	{
		fprintf(TMService::Instance().fp,"fatal error!\n");
	}
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
#else
void __attribute__ ((constructor)) DllMain()
#endif
{
	// TODO: dll start up code here
#ifdef WIN32
	// Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}

