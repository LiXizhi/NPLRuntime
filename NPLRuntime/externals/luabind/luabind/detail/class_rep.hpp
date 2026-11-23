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


#ifndef LUABIND_CLASS_REP_HPP_INCLUDED
#define LUABIND_CLASS_REP_HPP_INCLUDED

#include <boost/limits.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>

#include <string>
#include <utility>
#include <vector>

#include <luabind/config.hpp>
#include <luabind/lua_include.hpp>
#include <luabind/detail/object_rep.hpp>
#include <luabind/detail/garbage_collector.hpp>
#include <luabind/detail/operator_id.hpp>
#include <luabind/detail/class_registry.hpp>
#include <luabind/error.hpp>
#include <luabind/handle.hpp>
#include <luabind/detail/primitives.hpp>
#include <luabind/typeid.hpp>
#include <luabind/detail/ref.hpp>

namespace luabind { namespace detail
{

	LUABIND_API std::string stack_content_by_name(lua_State* L, int start_index);

	struct class_registration;

	struct conversion_storage;

	// This function is used as a tag to identify "properties".
	LUABIND_API int property_tag(lua_State*);

	class cast_graph;
	class class_id_map;

	class LUABIND_API class_rep
	{
	friend struct class_registration;
	friend int super_callback(lua_State*);
	friend int lua_class_gettable(lua_State*);
	friend int lua_class_settable(lua_State*);
	friend int static_class_gettable(lua_State*);
	public:

		enum class_type
		{
			cpp_class = 0,
			lua_class = 1
		};

		class_rep(type_id const& type
			, const char* name
			, lua_State* L
		);

		class_rep(lua_State* L, const char* name);

		~class_rep();

		std::pair<void*,void*> allocate(lua_State* L) const;

		static int constructor_dispatcher(lua_State* L);

		struct base_info
		{
			int pointer_offset;
			class_rep* base;
		};

		void add_base_class(const base_info& binfo);

		const std::vector<base_info>& bases() const noexcept { return m_bases; }

		void set_type(type_id const& t) { m_type = t; }
		type_id const& type() const noexcept { return m_type; }

		const char* name() const noexcept { return m_name; }

		// Returns the fast table-indexed metatable when the class has no properties,
		// or the C-dispatched instance metatable when properties are present.
		int metatable_ref() const noexcept { return m_instance_metatable; }
		int fallback_metatable_ref() const noexcept { return m_fallback_instance_metatable; }

		void get_table(lua_State* L) const { m_table.push(L); }
		void get_default_table(lua_State* L) const { m_default_table.push(L); }

		class_type get_class_type() const noexcept { return m_class_type; }

		void add_static_constant(const char* name, int val);

		static int super_callback(lua_State* L);

		static int lua_settable_dispatcher(lua_State* L);

		static int static_class_gettable(lua_State* L);

		bool has_operator_in_lua(lua_State*, int id);

		cast_graph const& casts() const noexcept
		{
			return *m_casts;
		}

		class_id_map const& classes() const noexcept
		{
			return *m_classes;
		}

		bool has_finalize() const noexcept
		{
			return m_has_finalize;
		}

		void refresh_member_flags(lua_State* L);

	private:

		void cache_operators(lua_State*);

		type_id m_type;
		std::vector<base_info> m_bases;
		const char* m_name;
		detail::lua_reference m_self_ref;
		handle m_table;
		handle m_default_table;
		class_type m_class_type;
		int m_instance_metatable;
		int m_fallback_instance_metatable;
		std::map<const char*, int, ltstr> m_static_constants;
		int m_operator_cache;
		cast_graph* m_casts;
		class_id_map* m_classes;
		bool m_has_properties;
		bool m_has_finalize;
	};

	bool is_class_rep(lua_State* L, int index);

}}

#endif // LUABIND_CLASS_REP_HPP_INCLUDED
