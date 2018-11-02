#pragma once
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "util/Mutex.hpp"
#include "ipc_message_queue.hpp"
#include "InterprocessQueue.hpp"
#include "IParaEngineApp.h"
#include "InterprocessMsg.h"
#include "AutoUpdaterApp.h"
#include <boost/asio/steady_timer.hpp>

namespace ParaEngine
{
	class CMyProcessAppThread;
	class CCommandLineParser;

	/** This is used to create an application using interprocess communication. */
	class CInterprocessAppClient: public ParaEngine::CAutoUpdaterApp
	{
	public:
		typedef boost::shared_ptr<boost::thread> Boost_Thread_ptr_type;
		
		CInterprocessAppClient(const char* appName = NULL);
		~CInterprocessAppClient();

	public:
		/** set the application name */
		void SetAppName(const char* appName);

		/** init command line so that we can use IPC calls before calling Run() method. */
		virtual int SetCommandLine(const char* sCmdLine);

		/** run the application using a given command line. */
		int Run(HINSTANCE hInst, const char* sCmdLine);
	public:
		/** set the parent window */
		void SetParentWindow(HWND hWndParent);

		int Start(int nMinResolutionWidth = 0, int nMinResolutionHeight = 0);

		int Stop();

		HWND GetMainHWnd(){return m_hWnd;}

		/** sent interprocess message to the host process. */
		virtual bool SendHostMsg(int nMsg, DWORD param1 = 0, DWORD param2 = 0, const char* filename = NULL, const char* sCode = NULL, int nPriority = 0);

	protected:
		static LRESULT CALLBACK DefaultWinThreadWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		/** return 0 if succeed, otherwise we will terminate the application. */
		int HandleAppMsg(InterProcessMessage& msg);

		/** load ParaEngineClient.dll if not loaded before. */
		bool CheckLoad();

		/** the main loop in the main thread.*/
		void handle_mainloop_timer(const boost::system::error_code& err);

		double GetElapsedAppTime();

		/** create the default win32 windows for rendering. and assigning to m_hWnd. 
		* window is created in another thread. Since windows uses single threaded apartment, the window procedure should also be in that thread. 
		*/
		HRESULT StartWindowThread();
		void DefaultWinThreadProc();

		/** print the GetLastError*/
		void PrintGetLastError();

		/** enable auto update when complete. */
		void EnableAutoUpdateWhenComplete(bool bEnable);

	protected:
		/** the application name is also the interprocess message queue name. */
		std::string m_app_name;
		/** the interprocess queue name for the host app.*/
		std::string m_app_host;
		std::string m_sUsage;
		/** whether we will automatically update the client when receiving PEAPP_Start. This is automatically set to true when command line contains m_sUsage="webplayer"*/
		bool m_bAutoUpdateWhenStart;
		int m_nMinWidth;
		int m_nMinHeight;

		HWND m_hWndParent;
		bool m_bStarted;
		ParaEngine::CPluginLoader m_ParaEngine_plugin;
		ParaEngine::IParaEngineCore* m_pParaEngine;
		ParaEngine::IParaEngineApp * m_pParaEngineApp;
		ParaEngine::IRenderWindow * m_pParaEngineRenderWindow;
		

		CInterprocessQueue* m_ipAppQueueIn;
		CInterprocessQueue* m_ipHostQueueOut;

		bool m_bQuit;
		bool m_bMainLoopExited;

		ParaEngine::Mutex	  m_mutex;

		/** the main game loop */
		boost::asio::io_service m_main_io_service;
		/** the main timer that ticks 30 times a second*/
		boost::asio::steady_timer m_main_timer;
		// main render window hWnd. 
		HWND m_hWnd;
		
		std::string m_sRedistDir;

		Domain_Map_t m_trusted_domains;

		//  the default window thread ptr. 
		Boost_Thread_ptr_type m_win_thread;
		ParaEngine::Mutex	  m_win_thread_mutex;
		DWORD m_dwWinThreadID;

		HINSTANCE m_hInst;
		friend class CMyProcessAppThread;
	};

}