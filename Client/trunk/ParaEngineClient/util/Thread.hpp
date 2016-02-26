#pragma once

// Use Win or Posix
#ifdef WIN32
#include <windows.h>
#else
#ifndef POSIX
#warning POSIX will be used (but you did not define it)
#endif
#include <pthread.h>
#include <signal.h>
#endif

namespace ParaEngine
{
	/**
	* cross platform thread. 
	* Thread th;
	* th.start();	    // start thread
	* ...
	* th.isRunning();  // fast look up
	* ...
	* th.wait();	    // wait thread end
	*
	* All methods may be inline by the compiler
	*/
	class Thread{
	private:
		/**
		* @brief Denied equality operator
		* @param none
		*/
		void operator=(const Thread &){}
		/**
		* @brief Denied copy constructor
		* @param none
		*/
		Thread(const Thread &){}

#ifdef WIN32
		HANDLE _handle;
#else
		pthread_t _thread; /**< Posix Thread*/
#endif

		bool _isRunning;  /**< Fast bool lookup */

		/**
		* @brief Static starter function to execute posix thread
		* @brief This function set thread->isRunning to false
		*/
#ifdef WIN32
		static DWORD WINAPI Starter(LPVOID in_thread){
#else
		static void* Starter(void* in_thread){
#endif
			Thread * thread = static_cast< Thread * >(in_thread);
			thread->_isRunning = true;
			thread->run();
			thread->_isRunning = false;

			return 0x00;
		}

	public:
		/**
		* @brief Constructor
		*/
		Thread(){
#ifdef WIN32
			_handle = 0x00;
#else
#endif
			_isRunning = false;
		}
		/**
		* @brief Destructor, Warning, it waits the end of the current thread
		*/
		virtual ~Thread(){
			if(_isRunning)
			{
				// if we destroy the thread until it has finished
				// there is a problem in your implementation algorithm
				// So we wait before destroying the thread!
				wait();	
			}
#ifdef WIN32
			if(_handle !=0)
				CloseHandle (_handle);
#else
#endif
		}

		/**
		* @brief start the thread
		* @return true if success else false
		*/
		bool start(){
			if(_isRunning) return false;
#ifdef WIN32
			_handle = CreateThread( 0x00, 0x00,Thread::Starter, static_cast< void* >(this), 0x00, 0x00);
			return _handle != NULL;
#else
			return pthread_create(&_thread, NULL, Thread::Starter, static_cast< void* >(this)) == 0;
#endif
		}

		/**
		* @brief Fast look up to know if a thread is running
		* @return true if running else false
		*/
		bool isRunning() const{
			return _isRunning;
		}

		/**
		* @brief Wait the end of a thread
		* @return false in case of error, true if all right
		*/
		bool wait() const{
			if(!_isRunning) return false;
#ifdef WIN32
			return WaitForSingleObject(_handle,INFINITE) == 0x00000000L;
#else
			return pthread_join(_thread, NULL) == 0;
#endif

		}

		/**
		* @brief the function is called when thread is starting
		* @must You must implement this methode!
		*/
		virtual void run() = 0;

		/**
		*
		*/
		bool kill(){
			if(!_isRunning) return false;

			_isRunning = false;
#ifdef WIN32
			bool success = TerminateThread(_handle,1) && CloseHandle(_handle);
			_handle = 0x00;
			return success;
#else
			return pthread_kill( _thread, SIGKILL) == 0;
#endif
		}

	};
}

