#pragma once

#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN)

#include "dirmonitor/basic_dir_monitor.hpp"
#include <boost/thread.hpp>
#include <queue>

namespace ParaEngine
{

	/** monitoring file changes.
	under windows: it uses ReadDirectoryChangesW
	under linux: it uses inotify.
	under macOS: kqueue
	*/
	class CFileSystemWatcher
	{
	public:
		typedef boost::asio::dir_monitor_event::event_type FileActionEnum;
		typedef boost::asio::dir_monitor_event DirMonitorEvent;
		typedef boost::signals2::signal<void(const DirMonitorEvent&)>  FileSystemEvent_t;
		typedef boost::signals2::connection FileSystemEvent_Connection_t;
		
		CFileSystemWatcher(const std::string& filename);
		CFileSystemWatcher();
		~CFileSystemWatcher();

	public:
		/** must be called before you wait for worker thread to exit. when worker thread is existed,
		* You can safely delete this class.
		*/
		void Destroy();

		/** add a directory to monitor. */
		bool add_directory(const std::string &dirname);

		/** remove a directory to monitor. */
		bool remove_directory(const std::string &dirname);

		/** this allows us to process queued messages in main thread's frame move.
		* this function only works when message are told to be dispatched from the main thread.
		* @return number of events dispatched.
		*/
		int DispatchEvents();

		/** if true(default), we will dispatch all callback event in the main thread. Otherwise event callback will be invoked from a IO thread created by this file watcher object*/
		void SetDispatchInMainThread(bool bMainThread);

		/** if true(default), we will dispatch all callback event in the main thread. Otherwise event callback will be invoked from a IO thread created by this file watcher object*/
		bool IsDispatchInMainThread();

		/** add an event call back, please note that the event callback may be called from the main thread or the io thread, depending on the IsDispatchInMainThread(). */
		FileSystemEvent_Connection_t AddEventCallback(FileSystemEvent_t::slot_type callback);

		const std::string& GetName() const;
		void SetName(const std::string& val);
	private:
		void FileHandler(const boost::system::error_code &ec, const boost::asio::dir_monitor_event &ev);

		/** get unprocessed event */
		std::queue < DirMonitorEvent > m_msg_queue;

		void* m_monitor_imp;

		/** locking the queue */
		ParaEngine::mutex m_mutex;

		/** the file event callback. */
		FileSystemEvent_t m_file_event;

		/** watcher name. */
		std::string m_name;

		/** if true(default), we will dispatch all callback event in the main thread. Otherwise event callback will be invoked from a IO thread created by this file watcher object*/
		bool m_bDispatchInMainThread;
	};
	typedef boost::shared_ptr<CFileSystemWatcher> CFileSystemWatcherPtr;

	/** file system watcher service. this is a singleton. */
	class CFileSystemWatcherService : public CRefCounted
	{
	public:
		typedef std::map<std::string, CFileSystemWatcherPtr > file_watcher_map_t;

		CFileSystemWatcherService();
		virtual ~CFileSystemWatcherService();

		static CFileSystemWatcherService* GetInstance();

		/** clear all system watcher references that is created by GetDirWatcher() */
		void Clear();

		/** create get a watcher by its name.
		* it is good practice to use the directory name as watcher name, since it will reuse it as much as possible.
		*/
		CFileSystemWatcherPtr GetDirWatcher(const std::string& name);

		/** delete a watcher, it will no longer receive callbacks.
		* @please note that if someone else still keeps a pointer to the directory watcher, it will not be deleted.
		*/
		void DeleteDirWatcher(const std::string& name);

		/** Dispatch event for all watchers.
		* this allows us to process queued messages in main thread's frame move.
		* this function only works when message are told to be dispatched from the main thread.
		* @return number of events dispatched.
		*/
		int DispatchEvents();

		/** get io service object. */
		boost::asio::io_service& GetIOService() {return *(m_io_service.get());}

		/** whether it is started. */
		bool IsStarted() {return m_bIsStarted;}

		/** start the io service in a different thread. this function can be called multiple times, where only the first time takes effect.  */
		bool Start();
	protected:
		int fileWatcherThreadMain();
	private:
		file_watcher_map_t m_file_watchers;

		boost::shared_ptr<boost::asio::io_service> m_io_service;
		boost::scoped_ptr<boost::asio::io_service::work> m_io_service_work;
		boost::shared_ptr< boost::thread > m_work_thread;
		bool m_bIsStarted;
	};

}
#else
namespace ParaEngine
{
	// empty implementation
	class CFileSystemWatcher
	{
	public:
	};
}
#endif
