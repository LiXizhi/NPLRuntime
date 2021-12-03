#ifndef WIN32
#include<list>
#include <sys/time.h>
#include <util/mutex.h>
#include "NPLRouter.h"
#include "NPLRouterStrategy.h"

using namespace NPL;

template <class T> class is_zero : public std::unary_function<T, bool> 
{
public:
	bool operator( ) ( T& val ) 
	{
		return ( val.iTries ) <= 0;
	}
};


	struct MsgInfo
	{
		string sActivateFile;
		string output;
		int iTries;
		int iTimeout;
		NPL::INPLRuntimeState* pState;
	};

	void AlarmHandler(int a);

	class RouterMsgSend
	{
	private:
		int iListCount;
		std::list <MsgInfo> MsgList;
		struct itimerval t;

		/** protecting MsgList. */
		ParaEngine::mutex m_mutex;
	public:
		RouterMsgSend()
		{
			iListCount = 0;
			t.it_value.tv_sec = 3;
			t.it_value.tv_usec = 300000;
			t.it_interval.tv_sec = 3;
			t.it_interval.tv_usec = 300000;
		};
		~RouterMsgSend(){};

		/** the singleton instance */
		static RouterMsgSend* GetInstance();
		void AddSendMsg(char* sActivateFile,const char *output,void* pVoid);
		void SendTimeOutMsg();
	};

#endif
