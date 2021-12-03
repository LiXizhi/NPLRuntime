#include "TMInterface.h"
#ifndef WIN32
#include <netdb.h>
#endif
#include "queue.hpp"
#include "TMService.hpp"
#include "MD5.h"

#ifdef WIN32
INT inet_aton(PCTSTR pszAddrString,PVOID pAddrBuf)
{
	return InetPton(AF_INET, pszAddrString, pAddrBuf);
}
#endif

int TMService::GetHeadLength(int iProxyFlag)
{
	if(iProxyFlag == PROXY_LOGIN || iProxyFlag == PROXY_SETGAMEFLAG || iProxyFlag == PROXY_POSTMSG|| iProxyFlag == PROXY_GETIDFROMEMAIL|| iProxyFlag == PROXY_GETEMAILFROMID || iProxyFlag == PROXY_GETLASTLOGINTIME || iProxyFlag == PROXY_VFYIMG || iProxyFlag == PROXY_VFYIMG_SESSION || iProxyFlag ==PROXY_USERINFO)
	{
		return DBPROXY_HEAD_LENGTH;
	}
	else if(iProxyFlag == PROXY_REGISTER)
	{
		return HEAD_LENGTH;
	}
	else if(iProxyFlag == PROXY_PAY)
	{
		return PAY_HEAD_LENGTH;
	}
	else if(iProxyFlag == PROXY_QUERY_MAGICWORD || iProxyFlag == PROXY_CONSUME_MAGICWORD)
	{
		return MAGICWORD_HEAD_LENGTH;
	}
	return -1;
}

int TMService::GetMsgLength(int iProxyFlag,char * recvbuf, int iHeadLength)
{
	if(iProxyFlag == PROXY_LOGIN || iProxyFlag == PROXY_SETGAMEFLAG || iProxyFlag == PROXY_POSTMSG|| iProxyFlag == PROXY_GETIDFROMEMAIL|| iProxyFlag == PROXY_GETEMAILFROMID || iProxyFlag == PROXY_GETLASTLOGINTIME || iProxyFlag == PROXY_VFYIMG || iProxyFlag == PROXY_VFYIMG_SESSION || iProxyFlag ==PROXY_USERINFO)
	{
		TMDBProxyHead msgHead;
		msgHead.Decode(recvbuf,iHeadLength);
		return msgHead.PkgLen;
	}
	else if(iProxyFlag == PROXY_REGISTER)
	{
		MsgTMHead msgHead;
		msgHead.Decode(recvbuf,iHeadLength);
		return msgHead.PkgLen;
	}
	else if(iProxyFlag == PROXY_PAY)
	{
		TMPayHead msgHead;
		msgHead.Decode(recvbuf,iHeadLength);
		return msgHead.PkgLen;
	}
	else if(iProxyFlag == PROXY_QUERY_MAGICWORD || iProxyFlag == PROXY_CONSUME_MAGICWORD)
	{
		TMMagicWordHead msgHead;
		msgHead.Decode(recvbuf,iHeadLength);
		return msgHead.PkgLen;
	}
	return -1;
}

void TMService::EncodeRegist(char *pkgbuf,int &iOutLength,const string& email,const string& passwd,const string& ip, const char& sex,const unsigned int& birthday,const string& realname, const string& idno,const double ifrom)
{
	MsgTMHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;
	MsgTMRegister msgRegister;
	//char sSrcString[32];
	//sprintf(sSrcString,"passwd");

	//unsigned char md5value[MD5::LENGTH];
	//MD5::GenerateDigest((const unsigned char*)(passwd.c_str()), passwd.length(), md5value);
	//string sDlMD5String = MD5::ToHexString(md5value);
	//sprintf(msgRegister.passwd,"%s",sDlMD5String.c_str());
	memcpy(msgRegister.passwd, passwd.c_str(), min(REGISTER_PASSWD_LENGTH, (int)passwd.size()));

	if(ip != "")
	{
		//struct hostent *remoteHost = gethostbyname(ip.c_str());
		//msgRegister.ip = *(u_long *) remoteHost->h_addr_list[0];

		struct in_addr inp;

		int r0 = inet_aton(ip.c_str(),&inp);
		if (r0!=0)
		{
			msgRegister.ip = inp.s_addr;
		}
	}

	snprintf(msgRegister.email,EMAIL_LENGTH,"%s",email.c_str());
	msgRegister.sex[0]=sex;
	msgRegister.birthday = birthday;
	
	if (realname != "")
	{	snprintf(msgRegister.realname,REALNAME_LENGTH,"%s", realname.c_str()); }

	if (idno != "")
	{	snprintf(msgRegister.idno, IDNO_LENGTH,"%s", idno.c_str());	}	
	
	msgRegister.regchannel = CHANNEL_ID;
	msgRegister.regfrom = GAME_ID;

	msgRegister.Encode(pkgbuf+HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	int bodylength = iOutLength;

	string sTemp=MD5::ToHexFormat(pkgbuf+HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength);
	fprintf(fp,"%s|body length:%d,%s\n",GetLogFormatTime(),bodylength,sTemp.c_str());

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);
	}
	
	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);
	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|verify head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	msgHead.PkgLen = bodylength+HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Version = 0;
	msgHead.CommandID = CMD_REGISTER;
	msgHead.UserID = 0;
	msgHead.Result = 0;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeRegist(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	MsgTMHead msgHead;
	MsgTMRegister msgRegister;
	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	if(iInLength < HEAD_LENGTH)
	{
		result = 501;			
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|len:%d,Version:%d,commandId:%d,UserID:%d,Regist Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Version,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.Result == 20010)
		{
			result = 404;
		}
		else if(msgHead.Result == 10007)
		{
			result = 401;
		}
		else if(msgHead.PkgLen > HEAD_LENGTH)
		{
			msgRegister.Decode(pkgbuf+HEAD_LENGTH,msgHead.PkgLen-HEAD_LENGTH);
			string sTemp=MD5::ToHexFormat(msgRegister.session,SESSION_LENGTH);
			msg["nid"] = (double)msgHead.UserID;
		}
		else
		{
			result = 501;
		}
	}

	msg["forward"] = forward;
	msg["result"] = double(result);
	if(result != 0)
	{
		fprintf(fp,"%s|Regist Result:%d\n",GetLogFormatTime(),result);
	}

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"out:%s\n",output.c_str());

	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeGetID(char *pkgbuf,int &iOutLength,const string& email,double ifrom)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetID msgGetID;
	MsgTMChannelVerifyHead msgChannelVerifyHead;
	string sTemp;

	//sprintf(msgGetID.email,"%s",email.c_str());
	snprintf(msgGetID.email,EMAIL_LENGTH-1,"%s",email.c_str());

	//msgGetID.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	msgGetID.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	//string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	printf("body length:%d,%s\n",iOutLength,sTemp.c_str());

	int bodylength=iOutLength;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] ='\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);		
	}

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);
	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	
	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	//msgHead.PkgLen = iOutLength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_GET_TAOMEE_ID;
	msgHead.Result = 0;
	msgHead.UserID = 0;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeGetID(const char *pkgbuf, const int iInLength, char * sMsg,int nMsgLength,const  string& passwd,const  string& ip, const string& vfysession, const string& login_vfycode,const string& callback, NPLInterface::NPLObjectProxy& forward,double isteen)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetID msgGetID;
	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	if(iInLength < DBPROXY_HEAD_LENGTH)
	{
		fprintf(fp,"%s|recv err,length=%d\n",GetLogFormatTime(),iInLength);
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
		{
			msgGetID.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH-VERIFYCHANNE_HEAD_LENGTH);
			fprintf(fp,"%s|return:%d,nid=%d\n",GetLogFormatTime(),msgGetID.map_userid,msgGetID.map_userid);
		}
	}
	if(msgHead.Result != 0)
	{
		result = 419;
		msg["forward"] = forward;
		msg["result"] = double(result);
		fprintf(fp,"%s|Login Result:%d\n",GetLogFormatTime(),result);
		
		std::string output;
		NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

		if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
		//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
		{
			fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
			return;
		}
		fprintf(TMService::Instance().fp,"--------------------\n");
		fflush(TMService::Instance().fp);
		return ;
	}
	char sendbuf[4096]={0};
	char user[64]={0};
	int iOutLength = 0;
	snprintf(user,63,"%d",msgGetID.map_userid);
	string email="";
	double ifrom=forward["data_table"]["req"]["from"];
	
	TMService::Instance().EncodeLogin(sendbuf,iOutLength,user,passwd,email,ip,ifrom,vfysession,login_vfycode,isteen);
	queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_LOGIN,sMsg,nMsgLength);
	fprintf(TMService::Instance().fp,"%s|len:%d,msg:%s\n",GetLogFormatTime(),nMsgLength,sMsg);
	fflush(TMService::Instance().fp);
}

