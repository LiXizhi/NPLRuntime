#pragma once

#include <memory>
namespace std {

#if USE_UNIQUE_PTR_AS_AUTO_PTR
    // A std::unique_ptr with move sematics for single-threaded environment
    // since std::auto_ptr is deprecated in C++11, use this one instead
    template<typename T>
    class movable_auto_ptr
    {
    public:
        movable_auto_ptr(T* p = 0) : m_ptr(p) {}
        movable_auto_ptr(const movable_auto_ptr& other) : m_ptr(std::move(const_cast<movable_auto_ptr&>(other).m_ptr)) {}
        movable_auto_ptr(movable_auto_ptr&& r) : m_ptr(std::move(r.m_ptr)) {}
        movable_auto_ptr& operator=(movable_auto_ptr&& r) {
            if (this != &r) {
                m_ptr = std::move(r.m_ptr);
            }
            return *this;
        }
        inline T* get() const { return m_ptr.get(); }
        inline T& operator*() const { return *m_ptr; }
        inline T* operator->() const { return m_ptr.get(); }
        inline void reset(T* p = 0) { m_ptr.reset(p); }
        inline T* release() { return m_ptr.release(); }
    private:
        std::unique_ptr<T> m_ptr;
    };
#else
    template <typename T>
    class movable_auto_ptr {
    private:
        T* ptr;
    public:
        explicit movable_auto_ptr(T* p = nullptr) : ptr(p) {}
        ~movable_auto_ptr() { 
            if(ptr) 
                delete ptr;
        }
        movable_auto_ptr(const movable_auto_ptr& u) : ptr(u.ptr) { ((movable_auto_ptr&)u).ptr = nullptr; }
        movable_auto_ptr(movable_auto_ptr&& u) : ptr(u.ptr) { u.ptr = nullptr; } 
        
        inline T& operator*() const { return *ptr; }
        inline T* operator->() const { return ptr; }
        inline bool operator!() const { return ptr == nullptr; }
        inline explicit operator bool() const { return ptr != nullptr; }
        inline T* get() const { return ptr; }
        inline T* release() {
            T* old_ptr = ptr;
            ptr = nullptr;
            return old_ptr;
        }
        inline void reset(T* p = nullptr)  {
            if (ptr != p) {
                if(ptr)
                    delete ptr;
                ptr = p;
            }
        }
    };
#endif
}

namespace boost {
    template<class T> T* get_pointer(std::movable_auto_ptr<T> const& p)
    {
        return p.get();
    }
}
