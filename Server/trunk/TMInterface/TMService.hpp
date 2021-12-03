#ifndef _TMSERVICE_HPP
#define _TMSERVICE_HPP
#include "TMInterface.h"

#include "MsgTM.hpp"
#include <string>
#include <setjmp.h>

#ifndef WIN32
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#endif

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
using boost::asio::ip::tcp;
using namespace std;

#define PROXY_REGISTER 1
#define PROXY_LOGIN 2
#define PROXY_SETGAMEFLAG 3
#define PROXY_LOGOUT 4
#define PROXY_POSTMSG 5
#define PROXY_GETIDFROMEMAIL 6
#define PROXY_PAY 7
#define PROXY_QUERY_MAGICWORD 8
#define PROXY_CONSUME_MAGICWORD 9
#define PROXY_GETEMAILFROMID 10
#define PROXY_GETLASTLOGINTIME 11
#define PROXY_VFYIMG 12
#define PROXY_VFYIMG_SESSION 13
#define PROXY_USERINFO 14

class TMService
{
public:
	TMService()
		:resolver_regist(ios_regist),query_regist(tcp::v4(), "tmregist.paraengine.com", "11016"),socket_regist(ios_regist)
		,resolver_dbproxy(ios_dbproxy),query_dbproxy(tcp::v4(), "tmdbproxy.paraengine.com", "11016"),socket_dbproxy(ios_regist)
		,resolver_post(ios_post),query_post(tcp::v4(), "tmdbpost.paraengine.com", "21001"),socket_post(ios_post)
		//:resolver_regist(ios_regist),query_regist(tcp::v4(), "114.80.98.7", "1863"),socket_regist(ios_regist)
		//,resolver_dbproxy(ios_dbproxy),query_dbproxy(tcp::v4(), "114.80.98.23", "21001"),socket_dbproxy(ios_regist)
		//:resolver_regist(ios_regist),query_regist(tcp::v4(), "114.80.99.91", "3200"),socket_regist(ios_regist)
		//,resolver_dbproxy(ios_dbproxy),query_dbproxy(tcp::v4(), "114.80.99.91", "21001"),socket_dbproxy(ios_regist)
	{
		error_regist = boost::asio::error::host_not_found;
		error_dbproxy = boost::asio::error::host_not_found;
		error_post = boost::asio::error::host_not_found;
		fp = fopen("./tminterface.log","a+");
		ipfp = fopen("./wanip.txt","r");
		if (ipfp != NULL)
		{
			fscanf(ipfp, "%s", pe_wanip_arr);
		}
		else
		{
			strcpy(pe_wanip_arr, "127.0.0.1");
		}
	}
	~TMService()
	{
		fclose(fp);
		fclose(ipfp);
	}

	static TMService& Instance()
	{
		static TMService ts;
		return ts;
	}

