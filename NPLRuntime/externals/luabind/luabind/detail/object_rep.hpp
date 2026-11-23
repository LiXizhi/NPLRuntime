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


#ifndef LUABIND_OBJECT_REP_HPP_INCLUDED
#define LUABIND_OBJECT_REP_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <luabind/config.hpp>
#include <luabind/lua_include.hpp>
#include <luabind/detail/instance_holder.hpp>
#include <luabind/detail/ref.hpp>

namespace luabind { namespace detail
{
	class class_rep;

	void finalize(lua_State* L, class_rep* crep);

	// Allocated inside Lua userdata for each bound C++ object/pointer.
	// Compact 96-byte layout on 64-bit: holds pointer_holder<P> (56B) and
	// compact_value_holder<T> (up to 32B T) inline with zero system heap allocations.
	class LUABIND_API object_rep
	{
	public:
		static constexpr std::size_t kInlineBufferSize = 56;
		static constexpr unsigned char kFlagIsConst    = 0x01;
		static constexpr unsigned char kFlagCustomFenv = 0x02;

		static std::uint32_t expected_magic() noexcept
		{
			return static_cast<std::uint32_t>(
				(reinterpret_cast<std::uintptr_t>(&s_magic_anchor) >> 3) ^ 0x4C554142u);
		}

		object_rep(instance_holder* instance, class_rep* crep) noexcept;
		~object_rep();

		bool has_valid_magic() const noexcept
		{
			return m_magic == expected_magic();
		}

		const class_rep* crep() const noexcept { return m_classrep; }
		class_rep* crep() noexcept { return m_classrep; }

		void set_instance(instance_holder* instance) noexcept
		{
			m_instance = instance;
			m_cached_ptr = 0;
			m_cached_cls_id = unknown_class;
			if (instance && instance->pointee_const())
				m_flags |= kFlagIsConst;
			else
				m_flags &= static_cast<unsigned char>(~kFlagIsConst);
		}

		void set_instance_fast(
			instance_holder* instance
		  , void* raw_ptr
		  , class_id static_cls_id
		  , bool is_const
		) noexcept
		{
			m_instance = instance;
			if (raw_ptr != 0)
			{
				m_cached_ptr = raw_ptr;
				m_cached_cls_id = static_cls_id;
			}
			else
			{
				m_cached_ptr = 0;
				m_cached_cls_id = unknown_class;
			}
			if (is_const)
				m_flags |= kFlagIsConst;
			else
				m_flags &= static_cast<unsigned char>(~kFlagIsConst);
		}

		// Single-comparison inline exact-type unwrap (zero virtual call, zero cast_graph).
		void* try_get_raw_fast(class_id target, bool require_nonconst) const noexcept
		{
			if (LUABIND_LIKELY(m_cached_cls_id == target))
			{
				if (LUABIND_UNLIKELY(require_nonconst && (m_flags & kFlagIsConst) != 0))
					return 0;
				return m_cached_ptr;
			}
			return 0;
		}

		void add_dependency(lua_State* L, int index);
		void release_dependency_refs(lua_State* L);

		std::size_t dependency_count() const noexcept
		{
			return m_dependency_cnt;
		}

		bool has_custom_fenv() const noexcept
		{
			return (m_flags & kFlagCustomFenv) != 0;
		}

		void mark_custom_fenv() noexcept
		{
			m_flags |= kFlagCustomFenv;
		}

		std::pair<void*, int> get_instance(class_id target) const
		{
			if (LUABIND_LIKELY(m_cached_cls_id == target))
				return std::pair<void*, int>(m_cached_ptr, 0);
			if (m_instance == 0)
				return std::pair<void*, int>((void*)0, -1);
			return m_instance->get(target);
		}

		bool is_const() const noexcept
		{
			return (m_flags & kFlagIsConst) != 0;
		}

		void release()
		{
			m_cached_ptr = 0;
			m_cached_cls_id = unknown_class;
			if (m_instance)
				m_instance->release();
		}

		void* allocate(std::size_t size, std::size_t align = alignof(double)) noexcept
		{
			if (LUABIND_LIKELY(size <= kInlineBufferSize && align <= alignof(double)))
				return static_cast<void*>(m_instance_buffer);
			return std::malloc(size);
		}

		void deallocate(void* storage) noexcept
		{
			if (LUABIND_LIKELY(storage == static_cast<void*>(m_instance_buffer)))
				return;
			std::free(storage);
		}

	private:
		object_rep(object_rep const&) = delete;
		void operator=(object_rep const&) = delete;

		static int s_magic_anchor;

		void* m_cached_ptr;
		class_id m_cached_cls_id;
		instance_holder* m_instance;
		class_rep* m_classrep;
		std::uint32_t m_magic;
		std::uint16_t m_dependency_cnt;
		unsigned char m_flags;
		alignas(double) unsigned char m_instance_buffer[kInlineBufferSize];
	};

	template<class T>
	struct delete_s
	{
		static void apply(void* ptr)
		{
			delete static_cast<T*>(ptr);
		}
	};

	template<class T>
	struct destruct_only_s
	{
		static void apply(void* ptr)
		{
			(void)ptr;
#ifndef NDEBUG
			int completeness_check[sizeof(T)];
			(void)completeness_check;
#endif
			static_cast<T*>(ptr)->~T();
		}
	};

	LUABIND_API object_rep* get_instance(lua_State* L, int index) noexcept;

	// Zero-stack-push inline check for luabind userdata.
	inline object_rep* get_instance_fast(lua_State* L, int index) noexcept
	{
		object_rep* result = static_cast<object_rep*>(lua_touserdata(L, index));
		if (LUABIND_LIKELY(result != 0))
		{
			if (LUABIND_LIKELY(lua_objlen(L, index) == sizeof(object_rep) && result->has_valid_magic()))
				return result;
		}
		return 0;
	}

	LUABIND_API void push_instance_metatable(lua_State* L);
	LUABIND_API object_rep* push_new_instance(lua_State* L, class_rep* cls);

}}

#endif // LUABIND_OBJECT_REP_HPP_INCLUDED
