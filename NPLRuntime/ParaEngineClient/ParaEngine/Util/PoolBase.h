#pragma once
#include "util/mutex.h"
#include <boost/pool/object_pool.hpp>

namespace ParaEngine
{
	// forward declare
	template <typename UserAllocator = boost::default_user_allocator_new_delete, typename Mutex=ParaEngine::mutex>
	class PoolThreadSafe;

	/** this is a thread safe version of boost::pool. It derives from boost:pool, but use a lock for its malloc and free function.  
	e.g.
	PoolThreadSafe<> s_memPool(32);
	char* A = s_memPool.malloc();
	s_memPool.free(A);
	*/
	template <typename UserAllocator, typename Mutex>
	class PoolThreadSafe : protected boost::pool<UserAllocator>
	{
	public:
		typedef typename UserAllocator::size_type size_type;
		typedef typename UserAllocator::difference_type difference_type;
		typedef Mutex mutex_type;
		typedef typename boost::pool<UserAllocator> pool_type;

		// The second parameter here is an extension!
		// pre: npartition_size != 0 && nnext_size != 0
		explicit PoolThreadSafe(const size_type nrequested_size,
			const size_type nnext_size = 32)
			:pool_type(nrequested_size, nnext_size) { }

		// Returns 0 if out-of-memory
		void * malloc()
		{
			ParaEngine::Lock lock_(s_mutex);
			return pool_type::malloc();
		}
		void free(void * const chunk)
		{ 
			ParaEngine::Lock lock_(s_mutex);
			return pool_type::free(chunk);
		}
	private:
		static mutex_type s_mutex;
	};
	template <typename UserAllocator, typename Mutex>
	Mutex PoolThreadSafe<UserAllocator, Mutex>::s_mutex;

	/** if one wants to create and delete many objects of the same type per frame, derive your class from PoolBase.
	* [thread safe]
	* Example:
	class Msg : public ParaEngine::PoolBase<Msg> { ... };

	void func() {
	boost::shared_ptr<Msg> m(new Msg(...));
	m->whatever();
	}
	*/
	template <class T>
	class PoolBase {
	public:
		static void* operator new(size_t size) {
			return s_memPool.malloc();
		}

		static void operator delete(void *p) {
			s_memPool.free(p);
		}

	private:
		static PoolThreadSafe<> s_memPool;
	};

	template <class T>
	PoolThreadSafe<> PoolBase<T>::s_memPool(sizeof(T));


	/** if one wants to create and delete many objects of the same type per frame, derive your class from PoolBase.
	* this class is same as PoolBase, except that it avoid using lock and is not thread-safe. 
	* One should always use PoolBase, unless the performance matters a lot in single threaded mode.
	* [not thread safe]: 
	* Example:
	class Msg : public ParaEngine::PoolBase_SingleThreaded<Msg> { ... };

	void func() {
	boost::shared_ptr<Msg> m(new Msg(...));
	m->whatever();
	}
	*/
	template <class T>
	class PoolBase_SingleThreaded {
	public:
		static void* operator new(size_t size) {
			return s_memPool.malloc();
		}

		static void operator delete(void *p) {
			s_memPool.free(p);
		}

	private:
		static boost::pool<> s_memPool;
	};

}