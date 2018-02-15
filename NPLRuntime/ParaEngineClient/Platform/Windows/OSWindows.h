#pragma once
#include "NPLMessageQueue.h"
#include <string>

namespace ParaEngine
{
	/** operating system info */
	class COSInfo
	{
	public:
		static BOOL GetOSDisplayString( LPTSTR pszOS);

		/** dump os info to log*/
		static void DumpSystemInfo();

		/** @return value > 5 if it is win vista or 7*/
		static DWORD GetOSMajorVersion();

		/** fetch OS information string. 
		* @param bRefresh: true to force all parameters to be refreshed. 
		* @return: the output string. each attribute is printed on a single line, such as "Processor type: 586"
		*/
		static const std::string& GetSystemInfoString(bool bRefresh = false);
	};

	/** raw win32 message */
	struct CWinRawMsg : 
		public ParaEngine::PoolBase<CWinRawMsg>,
		public ParaEngine::intrusive_ptr_thread_safe_base
	{
		CWinRawMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			: m_hWnd(hWnd), m_uMsg(uMsg), m_wParam(wParam), m_lParam(lParam), m_nID(0) {}
		CWinRawMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int nID)
			: m_hWnd(hWnd), m_uMsg(uMsg), m_wParam(wParam), m_lParam(lParam), m_nID(nID) {}
		CWinRawMsg(){};

		virtual ~CWinRawMsg(){};
	
		// optional ID to identify the message for its sender, etc. 
		int m_nID;

		HWND m_hWnd;
		UINT m_uMsg;
		WPARAM m_wParam;
		LPARAM m_lParam;
	};
	typedef ParaIntrusivePtr<CWinRawMsg> CWinRawMsg_ptr;

	/** the message queue. e.g. 
	CWinRawMsg_ptr msg(new CWinRawMsg(0,0,0,0));
	pWinRawMsgQueue->push(msg);
	*/
	class CWinRawMsgQueue : public NPL::concurrent_ptr_queue<CWinRawMsg_ptr>
	{
	public:
		CWinRawMsgQueue();
		CWinRawMsgQueue(int capacity);
		~CWinRawMsgQueue();
	};
	
}
