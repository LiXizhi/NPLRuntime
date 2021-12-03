#include <stdio.h>
#include "pdumanip.hpp"
#include "byteswap.hpp"
#include "MsgTM.hpp"
#include <string>
#include <boost/asio.hpp>
#include "MD5.h"
#include "TMService.hpp"

using boost::asio::ip::tcp;
using namespace std;
using namespace taomee;

void EncodeRegist(char *pkgbuf,int &iOutLength)
{
	MsgTMHead msgHead;
	MsgTMRegister msgRegister;
	char sSrcString[32];
	sprintf(sSrcString,"passwd");
	unsigned char md5value[MD5::LENGTH];
	MD5::GenerateDigest((const unsigned char*)sSrcString, 6, md5value);
	string sDlMD5String = MD5::ToHexString(md5value);
	printf("md5:%s\n",sDlMD5String.c_str());

	sprintf(msgRegister.passwd,"%s",sDlMD5String.c_str());
	sprintf(msgRegister.email,"gosling_test3@paraengine.com");
	msgRegister.sex[0]=1;
	msgRegister.register_type = 111;
	msgRegister.birthday = 1253000093;

	msgRegister.Encode(pkgbuf+HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+HEAD_LENGTH,iOutLength);
	printf("body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+HEAD_LENGTH;
	msgHead.Version = 1;
	msgHead.CommandID = 0x0002;
	msgHead.UserID = 13220;
	msgHead.Result = 0;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	printf("head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

	printf("Encoding End...................\n");
}

void DecodeRegist(const char *pkgbuf, const int iInLength)
{
	MsgTMHead msgHead;
	MsgTMRegister msgRegister;
	msgHead.Decode(pkgbuf,iInLength);
	printf("len:%d,Version:%d,commandId:%d,UserID:%d,Result:%d\n",msgHead.PkgLen,msgHead.Version,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.PkgLen > HEAD_LENGTH)
	{
		msgRegister.Decode(pkgbuf+HEAD_LENGTH,msgHead.PkgLen-HEAD_LENGTH);

		string sTemp=MD5::ToHexFormat(msgRegister.session,SESSION_LENGTH);
		printf("session,%s\n",sTemp.c_str());
		printf("session:%s\n",msgRegister.session);
	}
}


void EncodeGetID(char *pkgbuf,int &iOutLength)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetID msgGetID;

	sprintf(msgGetID.email,"gosling_test1@paraengine.com");

	msgGetID.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	printf("body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_GET_TAOMEE_ID;
	msgHead.Result = 0;
	msgHead.UserID = 0;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	printf("head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

	printf("Encoding End...................\n");
}

void DecodeGetID(const char *pkgbuf, const int iInLength)
{
	TMDBProxyHead msgHead;
	TMDBProxyGetID msgGetID;
	msgHead.Decode(pkgbuf,iInLength);
	printf("len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{
		msgGetID.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);

		printf("return:%d\n",msgGetID.map_userid);
	}
}

void EncodeLogin(char *pkgbuf,int &iOutLength)
{
	TMDBProxyHead msgHead;
	TMDBProxyLogin msgLogin;

	char sSrcString[32];
	sprintf(sSrcString,"passwd");
	unsigned char md5value[MD5::LENGTH];
	MD5::GenerateDigest((const unsigned char*)sSrcString, 6, md5value);
	string sDlMD5String = MD5::ToHexString(md5value);
	printf("md5:%s\n",sDlMD5String.c_str());
	
	msgLogin.SetPassword(sDlMD5String.c_str(),32);
	msgLogin.ip = 0;
	msgLogin.login_channel = 31;

	msgLogin.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	printf("body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_LOGIN;
	msgHead.Result = 0;
	msgHead.UserID = 40612277;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	printf("head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

	printf("Encoding End...................\n");
}

void DecodeLogin(const char *pkgbuf, const int iInLength)
{
	TMDBProxyHead msgHead;
	TMDBProxyLogin msgLogin;
	msgHead.Decode(pkgbuf,iInLength);
	printf("len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
	if(msgHead.PkgLen > DBPROXY_HEAD_LENGTH)
	{
		msgLogin.Decode(pkgbuf+DBPROXY_HEAD_LENGTH,msgHead.PkgLen-DBPROXY_HEAD_LENGTH);

		printf("return:%d\n",msgLogin.game_flag);
	}
}

void EncodeSetGameFlag(char *pkgbuf,int &iOutLength)
{
	TMDBProxyHead msgHead;
	TMDBProxySetGameFlag msgSetGameFlag;

	msgSetGameFlag.gameid_flag = 21;

	msgSetGameFlag.Encode(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	string sTemp=MD5::ToHexFormat(pkgbuf+DBPROXY_HEAD_LENGTH,iOutLength);
	printf("body length:%d,%s\n",iOutLength,sTemp.c_str());

	msgHead.PkgLen = iOutLength+DBPROXY_HEAD_LENGTH;
	msgHead.Seq = 1;
	msgHead.CommandID = CMD_SET_GAME_FLAG;
	msgHead.Result = 0;
	msgHead.UserID = 40612277;

	msgHead.Encode(pkgbuf,iOutLength);

	sTemp=MD5::ToHexFormat(pkgbuf,iOutLength);
	printf("head length:%d,%s\n",iOutLength,sTemp.c_str());

	iOutLength = msgHead.PkgLen;

	printf("Encoding End...................\n");
}

void DecodeSetGameFlag(const char *pkgbuf, const int iInLength)
{
	TMDBProxyHead msgHead;
	TMDBProxySetGameFlag msgSetGameFlag;
	msgHead.Decode(pkgbuf,iInLength);
	printf("len:%d,Seq:%d,commandId:%d,UserID:%d,Result:%d\n",msgHead.PkgLen,msgHead.Seq,msgHead.CommandID,msgHead.UserID,msgHead.Result);
}

void SendAndRecv(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength)
{
	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);

	//register test ip&port
	//tcp::resolver::query query(tcp::v4(), "114.80.99.91", "3200");


	//login&other test ip&port
	tcp::resolver::query query(tcp::v4(), "114.80.99.91", "21001");
	tcp::resolver::iterator iterator = resolver.resolve(query);

	tcp::socket s(io_service);
	s.connect(*iterator);
	boost::system::error_code error;

	boost::asio::write(s, boost::asio::buffer(sendbuf, iOutLength));

	memset(recvbuf,0,1024);
	iInLength=1024;
	size_t ret = boost::asio::read(s,boost::asio::buffer(recvbuf, iInLength),boost::asio::transfer_at_least(1), error);
	iInLength = ret;
	printf("recv:%d\n",iInLength);
	s.close();
}

int main()
{
	char sendbuf[1024]={0};
	char recvbuf[1024]={0};
	int iOutLength = 0,iInLength=1024;

	string email= "gosling_test5@paraengine.com";
	string user="40612277";
	string passwd= "passwd";
	string callback = "callback";

	TMService::Instance().EncodeRegist(sendbuf,iOutLength,email,passwd);
	TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,1);
	TMService::Instance().DecodeRegist(recvbuf,iInLength,callback);

	//TMService::Instance().EncodeLogin(sendbuf,iOutLength,email,passwd);
	//TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,2);
	//TMService::Instance().DecodeLogin(recvbuf,iInLength,callback);
	//

	//TMService::Instance().EncodeSetGameFlag(sendbuf,iOutLength,user);
	//TMService::Instance().DeliverWithTimeout(sendbuf,iOutLength,recvbuf, iInLength,2);
	//TMService::Instance().DecodeSetGameFlag(recvbuf,iInLength);
	return 0;
}

