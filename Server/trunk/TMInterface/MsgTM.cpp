#include "TMInterface.h"
#include <time.h>
#include "MsgTM.hpp"
#include "pdumanip.hpp"
#include "TMService.hpp"

void MsgTMHead::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, PkgLen, idx);
	pack2(pkgbuf, Version, idx);
	pack2(pkgbuf, CommandID, idx);
	pack2(pkgbuf, Result, idx);
	pack2(pkgbuf, UserID, idx);
	iOutLength = idx;
}

void MsgTMHead::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,PkgLen,idx);
	unpack2(pkgbuf, Version, idx);
	unpack2(pkgbuf, CommandID, idx);
	unpack2(pkgbuf, Result, idx);
	unpack2(pkgbuf, UserID, idx);
}

void MsgTMChannelVerifyHead::EncodeRegist(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack(pkgbuf, channelID, idx);
	pack(pkgbuf, channel_key,CHANNELKEY_LENGTH, idx);
	iOutLength = idx;
}

void MsgTMChannelVerifyHead::Encode2(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, channelID, idx);
	pack2(pkgbuf, channel_key,CHANNELKEY_LENGTH, idx);
	iOutLength = idx;
}

void MsgTMRegister::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, ip, idx);
	pack2(pkgbuf, passwd, REGISTER_PASSWD_LENGTH, idx);
	pack2(pkgbuf, email,EMAIL_LENGTH, idx);
	pack2(pkgbuf, sex,1, idx);
	pack2(pkgbuf, birthday, idx);
	pack2(pkgbuf, realname,REALNAME_LENGTH, idx);
	pack2(pkgbuf, idno,IDNO_LENGTH, idx);
	pack2(pkgbuf, regchannel, idx);
	pack2(pkgbuf, regfrom, idx);
	iOutLength = idx;
}

void MsgTMRegister::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, session, SESSION_LENGTH, idx);
}

void TMDBProxyHead::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, PkgLen, idx);
	pack2(pkgbuf, Seq, idx);
	pack2(pkgbuf, CommandID, idx);
	pack2(pkgbuf, Result, idx);
	pack2(pkgbuf, UserID, idx);
	iOutLength = idx;
}

void TMDBProxyHead::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,PkgLen,idx);
	unpack2(pkgbuf, Seq, idx);
	unpack2(pkgbuf, CommandID, idx);
	unpack2(pkgbuf, Result, idx);
	unpack2(pkgbuf, UserID, idx);
}

void TMDBProxyGetID::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, email,EMAIL_LENGTH, idx);
	iOutLength = idx;
}


void TMDBProxyGetID::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, map_userid, idx);
}

/*
void TMDBProxyLogin::SetPassword(const void* srcstring, size_t srclen)
{
	str2hex(srcstring, srclen, passwd);
}
void TMDBProxyLogin::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, passwd, LOGIN_PASSWD_LENGTH, idx);
	pack2(pkgbuf, ip, idx);
	//pack2(pkgbuf, login_channel, idx);
	iOutLength = idx;
}


void TMDBProxyLogin::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, game_flag, idx);
	unpack2(pkgbuf, email,EMAIL_LENGTH, idx);
}
*/

void TMDBProxyLogin::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, email, EMAIL_LENGTH, idx);
	pack2(pkgbuf, passwd, LOGIN_PASSWD_LENGTH, idx);
	pack2(pkgbuf, channelID, idx);
	pack2(pkgbuf, region, idx);
	pack2(pkgbuf, gameid, idx);
	pack2(pkgbuf, ip, idx);
	pack2(pkgbuf, vfy_session, SESSION_LENGTH, idx);
	pack2(pkgbuf, vfy_code, VFYCODE_LENGTH, idx);
	iOutLength = idx;
}

void TMDBProxyLogin::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, flag, idx);
}

void TMDBProxyLogin::DecodeSuccess( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, session, SESSION_LENGTH, idx);
	unpack2(pkgbuf, gameflag, idx);
}

void TMDBProxyLogin::DecodeVfyCode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, session, SESSION_LENGTH, idx);
	unpack2(pkgbuf, vfyimg_size, idx);
	unpack2(pkgbuf, vfyimg, vfyimg_size ,idx);
}

void TMDBProxyLogin::DecodeLastLogin( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, session, SESSION_LENGTH, idx);
	unpack2(pkgbuf, gameflag, idx);
	unpack2(pkgbuf, lastip, idx);
	unpack2(pkgbuf, lasttime, idx);
	unpack2(pkgbuf, lastcity,CITY_LENGTH,idx);
	unpack2(pkgbuf, currcity,CITY_LENGTH,idx);
}