//uint32 TMService::GetIDFromEmail(const string email)
//{
//	char sendbuf[4096]={0};
//	char recvbuf[4096]={0};
//	int iOutLength = 0,iInLength=4096;
//	uint32 nid;
//	EncodeGetID(sendbuf,iOutLength,email);
//	DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,2);
//	DecodeGetID(recvbuf,iInLength,nid);
//
//	return nid;
//}


void TMService::EncodeGetEmailByID(char *pkgbuf,int &iOutLength,double usernid, double ifrom)
{
	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;
	string sTemp;

	//uint32 nid;
	//nid = strtoul(usernid.c_str(),NULL,10);

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);		
	}
		
	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,0,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	//msgHead.PkgLen = DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_GET_EMAIL_BYID;
	msgHead.Result = 0;
	//msgHead.UserID = nid;
	msgHead.UserID = usernid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|GetEmailByID head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeGetEmailByID(const char *pkgbuf, const int iInLength, char * sMsg,int nMsgLength,const int gameflag,const string& sessionid,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetEmailByID msgGetEmailByID;
	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	string email="";

	if(iInLength < DBPROXY_HEAD_LENGTH)
	{
		fprintf(fp,"%s|GetEmailByID recv err,length=%d\n",GetLogFormatTime(),iInLength);
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%u,Result:%d,seesion:%s\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result,sessionid.c_str());
		if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
		{
			msgGetEmailByID.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
			email= msgGetEmailByID.email;
			fprintf(fp,"%s|return:email=%s\n",GetLogFormatTime(),email.c_str());			
		}
	}

	msg["forward"] = forward;
	msg["result"] = double(result);
	if(msgHead.Result !=0)
	{
		fprintf(fp,"%s|GetEmailByID Result:%d\n",GetLogFormatTime(),msgHead.Result);
	}

	//msg["gameflag"] = gameflag.c_str();
	msg["gameflag"] = double(gameflag);
	msg["sessionid"]= sessionid.c_str();
	msg["email"] = email;
	msg["nid"] = (double)msgHead.UserID;
	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}
/*
void TMService::EncodeLogin(char *pkgbuf,int &iOutLength,const  string& user,const  string& passwd,const  string& ip)
{
	uint32 nid;
	if(user.find("@") != string::npos )
	{
		nid=0;
	}
	else
	{
		nid = strtoul(user.c_str(),NULL,10);
	}
	//fprintf(fp,"try login:nid=%d\n",nid);
	TMDBProxyHead msgHead;
	TMDBProxyLogin msgLogin;

	
	if(ip != "")
	{
		struct hostent *remoteHost = gethostbyname(ip.c_str());
		msgLogin.ip = *(u_long *) remoteHost->h_addr_list[0];
	}

	msgLogin.SetPassword(passwd.c_str(),32);
	msgLogin.login_channel = 4;

	msgLogin.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|body length:%d,%s,ip=%u\n",GetLogFormatTime(),iOutLength,sTemp.c_str(),msgLogin.ip);

	msgHead.PkgLen = iOutLength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_LOGIN;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

}

void TMService::DecodeLogin(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBProxyLogin msgLogin;
	NPLInterface::NPLObjectProxy msg;
	int result = 0;

	if(iInLength < DBPROXY_HEAD_LENGTH)
	{
		result = 501;			
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%d,Login Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.Result == 1105)
		{
			result = 419;
		}
		else if(msgHead.Result == 1103)
		{
			result = 407;
		}
		else if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
		{
			msgLogin.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
			fprintf(fp,"%s|return:%d,email=%s,result=%d\n",GetLogFormatTime(),msgLogin.game_flag,msgLogin.email,result);
			msg["nid"] = (double)msgHead.UserID;
			msg["email"] = msgLogin.email;
		}
		else
		{
			result = 501;
		}
	}

	msg["forward"] = forward;
	msg["result"] = double(result);
	if(result != 0)
	{
		fprintf(fp,"%s|Login Result:%d\n",GetLogFormatTime(),result);
	}
	if(msgLogin.game_flag & 0x100000)
	{
		msg["gameflag"] = "true";
	}
	else
	{
		msg["gameflag"] = "false";
	}

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}
*/


