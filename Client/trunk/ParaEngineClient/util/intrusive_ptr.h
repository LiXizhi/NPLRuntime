#pragma once
//-----------------------------------------------------------------------------
// Class:	Intrusive_ptr
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.6.4
// Desc:  a thread-safe intrusive_ptr to replace boost::shared_ptr. It has smaller memory footprint than shared_ptr.
// since the reference counter is on the object itself. 
//-----------------------------------------------------------------------------
namespace ParaEngine
{
	class intrusive_ptr_thread_safe_base;
	class intrusive_ptr_single_thread_base;
}

namespace boost
{
	// the two function overloads must be in the boost namespace on most compilers:
	PE_CORE_DECL void intrusive_ptr_add_ref(ParaEngine::intrusive_ptr_thread_safe_base* ref);
	PE_CORE_DECL void intrusive_ptr_release(ParaEngine::intrusive_ptr_thread_safe_base* ref);

	PE_CORE_DECL void intrusive_ptr_add_ref(ParaEngine::intrusive_ptr_single_thread_base* ref);
	PE_CORE_DECL void intrusive_ptr_release(ParaEngine::intrusive_ptr_single_thread_base* ref);
}

#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>

#ifndef ParaIntrusivePtr
#define ParaIntrusivePtr boost::intrusive_ptr
#endif

namespace ParaEngine
{
	/** multi-threaded reference counted base class for boost::intrusive_ptr
	* all boost::intrusive_ptr<T>, should derive from this class.
	* this is a thead safe version.
	e.g.
	Class Foo : public ParaEngine::intrusive_ptr_thread_safe_base
	{
	// members
	}
	// so now Foo_ptr is the same as boost::shared_ptr
	typedef boost::intrusive_ptr<Foo> Foo_ptr;
	*/
	class PE_CORE_DECL intrusive_ptr_thread_safe_base
	{
		friend void boost::intrusive_ptr_add_ref(intrusive_ptr_thread_safe_base*);
		friend void boost::intrusive_ptr_release(intrusive_ptr_thread_safe_base*);
	public:
#ifdef WIN32
		mutable long m_ref_count;
#else
		mutable boost::detail::atomic_count m_ref_count;
#endif
	protected:
		intrusive_ptr_thread_safe_base() : m_ref_count(0){};
		intrusive_ptr_thread_safe_base(const intrusive_ptr_thread_safe_base&) : m_ref_count(0){};
		intrusive_ptr_thread_safe_base& operator=(const intrusive_ptr_thread_safe_base&) { return *this; }
		virtual ~intrusive_ptr_thread_safe_base(){};
	};

	/** single-threaded reference counted base class for boost::intrusive_ptr
	* all boost::intrusive_ptr<T>, should derive from this class.
	* this is NOT thead safe, use a lock when used by multiple thread.
	e.g.
	Class Foo : public ParaEngine::intrusive_ptr_single_thread_base
	{
	// members
	}
	// so now Foo_ptr is the same as boost::shared_ptr
	typedef boost::intrusive_ptr<Foo> Foo_ptr;
	*/
	class PE_CORE_DECL intrusive_ptr_single_thread_base
	{
		friend void boost::intrusive_ptr_add_ref(intrusive_ptr_single_thread_base*);
		friend void boost::intrusive_ptr_release(intrusive_ptr_single_thread_base*);

	public:
		mutable long m_ref_count;

	protected:
		intrusive_ptr_single_thread_base() : m_ref_count(0){};
		intrusive_ptr_single_thread_base(const intrusive_ptr_single_thread_base&) : m_ref_count(0){};
		intrusive_ptr_single_thread_base& operator=(const intrusive_ptr_single_thread_base&) { return *this; }
		virtual ~intrusive_ptr_single_thread_base(){};
	};
}
