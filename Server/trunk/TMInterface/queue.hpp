//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "connection.hpp"
#include "TMService.hpp"
#define THREADS_POOL_COUNT 10

///queue class.
class queue
  : private boost::noncopyable
{
public:
	static queue& Instance()
	{
		static queue instanceQueue(THREADS_POOL_COUNT);
		if(!instanceQueue.isRunning)
		{
			instanceQueue.isRunning = true;
			instanceQueue.run();
			instanceQueue.iWorkingSocketCount = 0;
		}
		return instanceQueue;
	}

  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit queue(std::size_t thread_pool_size);

  void start();

  /// Run the server's io_service loop.
  void run();

  /// Stop the server.
  void stop();
  void handle_request(const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength);
  bool isRunning;
  int iWorkingSocketCount;
  boost::mutex m_QueueCountMutex;

private:
  /// The number of threads that will call io_service::run().
  std::size_t thread_pool_size_;

  /// The io_service used to perform asynchronous operations.
  boost::asio::io_service io_service_;



  /** Work for the private m_io_service_dispatcher to perform. If we do not give the
  io_service some work to do then the io_service::run() function will exit immediately.*/
  boost::scoped_ptr<boost::asio::io_service::work> m_work_lifetime;

};

#endif // QUEUE_HPP