void TMDBProxyGetEmailByID::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, email,EMAIL_LENGTH, idx);
}

void TMDBProxySetGameFlag::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, idczone, idx);
	pack2(pkgbuf, gameid_flag, idx);
	iOutLength = idx;
}

void TMDBProxyGetLastLoginTime::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, count, idx);
	pack2(pkgbuf, UserID, idx);
	iOutLength = idx;
}

void TMDBProxyGetLastLoginTime::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, count, idx);
	unpack2(pkgbuf, UserID, idx);
	unpack2(pkgbuf, login_time, idx);
}
/////////////

void TMDBProxyPostMsg::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, type, idx);
	pack2(pkgbuf, nick, NICK_LENGTH, idx);
	pack2(pkgbuf, title, TITLE_LENGTH, idx);
	msglen = strlen(content);
	pack2(pkgbuf, msglen, idx);
	pack2(pkgbuf, content, msglen, idx);
	iOutLength = idx;
}


void TMDBProxyPostMsg::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, ret, idx);
}

void TMDBProxyLogout::Encode( char *pkgbuf, int &iOutLength ,double today_login_time,double today_logout_time)
{
	int idx = 0;
	login_time = GetLocalTime(today_login_time);
	logout_time = GetLocalTime(today_logout_time);
	pack2(pkgbuf, gameid, idx);
	pack2(pkgbuf, login_time, idx);
	pack2(pkgbuf, logout_time, idx);
	iOutLength = idx;
}

uint32 TMDBProxyLogout::GetLocalTime(double today_time)
{
#ifndef WIN32
	time_t timer = time(NULL);
	struct tm dt;
	localtime_r(&timer, &dt);
	dt.tm_hour = 0;
	dt.tm_min = 0;
	dt.tm_sec = 0;
	return mktime(&dt)+today_time;
#else
	return 0;
#endif
	//sprintf(sLogTime, "%04d-%02d-%02d %02d:%02d:%02d", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
	//return sLogTime;
}

void TMPayHead::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, PkgLen, idx);
	pack2(pkgbuf, Seq, idx);
	pack2(pkgbuf, CommandID, idx);
	pack2(pkgbuf, Result, idx);
	pack2(pkgbuf, UserID, idx);
	iOutLength = idx;
}

void TMPayHead::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,PkgLen,idx);
	unpack2(pkgbuf, Seq, idx);
	unpack2(pkgbuf, CommandID, idx);
	unpack2(pkgbuf, Result, idx);
	unpack2(pkgbuf, UserID, idx);
}

void TMPayGetProductPrice::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, product_id, idx);
	iOutLength = idx;
}

void TMPayGetProductPrice::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,product_id,idx);
	unpack2(pkgbuf, price, idx);
	unpack2(pkgbuf, vip_price, idx);
	unpack2(pkgbuf, nonvip_price, idx);
}

void TMPayGetProductInfo::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, product_id, idx);
	iOutLength = idx;
}

void TMPayGetProductInfo::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,product_id,idx);
	unpack2(pkgbuf, price, idx);
	unpack2(pkgbuf, vip_price, idx);
	unpack2(pkgbuf, nonvip_price, idx);
	unpack2(pkgbuf, type, idx);
	unpack2(pkgbuf, category, idx);
	unpack2(pkgbuf, must_vip, idx);
	unpack2(pkgbuf, max_limit, idx);
	unpack2(pkgbuf, total_count, idx);
	unpack2(pkgbuf, current_count, idx);
	unpack2(pkgbuf, is_valid, idx);
	unpack2(pkgbuf, flag, idx);
}

void TMPayBuyProduct::SetPassword(const void* srcstring, size_t srclen)
{
	str2hex(srcstring, srclen, pay_passwd);
}

void TMPayBuyProduct::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, dest_user_id, idx);
	pack2(pkgbuf, product_id, idx);
	pack2(pkgbuf, product_count, idx);
	pack2(pkgbuf, is_vip, idx);
	pack2(pkgbuf, buy_place, idx);
	pack2(pkgbuf, pay_passwd, 16, idx);
	iOutLength = idx;
}

void TMPayBuyProduct::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,op_id,idx);
	unpack2(pkgbuf, mb_number, idx);
	unpack2(pkgbuf, mb_balance, idx);
}

