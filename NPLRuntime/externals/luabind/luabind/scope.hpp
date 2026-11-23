// Copyright (c) 2004 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef NEW_SCOPE_040211_HPP
#define NEW_SCOPE_040211_HPP

#include <luabind/prefix.hpp>
#include <luabind/config.hpp>
#include <luabind/lua_include.hpp>
#include <luabind/memory.hpp>
#include <new>

namespace luabind {

    struct scope;

} // namespace luabind

namespace luabind { namespace detail {

    // Non-virtual registration node: eliminates vtables, RTTI typeinfo, and
    // per-template virtual destructors across thousands of def()/.def() sites.
    struct LUABIND_API registration
    {
        typedef void (*register_fn_t)(lua_State*, registration const*);
        typedef void (*destroy_fn_t)(registration*);

        explicit registration(
            register_fn_t reg_fn
          , destroy_fn_t destroy_fn = &registration::default_destroy
        ) noexcept
          : m_register_fn(reg_fn)
          , m_destroy_fn(destroy_fn)
          , m_next(0)
        {}

        void register_(lua_State* L) const
        {
            m_register_fn(L, this);
        }

        static void default_destroy(registration* self) noexcept
        {
            ::operator delete(self);
        }

        template <class Derived>
        static void typed_destroy(registration* self) noexcept
        {
            delete static_cast<Derived*>(self);
        }

    private:
        friend struct ::luabind::scope;
        register_fn_t m_register_fn;
        destroy_fn_t m_destroy_fn;
        registration* m_next;
    };

}} // namespace luabind::detail

namespace luabind {

    struct LUABIND_API scope
    {
        scope() noexcept
          : m_chain(0)
          , m_tail(0)
        {}

        explicit scope(detail::registration* reg) noexcept
          : m_chain(reg)
          , m_tail(reg)
        {}

        explicit scope(std::movable_auto_ptr<detail::registration> reg) noexcept
          : m_chain(reg.release())
          , m_tail(m_chain)
        {}

        scope(scope const& other_) noexcept
          : m_chain(other_.m_chain)
          , m_tail(other_.m_tail)
        {
            const_cast<scope&>(other_).m_chain = 0;
            const_cast<scope&>(other_).m_tail = 0;
        }

        scope(scope&& other_) noexcept
          : m_chain(other_.m_chain)
          , m_tail(other_.m_tail)
        {
            other_.m_chain = 0;
            other_.m_tail = 0;
        }

        ~scope() noexcept
        {
            if (LUABIND_UNLIKELY(m_chain != 0))
                destroy_chain();
        }

        scope& operator,(scope s) noexcept
        {
            if (!s.m_chain)
                return *this;

            if (!m_chain)
            {
                m_chain = s.m_chain;
                m_tail = s.m_tail ? s.m_tail : s.m_chain;
            }
            else
            {
                detail::registration* tail = m_tail ? m_tail : m_chain;
                while (tail->m_next)
                    tail = tail->m_next;
                tail->m_next = s.m_chain;
                m_tail = s.m_tail ? s.m_tail : s.m_chain;
            }

            s.m_chain = 0;
            s.m_tail = 0;
            return *this;
        }

        void register_(lua_State* L) const;

    private:
        void destroy_chain() noexcept;

        detail::registration* m_chain;
        detail::registration* m_tail;
    };

    class LUABIND_API namespace_ : public scope
    {
    public:
        explicit namespace_(char const* name);
        namespace_& operator[](scope s);

    private:
        struct registration_;
        registration_* m_registration;
    };

    class LUABIND_API module_
    {
    public:
        module_(lua_State* L_, char const* name);
        void operator[](scope s);

    private:
        lua_State* m_state;
        char const* m_name;
    };

    inline module_ module(lua_State* L, char const* name = 0)
    {
        return module_(L, name);
    }

} // namespace luabind

#endif // NEW_SCOPE_040211_HPP
