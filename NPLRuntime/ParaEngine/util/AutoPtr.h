#ifndef __AUTO_PTR_H__
#define __AUTO_PTR_H__

#ifdef EMSCRIPTEN_SINGLE_THREAD
#include <memory>
namespace std {
    template<typename T>
    class auto_ptr
    {
    public:
        auto_ptr(T* p = 0): m_ptr(p) {}
        auto_ptr(auto_ptr& r): m_ptr(std::move(r.m_ptr)) {}
        T* get() const  { return m_ptr.get(); }
        T& operator*() const { return *m_ptr; }
        T* operator->() const { return m_ptr.get(); }
        void reset( T* p = 0 ) { m_ptr.reset(p); }
        T* release() { return m_ptr.release(); }
    private:
        std::unique_ptr<T> m_ptr;
    };
}

namespace boost {
    template<class T> T * get_pointer(std::auto_ptr<T> const& p)
    {
        return p.get();
    }
}
#endif
#endif