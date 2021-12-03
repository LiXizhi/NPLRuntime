#include "pdumanip.hpp"
#ifndef MSG_TM_HPP_
#define MSG_TM_HPP_

using namespace taomee;
#define  HEAD_LENGTH 18
//#define  HEAD_LENGTH 17
#define  DBPROXY_HEAD_LENGTH 18
#define  PAY_HEAD_LENGTH 18
#define  MAGICWORD_HEAD_LENGTH 18
#define  REGISTER_PASSWD_LENGTH 32
#define  EMAIL_LENGTH 64
#define  SESSION_LENGTH 16
#define  VFYCODE_LENGTH 6

#define	 CITY_LENGTH 64

#define	 REALNAME_LENGTH 15
#define	 IDNO_LENGTH 18

#define	 TELEPHONE_LENGTH 16
#define	 MOBILE_LENGTH 15
#define	 POSTCODE_LENGTH 6
#define	 ADDR_LENGTH 192
#define	 NICKNAME_LENGTH 90
#define	 SIGNATURE_LENGTH 384

#define  LOGIN_PASSWD_LENGTH 16
#define  TITLE_LENGTH 60
#define  NICK_LENGTH 16
#define  CONTENT_LENGTH 2048
#define	 MAX_GIFT_COUNT 256

#define	 CHANNELKEY_LENGTH 32
#define	 VERIFYCHANNE_HEAD_LENGTH 34

#define	 VFY_IMGSIZE 1024

//#define CMD_REGISTER 0x0003
#define CMD_REGISTER 0x6101
#define CMD_GET_TAOMEE_ID 0x4004
#define CMD_GET_EMAIL_BYID 0x0044
//#define CMD_LOGIN 0x000D
#define CMD_LOGIN 0xA026
#define CMD_GET_VERIFY_IMG_ORNOT 0xA027
#define CMD_GET_VERIFY_IMG_BYSESSION 0xA031
//#define CMD_SET_GAME_FLAG 0x0108
#define CMD_SET_GAME_FLAG 0xA123
#define USER_LOGIN_ADD 0xA125
#define CMD_GET_LASTLOGIN_TIME 0xF009
#define CMD_GET_USERINFO 0x0022

#define CMD_PAY_GET_PRODUCT_PRICE 1001
#define CMD_PAY_MUTI_PRODUCT_PRICE 1002
#define USER_PAY_BUY_PRODUCT_INFO 1003
#define CMD_PAY_BUY_PRODUCT 2004

#define CHANNEL_ID 31
#define CHANNEL_KEY "19801322"
//#define CHANNEL_KEY "12345678"
#define GAME_ID 21

class MsgTMHead
{
public:
	MsgTMHead()
	{
	}
	~MsgTMHead()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 PkgLen;
	//uint8  Version;
	uint32  Version;
	//uint32 CommandID;
	uint16 CommandID;
	uint32 Result; // status_code
	uint32 UserID;
} ;

class MsgTMChannelVerifyHead
{
public:
	MsgTMChannelVerifyHead()
	{
		memset(this, 0, sizeof(MsgTMChannelVerifyHead));
	}
	~MsgTMChannelVerifyHead()
	{
	}

	void  EncodeRegist( char *pkgbuf, int &iOutLength );
	void  Encode2( char *pkgbuf, int &iOutLength );

	uint16 channelID;
	char channel_key[CHANNELKEY_LENGTH];
} ;

class MsgTMRegister
{
public:
	MsgTMRegister()
	{
		//memset(passwd,0,REGISTER_PASSWD_LENGTH);
		//memset(email,0,EMAIL_LENGTH);
		//memset(session,0,SESSION_LENGTH);
		memset(this, 0, sizeof(MsgTMRegister));
	}
	~MsgTMRegister()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 ip;
	char   passwd[REGISTER_PASSWD_LENGTH];
	char   email[EMAIL_LENGTH];
	char   sex[1];
	uint32 birthday;
	char   realname[REALNAME_LENGTH];
	char   idno[IDNO_LENGTH];
	uint16 regchannel;
	uint16 regfrom;
	char   session[SESSION_LENGTH];
} ;

