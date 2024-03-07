#pragma once

#include <memory>
namespace std {
	template<typename T>

	// A std::unique_ptr with move sematics for single-threaded environment
	// since std::auto_ptr is deprecated in C++11, use this one instead
	class movable_auto_ptr
	{
	public:
		movable_auto_ptr(T* p = 0): m_ptr(p) {}
		movable_auto_ptr(movable_auto_ptr& r): m_ptr(std::move(r.m_ptr)) {}
		inline T* get() const  { return m_ptr.get(); }
		inline T& operator*() const { return *m_ptr; }
		inline T* operator->() const { return m_ptr.get(); }
		inline void reset( T* p = 0 ) { m_ptr.reset(p); }
		inline T* release() { return m_ptr.release(); }
	private:
		std::unique_ptr<T> m_ptr;
	};
}	

namespace boost {
	template<class T> T* get_pointer(std::movable_auto_ptr<T> const& p)
	{
		return p.get();
	}
}