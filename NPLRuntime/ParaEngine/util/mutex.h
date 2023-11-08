#pragma once

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#endif
// Note: if one wants to use nested mutex, use this one. However, it is slightly heavier than traditional mutex. On windows, mutex can be nested by default. 
// #include <boost/thread/recursive_mutex.hpp>

namespace ParaEngine
{
	/** simple scoped lock function */
	template <typename Mutex>
	class scoped_Lock
	{
	public:
		// Constructor acquires the lock.
		scoped_Lock(Mutex& m)
			: mutex_(m)
		{
			mutex_.lock();
			locked_ = true;
		}

		// Destructor releases the lock.
		~scoped_Lock()
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

	/** a dummy lock object with all dummy interface functions*/
	template <typename Mutex>
	class no_lock{
	public:
		no_lock(Mutex& m){}
		bool locked() const { return true; };
		void unlock() {};
	};

	/**
	* cross platform mutex
	*
	* This class represent a simple way to use mutex
	*
	* mutex mut;
	*
	* mutex::ScopedLock lock_(mut); // scoped lock.
	*
	* mut.lock();      // lock
	* ...
	* mut.islocked();  // fast look up
	* ...
	* mut.unlock();    // unlock
	* mut.tryLock();   // try lock
	*/
	class mutex
	{
	public:
		typedef scoped_Lock<mutex>  ScopedLock;
	private:

#ifndef EMSCRIPTEN_SINGLE_THREAD
#ifdef WIN32
		CRITICAL_SECTION _mutex; /**< Window mutex */
#else
		pthread_mutex_t _mutex; /**< posix mutex */
#endif
#endif
		bool _locked;           /**< Fast locked look up used for copying */

		void init(){
#ifndef EMSCRIPTEN_SINGLE_THREAD
#ifdef WIN32
			InitializeCriticalSection(&_mutex);
#else
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&_mutex, &attr);
			pthread_mutexattr_destroy(&attr);
#endif
#endif
			_locked = false;
		}

	public:

		/**
		* @brief Construct a mutex
		* @brief Posix and Win mutex
		*/
		mutex(){
			init();
		}
		/**
		* @brief Copy Constructor a mutex (copy the locked state only)
		* @param Based mutex
		*
		*/
		mutex(const mutex &in_mutex) {
			init();

			if (in_mutex._locked && !_locked) lock();
			else if (!in_mutex._locked && _locked) unlock();
		}

		/**
		* @brief Copy a mutex (copy the locked state only)
		* @param Based mutex
		* @return Current mutex
		*/
		mutex& operator=(const mutex &in_mutex) {
			if (in_mutex._locked && !_locked) lock();
			else if (!in_mutex._locked && _locked) unlock();
			return *this;
		}

		/**
		* @brief Destructor
		*/
		virtual ~mutex(){
#ifndef EMSCRIPTEN_SINGLE_THREAD
#ifdef WIN32
			DeleteCriticalSection(&_mutex);
#else
			pthread_mutex_unlock(&_mutex);
			pthread_mutex_destroy(&_mutex);
#endif
#endif
		}

		/**
		* @brief lock a mutex
		* @return WIN true
		* @return POSIX true if success
		*/
		bool lock(){
#ifdef EMSCRIPTEN_SINGLE_THREAD
			return true;
#else
			_locked = true;
#ifdef WIN32
			EnterCriticalSection(&_mutex);
			return true;
#else
			return pthread_mutex_lock(&_mutex) == 0;
#endif
#endif
		}

		/**
		* @brief lock a mutex
		* @return true if success else false (if busy or error)
		*/
		bool tryLock(){
#ifdef EMSCRIPTEN_SINGLE_THREAD
			return true;
#else
			_locked = true;
#ifdef WIN32
			return !!TryEnterCriticalSection(&_mutex);
#else
			return pthread_mutex_trylock(&_mutex) == 0;
#endif
#endif
		}

		/**
		* @brief unlock a mutex
		* @return WIN true in every cases
		* @return POSIX true if success
		*/
		bool unlock() {
#ifdef EMSCRIPTEN_SINGLE_THREAD
			return true;
#else
			_locked = false;
#ifdef WIN32
			LeaveCriticalSection(&_mutex);
			return true;
#else
			return pthread_mutex_unlock(&_mutex) == 0;
#endif
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
	typedef mutex::ScopedLock Lock;
}

