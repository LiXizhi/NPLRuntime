#pragma once

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#endif
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/bind.hpp>

namespace ParaEngine 
{
	class MainLoopBase
	{
	public:
		MainLoopBase();
		virtual ~MainLoopBase();

#ifndef EMSCRIPTEN_SINGLE_THREAD
		template <class classname>
		void NextLoop(int milliseconds, void (classname::*func)(const boost::system::error_code&), classname* classpoint)
		{
			m_main_timer.expires_from_now(std::chrono::milliseconds(milliseconds));
			m_main_timer.async_wait(boost::bind(func, classpoint, boost::asio::placeholders::error));
		}
#endif

		void MainLoopRun();

	private:
#ifndef EMSCRIPTEN_SINGLE_THREAD
		/** the main game loop */
		boost::asio::io_service m_main_io_service;
		/** the main timer that ticks 30 times a second*/
		boost::asio::steady_timer m_main_timer;
#endif
	};
}