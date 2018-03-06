#pragma once

// Use Window or Posix
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace ParaEngine
{
	/** simple scoped lock function */
	template <typename Mutex>
	class Scoped_Lock
	{
	public:
		// Constructor acquires the lock.
		Scoped_Lock(Mutex& m)
			: mutex_(m)
		{
			mutex_.lock();
			locked_ = true;
		}

		// Destructor releases the lock.
		~Scoped_Lock()
		{
			if (locked_)
				mutex_.unlock();
		}

		// Explicitly acquire the lock.
		void lock()
		{
			if (!locked_)
			{
				mutex_.lock();
				locked_ = true;
			}
		}

		// Explicitly release the lock.
		void unlock()
		{
			if (locked_)
			{
				mutex_.unlock();
				locked_ = false;
			}
		}

		// Test whether the lock is held.
		bool locked() const
		{
			return locked_;
		}

		// Get the underlying mutex.
		Mutex& mutex()
		{
			return mutex_;
		}

	private:
		// The underlying mutex.
		Mutex& mutex_;

		// Whether the mutex is currently locked or unlocked.
		bool locked_;
	};

	/**
	* cross platform mutex
	*
	* This class represent a simple way to use mutex
	*
	* Mutex mut;
	*
	* Mutex::ScopedLock lock_(mut); // scoped lock. 
	*
	* mut.lock();      // lock
	* ...
	* mut.islocked();  // fast look up
	* ...
	* mut.unlock();    // unlock
	* mut.tryLock();   // try lock
	*/
	class Mutex
	{
	public:
		typedef Scoped_Lock<Mutex>  ScopedLock;
	private:

#ifdef WIN32
		CRITICAL_SECTION _mutex; /**< Window mutex */
#else
		pthread_mutex_t _mutex; /**< posix mutex */
#endif

		bool _locked;           /**< Fast locked look up used for copying */

		void init(){
#ifdef WIN32
			InitializeCriticalSection(&_mutex);
#else
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&_mutex,&attr);
			pthread_mutexattr_destroy(&attr);
#endif
			_locked = false;
		}

	public:

		/**
		* @brief Construct a Mutex
		* @brief Posix and Win mutex
		*/
		Mutex(){
			init();
		}
		/**
		* @brief Copy Constructor a mutex (copy the locked state only)
		* @param Based mutex
		*
		*/
		Mutex( const Mutex &in_mutex ) {
			init();

			if(in_mutex._locked && !_locked) lock();
			else if(!in_mutex._locked && _locked) unlock();
		}

		/**
		* @brief Copy a mutex (copy the locked state only)
		* @param Based mutex
		* @return Current mutex
		*/
		Mutex& operator=(const Mutex &in_mutex) {
			if(in_mutex._locked && !_locked) lock();
			else if(!in_mutex._locked && _locked) unlock();
			return *this;
		}

		/**
		* @brief Destructor
		*/
		virtual ~Mutex(){
#ifdef WIN32
			DeleteCriticalSection(&_mutex);
#else
			pthread_mutex_unlock(&_mutex);
			pthread_mutex_destroy(&_mutex);
#endif
		}

		/**
		* @brief lock a mutex
		* @return WIN true
		* @return POSIX true if success
		*/
		bool lock(){
			_locked = true;
#ifdef WIN32
			EnterCriticalSection(&_mutex);
			return true;
#else
			return pthread_mutex_lock(&_mutex) == 0;
#endif
		}

		/**
		* @brief lock a mutex
		* @return true if success else false (if busy or error)
		*/
		bool tryLock(){
			_locked = true;
#ifdef WIN32
			return !!TryEnterCriticalSection(&_mutex);
#else
			return pthread_mutex_trylock(&_mutex) == 0;
#endif
		}

		/**
		* @brief unlock a mutex
		* @return WIN true in every cases
		* @return POSIX true if success
		*/
		bool unlock(){
			_locked = false;
#ifdef WIN32
			LeaveCriticalSection(&_mutex);
			return true;
#else
			return pthread_mutex_unlock(&_mutex) == 0;
#endif
		}

		/**
		* @brief Fast locked look up
		* @return true if locked else false
		* This methode use the fast look up variable but still CONST
		* if you want to test perfectly do :
		* if(myMutex.tryLock()){
		*      myMutex.unlock();
		*      // I am sure that the mutex is not locked
		* }
		* else{
		*      // The mutex is locked
		* }
		*/
		bool isLocked() const{
			return _locked;
		}

	};
}