void TMService::EncodeLogin(char *pkgbuf,int &iOutLength,const string& user, const string& passwd, const string& email, const string& ip,const double ifrom, const string& vfysession, const string& login_vfycode, const double isteen)
{
	uint32 nid;

	nid = strtoul(user.c_str(),NULL,10);
    printf("try login:nid=%d\n",nid);

	TMDBProxyHead msgHead;
    MsgTMChannelVerifyHead msgChannelVerifyHead;
    TMDBProxyLogin msgLogin;

    unsigned char md5value[MD5::LENGTH];
    unsigned char md5value_32[32];
	string sTemp;
    //unsigned char passwd2md5[MD5::LENGTH];

    MD5::GenerateDigest((const unsigned char*)(passwd.c_str()), passwd.length(), md5value);
    string sDlMD5String = MD5::ToHexString(md5value);
	//printf("md5:%s\n",sDlMD5String.c_str());
	//MD5::GenerateDigest((const unsigned char*)(sDlMD5String.c_str()), sDlMD5String.length(), passwd2md5);
	//memcpy(msgLogin.passwd,passwd2md5,16);

	memcpy(msgLogin.passwd,md5value,16);
	snprintf(msgLogin.email,EMAIL_LENGTH,"%s",email.c_str());	
	msgLogin.channelID = CHANNEL_ID;
	msgLogin.region=0;

	if (isteen)
	{
		msgLogin.gameid=22;
	}
	else
	{
		msgLogin.gameid=21;
	}
	

	if(ip != "")
	{
//		struct hostent *remoteHost = gethostbyname(ip.c_str());
//		msgLogin.ip = *(u_long *) remoteHost->h_addr_list[0];
		struct in_addr inp;
		int r0 = inet_aton(ip.c_str(),&inp);
		if (r0!=0)
		{
			msgLogin.ip = inp.s_addr;
		}
	}

	fprintf(fp,"%s|vfy_session:%s,login_vfycode:%s,email:%s\n",GetLogFormatTime(),vfysession.c_str(),login_vfycode.c_str(),email.c_str());
	fflush(TMService::Instance().fp);

	if (vfysession != "")
	{
		MD5::ToHex2(msgLogin.vfy_session,(const unsigned char*)(vfysession.c_str()),SESSION_LENGTH);
	}
	else
	{
		memset(msgLogin.vfy_session,0,SESSION_LENGTH);
	}

	if (login_vfycode != "")
	{
		memset(msgLogin.vfy_code,0,VFYCODE_LENGTH);
		snprintf(msgLogin.vfy_code,VFYCODE_LENGTH,"%s",login_vfycode.c_str());
	}	


	//msgLogin.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	msgLogin.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	//string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	/*string vsTemp, vcTemp;
	vsTemp.insert(0, msgLogin.vfy_session);
	vsTemp[SESSION_LENGTH]='\0';
	vcTemp.insert(0, msgLogin.vfy_code);
	vcTemp[VFYCODE_LENGTH]='\0';*/

	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	//fprintf(fp,"%s|body length:%d,%s,ip=%u,vfysession=%s,vfycode=%s,invfs=%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str(),msgLogin.ip,vsTemp.c_str(),vcTemp.c_str(),vfysession.c_str());
	fprintf(fp,"%s|body length:%d,%s,invfs=%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str(),vfysession.c_str());
	int bodylength=iOutLength;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);		
	}
	
	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	//msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_LOGIN;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	string sTemp0=reg_channelkey;

	fprintf(fp,"%s|reg_channelkey:%s\n",GetLogFormatTime(),sTemp0.c_str());	
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());	

	iOutLength = msgHead.PkgLen;
}

//#define LOGDEBUG(X)   fprintf(fp, X);fflush(fp);

void TMService::DecodeLogin(const char *pkgbuf, const int iInLength,char * sMsg,int nMsgLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBProxyLogin msgLogin;
	NPLInterface::NPLObjectProxy msg;
	string sessionid;
	int result = 0;

	//string _s = MD5::ToHexFormat(pkgbuf,iInLength);
	//fprintf(fp,"DecodeLogin input:%s\n",_s.c_str());

	msgHead.Decode(pkgbuf,iInLength);
	
	fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%u,Login Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);

	if(msgHead.Result == 1105)
	{
		result = 419;
	}
	else if(msgHead.Result == 1103)
	{
		result = 407;
	}
	else if(msgHead.Result == 1107)
	{
		result = 413;
	}
	else if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{   		
		if ((msgHead.PkgLen-DBPROXY_HEAD_LENGTH)>0)
		{
			msgLogin.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
		}
		else
		{
			result = 501;
			fprintf(fp,"fatal msgHead.PkgLen-DBPROXY_HEAD_LENGTH");
		}
		
		if (msgLogin.flag==0)
		{
			msgLogin.DecodeSuccess(pkgbuf+DBPROXY_HEAD_LENGTH+4,msgHead.PkgLen-DBPROXY_HEAD_LENGTH-4);
			string sTemp=MD5::ToHexFormat(msgLogin.session,SESSION_LENGTH);
			sessionid = sTemp.c_str();
			fprintf(fp,"%s|return:gameflag=%d,session=%s,result=%d\n",GetLogFormatTime(),msgLogin.gameflag,sTemp.c_str(),result);
		}
		else if (msgLogin.flag==1||msgLogin.flag==4||msgLogin.flag==5)
		{
			if ((msgHead.PkgLen-DBPROXY_HEAD_LENGTH-4)>0)
			{
				msgLogin.DecodeVfyCode(pkgbuf+DBPROXY_HEAD_LENGTH+4,msgHead.PkgLen-DBPROXY_HEAD_LENGTH-4);
			}
			else
			{
				fprintf(fp,"fatal msgHead.PkgLen-DBPROXY_HEAD_LENGTH-4");
			}
			string sTemp=MD5::ToHexFormat(msgLogin.session,SESSION_LENGTH);
			sessionid = sTemp.c_str();
			fprintf(fp,"%s|Verifycode Need, return:flag=%d,session=%s,result=%d\n",GetLogFormatTime(),msgLogin.flag,sTemp.c_str(),result);
			int imgsize = msgLogin.vfyimg_size;
			msgLogin.vfyimg[imgsize]='\0';

			if (msgLogin.flag==1)
			{
				result = 446; // login error too much, need input verify code
			}
			else if (msgLogin.flag==4)
			{
				result = 444; // verify code error
			}
			else if (msgLogin.flag==5)
			{
				result = 447; // same nid or same ip password error too much, need input verify code
			}
			
		}
		else if (msgLogin.flag==2||msgLogin.flag==3)
		{
			msgLogin.DecodeLastLogin(pkgbuf+DBPROXY_HEAD_LENGTH+4,msgHead.PkgLen-DBPROXY_HEAD_LENGTH-4);
			string sTemp=MD5::ToHexFormat(msgLogin.session,SESSION_LENGTH);
			sessionid = sTemp.c_str();
			fprintf(fp,"%s|OtherCity login, return:flag:%d,gameflag=%d,session=%s,result=%d\n",GetLogFormatTime(),msgLogin.flag,msgLogin.gameflag,sTemp.c_str(),result);
		}
	}
	else
	{
		result = 501;
	}

	std::string gameflag;
	if(msgLogin.gameflag & 0x100000)
	{
		gameflag="true";
		//printf("gameflag = true\n");
	}
	else
	{
		gameflag="false";
		//printf("gameflag = false\n");
	}

	msg["forward"] = forward;
	msg["result"] = double(result);
	msg["gameflag"] = gameflag.c_str();

	if (result!=0)
	{
		std::string output;
		if (result==444 || result==445 ||result==447 )
		{
			NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
			tabMsg["result"] = double(result);
			tabMsg["gameflag"] = gameflag.c_str();
			tabMsg["img_size"]=(double)msgLogin.vfyimg_size;
			
			if (msgLogin.vfyimg_size>0 && msgLogin.vfyimg_size<=1024)
			{
				tabMsg["vfyimg"] = MD5::Base64EncodeString(msgLogin.vfyimg,msgLogin.vfyimg_size);
			}			
			tabMsg["sessionid"]=sessionid;		
			NPLInterface::NPLHelper::NPLTableToString("msg", tabMsg, output);
		}
		else
		{
			NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);
		}
		

		if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
		{
			fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
			return;
		}
		fprintf(TMService::Instance().fp,"%s|ouput:%s\n",GetLogFormatTime(),output.c_str());
		fprintf(TMService::Instance().fp,"--------------------\n");
		fflush(TMService::Instance().fp);
	}
	else
	{
		char sendbuf[4096]={0};

		NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
		tabMsg["result"] = double(result);
		tabMsg["params"]["sessionid"]=sessionid;		
		tabMsg["params"]["loginflag"]=(double)msgLogin.flag;
        tabMsg["params"]["gameflag"] = gameflag.c_str();

		if (msgLogin.flag==2||msgLogin.flag==3)
		{
			struct in_addr addr;
			addr.s_addr= msgLogin.lastip;
			string sLastlogin_ip = inet_ntoa(addr);

			char lastlogin_time[10];
			struct tm* stTm;
			time_t t(msgLogin.lasttime);
			stTm = localtime(&t); 
			strftime(lastlogin_time,sizeof(lastlogin_time),"%Y-%m-%d",stTm);
			
			string sLastlogin_time = &(lastlogin_time[0]);
			string sLastlogin_city = &(msgLogin.lastcity[0]);
			string sCurrent_city = &(msgLogin.currcity[0]);

			tabMsg["params"]["lastlogin_ip"]= sLastlogin_ip.c_str();
			tabMsg["params"]["lastlogin_time"]= sLastlogin_time.c_str();
			tabMsg["params"]["lastlogin_city"]= sLastlogin_city.c_str();
			tabMsg["params"]["current_city"]= sCurrent_city.c_str();
		}
		std::string sOutputMsg;
		NPLInterface::NPLHelper::NPLTableToString("msg", tabMsg, sOutputMsg);

		double ifrom = forward["data_table"]["req"]["from"];
		
		int iOutLength = 0;
		fprintf(fp,"%s|ifrom:%d,output:%s\n",GetLogFormatTime(),(int)ifrom,sOutputMsg.c_str());

		TMService::Instance().EncodeGetEmailByID(sendbuf,iOutLength,msgHead.UserID,ifrom);
		queue::Instance().handle_request("tmdbproxy.paraengine.com","11016",sendbuf,iOutLength,PROXY_GETEMAILFROMID,sOutputMsg.c_str(),nMsgLength);
		fprintf(TMService::Instance().fp,"%s|len:%d,gameflg:%s,msg:%s\n",GetLogFormatTime(),nMsgLength,gameflag.c_str(),sOutputMsg.c_str());
		fflush(TMService::Instance().fp);
	}

}

