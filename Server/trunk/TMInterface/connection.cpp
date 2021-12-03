//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "TMInterface.h"
#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "queue.hpp"
#include "ConnectionManager.h"

using namespace boost::asio;

connection::connection(boost::asio::io_service& io_service)
  : strand_(io_service),
    socket_(io_service),
	 resolver_(io_service),
	 _timer(io_service)
{
	_IsSuccess = false;
	_IsStopConnection = false;
}

connection::~connection()
{
	/*boost::mutex::scoped_lock  lock_(queue::Instance().m_QueueCountMutex);
	queue::Instance().iWorkingSocketCount--;*/
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::set_callback(const string& callback)
{
	_callback = callback;
}

void connection::set_forword(NPLInterface::NPLObjectProxy& forward)
{
	_forward = forward;
}

void connection::start(const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength)
{
	if(iProxyFlag == PROXY_GETIDFROMEMAIL||iProxyFlag == PROXY_GETEMAILFROMID || iProxyFlag == PROXY_LOGIN)
	{
		memset(_sMsg,0,FORWARD_MSG_LENGTH);
		_nMsgLength = iMsgLength;
		if(iMsgLength >= (FORWARD_MSG_LENGTH-1))
		{		
			fprintf(TMService::Instance().fp,"%s|msg too long:%s!\n",TMService::Instance().GetLogFormatTime(),sMsg);
			fflush(TMService::Instance().fp);
		}
		memcpy(_sMsg,sMsg,iMsgLength);
		//fprintf(TMService::Instance().fp,"star:len:%d,msg:%s\n",_nMsgLength,_sMsg);
	}
	tabMsg = NPLInterface::NPLHelper::MsgStringToNPLTable(sMsg);
	NPLInterface::NPLObjectProxy forward = tabMsg["forward"];
	_forward = forward;
	_iProxyFlag = iProxyFlag;
	_iOutLength = iOutLength;
	_iInLength = 4096;
	all_transferred_bytes=0;
	memset(_recvbuf,0,4096);
	memset(_sendbuf,0,4096);
	memcpy(_sendbuf,sendbuf,iOutLength);
	boost::asio::ip::tcp::resolver::query query(server, port);
	resolver_.async_resolve(query,
	  boost::bind(&connection::handle_resolve, shared_from_this(),
	  boost::asio::placeholders::error,
	  boost::asio::placeholders::iterator));
	//fprintf(TMService::Instance().fp,"starting!\n");
	_timer.expires_from_now(boost::posix_time::seconds(TIME_OUT_SECOND));
	_timer.async_wait(boost::bind(&connection::handle_timeout, shared_from_this()));
}

void connection::handle_resolve(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to the first endpoint in the list. Each endpoint
		// will be tried until we successfully establish a connection.
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		socket_.async_connect(endpoint,
			boost::bind(&connection::handle_connect, shared_from_this(),
			boost::asio::placeholders::error, ++endpoint_iterator));
		//fprintf(TMService::Instance().fp,"connecting!\n");
	}
	else
	{
		fprintf(TMService::Instance().fp,"%s|query err!%s\n",TMService::Instance().GetLogFormatTime(),err.message().c_str());
		stop_connection();
	}
}

void connection::handle_connect(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// The connection was successful. Send the request.
		boost::asio::async_write(socket_, boost::asio::buffer(_sendbuf, _iOutLength),
			boost::bind(&connection::handle_write_request, shared_from_this(),
			boost::asio::placeholders::error));
		//fprintf(TMService::Instance().fp,"sending datas!\n");
	}
	else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
	{
		// The connection failed. Try the next endpoint in the list.
		socket_.close();
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		socket_.async_connect(endpoint,
			boost::bind(&connection::handle_connect, shared_from_this(),
			boost::asio::placeholders::error, ++endpoint_iterator));
		//fprintf(TMService::Instance().fp,"reconnecting!\n");
	}
	else
	{
		fprintf(TMService::Instance().fp,"%s|connecting err!%s\n",TMService::Instance().GetLogFormatTime(),err.message().c_str());
		stop_connection();
	}
}

