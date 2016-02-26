// 
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de> 
// 
// Distributed under the Boost Software License, Version 1.0. (See accompanying 
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 
// 

#ifndef BOOST_ASIO_DIR_MONITOR_IMPL_HPP 
#define BOOST_ASIO_DIR_MONITOR_IMPL_HPP 

#include <boost/enable_shared_from_this.hpp> 
#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <boost/bind.hpp> 
#include <boost/scoped_ptr.hpp> 
#include <boost/array.hpp> 
#include <boost/bimap.hpp> 
#include <boost/system/error_code.hpp> 
#include <boost/system/system_error.hpp> 
#include <string> 
#include <deque> 
#include <sys/inotify.h> 
#include <errno.h> 

namespace boost { 
namespace asio { 

class dir_monitor_impl : 
    public boost::enable_shared_from_this<dir_monitor_impl> 
{ 
public: 
    dir_monitor_impl() 
        : fd_(init_fd()), 
        stream_descriptor_(inotify_io_service_, fd_), 
        inotify_work_(new boost::asio::io_service::work(inotify_io_service_)), 
        inotify_work_thread_(boost::bind(&boost::asio::io_service::run, &inotify_io_service_)), 
        run_(true) 
    { 
    } 

    void add_directory(const std::string &dirname) 
    { 
        int wd = inotify_add_watch(fd_, dirname.c_str(), IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO); 
        if (wd == -1) 
        { 
            boost::system::system_error e(boost::system::error_code(errno, boost::system::get_system_category()), "boost::asio::dir_monitor_impl::add_directory: inotify_add_watch failed"); 
            boost::throw_exception(e); 
        } 

        boost::unique_lock<boost::mutex> lock(watch_descriptors_mutex_); 
        watch_descriptors_.insert(watch_descriptors_t::value_type(wd, dirname)); 
    } 

    void remove_directory(const std::string &dirname) 
    { 
        boost::unique_lock<boost::mutex> lock(watch_descriptors_mutex_); 
        watch_descriptors_t::right_map::iterator it = watch_descriptors_.right.find(dirname); 
        if (it != watch_descriptors_.right.end()) 
        { 
            inotify_rm_watch(fd_, it->second); 
            watch_descriptors_.right.erase(it); 
        } 
    } 

    void destroy() 
    { 
        inotify_work_.reset(); 
        inotify_io_service_.stop(); 
        inotify_work_thread_.join(); 

        boost::unique_lock<boost::mutex> lock(events_mutex_); 
        run_ = false; 
        events_cond_.notify_all(); 
    } 

    dir_monitor_event popfront_event(boost::system::error_code &ec) 
    { 
        boost::unique_lock<boost::mutex> lock(events_mutex_); 
        while (run_ && events_.empty()) 
            events_cond_.wait(lock); 
        dir_monitor_event ev; 
        if (!events_.empty()) 
        { 
            ec = boost::system::error_code(); 
            ev = events_.front(); 
            events_.pop_front(); 
        } 
        else 
            ec = boost::asio::error::operation_aborted; 
        return ev; 
    } 

    void pushback_event(dir_monitor_event ev) 
    { 
        boost::unique_lock<boost::mutex> lock(events_mutex_); 
        if (run_) 
        { 
            events_.push_back(ev); 
            events_cond_.notify_all(); 
        } 
    } 

private: 
    int init_fd() 
    { 
        int fd = inotify_init(); 
        if (fd == -1) 
        { 
            boost::system::system_error e(boost::system::error_code(errno, boost::system::get_system_category()), "boost::asio::dir_monitor_impl::init_fd: init_inotify failed"); 
            boost::throw_exception(e); 
        } 
        return fd; 
    } 

public: 
    void begin_read() 
    { 
        stream_descriptor_.async_read_some(boost::asio::buffer(read_buffer_), 
            boost::bind(&dir_monitor_impl::end_read, shared_from_this(), 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)); 
    } 

private: 
    void end_read(const boost::system::error_code &ec, std::size_t bytes_transferred) 
    { 
        if (!ec) 
        { 
            pending_read_buffer_ += std::string(read_buffer_.data(), bytes_transferred); 
            while (pending_read_buffer_.size() > sizeof(inotify_event)) 
            { 
                const inotify_event *iev = reinterpret_cast<const inotify_event*>(pending_read_buffer_.data()); 
                dir_monitor_event::event_type type = dir_monitor_event::null; 
                switch (iev->mask) 
                { 
                case IN_CREATE: type = dir_monitor_event::added; break; 
                case IN_DELETE: type = dir_monitor_event::removed; break; 
                case IN_MOVED_FROM: type = dir_monitor_event::renamed_old_name; break; 
                case IN_MOVED_TO: type = dir_monitor_event::renamed_new_name; break; 
                } 
                pushback_event(dir_monitor_event(get_dirname(iev->wd), iev->name, type)); 
                pending_read_buffer_.erase(0, sizeof(inotify_event) + iev->len); 
            } 

            begin_read(); 
        } 
        else if (ec != boost::asio::error::operation_aborted) 
        { 
            boost::system::system_error e(ec); 
            boost::throw_exception(e); 
        } 
    } 

    std::string get_dirname(int wd) 
    { 
        boost::unique_lock<boost::mutex> lock(watch_descriptors_mutex_); 
        watch_descriptors_t::left_map::iterator it = watch_descriptors_.left.find(wd); 
        return it != watch_descriptors_.left.end() ? it->second : ""; 
    } 

    int fd_; 
    boost::asio::io_service inotify_io_service_; 
    boost::asio::posix::stream_descriptor stream_descriptor_; 
    boost::scoped_ptr<boost::asio::io_service::work> inotify_work_; 
    boost::thread inotify_work_thread_; 
    boost::array<char, 4096> read_buffer_; 
    std::string pending_read_buffer_; 
    boost::mutex watch_descriptors_mutex_; 
    typedef boost::bimap<int, std::string> watch_descriptors_t; 
    watch_descriptors_t watch_descriptors_; 
    boost::mutex events_mutex_; 
    boost::condition_variable events_cond_; 
    bool run_; 
    std::deque<dir_monitor_event> events_; 
}; 

} 
} 

#endif 
