//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "TMInterface.h"
#include "queue.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "ConnectionManager.h"


queue::queue(std::size_t thread_pool_size)
  : thread_pool_size_(thread_pool_size)
{
	//new_connection_(new connection(io_service_));
	//new_connection_.reset(new connection(io_service_));
	isRunning = false;
}
void queue::start()
{
	fprintf(TMService::Instance().fp,"%s|interface stared!\n",TMService::Instance().GetLogFormatTime());
	fprintf(TMService::Instance().fp,"--------------------\n");
}
void queue::run()
{
  m_work_lifetime.reset(new boost::asio::io_service::work(io_service_));

  // Create a pool of threads to run all of the io_services.
  std::vector<boost::shared_ptr<boost::thread> > threads;
  for (std::size_t i = 0; i < thread_pool_size_; ++i)
  {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &io_service_)));
    threads.push_back(thread);
  }

  // Wait for all threads in the pool to exit.
  //for (std::size_t i = 0; i < threads.size(); ++i)
  //  threads[i]->join();
}

void queue::stop()
{
	m_work_lifetime.reset();
	//for (std::size_t i = 0; i < thread_pool_size_; ++i)
	//	 threads[i]->join();
	io_service_.stop();
}

void queue::handle_request(const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength)
{
	/*boost::mutex::scoped_lock  lock_(m_QueueCountMutex);
	if(iWorkingSocketCount > MAX_WORKING_SOCKET_COUNT)
	{
		fprintf(TMService::Instance().fp,"%s|working socket count=%d\n",TMService::Instance().GetLogFormatTime(),iWorkingSocketCount);
		return ;
	}
	if(iWorkingSocketCount > THREADS_POOL_COUNT)
	{
		fprintf(TMService::Instance().fp,"%s|working socket count=%d\n",TMService::Instance().GetLogFormatTime(),iWorkingSocketCount);
	}*/

	/// The next connection to be accepted.
	connection_ptr new_connection_;  

    new_connection_.reset(new connection(io_service_));
	CConnectionManager::GetSingleton().start(new_connection_, server,port,sendbuf,iOutLength,iProxyFlag,sMsg,iMsgLength);
	// new_connection_->start(server,port,sendbuf,iOutLength,iProxyFlag,sMsg,iMsgLength);
	// iWorkingSocketCount++;
}

