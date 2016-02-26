#pragma once

#include "util/mutex.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/asio/steady_timer.hpp>

#include "IParaEngineApp.h"

namespace ParaEngine
{
	/** The default ParaEngine Window. 
	It can used as an example of how to write main game loop using ParaEngine library. 

	e.g. 

	CD3DWindowDefault defaultWin;
	defaultWin.SetAppInterface(CParaEngineApp::GetInstance());

	return defaultWin.Run(hInstance);

	*/
	class CD3DWindowDefault
	{
	public:
		typedef boost::shared_ptr<boost::thread> Boost_Thread_ptr_type;

		CD3DWindowDefault();
		virtual ~CD3DWindowDefault(){}

	public:
		/** create the default win32 windows for rendering. and assigning to m_hWnd. 
		* window is created in another thread. Since windows uses single threaded apartment, the window procedure should also be in that thread. 
		*/
		HRESULT StartWindowThread(HINSTANCE hInstance);
		void DefaultWinThreadProc(HINSTANCE hInstance);
		void handle_mainloop_timer(const boost::system::error_code& err);

		int Run(HINSTANCE hInstance);

		void SetAppInterface(IParaEngineApp* pApp) {m_pApp = pApp;};
		IParaEngineApp* GetAppInterface(){return m_pApp;}

		/** return true if this is a render tick, otherwise false. 
		* @param fIdealInterval: the ideal interval (FPS) when this function will return true at ideal FPS. 
		* @param pNextInterval: main_loop timer interval.
		* @return frameDelta. if this is bigger than 0, we will render a frame. 
		*/
		int CalculateRenderTime(double fIdealInterval, double* pNextInterval);

	private:
		bool m_bQuit;
		IParaEngineApp* m_pApp;

		//  the default window thread ptr. 
		Boost_Thread_ptr_type m_win_thread;
		ParaEngine::mutex	  m_win_thread_mutex;

		bool m_bMainLoopExited;

		/** the main game loop */
		boost::asio::io_service m_main_io_service;
		/** the main timer that ticks 30 times a second*/
		boost::asio::steady_timer m_main_timer;
		// main render window hWnd. 
		HWND m_hWnd;
	};
}