void connection::handle_write_request(const boost::system::error_code& err)
{
	if (!err)
	{
		if(_iProxyFlag == PROXY_LOGOUT)
		{
			fprintf(TMService::Instance().fp,"%s|write end!\n",TMService::Instance().GetLogFormatTime());
			fprintf(TMService::Instance().fp,"--------------------\n");
			fflush(TMService::Instance().fp);
			_IsSuccess = true;
			stop_connection();
		}
		else
		{
			socket_.async_read_some(boost::asio::buffer(_recvbuf, _iInLength),
				strand_.wrap(
				boost::bind(&connection::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));
			//fprintf(TMService::Instance().fp,"recving datas!\n");
		}
	}
	else
	{
		fprintf(TMService::Instance().fp,"%s|recving err!%s\n",TMService::Instance().GetLogFormatTime(),err.message().c_str());
		stop_connection();
	}
}

void connection::handle_read(const boost::system::error_code& e,std::size_t bytes_transferred)
{
  if (!e)
  {
    all_transferred_bytes += bytes_transferred;
	int iHeadLength = TMService::Instance().GetHeadLength(_iProxyFlag);
	if(iHeadLength < 0)
	{
		fprintf(TMService::Instance().fp,"%s|Wrong protocal!\n",TMService::Instance().GetLogFormatTime());
	}
	int iPkgLength = 0;
	if(all_transferred_bytes >= iHeadLength)
	{
		iPkgLength = TMService::Instance().GetMsgLength(_iProxyFlag,_recvbuf, iHeadLength);
	}

	fprintf(TMService::Instance().fp,"%s|head length:%d,pkg length:%d,recv length:%d,all_length:%d!\n",TMService::Instance().GetLogFormatTime(),iHeadLength,iPkgLength,bytes_transferred,all_transferred_bytes);

    if (iPkgLength >= all_transferred_bytes)
	{
		//handle response
		//fprintf(TMService::Instance().fp,"%s|recv length:%d\n",TMService::Instance().GetLogFormatTime(),bytes_transferred);
		if(_iProxyFlag == PROXY_LOGIN)
		{
			TMService::Instance().DecodeLogin(_recvbuf,all_transferred_bytes, _sMsg,_nMsgLength,tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_REGISTER)
		{
			TMService::Instance().DecodeRegist(_recvbuf,all_transferred_bytes, tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_SETGAMEFLAG)
		{
			TMService::Instance().DecodeSetGameFlag(_recvbuf,all_transferred_bytes);
		}
		else if(_iProxyFlag == PROXY_POSTMSG)
		{
			TMService::Instance().DecodePostMsg(_recvbuf,all_transferred_bytes, tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_GETIDFROMEMAIL)
		{
			//fprintf(TMService::Instance().fp,"call decode:len:%d,msg:%s\n",_nMsgLength,_sMsg);
			TMService::Instance().DecodeGetID(_recvbuf,all_transferred_bytes,_sMsg,_nMsgLength,tabMsg["params"]["passwd"],tabMsg["params"]["ip"],tabMsg["params"]["vfysession"],tabMsg["params"]["vfycode"],tabMsg["callback"],_forward,tabMsg["params"]["v"]);
		}
		else if(_iProxyFlag == PROXY_GETEMAILFROMID)
		{
			TMService::Instance().DecodeGetEmailByID(_recvbuf,all_transferred_bytes,_sMsg,_nMsgLength,tabMsg["params"]["loginflag"],tabMsg["params"]["sessionid"],tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_PAY)
		{
			TMService::Instance().DecodeBuyProduct(_recvbuf,all_transferred_bytes, tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_QUERY_MAGICWORD)
		{
			TMService::Instance().DecodeQueryMagicWord(_recvbuf,all_transferred_bytes, tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_CONSUME_MAGICWORD)
		{
			TMService::Instance().DecodeConsumeMagicWord(_recvbuf,all_transferred_bytes, tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_VFYIMG)
		{
			TMService::Instance().DecodeGetVfyImg(_recvbuf,all_transferred_bytes,tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_VFYIMG_SESSION)
		{
			TMService::Instance().DecodeGetVfyImg(_recvbuf,all_transferred_bytes,tabMsg["callback"],_forward);
		}
		else if(_iProxyFlag == PROXY_USERINFO)
		{
			TMService::Instance().DecodeGetUserInfo(_recvbuf,all_transferred_bytes,tabMsg["callback"],_forward);
		}
		_IsSuccess = true;
		stop_connection();
    }
    else if (iPkgLength < 0)
    {
		//log err
		stop_connection();
		//boost::system::error_code ignored_ec;
		//socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		//std::cout << "Error: " << "exit!" << "\n";

	}
    else
    {
		fprintf(TMService::Instance().fp,"%s|need read again!\n",TMService::Instance().GetLogFormatTime());
		socket_.async_read_some(boost::asio::buffer(_recvbuf+all_transferred_bytes, _iInLength-all_transferred_bytes),
          strand_.wrap(
            boost::bind(&connection::handle_read, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void connection::stop_connection()
{
	if(!_IsStopConnection)
	{
		_IsStopConnection = true;
		CConnectionManager::GetSingleton().stop(shared_from_this());
	}
}

void connection::stop()
{
	_timer.cancel();
	// Post a call to the stop function so that stop() is safe to call from any thread.
	socket_.get_io_service().post(boost::bind(&connection::handle_stop, shared_from_this()));
}

void connection::handle_stop()
{
	boost::system::error_code ignored_ec;
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void connection::handle_timeout()
{
	if(!_IsSuccess)
	{
		fprintf(TMService::Instance().fp,"%s|time out recv!\n",TMService::Instance().GetLogFormatTime());
		fflush(TMService::Instance().fp);

	}
	stop_connection();
}
