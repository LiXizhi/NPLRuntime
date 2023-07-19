#pragma once
#ifndef PARAENGINE_MOBILE
#include "util/dl_malloc_include.h"
#endif
#include "util/PoolBase.h"
#include <memory>

#ifdef max
#undef max
#endif

namespace ParaEngine
{
	
	// #define FixedSizedAllocator		DL_Allocator 
	// #define FixedSizedAllocator		std::allocator
#ifdef PARAENGINE_MOBILE
#define FixedSizedAllocator		std::allocator
#else
	/** this provides a short cut name for changing memory policies for the default FixedSizedAllocator. */
#define FixedSizedAllocator		Pool_Char_alloc
	extern void*  mainthread_malloc(size_t bytes);
	extern void	  mainthread_free(void* mem);

	struct default_user_allocator_dl_malloc
	{
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char * malloc(const size_type bytes)
		{ return (char*) mainthread_malloc(bytes); }

		static void free(char * const block)
		{ mainthread_free(block); }
	};

	struct default_user_allocator_new_delete
	{
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char * malloc(const size_type bytes)
		{ return new (std::nothrow) char[bytes]; }
		static void free(char * const block)
		{ delete [] block; }
	};

	struct default_user_allocator_malloc_free
	{
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char * malloc(const size_type bytes)
		{ return reinterpret_cast<char *>(std::malloc(bytes)); }
		static void free(char * const block)
		{ std::free(block); }
	};

	//////////////////////////////////////////////////////////////////////////
	//
	// DL_Allocator: [single threaded] for variable sized stl containers like vector, queue, stack, etc.
	//
	//////////////////////////////////////////////////////////////////////////

	// forward declare
	template<typename T, typename UserAllocator = default_user_allocator_dl_malloc>
	class DL_Allocator;

	/**
	* this is a general single-threaded allocator, which is used by the main game (rendering) thread in ParaEngine.
	* it is for temporary small object allocations/dellocations. The current implementation is based on std::allocator and dlmalloc.  
	* @param UserAllocator: it default to dl_malloc, i.e. default_user_allocator_dl_malloc, other possible values are default_user_allocator_new_delete, default_user_allocator_malloc_free
	* Examples:
	typedef ParaEngine::DL_Allocator<int>	DL_Allocator_Int;
	typedef std::list<int, DL_Allocator_Int >	List_Int_Type;

	typedef ParaEngine::DL_Allocator<void*>	DL_Allocator_Void;
	typedef std::list<void*, DL_Allocator_Void >	List_Void_Type;
	*/
	template<typename T, typename UserAllocator>
	class DL_Allocator {
	public : 
		//    typedefs
		typedef T value_type;
		typedef UserAllocator user_allocator;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

	public : 
		//    convert an allocator<T> to allocator<U>
		template<typename U>
		struct rebind {
			typedef DL_Allocator<U> other;
		};

	public : 
		inline explicit DL_Allocator() {}
		inline ~DL_Allocator() {}
		inline explicit DL_Allocator(DL_Allocator const&) {}
		template<typename U>
		inline explicit DL_Allocator(DL_Allocator<U> const&) {}

		//    address
		inline pointer address(reference r) { return &r; }
		inline const_pointer address(const_reference r) { return &r; }

		//    size
		inline size_type max_size() const { 
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		//    memory allocation
// #ifdef EMSCRIPTEN_SINGLE_THREAD
#ifdef EMSCRIPTEN
		inline pointer allocate(size_type cnt, const void* _ = 0) 
#else
		inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) 
#endif
		{ 
			return reinterpret_cast<pointer>(user_allocator::malloc(cnt * sizeof (T))); 
		}
		inline void deallocate(pointer p, size_type) 
		{ 
			user_allocator::free((char * const)p); 
		}

		//    construction/destruction
		inline void construct(pointer p, const T& t) { new(p) T(t); }
		inline void destroy(pointer p) { p->~T(); }

		inline bool operator==(DL_Allocator const&) { return true; }
		inline bool operator!=(DL_Allocator const& a) { return !operator==(a); }
	};    //    end of class DL_Allocator 


	//////////////////////////////////////////////////////////////////////////
	//
	// Pool_Char_alloc: [single threaded] for fixed sized stl containers like list, map.
	//
	//////////////////////////////////////////////////////////////////////////

	// forward declare
	template <typename T, typename UserAllocator = default_user_allocator_dl_malloc>
	class Pool_Char_alloc;


