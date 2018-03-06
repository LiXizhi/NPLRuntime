//-----------------------------------------------------------------------------
// Class:	DL_Allocator
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.19
// Desc:  memory policy in ParaEngine
/**
---+++ Memory allocation policies in ParaEngine
   1.For all persistent objects in any threads, prefer to use the system's default malloc. 
   1.For frequently allocated temporary objects in multiple threads (e.g. messages in the NPL modules), prefer to use special fixed-sized memory pools for each object type. such as CNPLPool_Char_alloc 
   1.For general-purpose object allocations that is only used(allocated/deallocated) by a single thread, prefer to use dl_malloc without locks. e.g. NPL scripting state uses npl_mem_dl_alloc; an alternative is to use CNPLStateMemAllocator (but not as fast as former, since realloc() is not supported natively).
   1.For variable sized temporary string/buffer in multi-thread environments, use ParaEngine::StringBuilder (which internally use several fixed sized memory pools with fast mutexes).
   1.For variable sized temporary objects(queue, vector, string) in the main gaming thread, prefer to use DL_Allocator or Pool_Char_alloc, according to usage
   1.For fixed-sized single-threaded temporary objects(list, map), use Pool_Char_alloc.
   1.For fixed-sized multi-threaded temporary objects(list, map), use default allocator with a memory pool.

For temporary object that live just a frame in the game engine, I prefer to use DL_Allocator, especially for stl based object. 
because dl_allocator has its own private heap, keep temporary objects close to each other. 

There are several other allocators
   1. Loki: this is the famous Loki small object allocator. But since dlmalloc already optimized for small objects, we need not create pools in the applications layers
   1. EASTL: this is a paper with design rationales. I like the idea of separating persistent memory allocation with temporary allocation. From Low memory to high memory, more locality is achieved in this way. 
   1. Dr.Dobbs's stl_pool: Improving Performance with Custom Pool Allocators for STL. 
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/ParaMemPool.h"
#include "util/fixed_string.hpp"
#include "util/stack_containers.hpp"

namespace ParaEngine
{
#ifndef PARAENGINE_MOBILE

	static mspace m_dl_mspace = 0;

	void*  mainthread_malloc(size_t bytes) 
	{
		if (m_dl_mspace == 0) 
			m_dl_mspace = create_mspace(0, 0);
		return mspace_malloc(m_dl_mspace, bytes);
	}

	void  mainthread_free(void* mem) 
	{ 
		mspace_free(m_dl_mspace, mem); 
	}

#endif

//#define TEST_ME
#ifdef TEST_ME	
	void TestParaMemPool()
	{
		typedef std::list<void*, Pool_Char_alloc<void*> > MyList;
		MyList list_;

		typedef ParaEngine::DL_Allocator<void*>	DL_Allocator_Void;
		typedef std::list<void*, DL_Allocator_Void >	List_Void_Type;
		List_Void_Type list2_;

		fixed_string<char, 128> fixedStr;
		fixedStr = "test";
		fixedStr.append("a");
		fixedStr.clear();

		StackString<64> stackStr;
		stackStr->append("test");
		stackStr->clear();
	}
#endif
}


