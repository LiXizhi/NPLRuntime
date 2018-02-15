//-----------------------------------------------------------------------------
// Class:	NPLStateMemAllocator
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.17
// Desc:  custom memory allocator for NPL runtime state. 
// Lua state has many realloc() calls, which the memory allocator does not handle very well. 
// the system's allocator has a better performance.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef PARAENGINE_MOBILE
#include "util/dl_malloc_include.h"
#endif
#include "NPLStateMemAllocator.h"

namespace NPL
{
#ifndef PARAENGINE_MOBILE
	void * npl_mem_dl_alloc (void *ud, void *ptr, size_t osize, size_t nsize) 
	{
		if (nsize == 0) 
		{
			mspace_free(ud, ptr);
			return NULL;
		}
		else
		{
			/* uncomment to test "out of memory", set s_SimulateError to 1, when you want memory to fail. 
local file = ParaIO.open("temp/mem.txt", "w");
if(file:IsValid()) then
	local out = {};
	for i=1,52 do
		out[#out+1] = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
	end
	file:WriteString(table.concat(out)); -- this will run out of memory.
	file:close();
end	
			static int s_SimulateError = 0;
			if (nsize > 5000)
			{
				if (s_SimulateError == 0)
					return mspace_realloc(ud, ptr, nsize);
				else
					return NULL;
			}*/
			return mspace_realloc(ud, ptr, nsize);
		}
	}

	void * npl_mem_alloc (void *ud, void *ptr, size_t osize, size_t nsize) 
	{
		if (nsize == 0) 
		{
			// OUTPUT_LOG("free: osize %d, %h\n", (int)osize, ptr);
			// ::free(ptr);

			((CNPLStateMemAllocator*)ud)->deallocate(ptr, osize);

			return NULL;
		}
		else
		{
			// OUTPUT_LOG("real: osize %d, nsize %d, %h\n", (int)osize, (int)nsize, ptr);
			// return ::realloc(ptr, nsize);
			return ((CNPLStateMemAllocator*)ud)->reallocate(ptr, osize, nsize);
		}
	}
#endif
	CNPLStateMemAllocator::CNPLStateMemAllocator()
	{
		for(int i=0;i<s_pool_count; ++i)
		{
			m_mem_pools[i] = new pool_type(s_pool_init_size_bytes<<i, 2048>>i);
		}
	}

	CNPLStateMemAllocator::~CNPLStateMemAllocator()
	{
		for(int i=0;i<s_pool_count; ++i)
		{
			delete m_mem_pools[i];
		}
	}

	/** reallocate a buffer, old buffer content are copied to new buffer is necessary */
	void* CNPLStateMemAllocator::reallocate(void *ptr, size_t osize, size_t nsize)
	{
		if(osize == nsize)
			return ptr;
		else if(nsize == 0)
		{
			deallocate(ptr, osize);
			return 0;
		}
		void * ret = 0;
		int nIndex = ParaEngine::Math::log2_ceil(nsize) - s_pool_init_size;
		if(nIndex<0)
			nIndex = 0;
		if(osize == 0 || ptr == 0)
		{
			if(nIndex<s_pool_count)
			{
				ret = m_mem_pools[nIndex]->malloc();
			}
			else
			{
				ret = ::malloc(nsize);
			}
		}
		else
		{
			int oIndex = ParaEngine::Math::log2_ceil(osize) - s_pool_init_size;
			if(oIndex<0)
				oIndex = 0;
			if(nIndex<s_pool_count)
			{
				if(oIndex == nIndex)
					return ptr;
				else if(oIndex < nIndex)
				{
					//OUTPUT_LOG("cpy %d, %d\n", osize, nsize);
					//return NULL;
					ret = m_mem_pools[nIndex]->malloc();
					memcpy(ret, ptr, min(osize,nsize));
					m_mem_pools[oIndex]->free(ptr);
				}
				else
				{
					if(oIndex < s_pool_count)
					{
						//OUTPUT_LOG("cpy %d, %d\n", osize, nsize);
						//return NULL;
						ret = m_mem_pools[nIndex]->malloc();
						memcpy(ret, ptr, min(osize,nsize));
						m_mem_pools[oIndex]->free(ptr);
					}
					else
					{
						//OUTPUT_LOG("cpy %d, %d\n", osize, nsize);
						//return NULL;
						ret = m_mem_pools[nIndex]->malloc();
						memcpy(ret, ptr, min(osize,nsize));
						::free(ptr);
					}
				}
			}
			else
			{
				if(oIndex<s_pool_count)
				{
					//OUTPUT_LOG("cpy %d, %d\n", osize, nsize);
					//return NULL;
					
					ret = ::malloc(nsize);
					memcpy(ret, ptr, min(osize,nsize));
					m_mem_pools[oIndex]->free(ptr);
				}
				else
				{
					ret = ::realloc(ptr, nsize);
				}
			}
		}
		if (ret == 0)
		{
			OUTPUT_LOG("error: NPL memory allocation error. not enough memory \n");
		}
		return ret;
	}

	/** free an old buffer. */
	void CNPLStateMemAllocator::deallocate(void* const ptr, const size_t n)
	{
		if(ptr == 0) 
			return;
		int nIndex = ParaEngine::Math::log2_ceil(n) - s_pool_init_size;
		if(nIndex<0)
			nIndex = 0;
		if(nIndex < s_pool_count)
		{
			m_mem_pools[nIndex]->free(ptr);
		}
		else
		{
			::free(ptr);
		}
	}
}//NPL