void TMMagicWordHead::Encode(char *pkgbuf, int &iOutLength)
{
	int idx = 0;
	pack2(pkgbuf, PkgLen, idx);
	pack2(pkgbuf, version, idx);
	pack2(pkgbuf, CommandID, idx);
	pack2(pkgbuf, Result, idx);
	pack2(pkgbuf, UserID, idx);
	iOutLength = idx;
}

void TMMagicWordHead::Decode(const char *pkgbuf, const int iInLength)
{
	int idx = 0;
	unpack2(pkgbuf,PkgLen,idx);
	unpack2(pkgbuf, version,idx);
	unpack2(pkgbuf, CommandID, idx);
	unpack2(pkgbuf, Result, idx);
	unpack2(pkgbuf, UserID, idx);
}

void TMQueryMagicWord::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, magic_word, 32, idx);
	iOutLength = idx;
}

void TMQueryMagicWord::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, can_choose_count, idx);
	unpack2(pkgbuf, use_count, idx);
	unpack2(pkgbuf, has_gift_count, idx);
	for(int i=0;i<has_gift_count;i++)
	{
		unpack2(pkgbuf, gift_id[i], idx);
		unpack2(pkgbuf, gift_count[i], idx);
	}
}

void TMConsumeMagicWord::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, magic_word, 32, idx);
	pack2(pkgbuf, nid, idx);
	pack2(pkgbuf, ip, idx);
	pack2(pkgbuf, gift_count, idx);
	for(int i=0;i<gift_count;i++)
	{
		pack2(pkgbuf, gift_id[i], idx);
		pack2(pkgbuf, gift_count_array[i], idx);
	}
	iOutLength = idx;
}

void TMConsumeMagicWord::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	fprintf(TMService::Instance().fp,"Decode Begin\n");
	fflush(TMService::Instance().fp);
	unpack2(pkgbuf, nid, idx);
	unpack2(pkgbuf, gift_right, idx);
	if(gift_right == 0)
	{
		unpack2(pkgbuf, gift_count, idx);
		fprintf(TMService::Instance().fp,"nid=%d,gift_right=%d,gift_count=%d\n",nid,gift_right,gift_count);
		fflush(TMService::Instance().fp);
		for(int i=0;i<gift_count;i++)
		{
			unpack2(pkgbuf, gift_id[i], idx);
			unpack2(pkgbuf, gift_count_array[i], idx);
			fprintf(TMService::Instance().fp,"i=%d,gift_id=%d\n",i,gift_id[i]);
			fflush(TMService::Instance().fp);
		}
	}
}


void TMDBGetVfyImg::Encode( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, channelID, idx);
	pack2(pkgbuf, ip, idx);
	iOutLength = idx;
}

void TMDBGetVfyImg::Decode( const char *pkgbuf, const int iInLength )
{
	int idx = 0;
	unpack2(pkgbuf, flag, idx);
	unpack2(pkgbuf, session, SESSION_LENGTH, idx);
	unpack2(pkgbuf, vfyimg_size, idx);
	unpack2(pkgbuf, vfyimg, vfyimg_size, idx);
}

void TMDBGetVfyImg::EncodeBySession( char *pkgbuf, int &iOutLength )
{
	int idx = 0;
	pack2(pkgbuf, channelID, idx);
	pack2(pkgbuf, ip, idx);
	pack2(pkgbuf, session, SESSION_LENGTH, idx);
	iOutLength = idx;
}

void TMDBGetUserInfo::Decode( const char *pkgbuf, const int iInLength )
{	
	int idx = 0;
	unpack2(pkgbuf, sex,1, idx);
	unpack2(pkgbuf, birthday, idx);
	unpack2(pkgbuf, tel, TELEPHONE_LENGTH, idx);
	unpack2(pkgbuf, mobile, MOBILE_LENGTH, idx);
	unpack2(pkgbuf, postcode, POSTCODE_LENGTH, idx);
	unpack2(pkgbuf, addr_province, idx);
	unpack2(pkgbuf, addr_city, idx);
	unpack2(pkgbuf, addr, ADDR_LENGTH, idx);
	unpack2(pkgbuf, interest, ADDR_LENGTH, idx);
	unpack2(pkgbuf, realname, REALNAME_LENGTH, idx);
	unpack2(pkgbuf, idno, IDNO_LENGTH, idx);
	unpack2(pkgbuf, nickname, NICKNAME_LENGTH, idx);
	unpack2(pkgbuf, signature, SIGNATURE_LENGTH, idx);
}