class TMDBProxyHead
{
public:
	TMDBProxyHead()
	{
	}
	~TMDBProxyHead()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 PkgLen;
	uint32 Seq;
	uint16 CommandID;
	uint32 Result;
	uint32 UserID;
} ;

class TMDBProxyGetID
{
public:
	TMDBProxyGetID()
	{
		memset(email,0,EMAIL_LENGTH);
	}
	~TMDBProxyGetID()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	char email[EMAIL_LENGTH];
	uint32 map_userid;
} ;
/*
class TMDBProxyLogin
{
public:
	TMDBProxyLogin()
	{
		memset(passwd,0,LOGIN_PASSWD_LENGTH);
		memset(email,0,EMAIL_LENGTH);
		ip = 0;
	}
	~TMDBProxyLogin()
	{
	}

	void SetPassword(const void* srcstring, size_t srclen);
	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	char passwd[LOGIN_PASSWD_LENGTH];
	char email[EMAIL_LENGTH];
	uint32 ip;
	uint32 login_channel;
	uint32 game_flag;
} ;
*/
class TMDBProxyLogin
{
public:
	TMDBProxyLogin()
	{
		/*memset(passwd,0,LOGIN_PASSWD_LENGTH);
		memset(session,0,SESSION_LENGTH);
		memset(vfy_session,0,SESSION_LENGTH);
		memset(vfy_code,0,VFYCODE_LENGTH);
		memset(vfyimg,0,VFY_IMGSIZE);*/
		memset(this, 0, sizeof(TMDBProxyLogin));
	}
	~TMDBProxyLogin()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );
	void  DecodeSuccess( const char *pkgbuf, const int iInLength );
	void  DecodeVfyCode( const char *pkgbuf, const int iInLength );
	void  DecodeLastLogin( const char *pkgbuf, const int iInLength );

	char email[EMAIL_LENGTH];
	char passwd[LOGIN_PASSWD_LENGTH];
	uint16 channelID;
	uint16 region;
	uint16 gameid;
	uint32 ip;
	char   vfy_session[SESSION_LENGTH];
	char   vfy_code[VFYCODE_LENGTH];

	uint32  flag;

	char	session[SESSION_LENGTH];
	uint32  gameflag;
	uint32  vfyimg_size;
	unsigned char    vfyimg[VFY_IMGSIZE];
	uint32  lastip;
	uint32  lasttime;
	char	lastcity[CITY_LENGTH];
	char	currcity[CITY_LENGTH];
} ;


class TMDBProxyGetEmailByID
{
public:
	TMDBProxyGetEmailByID()
	{
		memset(email,0,EMAIL_LENGTH);
	}
	~TMDBProxyGetEmailByID()
	{
	}
	void  Decode( const char *pkgbuf, const int iInLength );

	char email[EMAIL_LENGTH];
} ;

class TMDBProxySetGameFlag
{
public:
	TMDBProxySetGameFlag()
	{
	}
	~TMDBProxySetGameFlag()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );

	uint32 idczone;
	uint32 gameid_flag;
} ;

class TMDBProxyLogout
{
public:
	TMDBProxyLogout()
	{
		login_time = 0;
		logout_time = 0;
		gameid = 21;
	}
	~TMDBProxyLogout()
	{
	}
	uint32 GetLocalTime(double today_time);

	void  Encode( char *pkgbuf, int &iOutLength ,double today_login_time,double today_logout_time);

	uint32 gameid;
	uint32 login_time;
	uint32 logout_time;
} ;


class TMDBProxyGetLastLoginTime
{
public:
	TMDBProxyGetLastLoginTime()
	{
		login_time = 0;
	}
	~TMDBProxyGetLastLoginTime()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 count;
	uint32 UserID;
	uint32 login_time;
} ;

class TMDBProxyPostMsg
{
public:
	TMDBProxyPostMsg()
	{
		memset(title,0,TITLE_LENGTH);
		memset(nick,0,NICK_LENGTH);
		memset(content,0,CONTENT_LENGTH);
		type = 0;
		msglen = 0;
	}
	~TMDBProxyPostMsg()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength);

	char title[TITLE_LENGTH];
	char nick[NICK_LENGTH];
	char content[CONTENT_LENGTH];
	uint32 type;
	uint32 msglen;
	uint32 ret;
} ;