void TMService::DecodeSuperLogin(const  string&  user,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	NPLInterface::NPLObjectProxy msg;
	uint32 nid;
	if(user.find("@") != string::npos )
	{
		return;
	}
	else
	{
		//nid=atol(user.c_str());
		nid = strtoul(user.c_str(),NULL,10);
	}
	msg["nid"] = (double)nid;
	msg["email"] = "";
	msg["forward"] = forward;
	msg["result"] = double(0);
	msg["gameflag"] = "true";

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeLogout(char *pkgbuf,int &iOutLength,double nid,double login_time,double logout_time,const double ifrom,double isteen)
{
	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;
	TMDBProxyLogout msgLogout;	
	//fprintf(fp,"%s|logout begin\n",GetLogFormatTime());
	//fflush(TMService::Instance().fp);
	
	if (isteen)
	{
		msgLogout.gameid = 22;
	}
	else
	{
		msgLogout.gameid = 21;
	}	

	msgLogout.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength,login_time,logout_time);
	int bodylength = iOutLength;

	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|body length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);		
	}

	
	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = USER_LOGIN_ADD;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
	iOutLength = msgHead.PkgLen;

}

void TMService::EncodeSetGameFlag(char *pkgbuf,int &iOutLength,double nid,double isteen)
{
	TMDBProxyHead msgHead;
	TMDBProxySetGameFlag msgSetGameFlag;
	MsgTMChannelVerifyHead msgChannelVerifyHead;

	msgSetGameFlag.idczone = 0;	

	if (isteen)
	{
		msgSetGameFlag.gameid_flag = 22;
	}
	else
	{
		msgSetGameFlag.gameid_flag = 21;
	}	

	msgSetGameFlag.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|body length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());
	int bodylength=iOutLength;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	const char *reg_channelkey= CHANNEL_KEY;

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);
	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);
	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_SET_GAME_FLAG;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

}

void TMService::DecodeSetGameFlag(const char *pkgbuf, const int iInLength)
{
	TMDBProxyHead msgHead;
	TMDBProxySetGameFlag msgSetGameFlag;
	if(iInLength >= DBPROXY_HEAD_LENGTH)
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

// ifrom -- joint-operation id
void TMService::EncodeGetLastLoginTime(char *pkgbuf,int &iOutLength,double nid, const double ifrom)
{
	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;
	TMDBProxyGetLastLoginTime msgLastLoginTime;

	unsigned char md5value[MD5::LENGTH];
	unsigned char md5value_32[32];

	msgLastLoginTime.count = 1;
	msgLastLoginTime.UserID = nid;

	msgLastLoginTime.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	int bodylength=iOutLength;

	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|body length:%d,GetLastLoginTime:%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{
		memcpy(reg_channelkey,CHANNEL_KEY,8);		
	}

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_GET_LASTLOGIN_TIME;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	string sTemp0=reg_channelkey;

	fprintf(fp,"%s|reg_channelkey:%s\n",GetLogFormatTime(),sTemp0.c_str());	
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());	

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeGetLastLoginTime(const char *pkgbuf, const int iInLength,char * sMsg,int nMsgLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetLastLoginTime msgLastLoginTime;
	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	uint32 login_time=0;

	msgHead.Decode(pkgbuf,iInLength);
	fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%u,GetLastLoginTime Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.Result == 1105)
	{
		result = 419;
	}
	else if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{
		msgLastLoginTime.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
		login_time=msgLastLoginTime.login_time;
		uint32 nid = msgLastLoginTime.UserID;
		fprintf(fp,"%s|return:login_time=%u,nid=%u\n",GetLogFormatTime(),login_time,nid);
	}
	else
	{
		result = 501;
	}

	msg["forward"] = forward;
	msg["result"] = double(result);

	if(result !=0)
	{
		fprintf(fp,"%s|GetLastLoginTime Result:%d\n",GetLogFormatTime(),result);
	}

	msg["lastlogintime"] = double(login_time);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodePostMsg(char *pkgbuf,int &iOutLength,const string& title,const string& content,double nid,double type,const string& nick)
{
	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;

	TMDBProxyPostMsg msgPostMsg;

	snprintf(msgPostMsg.title,TITLE_LENGTH,"%s",title.c_str());
	snprintf(msgPostMsg.content,CONTENT_LENGTH,"%s",content.c_str());
	snprintf(msgPostMsg.nick,NICK_LENGTH,"%s",nick.c_str());
	msgPostMsg.type = type;

	msgPostMsg.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	int bodylength = iOutLength;

	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"body length:%d,%s\n",iOutLength,sTemp.c_str());

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	//const char *reg_channelkey= CHANNEL_KEY;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	memcpy(reg_channelkey,CHANNEL_KEY,8);		

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	//sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	
	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = 0xF124;
	msgHead.Result = 0;
	//msgHead.UserID = strtoul(nid.c_str(),NULL,10);
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

}