	const char * GetLogFormatTime()
	{
#ifndef WIN32
		time_t timer = time(NULL);
		struct tm dt;
		localtime_r(&timer, &dt);
		sprintf(sLogTime, "%04d-%02d-%02d %02d:%02d:%02d", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
		return sLogTime;
#else
		sLogTime[0] = '\0';
		return sLogTime;
#endif
	}

	int GetHeadLength(int iProxyFlag);
	int GetMsgLength(int iProxyFlag,char * recvbuf, int iHeadLength);
	void EncodeGetID(char *pkgbuf,int &iOutLength,const string& email,const double ifrom);
	void DecodeGetID(const char *pkgbuf, const int iInLength, char * sMsg,int nMsgLength,const  string& passwd,const  string& ip, const string& vfysession, const string& login_vfycode,const string& callback, NPLInterface::NPLObjectProxy& forward, double isteen);
	//uint32 GetIDFromEmail(const string email);
	
	//void EncodeGetEmailByID(char *pkgbuf,int &iOutLength,const string& usernid);
	void EncodeGetEmailByID(char *pkgbuf,int &iOutLength,const double usernid,const double ifrom);
	void DecodeGetEmailByID(const char *pkgbuf, const int iInLength, char * sMsg,int nMsgLength, const int gameflag,const string& sessionid,const string& callback, NPLInterface::NPLObjectProxy& forward);
	
	void EncodeRegist(char *pkgbuf,int &iOutLength,const  string& email,const  string& passwd,const string& ip, const char& sex,const unsigned int& birthday,const string& realname, const string& idno, const double ifrom);
	void DecodeRegist(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);

	void EncodeLogin(char *pkgbuf,int &iOutLength,const string& user, const string& passwd, const string& email, const string& ip,const double ifrom, const string& vfysession, const string& login_vfycode, const double isteen);
	void DecodeLogin(const char *pkgbuf, const int iInLength,char * sMsg,int nMsgLength,const string& callback, NPLInterface::NPLObjectProxy& forward);		 
	void DecodeSuperLogin(const  string&  user,const string& callback, NPLInterface::NPLObjectProxy& forward);
	
	void EncodeSetGameFlag(char *pkgbuf,int &iOutLength,double nid,double isteen);
	void DecodeSetGameFlag(const char *pkgbuf, const int iInLength);
	
	void EncodeLogout(char *pkgbuf,int &iOutLength,double nid,double login_time,double logout_time,const double ifrom,double isteen);

	void EncodeGetLastLoginTime(char *pkgbuf,int &iOutLength,double nid, const double ifrom);
	void DecodeGetLastLoginTime(const char *pkgbuf, const int iInLength,char * sMsg,int nMsgLength,const string& callback, NPLInterface::NPLObjectProxy& forward);

	void EncodePostMsg(char *pkgbuf,int &iOutLength,const string& title,const string& content,double nid,double type,const string& nick);
	void DecodePostMsg(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);

	void EncodeGetVfyImg(char *pkgbuf,int &iOutLength, double nid, const string& ip, const double ifrom);
	void DecodeGetVfyImg(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);
	void EncodeGetVfyImgBySession(char *pkgbuf,int &iOutLength, double nid, const string& ip, const double ifrom,const string& vfysession);

	void EncodeGetUserInfo(char *pkgbuf,int &iOutLength, double nid, const double ifrom);
	void DecodeGetUserInfo(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);

	void EncodeBuyProduct(char *pkgbuf,int &iOutLength,double product_id,double nid,double dest_user_id,double product_count,const string& pay_passwd);
	void DecodeBuyProduct(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);
	
	void EncodeQueryMagicWord(char *pkgbuf,int &iOutLength,double nid,const string& magic_word);
	void DecodeQueryMagicWord(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);
	
	void EncodeConsumeMagicWord(char *pkgbuf,int &iOutLength,double nid,const string& magic_word,const string& ip);
	void DecodeConsumeMagicWord(const char *pkgbuf, const int iInLength,const string& callback, NPLInterface::NPLObjectProxy& forward);
	
	int SendAndRecvRegist(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength);
	int SendAndRecvDBProxy(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength);
	int SendAndRecvPost(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength);
	void DeliverWithTimeout(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength, int flag);
	int SendAndRecvData(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength,const char* host,const char* port);
	int SendOnly(const char *sendbuf, const int iOutLength,char *recvbuf, int &iInLength,const char* host,const char* port);
	static void AlarmHandler(int signo);
	bool bTimeOut;
	//sigjmp_buf env;
	FILE *fp,*ipfp;
	NPL::INPLRuntimeState* _pState;
	char pe_wanip_arr[15];
private:
	//boost::mutex _mutex;
	boost::mutex m_mutex;
	boost::asio::io_service ios_regist;
	tcp::resolver resolver_regist;
	tcp::resolver::query query_regist;
	boost::asio::ip::tcp::socket socket_regist;
	tcp::resolver::iterator iterator_regist;
	tcp::resolver::iterator end;
	boost::system::error_code error_regist;

	boost::asio::io_service ios_dbproxy;
	tcp::resolver resolver_dbproxy;
	tcp::resolver::query query_dbproxy;
	boost::asio::ip::tcp::socket socket_dbproxy;
	tcp::resolver::iterator iterator_dbproxy;
	boost::system::error_code error_dbproxy;

	boost::asio::io_service ios_post;
	tcp::resolver resolver_post;
	tcp::resolver::query query_post;
	boost::asio::ip::tcp::socket socket_post;
	tcp::resolver::iterator iterator_post;
	boost::system::error_code error_post;

	char sLogTime[20];
} ;

#endif  //_TMSERVICE_HPP
