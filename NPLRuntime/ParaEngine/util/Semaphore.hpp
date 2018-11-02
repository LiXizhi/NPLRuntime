#pragma once

// Use Win or Posix
#ifdef WIN32
#include <windows.h>
#define LMAXIMUMCOUNT 99999999 /**< Maximum semaphore value in Windows*/
#else
#ifndef POSIX
#warning POSIX will be used (but you did not define it)
#endif
#include <semaphore.h>
#endif

namespace ParaEngine
{
	/**
	* cross platform condition variable. 
	*
	* This class represent a simple way to use semaphore
	*/
	class Semaphore{
	protected:

#ifdef WIN32
		HANDLE _sem;	/**< Win semaphore*/
#else
		sem_t _sem;		/**< Posix semaphore*/
#endif

	public:
		/**
		* @brief Constructor
		* @param in_init original value
		*/
		Semaphore( int in_init = 0 ){
#ifdef WIN32
			_sem = CreateSemaphore(0,in_init,LMAXIMUMCOUNT,0);
#else
			sem_init(&_sem,0,in_init);
#endif
		}

		/**
		* @brief Copy constructor
		* @param in_sem original semaphore
		*/
		Semaphore(const Semaphore &in_sem){
			int value = in_sem.value();
#ifdef WIN32
			_sem = CreateSemaphore(0,value,LMAXIMUMCOUNT,0);
#else
			sem_init(&_sem,0,value);
#endif
		}

		/**
		* @brief Copy method
		* @param in_sem original semaphore
		*/
		void operator=(const Semaphore &in_sem){
			reset(in_sem.value());
		}

		/**
		* @brief destroy semaphore
		*/
		virtual ~Semaphore(){
#ifdef WIN32
			CloseHandle(_sem);
#else
			sem_destroy(&_sem);
#endif
		}

		/**
		* @brief Wait until the semaphore is called by another thread
		* @return true if success or false if timeout or error
		*/
		bool wait() const {
#ifdef WIN32
			return WaitForSingleObject(static_cast< HANDLE >(_sem),INFINITE) == 0x00000000L;
#else
			return sem_wait(const_cast<sem_t*>(&_sem)) == 0;
#endif
		}

		/**
		* @brief post a token
		* @return true if success or false if error
		*/
		bool post(){
#ifdef WIN32
			return ReleaseSemaphore(static_cast< HANDLE >(_sem),1,0) != 0;
#else
			return sem_post(&_sem) == 0;
#endif
		}

		/**
		* @brief get current value
		* @return value
		*/
		int value() const{
#ifdef WIN32
			long value = -1;
			ReleaseSemaphore(static_cast<const HANDLE>(_sem),0,&value);
			return value;
#else
			int value = -1;
			sem_getvalue(const_cast<sem_t*>(&_sem),&value);
			return value;
#endif
		}

		/**
		* @brief release current semaphore and create a new one
		* @param init the value after reset
		*/
		void reset( int in_init = 0 ){
#ifdef WIN32
			CloseHandle(_sem);
			_sem = CreateSemaphore(0,in_init,LMAXIMUMCOUNT,0);
#else
			sem_destroy(&_sem);
			sem_init(&_sem,0,in_init);
#endif
		}
	};
}
