#pragma once
#ifndef EMSCRIPTEN_SINGLE_THREAD

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#endif
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iostream>
#include <boost/scoped_ptr.hpp>

namespace ParaEngine
{
#ifndef EMSCRIPTEN_SINGLE_THREAD
	using namespace boost::asio;
#endif

	/**
	* this allows ParaEngine to operate as a system service without GUI, such as in server mode.  
	* normally, one should use CParaEngineApp, which is the stand ParaEngine Application with 3D graphics. 
	* @note: this class is cross platform. 
	*/
	class CParaEngineService
	{
	public:
		CParaEngineService();
		~CParaEngineService();

		/** this function does not return util service is stopped. 
		* @param pCommandLine: the command line. 
		* @param pApp: if NULL, a new app is created. 
		* @return the exit code. 
		*/
		int Run(const char* pCommandLine = NULL, IParaEngineApp* pApp = NULL);

		/** stop the service */
		void StopService();

		/* signal handler function for linux */
		static void Signal_Handler(int sig);

		/** call this function before calling Run() to run as a linux daemon */
		static void InitDaemon(void);

		/** whether we will accept key stroke. such as the "ENTER" key will terminate the program.*/
		void AcceptKeyStroke(bool bAccept = true);

		/** whether we will accept key stroke */
		inline bool IsAcceptKeyStroke() {return m_bAcceptKeyStroke;};
	protected:
		/** the main timer time out. */
		void handle_timeout(const boost::system::error_code& err);

	protected:
		/** the main loop */
		boost::asio::io_service m_main_io_service;

		/** Work for the private m_io_service_dispatcher to perform. If we do not give the
		io_service some work to do then the io_service::run() function will exit immediately.*/
		boost::scoped_ptr<boost::asio::io_service::work> m_work_lifetime;

		/** the main timer that ticks 30 times a second*/
		typedef basic_waitable_timer<boost::chrono::steady_clock> timer_type;
		timer_type m_main_timer;

	private:
		/** if true timer will quick. */
		bool m_bQuit;

		/** accept key stroke */
		bool m_bAcceptKeyStroke;

		IParaEngineApp* m_pParaEngineApp;
	};

}
#endif