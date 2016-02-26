// 
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de> 
// 
// Distributed under the Boost Software License, Version 1.0. (See accompanying 
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 
// 

#ifndef BOOST_ASIO_BASIC_DIR_MONITOR_HPP 
#define BOOST_ASIO_BASIC_DIR_MONITOR_HPP 

#include <boost/asio.hpp> 
#include <string> 

namespace boost { 
namespace asio { 

struct dir_monitor_event 
{ 
    enum event_type 
    { 
        null = 0, 
        added = 1, 
        removed = 2, 
        modified = 3, 
        renamed_old_name = 4, 
        renamed_new_name = 5 
    }; 

    dir_monitor_event() 
        : type(null) { } 

    dir_monitor_event(const std::string &d, const std::string &f, event_type t) 
        : dirname(d), filename(f), type(t) { } 

    std::string dirname; 
    std::string filename; 
    event_type type; 
}; 

template <typename Service> 
class basic_dir_monitor 
    : public boost::asio::basic_io_object<Service> 
{ 
public: 
    explicit basic_dir_monitor(boost::asio::io_service &io_service) 
        : boost::asio::basic_io_object<Service>(io_service) 
    { 
    } 

    void add_directory(const std::string &dirname) 
    { 
        this->service.add_directory(this->implementation, dirname); 
    } 

    void remove_directory(const std::string &dirname) 
    { 
        this->service.remove_directory(this->implementation, dirname); 
    } 

    dir_monitor_event monitor() 
    { 
        boost::system::error_code ec; 
        dir_monitor_event ev = this->service.monitor(this->implementation, ec); 
        boost::asio::detail::throw_error(ec); 
        return ev; 
    } 

    dir_monitor_event monitor(boost::system::error_code &ec) 
    { 
        return this->service.monitor(this->implementation, ec); 
    } 

    template <typename Handler> 
    void async_monitor(Handler handler) 
    { 
        this->service.async_monitor(this->implementation, handler); 
    } 
}; 

} 
} 

#endif 