void TMService::DecodePostMsg(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBProxyPostMsg msgPostMsg;

	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	if(iInLength < DBPROXY_HEAD_LENGTH)
	{
		result = 500;			
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.Result == 0 && msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
		{
			msgPostMsg.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
			msg["id"] = (double)msgPostMsg.ret;
			fprintf(fp,"%s|return:%d\n",GetLogFormatTime(),msgPostMsg.ret);
		}
		else
		{
			result = 501;
		}
	}

	//msg["id"] = (double)0;
	msg["forward"] = forward;
	msg["result"] = double(result);
	//fprintf(fp,"%s|post msg result Immediately:%d\n",GetLogFormatTime(),result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeGetVfyImg(char *pkgbuf,int &iOutLength, double nid, const string& ip, const double ifrom)
{
//	printf("try login:nid=%s\n",nid);
	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;

	TMDBGetVfyImg msgGetVfyImg;
	string sTemp;

	msgGetVfyImg.channelID = CHANNEL_ID;

	if(ip != "")
	{
//		struct hostent *remoteHost = gethostbyname(ip.c_str());
//		msgGetVfyImg.ip = *(u_long *) remoteHost->h_addr_list[0];
		struct in_addr inp;
		int r0 = inet_aton(ip.c_str(),&inp);
		if (r0!=0)
		{
			msgGetVfyImg.ip = inp.s_addr;
		}

	}

	msgGetVfyImg.Encode(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|EncodeGetVfyImg body length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());
	int bodylength=iOutLength;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{	memcpy(reg_channelkey,CHANNEL_KEY,8);}	

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);
	//printf("channel key generated!\n");

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_GET_VERIFY_IMG_ORNOT;
	msgHead.Result = 0;
	msgHead.UserID = nid;		

	msgHead.Encode(pkgbuf,iOutLength);	

	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|verify head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());
	string sTemp0=reg_channelkey;	
	fprintf(fp,"%s|reg_channelkey:%s\n",GetLogFormatTime(),sTemp0.c_str());		

	sTemp=MD5::ToHexFormat(pkgbuf,msgHead.PkgLen);
	fprintf(fp,"%s|total length:%d,%s\n",GetLogFormatTime(),msgHead.PkgLen,sTemp.c_str());
	iOutLength = msgHead.PkgLen;

//	printf("Encoding End...................\n");
}

