#pragma once
#include "util/mutex.h"
#include "math/ParaMath.h"
#include "util/PoolBase.h"
#include <boost/pool/object_pool.hpp>

namespace ParaEngine
{
	// forward declare
	template <typename UserAllocator = boost::default_user_allocator_new_delete, typename Mutex = ParaEngine::mutex>
	class CNPLPool_Char_alloc;

	/** a quick define*/
	typedef CNPLPool_Char_alloc<>  CNPLPool_Char_allocator;

	/** NPL String can be used instead of std::string, for strings which are created and deleted very regularly. 
	* iternally it uses memory pool CNPLPool_Char_alloc for memory allocation. 
	* NPL String uses SSO(small string optimization 16 bytes) as std::string; for string smaller or equal to 2048 Bytes, it uses one of its internal memory pools,
	* for even bigger string, standard malloc is used(default to std::string behavior). 
	*/
	typedef std::basic_string<char, std::char_traits<char>, ParaEngine::CNPLPool_Char_alloc<> > NPLString;	

/** the initial smallest fixed sized chunk that the memory pool will create. default to 32 bytes */
#define NPL_char_pool_init_size_bytes 32
/** this should be log2(NPL_char_pool_init_size_bytes) */
#define NPL_char_pool_init_size 5
/** how many free lists are there in the pool. NPL_char_pool_init_size_bytes^2 is the biggest chunk.  currently it is 2048 bytes */
#define NPL_char_pool_count  7

	/** This class can be used as a custom allocator for stl containers, which constantly create and delete small or medium sized objects.  
	* it is optimized for frequently used temporary NPL objects, such as NPL related message body, file names, message bodies, etc.
	* it is faster than std::allocator and boost::pool_alloc. But it costs more (double at most) memory than boost::pool_alloc, because we may return more memory than requested. 
	*
	* Internally, it has 7 free lists (boost::pool). Each free list only serves one fixed-sized object at a time. 
	* Each pool has their own light weighted locks for thread-safe access. 
	* The free list sizes are 32,64,128,256,512,1024,2048; we will use the default allocator for sizes over 2048. 
	* One can configure the internal freelists by editing the macro NPL_char_pool_init_size_bytes, NPL_char_pool_init_size, NPL_char_pool_count
	*
	* [thread safe]
	* Example:
		typedef std::basic_string<char, std::char_traits<char>, ParaEngine::CNPLPool_Char_alloc<> > NPLString;	

	* This is the standard boost version(DONOT use this)
		#include <boost/pool/pool_alloc.hpp>
		typedef std::basic_string<char, std::char_traits<char>, boost::pool_allocator > String;
	*/
	template <typename UserAllocator, 
		typename Mutex>
	class CNPLPool_Char_alloc
	{
	public:
		typedef char value_type;
		typedef UserAllocator user_allocator;
		typedef Mutex mutex_type;
		
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef typename ParaEngine::PoolThreadSafe<UserAllocator, Mutex> pool_type;
		typedef typename pool_type::size_type size_type;
		typedef typename pool_type::difference_type difference_type;
		
		template <typename U>
		struct rebind
		{
			typedef CNPLPool_Char_alloc<UserAllocator, Mutex> other;
		};
	protected:
		struct SingletonMemPool_Type
		{
			SingletonMemPool_Type()
			{
				int i=0;
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes,1024); // 32
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2,512); // 64
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2*2,256); // 128
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2*2*2,128); // 256
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2*2*2*2,64); // 512
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2*2*2*2*2,32); // 1024
				m_mem_pools[i++] = new pool_type(NPL_char_pool_init_size_bytes*2*2*2*2*2*2,32); // 2048
				PE_ASSERT(i==NPL_char_pool_count);
			}
			~SingletonMemPool_Type()
			{
				for(int i=0;i<NPL_char_pool_count;++i)
				{
					delete m_mem_pools[i];
				}
			}
			pool_type& operator [] (int nIndex)
			{
				return *(m_mem_pools[nIndex]);
			}
		private:			
			pool_type* m_mem_pools[NPL_char_pool_count];
		};
		static SingletonMemPool_Type s_mem_pools;
	public:
		CNPLPool_Char_alloc()
		{
		}

		// default copy constructor

		// default assignment operator

		// not explicit, mimicking std::allocator [20.4.1]
		CNPLPool_Char_alloc(const CNPLPool_Char_alloc<UserAllocator, Mutex> &)
		{
		}
		//// default destructor

		static pointer address(reference r)
		{ return &r; }
		static const_pointer address(const_reference s)
		{ return &s; }
		static size_type max_size()
		{ return (std::numeric_limits<size_type>::max)(); }

		bool operator==(const CNPLPool_Char_alloc &) const
		{ return true; }
		bool operator!=(const CNPLPool_Char_alloc &) const
		{ return false; }

		static pointer allocate(const size_type n)
		{
			int nIndex = Math::log2_ceil(n) - NPL_char_pool_init_size;
			if(nIndex<0)
				nIndex = 0;
			pointer ret = 0;
			if(nIndex < NPL_char_pool_count)
			{
				ret = static_cast<pointer>(s_mem_pools[nIndex].malloc());
			}
			else
			{
				ret = UserAllocator::malloc(n);
			}
			if (ret == 0)
				boost::throw_exception(std::bad_alloc());
			return ret;
		}
		static pointer allocate(const size_type n, const void * const)
		{ return allocate(n); }
		static void deallocate(const pointer ptr, const size_type n)
		{
			if (ptr == 0 || n == 0)
				return;
			int nIndex = Math::log2_ceil(n) - NPL_char_pool_init_size;
			if(nIndex<0)
				nIndex = 0;
			if(nIndex < NPL_char_pool_count)
			{
				s_mem_pools[nIndex].free(ptr);
			}
			else
			{
				UserAllocator::free(ptr);
			}
		}
	};

	/** predefined memory pool (free lists)	*/
	template <typename UserAllocator, typename Mutex>
	typename CNPLPool_Char_alloc<UserAllocator, Mutex>::SingletonMemPool_Type CNPLPool_Char_alloc<UserAllocator, Mutex>::s_mem_pools;
}

// required by DLL interface
//EXPIMP_TEMPLATE template class PE_CORE_DECL ParaEngine::CNPLPool_Char_alloc<>;
//EXPIMP_TEMPLATE template class PE_CORE_DECL std::basic_string<char, std::char_traits<char>, ParaEngine::CNPLPool_Char_alloc<> >;