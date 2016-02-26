//-----------------------------------------------------------------------------
// Class:	Intrusive_ptr
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.4
// Desc:  a thread-safe intrusive_ptr to replace boost::shared_ptr. It has smaller memory footprint than shared_ptr.
// since the reference counter is on the object itself. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "intrusive_ptr.h"
#ifdef WIN32
#include <boost/detail/interlocked.hpp>
#endif

namespace boost
{
	void intrusive_ptr_add_ref(ParaEngine::intrusive_ptr_thread_safe_base* ref)
	{ 
#ifdef WIN32
		BOOST_INTERLOCKED_INCREMENT(&ref->m_ref_count);
#else
		++(ref->m_ref_count);
#endif
	}

	void intrusive_ptr_release(ParaEngine::intrusive_ptr_thread_safe_base* ref)
	{ 
#ifdef WIN32
		if(!BOOST_INTERLOCKED_DECREMENT(&ref->m_ref_count))
		{
			delete ref;
		}
#else
		if (--(ref->m_ref_count) == 0)
			delete ref;
#endif
	}

	void intrusive_ptr_add_ref(ParaEngine::intrusive_ptr_single_thread_base* ref)
	{ 
		// increment reference count of object *ref
		++(ref->m_ref_count);
	}

	void intrusive_ptr_release(ParaEngine::intrusive_ptr_single_thread_base* ref)
	{ 
		// decrement reference count, and delete object when reference count reaches 0
		if (--(ref->m_ref_count) == 0)
			delete ref;
	}
}