void TMService::DecodeGetVfyImg(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBGetVfyImg msgGetVfyImg;
	NPLInterface::NPLObjectProxy msg;

	string sessionid;
	int result = 0;

	msgHead.Decode(pkgbuf,iInLength);
	fprintf(fp,"%s|DecodeGetVfyImg len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.Result == 1105)
	{
		result = 419;
	}
	else if(msgHead.Result == 1103)
	{
		result = 407;
	}
	else if(msgHead.Result == 1107)
	{
		result = 413;
	}
	else if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{
		msgGetVfyImg.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
		sessionid=MD5::ToHexFormat(msgGetVfyImg.session,SESSION_LENGTH);
		fprintf(fp,"%s|return:flag=%d,session=%s,result=%d\n",GetLogFormatTime(),msgGetVfyImg.flag,sessionid.c_str(),result);

		int imgsize = msgGetVfyImg.vfyimg_size;
		//printf(" img_size=%d\n",imgsize);
		if(imgsize >=0 && imgsize<VFY_IMGSIZE)
		{
			msgGetVfyImg.vfyimg[imgsize]='\0';
		}

		//ofstream fout("vfy.png", ios::binary);
		//fout.write(msgGetVfyImg.vfyimg, sizeof(char)*imgsize);
		//fout.close();
		msg["vfyimg_size"]=(double)msgGetVfyImg.vfyimg_size;
		msg["sessionid"]=sessionid;		
		if (msgGetVfyImg.vfyimg_size>0 && msgGetVfyImg.vfyimg_size<=VFY_IMGSIZE)
		{
			msg["vfyimg"]= MD5::Base64EncodeString(msgGetVfyImg.vfyimg,msgGetVfyImg.vfyimg_size);
		}
		else
		{
			result = 501;
		}
	}

	msg["forward"] = forward;
	msg["result"] = double(result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeGetVfyImgBySession(char *pkgbuf,int &iOutLength, double nid, const string& ip, const double ifrom,const string& vfysession)
{
	//printf("try login:nid=%d\n",nid);

	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;

	TMDBGetVfyImg msgGetVfyImg;
	string sTemp;

	msgGetVfyImg.channelID = CHANNEL_ID;

	if(ip != "")
	{
//		struct hostent *remoteHost = gethostbyname(ip.c_str());
//		msgGetVfyImg.ip = *(u_long *) remoteHost->h_addr_list[0];
		struct in_addr inp;
		int r0 = inet_aton(ip.c_str(),&inp);
		if (r0!=0)
		{
			msgGetVfyImg.ip = inp.s_addr;
		}
	}

	if (vfysession != "")
	{
		snprintf(msgGetVfyImg.session,SESSION_LENGTH,"%s",vfysession.c_str());
	}

	msgGetVfyImg.EncodeBySession(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|EncodeGetVfyImgBySession body length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	int bodylength=iOutLength;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{	memcpy(reg_channelkey,CHANNEL_KEY,8);}	

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,bodylength,vfy_code);
	//printf("channel key generated!\n");

	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);

	msgHead.PkgLen = bodylength+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_GET_VERIFY_IMG_BYSESSION;
	msgHead.Result = 0;
	msgHead.UserID = nid;		

	msgHead.Encode(pkgbuf,iOutLength);	

	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|verify head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());
	string sTemp0=reg_channelkey;	
	fprintf(fp,"%s|reg_channelkey:%s\n",GetLogFormatTime(),sTemp0.c_str());

	sTemp=MD5::ToHexFormat(pkgbuf,msgHead.PkgLen);
	fprintf(fp,"%s|total length:%d,%s\n",GetLogFormatTime(),msgHead.PkgLen,sTemp.c_str());
	iOutLength = msgHead.PkgLen;

	//printf("Encoding End...................\n");
}

void TMService::EncodeGetUserInfo(char *pkgbuf,int &iOutLength, double nid, const double ifrom)
{
	fprintf(fp,"%s|try get userInfo\n",GetLogFormatTime());

	TMDBProxyHead msgHead;
	MsgTMChannelVerifyHead msgChannelVerifyHead;

	TMDBGetVfyImg msgGetVfyImg;
	string sTemp;

	msgGetVfyImg.channelID = CHANNEL_ID;

	unsigned char vfy_code[CHANNELKEY_LENGTH];
	msgChannelVerifyHead.channelID = CHANNEL_ID;
	char reg_channelkey[9];
	reg_channelkey[8] = '\0';
	if (ifrom ==0)
	{	
		memcpy(reg_channelkey,CHANNEL_KEY,8);
	}	

	MD5::gen_chnlhash32(msgChannelVerifyHead.channelID,reg_channelkey,pkgbuf+DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH,0,vfy_code);
	printf("channel key generated!\n");
	memcpy(msgChannelVerifyHead.channel_key,vfy_code,CHANNELKEY_LENGTH);

	msgChannelVerifyHead.Encode2(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	msgHead.PkgLen = DBPROXY_HEAD_LENGTH+VERIFYCHANNE_HEAD_LENGTH;
	msgHead.Seq = 0;
	msgHead.CommandID = CMD_GET_USERINFO;
	msgHead.Result = 0;
	msgHead.UserID = nid;		

	msgHead.Encode(pkgbuf,iOutLength);	
	sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	//printf("verify head length:%d,%s\n",iOutLength,sTemp.c_str());
	string sTemp0=reg_channelkey;	
	//printf("reg_channelkey:%s\n",sTemp0.c_str());		

	sTemp=MD5::ToHexFormat(pkgbuf,msgHead.PkgLen);
	fprintf(fp,"%s|total length:%d,%s\n",GetLogFormatTime(),msgHead.PkgLen,sTemp.c_str());
	iOutLength = msgHead.PkgLen;
	
//	printf("Encoding End...................\n");
}

void TMService::DecodeGetUserInfo(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMDBProxyHead msgHead;
	TMDBGetUserInfo msgGetUserInfo;
	NPLInterface::NPLObjectProxy msg;

	string sessionid;
	int result = 0;

	msgHead.Decode(pkgbuf,iInLength);
	fprintf(fp,"%s|DecodeGetUserInfo len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.Result == 1105)
	{
		result = 419;
	}
	else if(msgHead.Result == 1103)
	{
		result = 407;
	}
	else if(msgHead.Result == 1107)
	{
		result = 413;
	}
	else if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{
		msgGetUserInfo.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);
		string sTemp=MD5::ToHexFormat(msgGetUserInfo.realname,15);

		//if (sTemp=="000000000000000000000000000000")
		//{ printf("no realname! realname=%s,result=%d\n",sTemp.c_str(),result);}
		//else
		//{ printf("had realname! realname=%s,result=%d\n",sTemp.c_str(),result);	}

		//printf(" sex: %d, birthday: %d\n",msgGetUserInfo.sex, msgGetUserInfo.birthday);

		msg["realname"] = sTemp.c_str();
//		msg["sex"] = msgGetUserInfo.sex;
		msg["sex"] = "0";
		msg["birthday"]=double(msgGetUserInfo.birthday);

		sTemp=MD5::ToHexFormat(msgGetUserInfo.idno,IDNO_LENGTH);
		msg["idno"] = sTemp.c_str();
		//printf(" idno: %s\n",sTemp.c_str());
	}

	msg["forward"] = forward;
	msg["result"] = double(result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"%s|ouput:%s\n",GetLogFormatTime(),output.c_str());	
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeQueryMagicWord(char *pkgbuf,int &iOutLength,double nid,const string& magic_word)
{
	TMMagicWordHead msgHead;
	TMQueryMagicWord msgQueryMagicWord;

	snprintf(msgQueryMagicWord.magic_word,32,"%s",magic_word.c_str());

	msgQueryMagicWord.Encode(pkgbuf+MAGICWORD_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+MAGICWORD_HEAD_LENGTH,iOutLength);
	fprintf(fp,"body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+MAGICWORD_HEAD_LENGTH;
	msgHead.version = 0;
	msgHead.CommandID = 2501;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeQueryMagicWord(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMMagicWordHead msgHead;
	TMQueryMagicWord msgQueryMagicWord;

	string sTemp=MD5::ToHexFormat(pkgbuf,iInLength);
	fprintf(fp,"length:%d,%s\n",iInLength,sTemp.c_str());
	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	if(iInLength < MAGICWORD_HEAD_LENGTH)
	{
		result = 500;			
	}
	else
	{
		char gsids[1024]={0};
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Version:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.version,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.Result == 0 && msgHead.PkgLen > MAGICWORD_HEAD_LENGTH)
		{
			msgQueryMagicWord.Decode(pkgbuf+MAGICWORD_HEAD_LENGTH,msgHead.PkgLen-MAGICWORD_HEAD_LENGTH);
			//msg["id"] = (double)msgPostMsg.ret;
			fprintf(fp,"%s|return:%d,can_choose_count:%d,has_count:%d,use_count:%d,",GetLogFormatTime(),msgHead.Result,msgQueryMagicWord.can_choose_count,msgQueryMagicWord.has_gift_count,msgQueryMagicWord.use_count);
			for(int i=0;i<msgQueryMagicWord.has_gift_count;i++)
			{
				fprintf(fp,"%d|",msgQueryMagicWord.gift_id[i]);
				if(i==0)
				{
					sprintf(gsids,"%d:%d",msgQueryMagicWord.gift_id[i],msgQueryMagicWord.gift_count[i]);
				}
				else
				{
					sprintf(gsids+strlen(gsids),",");
					sprintf(gsids+strlen(gsids),"%d:%d",msgQueryMagicWord.gift_id[i],msgQueryMagicWord.gift_count[i]);
				}
			}
			fprintf(fp,"\n");
			fprintf(fp,"gsids=%s\n",gsids);
		}
		else if(msgHead.Result == 2)
		{
			result = 497;
		}
		else if(msgHead.Result == 6)
		{
			result = 421;
		}
		else
		{
			result = 501;
		}
		msg["nid"] = (double)msgHead.UserID;
		msg["gsids"] = gsids;
	}

	//msg["id"] = (double)0;
	msg["forward"] = forward;
	msg["result"] = double(result);
	//fprintf(fp,"%s|post msg result Immediately:%d\n",GetLogFormatTime(),result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

void TMService::EncodeConsumeMagicWord(char *pkgbuf,int &iOutLength,double nid,const string& magic_word,const string& ip)
{
	TMMagicWordHead msgHead;
	TMConsumeMagicWord msgConsumeMagicWord;

	snprintf(msgConsumeMagicWord.magic_word,32,"%s",magic_word.c_str());
	msgConsumeMagicWord.nid = nid;
	msgConsumeMagicWord.gift_count = 0;


	if(ip != "")
	{
//		struct hostent *remoteHost = gethostbyname(ip.c_str());
//		msgConsumeMagicWord.ip = *(u_long *) remoteHost->h_addr_list[0];
		struct in_addr inp;
		int r0 = inet_aton(ip.c_str(),&inp);
		if (r0!=0)
		{
			msgConsumeMagicWord.ip = inp.s_addr;
		}
	}

	msgConsumeMagicWord.Encode(pkgbuf+MAGICWORD_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+MAGICWORD_HEAD_LENGTH,iOutLength);
	fprintf(fp,"body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+MAGICWORD_HEAD_LENGTH;
	msgHead.version = 0;
	msgHead.CommandID = 2502;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeConsumeMagicWord(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	TMMagicWordHead msgHead;
	TMConsumeMagicWord msgConsumeMagicWord;

	string sTemp=MD5::ToHexFormat(pkgbuf,iInLength);
	fprintf(fp,"length:%d,%s\n",iInLength,sTemp.c_str());
	fflush(TMService::Instance().fp);

	NPLInterface::NPLObjectProxy msg;
	int result = 0;
	if(iInLength < MAGICWORD_HEAD_LENGTH)
	{
		result = 500;			
	}
	else
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Version:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.version,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		fflush(TMService::Instance().fp);
		if(msgHead.Result == 0 && msgHead.PkgLen > MAGICWORD_HEAD_LENGTH)
		{
			msgConsumeMagicWord.Decode(pkgbuf+MAGICWORD_HEAD_LENGTH,msgHead.PkgLen-MAGICWORD_HEAD_LENGTH);
			//msg["id"] = (double)msgPostMsg.ret;
			fprintf(fp,"%s|return:%d,gift_count:%d,gift1:%d\n",GetLogFormatTime(),msgHead.Result,msgConsumeMagicWord.gift_count,msgConsumeMagicWord.gift_id[0]);
			fflush(TMService::Instance().fp);
		}
		else if(msgHead.Result == 2)
		{
			result = 497;
		}
		else if(msgHead.Result == 6)
		{
			result = 421;
		}
		else
		{
			result = 501;
		}
		msg["nid"] = (double)msgHead.UserID;
	}

	//msg["id"] = (double)0;
	msg["forward"] = forward;
	msg["result"] = double(result);
	//fprintf(fp,"%s|post msg result Immediately:%d\n",GetLogFormatTime(),result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

int TMService::SendAndRecvRegist(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength)
{
	try
	{
		iterator_regist = resolver_regist.resolve(query_regist);
		//while (error_regist && iterator_regist != end)
		if (error_regist)
		{
			socket_regist.close();
			fprintf(fp,"%s|try to connect regist server\n",GetLogFormatTime());
			socket_regist.connect(*iterator_regist++, error_regist);
		}
		if (error_regist)
			throw boost::system::system_error(error_regist);

		boost::asio::write(socket_regist, boost::asio::buffer(sendbuf, iOutLength));

		size_t ret = boost::asio::read(socket_regist,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), error_regist);
		iInLength = ret;
		if (error_regist)
			throw boost::system::system_error(error_regist);
		fprintf(fp,"%s|regist recv:%d\n",GetLogFormatTime(),iInLength);
		error_regist = boost::asio::error::host_not_found;

	}
	catch (std::exception& e)
	{
		fprintf(fp,"%s|SendAndRecvRegist exception %s\n",GetLogFormatTime(),e.what());
		error_regist = boost::asio::error::host_not_found;
		return 1;
	}
	return 0;
}

void TMService::EncodeBuyProduct(char *pkgbuf,int &iOutLength,double product_id,double nid,double dest_user_id,double product_count,const string& pay_passwd)
{
	TMPayHead msgHead;
	TMPayBuyProduct msgPayBuyProduct;

	unsigned char md5value[MD5::LENGTH];
	MD5::GenerateDigest((const unsigned char*)(pay_passwd.c_str()), pay_passwd.length(), md5value);
	string sDlMD5String = MD5::ToHexString(md5value);

	msgPayBuyProduct.dest_user_id = dest_user_id;
	msgPayBuyProduct.product_id = product_id;
	msgPayBuyProduct.product_count = product_count;
	msgPayBuyProduct.is_vip = 1;
	msgPayBuyProduct.buy_place = 0;
	//memcpy(msgPayBuyProduct.pay_passwd,pay_passwd.c_str(),16);

	msgPayBuyProduct.SetPassword(sDlMD5String.c_str(),32);
	msgPayBuyProduct.Encode(pkgbuf+PAY_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+PAY_HEAD_LENGTH,iOutLength);
	fprintf(fp,"%s|body length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+PAY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_PAY_BUY_PRODUCT;
	msgHead.Result = 0;
	msgHead.UserID = nid;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	fprintf(fp,"%s|head length:%d,%s\n",GetLogFormatTime(),iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;
}

void TMService::DecodeBuyProduct(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward)
{
	int result=0;
	TMPayHead msgHead;
	TMPayBuyProduct msgPayBuyProduct;
	NPLInterface::NPLObjectProxy msg;

	if(iInLength >= PAY_HEAD_LENGTH)
	{
		msgHead.Decode(pkgbuf,iInLength);
		fprintf(fp,"%s|return:len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",GetLogFormatTime(),msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
		if(msgHead.Result == 0 && msgHead.PkgLen > PAY_HEAD_LENGTH)
		{
			msgPayBuyProduct.Decode(pkgbuf+PAY_HEAD_LENGTH,msgHead.PkgLen-PAY_HEAD_LENGTH);
			msg["op_id"] = double(msgPayBuyProduct.op_id);
			msg["mb_number"] = double(msgPayBuyProduct.mb_number);
			msg["mb_balance"] = double(msgPayBuyProduct.mb_balance);
			fprintf(fp,"%s|return:op_id=%d,mb_number=%d,mb_balance=%d\n",GetLogFormatTime(),msgPayBuyProduct.op_id,msgPayBuyProduct.mb_number,msgPayBuyProduct.mb_balance);
		}
		else
		{
			switch(msgHead.Result)
			{
			case 1:
			case 2:
			case 3:	result = 493;break;
			case 4: result = 500;break;
			case 10:result = 497;break;
			case 11:result = 419;break;
			case 12:result = 420;break;
			case 15:result = 424;break;
			case 102:	result = 439;break;
			case 104:	result = 440;break;
			case 105:	result = 411;break;
			case 106:	result = 499;break;
			case 107:	result = 441;break;
			case 108:	result = 442;break;
			default:	result = 500;
			}
			fprintf(fp,"%s|tm result: %d,haqi result:%d\n",GetLogFormatTime(),msgHead.Result,result);
		}
	}

	msg["forward"] = forward;
	msg["result"] = double(result);

	std::string output;
	NPLInterface::NPLHelper::NPLTableToString("msg", msg, output);

	if(_pState->activate(callback.c_str(),output.c_str(),output.size()) != 0)
	//if(ParaScripting::CNPL::activate2_(callback.c_str(),output.c_str()) != 0)
	{
		fprintf(fp,"%s|active err:%s,%s\n",GetLogFormatTime(),callback.c_str(),output.c_str());
		return;
	}
	fprintf(TMService::Instance().fp,"--------------------\n");
	fflush(TMService::Instance().fp);
}

int TMService::SendAndRecvDBProxy(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength)
{
	try
	{
		fprintf(fp,"%s|try to resolver\n",GetLogFormatTime());
		iterator_dbproxy = resolver_dbproxy.resolve(query_dbproxy);
		if (error_dbproxy)
		{
			fprintf(fp,"%s|try to close last connection\n",GetLogFormatTime());
			socket_dbproxy.close();
			fprintf(fp,"%s|try to connect dbproxy server\n",GetLogFormatTime());
			//fflush(TMService::Instance().fp);
			socket_dbproxy.connect(*iterator_dbproxy++, error_dbproxy);
		}
		if (error_dbproxy)
			throw boost::system::system_error(error_dbproxy);

		boost::asio::write(socket_dbproxy, boost::asio::buffer(sendbuf, iOutLength));

		size_t ret = boost::asio::read(socket_dbproxy,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), error_dbproxy);
		if(ret == 0)
		{
			error_dbproxy = boost::asio::error::host_not_found;
		}
		iInLength = ret;
		if (error_dbproxy)
			throw boost::system::system_error(error_dbproxy);
		fprintf(fp,"%s|dbproxy recv:%d\n",GetLogFormatTime(),iInLength);
		//fflush(TMService::Instance().fp);
	}
	catch (std::exception& e)
	{
		fprintf(fp,"SendAndRecvDBProxy exception %s\n",e.what());
		//fflush(TMService::Instance().fp);
		error_dbproxy = boost::asio::error::host_not_found;
		return 1;
	}
	return 0;
}

int TMService::SendAndRecvPost(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength)
{
	try
	{
		iterator_post = resolver_post.resolve(query_post);
		//while (error_post && iterator_post != end)
		if (error_post)
		{
			socket_post.close();
			fprintf(fp,"%s|try to connect post server\n",GetLogFormatTime());
			//fflush(TMService::Instance().fp);
			socket_post.connect(*iterator_post++, error_post);
		}
		if (error_post)
			throw boost::system::system_error(error_post);

		boost::asio::write(socket_post, boost::asio::buffer(sendbuf, iOutLength));

		size_t ret = boost::asio::read(socket_post,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), error_post);
		if(ret == 0)
		{
			error_post = boost::asio::error::host_not_found;
		}
		iInLength = ret;
		if (error_post)
			throw boost::system::system_error(error_post);
		fprintf(fp,"%s|post recv:%d\n",GetLogFormatTime(),iInLength);
		//fflush(TMService::Instance().fp);
	}
	catch (std::exception& e)
	{
		fprintf(fp,"SendAndRecvPost exception %s\n",e.what());
		//fflush(TMService::Instance().fp);
		error_post = boost::asio::error::host_not_found;
		return 1;
	}
	return 0;
}

int TMService::SendAndRecvData(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength,const char* host,const char* port)
{
	size_t ret;
	boost::asio::io_service io_service;
	boost::system::error_code myerror;

	fprintf(fp,"%s|try to connect remote server\n",GetLogFormatTime());
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), host, port);
	tcp::resolver::iterator iterator = resolver.resolve(query);

	tcp::socket s(io_service);
	s.connect(*iterator, myerror);
	if(myerror)
	{
		fprintf(fp,"%s|connect failed\n",GetLogFormatTime(),iInLength);
		return -2;
	}
	boost::asio::write(s, boost::asio::buffer(sendbuf, iOutLength));
	ret = boost::asio::read(s,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), myerror);
	fprintf(fp,"%s|dbproxy recv:%d\n",GetLogFormatTime(),iInLength);
	if(myerror)
	{
		s.close();
		return -1;
	}
	s.close();

	return ret;
}

int TMService::SendOnly(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength,const char* host,const char* port)
{
	size_t ret;
	boost::asio::io_service io_service;
	boost::system::error_code myerror;

	fprintf(fp,"%s|try to connect remote server\n",GetLogFormatTime());
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), host, port);
	tcp::resolver::iterator iterator = resolver.resolve(query);

	tcp::socket s(io_service);
	s.connect(*iterator, myerror);
	if(myerror)
	{
		fprintf(fp,"%s|connect failed\n",GetLogFormatTime(),iInLength);
		return -2;
	}
	boost::asio::write(s, boost::asio::buffer(sendbuf, iOutLength));
	//ret = boost::asio::read(s,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), myerror);
	//fprintf(fp,"%s|dbproxy recv:%d\n",GetLogFormatTime(),iInLength);
	//if(myerror)
	//{
	//	s.close();
	//	return -1;
	//}
	s.close();

	return 1;
}

//void TMService::AlarmHandler(int signo)
//{
//	TMService::Instance().bTimeOut = true;
//	signal(SIGALRM, TMService::AlarmHandler);
//	siglongjmp(TMService::Instance().env, 1);
//}
//
//void TMService::DeliverWithTimeout(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength, int flag)
//{
//	//boost::boost::mutex::scoped_lock lock(_mutex);
//	boost::mutex::scoped_lock  lock_(m_mutex);
//	signal(SIGALRM, AlarmHandler);
//	alarm(5);
//	TMService::Instance().bTimeOut = false;
//
//	if (!sigsetjmp(TMService::Instance().env, 1))
//	{
//		int ret = 0;
//		if(flag == 1)
//		{
//			ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmregist.paraengine.com", "1863");
//		}
//		else if(flag == 2)
//		{
//			ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "21001");
//		}
//		else
//		{
//			ret = SendOnly(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "21001");
//		}
//		if(ret <= 0)
//		{
//			fprintf(fp,"%s|reget because err received!ret=%d\n",GetLogFormatTime(),ret);
//			if(flag == 1)
//			{
//				ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmregist.paraengine.com", "1863");
//			}
//			else if(flag == 2)
//			{
//				ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "21001");
//			}
//			else
//			{
//				ret = SendOnly(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "21001");
//			}
//		}
//	}
//	alarm(0);
//
//	if(TMService::Instance().bTimeOut)
//	{
//		fprintf(fp,"%s|Time out!\n",GetLogFormatTime());
//		iInLength = 0;
//		if(flag == 1)
//		{
//			error_regist = boost::asio::error::host_not_found;
//		}
//		else
//		{
//			error_dbproxy = boost::asio::error::host_not_found;
//		}
//	}
//}

void TMService::DeliverWithTimeout(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength, int flag)
{
	//boost::boost::mutex::scoped_lock lock(_mutex);
	//boost::mutex::scoped_lock  lock_(m_mutex);

	int ret = 0;
	if(flag == 1)
	{
		ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmregist.paraengine.com", "11016");
	}
	else if(flag == 2)
	{
		ret = SendAndRecvData(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "11016");
	}
	else
	{
		ret = SendOnly(sendbuf, iOutLength,recvbuf, iInLength,"tmdbproxy.paraengine.com", "11016");
	}

}