	/** This class can be used as a custom allocator for stl containers, which constantly create and delete small or medium sized objects.  
	* it is optimized for frequently used temporary objects.
	* it is faster than std::allocator and boost::pool_alloc. But it costs more (double at most) memory than boost::pool_alloc, because we may return more memory than requested. 
	*
	* Internally, it has 7 free lists (boost::pool). Each free list only serves one fixed-sized object at a time. 
	* Each pool has their own light weighted locks for thread-safe access. 
	* The free list sizes are 32,64,128,256,512,1024,2048; we will use the default allocator for sizes over 2048. 
	* One can configure the internal freelists by editing the macro char_pool_init_size_bytes, char_pool_init_size, char_pool_count
	*
	* [NOT thread safe]: for a thread safe implementation, refer to CNPLPool_Char_alloc in NPLMemPool.h
	* Example:
	typedef std::list<void*, Pool_Char_alloc<void*> > MyList;	
	*/
	template <typename T, typename UserAllocator>
	class Pool_Char_alloc
	{
	public:
		typedef T value_type;
		typedef UserAllocator user_allocator;

		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef boost::pool<UserAllocator> pool_type;
		typedef typename pool_type::size_type size_type;
		typedef typename pool_type::difference_type difference_type;

		/** the initial smallest fixed sized chunk that the memory pool will create. default to 32 bytes */
		static const int char_pool_init_size_bytes  = 32;
		/** this should be log2(char_pool_init_size_bytes) */
		static const int char_pool_init_size = 5;
		/** how many free lists are there in the pool. char_pool_init_size_bytes^2 is the biggest chunk.  currently it is 2048 bytes */
		static const int char_pool_count = 7;

		struct SingletonMemPool_Type
		{
			SingletonMemPool_Type()
			{
				int i=0;
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes); // 32
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2); // 64
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2*2); // 128
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2*2*2); // 256
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2*2*2*2); // 512
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2*2*2*2*2); // 1024
				m_mem_pools[i++] = new pool_type(char_pool_init_size_bytes*2*2*2*2*2*2); // 2048
				PE_ASSERT(i==char_pool_count);
			}
			~SingletonMemPool_Type()
			{
				for(int i=0;i<char_pool_count;++i)
				{
					delete m_mem_pools[i];
				}
			}
			pool_type& operator [] (int nIndex)
			{
				return *(m_mem_pools[nIndex]);
			}
		private:			
			pool_type* m_mem_pools[char_pool_count];
		};

		static SingletonMemPool_Type s_mem_pools;

		template <typename U>
		struct rebind
		{
			typedef Pool_Char_alloc<U, UserAllocator> other;
		};
	public:
		inline explicit Pool_Char_alloc(){}
		inline ~Pool_Char_alloc() {}
		inline explicit Pool_Char_alloc(Pool_Char_alloc const&) {}
		template<typename U>
		inline Pool_Char_alloc(const Pool_Char_alloc<U, UserAllocator> &){}

		//    address
		inline pointer address(reference r) { return &r; }
		inline const_pointer address(const_reference r) { return &r; }
		//    size
		inline size_type max_size() const { 
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		//    memory allocation
// #ifdef EMSCRIPTEN_SINGLE_THREAD
#ifdef EMSCRIPTEN
		inline pointer allocate(size_type cnt, const void* _ = 0) 
#else
		inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) 
#endif
		{
			int n = (int)(cnt * sizeof (T));
			int nIndex = Math::log2_ceil(n) - char_pool_init_size;
			if(nIndex<0)
				nIndex = 0;
			pointer ret = 0;
			if(nIndex < char_pool_count)
			{
				ret = static_cast<pointer>(s_mem_pools[nIndex].malloc());
			}
			else
			{
				ret = reinterpret_cast<pointer>(user_allocator::malloc(n));
			}
			if (ret == 0)
				boost::throw_exception(std::bad_alloc());
			return ret;
		}

		inline void deallocate(pointer ptr, size_type cnt) 
		{
			int n = (int)(cnt * sizeof (T));
			if (ptr == 0 || n == 0)
				return;
			int nIndex = Math::log2_ceil(n) - char_pool_init_size;
			if(nIndex<0)
				nIndex = 0;
			if(nIndex < char_pool_count)
			{
				s_mem_pools[nIndex].free(ptr);
			}
			else
			{
				user_allocator::free((char * const)ptr);
			}
		}

		//    construction/destruction
		inline void construct(pointer p, const T& t) { new(p) T(t); }
		inline void destroy(pointer p) { p->~T(); }

		inline bool operator==(Pool_Char_alloc const&) { return true; }
		inline bool operator!=(Pool_Char_alloc const& a) { return !operator==(a); }
	};

	/** predefined memory pool (free lists)
	*/
	template <typename T, typename UserAllocator>
	typename Pool_Char_alloc<T, UserAllocator>::SingletonMemPool_Type Pool_Char_alloc<T, UserAllocator>::s_mem_pools;
#endif
}