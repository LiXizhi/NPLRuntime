// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

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


#ifndef LUABIND_REF_HPP_INCLUDED
#define LUABIND_REF_HPP_INCLUDED

#include <cassert>
#include <algorithm>

#include <luabind/config.hpp>
#include <luabind/lua_include.hpp>

namespace luabind
{

namespace detail
{

	struct lua_reference
	{
		lua_reference(lua_State* L_ = 0) noexcept
			: L(L_)
			, m_ref(LUA_NOREF)
		{}
		lua_reference(lua_reference const& r)
			: L(r.L)
			, m_ref(LUA_NOREF)
		{
			if (!r.is_valid()) return;
			r.get(L);
			set(L);
		}
		lua_reference(lua_reference&& r) noexcept
			: L(r.L)
			, m_ref(r.m_ref)
		{
			r.L = 0;
			r.m_ref = LUA_NOREF;
		}
		~lua_reference() { reset(); }

		lua_State* state() const noexcept { return L; }

		void operator=(lua_reference const& r)
		{
			if (this == &r) return;
			reset();
			if (!r.is_valid()) return;
			r.get(r.state());
			set(r.state());
		}

		void operator=(lua_reference&& r) noexcept
		{
			if (this == &r) return;
			reset();
			L = r.L;
			m_ref = r.m_ref;
			r.L = 0;
			r.m_ref = LUA_NOREF;
		}

		bool is_valid() const noexcept
		{ return m_ref != LUA_NOREF; }

		void set(lua_State* L_)
		{
			reset();
			L = L_;
			m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		void replace(lua_State* L_)
		{
			lua_rawseti(L_, LUA_REGISTRYINDEX, m_ref);
		}

		void get(lua_State* L_) const
		{
			assert(m_ref != LUA_NOREF);
			assert(L_);
			lua_rawgeti(L_, LUA_REGISTRYINDEX, m_ref);
		}

		void reset()
		{
			if (L && m_ref != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, m_ref);
			m_ref = LUA_NOREF;
		}

		void swap(lua_reference& r) noexcept
		{
			assert(r.L == L);
			std::swap(r.m_ref, m_ref);
		}

	private:
		lua_State* L;
		int m_ref;
	};

}}

#endif // LUABIND_REF_HPP_INCLUDED
