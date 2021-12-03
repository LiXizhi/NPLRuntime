//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "TMService.hpp"

#define TIME_OUT_SECOND 15
#define FORWARD_MSG_LENGTH 40960

using namespace boost::asio;
/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit connection(boost::asio::io_service& io_service);

  /// Get the socket associated with the connection.
  boost::asio::ip::tcp::socket& socket();

  /// Start the first asynchronous operation for the connection.
  void start(const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength);

  void stop();


  void set_callback(const string& callback);
  void set_forword(NPLInterface::NPLObjectProxy& forward);
  ~connection();
  void stop_connection();

private:
	void handle_resolve(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_write_request(const boost::system::error_code& err);

  /// Handle completion of a read operation.
  void handle_read(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_timeout();

    /// Handle completion of a write operation.
  void handle_stop();

  /// Strand to ensure the connection's handlers are not called concurrently.
  boost::asio::io_service::strand strand_;

  /// Socket for the connection.
  boost::asio::ip::tcp::socket socket_;

  boost::asio::ip::tcp::resolver resolver_;

  /// Buffer for incoming data.
  //boost::array<char, 8192> buffer_;

  NPLInterface::NPLObjectProxy tabMsg;
  char _sendbuf[4096];
  char _recvbuf[4096];
  int _iOutLength;
  int _iInLength;
  int _iProxyFlag;
  string _callback;
  NPLInterface::NPLObjectProxy _forward;
  char _sMsg[FORWARD_MSG_LENGTH];
  int _nMsgLength;
  deadline_timer _timer;
  bool _IsSuccess;
  bool _IsStopConnection;
  std::size_t all_transferred_bytes;
};

typedef boost::shared_ptr<connection> connection_ptr;

/** a compare class connection ptr */
struct Connection_PtrOps
{	// functor for operator<
	bool operator()(const connection_ptr& _Left, const connection_ptr& _Right) const
	{	// apply operator< to operands
		return (_Left.get()<_Right.get());
	}
};

#endif // _CONNECTION_HPP
