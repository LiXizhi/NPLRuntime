#pragma once

#include "math/ParaMath.h"
#include <boost/pool/object_pool.hpp>

namespace NPL
{
#ifndef PARAENGINE_MOBILE
	/** this function can be provided as Lua memory allocator using CNPLStateMemAllocator
	e.g. 
	CNPLStateMemAllocator* m_pMemAlloc = new CNPLStateMemAllocator();
	m_pState = lua_newstate(NPL::npl_mem_alloc, m_pMemAlloc);

	@param ud: this must be pointer to CNPLStateMemAllocator
	*/
	extern void * npl_mem_alloc(void *ud, void *ptr, size_t osize, size_t nsize);

	/** this function can be provided as Lua memory allocator using dl_malloc
	e.g.
	void* m_mspace = create_mspace(0, 0);
	m_pState = lua_newstate(NPL::npl_mem_dl_alloc, m_mspace);

	@param ud: this must be mspace returned by create_mspace(0, 0);
	*/
	extern void * npl_mem_dl_alloc(void *ud, void *ptr, size_t osize, size_t nsize);
#endif
	/** memory allocator for NPL runtime state. Since NPL state is single threaded, no lock is needed for this memory allocator. 
	This should be the fastest memory allocator on earth. 
	Internal it uses memory pool of predefined sizes. Sizes are 32,64,128,256,512,1024,2048; we will use the default allocator for sizes over 2048. 
	Most lua objects are 20-30 bytes; and most allocates are 32,64, 128, 256.
	*/
	class CNPLStateMemAllocator
	{
	public:
		typedef boost::pool<> pool_type;
		/** the initial smallest fixed sized chunk that the memory pool will create. default to 32 bytes */
		const static int s_pool_init_size_bytes = 32; // default 32
		/** this should be log2(s_pool_init_size_bytes) */
		const static int s_pool_init_size = 5; // default 5
		/** how many free lists are there in the pool. s_pool_init_size_bytes^2 is the biggest chunk.  currently it is 2048 bytes */
		const static int s_pool_count = 4; //default 4

	private:
		/** several free lists */
		pool_type * m_mem_pools[s_pool_count];

	public:
		CNPLStateMemAllocator();

		~CNPLStateMemAllocator();

		/** allocate a new buffer */
		inline void* allocate(void *ptr, size_t nsize)
		{
			return reallocate(0, 0, nsize);
		}

		/** reallocate a buffer, old buffer content are copied to new buffer is necessary */
		void* reallocate(void *ptr, size_t osize, size_t nsize);
		
		/** free an old buffer. */
		void deallocate(void* const ptr, const size_t n);
	};
}// NPL