class TMPayHead
{
public:
	TMPayHead()
	{
	}
	~TMPayHead()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 PkgLen;
	uint32 Seq;
	uint16 CommandID;
	uint32 Result;
	uint32 UserID;
} ;

class TMPayGetProductPrice
{
public:
	TMPayGetProductPrice()
	{
	}
	~TMPayGetProductPrice()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 product_id;
	uint32 price;
	uint32 vip_price;
	uint32 nonvip_price;
};

class TMPayGetProductInfo
{
public:
	TMPayGetProductInfo()
	{
	}
	~TMPayGetProductInfo()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 product_id;
	uint32 price;
	uint32 vip_price;
	uint32 nonvip_price;
	uint8 type;
	uint8 category;
	uint8 must_vip;
	uint32 max_limit;
	uint32 total_count;
	uint32 current_count;
	uint8 is_valid;
	uint8 flag;
};

class TMPayBuyProduct
{
public:
	TMPayBuyProduct()
	{
		memset(pay_passwd,0,16);
	}
	~TMPayBuyProduct()
	{
	}

	void SetPassword(const void* srcstring, size_t srclen);
	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 dest_user_id;
	uint32 product_id;
	uint16 product_count;
	uint8 is_vip;
	uint8 buy_place;
	char pay_passwd[16];
	uint32 op_id;
	uint32 mb_number;
	uint32 mb_balance;
};

class TMMagicWordHead
{
public:
	TMMagicWordHead()
	{
	}
	~TMMagicWordHead()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	uint32 PkgLen;
	uint32 version;
	uint16 CommandID;
	uint32 Result;
	uint32 UserID;
} ;

class TMQueryMagicWord
{
public:
	TMQueryMagicWord()
	{
		memset(magic_word,0,32);
	}
	~TMQueryMagicWord()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	char magic_word[32];
	uint16 can_choose_count;
	uint16 use_count;
	uint16 has_gift_count;
	uint32 gift_id[MAX_GIFT_COUNT];
	uint16 gift_count[MAX_GIFT_COUNT];
};

class TMConsumeMagicWord
{
public:
	TMConsumeMagicWord()
	{
		memset(magic_word,0,32);
	}
	~TMConsumeMagicWord()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );

	char magic_word[32];
	uint32 nid;
	uint32 ip;
	uint16 gift_count;
	uint32 gift_id[MAX_GIFT_COUNT];
	uint32 gift_count_array[MAX_GIFT_COUNT];
	uint8 gift_right;
};


class TMDBGetVfyImg
{
public:
	TMDBGetVfyImg()
	{
	/*	memset(session,0,SESSION_LENGTH);*/
		memset(this, 0, sizeof(TMDBGetVfyImg));
	}
	~TMDBGetVfyImg()
	{
	}

	void  Encode( char *pkgbuf, int &iOutLength );
	void  Decode( const char *pkgbuf, const int iInLength );
	void  EncodeBySession( char *pkgbuf, int &iOutLength );

	uint16	channelID;
	uint32	ip;
	uint32	flag;
	char	session[SESSION_LENGTH];
	uint32	vfyimg_size;
	unsigned char	vfyimg[VFY_IMGSIZE];
} ;

class TMDBGetUserInfo
{
public:
	TMDBGetUserInfo()
	{
		memset(this, 0, sizeof(TMDBGetUserInfo));
	}
	~TMDBGetUserInfo()
	{
	}

	void  Decode( const char *pkgbuf, const int iInLength );

	char   sex[1];
	uint32 birthday;
	char   tel[TELEPHONE_LENGTH];
	char   mobile[MOBILE_LENGTH];
	char   postcode[POSTCODE_LENGTH];
	uint16 addr_province;
	uint16 addr_city;
	char	addr[ADDR_LENGTH];
	char	interest[ADDR_LENGTH];
	char	realname[REALNAME_LENGTH];
	char	idno[IDNO_LENGTH];
	char	nickname[NICKNAME_LENGTH];
	char	signature[SIGNATURE_LENGTH];
} ;

#endif // MSG_TM_HPP_
