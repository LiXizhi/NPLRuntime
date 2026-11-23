// Copyright Daniel Wallin 2009. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_DETAIL_MAKE_INSTANCE_090310_HPP
# define LUABIND_DETAIL_MAKE_INSTANCE_090310_HPP

# include <boost/type_traits/is_polymorphic.hpp>
# include <luabind/detail/class_registry.hpp>
# include <luabind/detail/class_rep.hpp>
# include <luabind/detail/inheritance.hpp>
# include <luabind/detail/object_rep.hpp>

namespace luabind { namespace detail {

inline class_id_map* get_class_id_map(lua_State* L)
{
    if (state_context* ctx = get_state_context(L))
        return ctx->class_ids;

    lua_pushliteral(L, "__luabind_class_id_map");
    lua_rawget(L, LUA_REGISTRYINDEX);
    class_id_map* m = static_cast<class_id_map*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return m;
}

inline class_map const* get_class_map(lua_State* L)
{
    if (state_context* ctx = get_state_context(L))
        return ctx->classes;

    lua_pushliteral(L, "__luabind_class_map");
    lua_rawget(L, LUA_REGISTRYINDEX);
    class_map const* m = static_cast<class_map const*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return m;
}

template <class T>
std::pair<class_id, void*> get_dynamic_class_aux(
    lua_State* L, T const* p, mpl::true_)
{
    if (!p)
        return std::pair<class_id, void*>(registered_class<T>::id, (void*)0);

    class_id_map* class_ids = get_class_id_map(L);
    return std::pair<class_id, void*>(
        class_ids->get_local(typeid(*p))
      , dynamic_cast<void*>(const_cast<T*>(p))
    );
}

template <class T>
inline std::pair<class_id, void*> get_dynamic_class_aux(
    lua_State*, T const* p, mpl::false_) noexcept
{
    return std::pair<class_id, void*>(registered_class<T>::id, const_cast<T*>(p));
}

template <class T>
inline std::pair<class_id, void*> get_dynamic_class(lua_State* L, T* p)
{
    return get_dynamic_class_aux(L, p, boost::is_polymorphic<T>());
}

template <class T>
inline class_rep* get_pointee_class(class_map const& classes, T*)
{
    return classes.get(registered_class<T>::id);
}

template <class P>
inline class_rep* get_pointee_class(lua_State* L, P const& p, class_id dynamic_id)
{
    class_map const* classes = get_class_map(L);
    if (LUABIND_UNLIKELY(!classes))
        return 0;

    class_rep* cls = classes->get(dynamic_id);
    if (!cls)
        cls = get_pointee_class(*classes, get_pointer(p));

    return cls;
}

LUABIND_API void throw_unregistered_class();

// Compact fast path for non-polymorphic, non-auto_ptr pointer/wrapper types.
template <class P, class T>
void make_instance_dispatch(lua_State* L, P& p, T*, mpl::true_)
{
    class_id const static_id = registered_class<T>::id;
    class_map const* classes = get_class_map(L);
    class_rep* cls = classes ? classes->get(static_id) : 0;
    if (LUABIND_UNLIKELY(!cls))
        throw_unregistered_class();

    object_rep* instance = push_new_instance(L, cls);
    typedef compact_pointer_holder<P> holder_type;
    void* storage = instance->allocate(sizeof(holder_type), alignof(holder_type));

#ifndef LUABIND_NO_EXCEPTIONS
    try
    {
        holder_type* h = new (storage) holder_type(static_cast<P&&>(p), cls);
        instance->set_instance_fast(h, h->naked_ptr(), static_id, h->pointee_const());
    }
    catch (...)
    {
        instance->deallocate(storage);
        lua_pop(L, 1);
        throw;
    }
#else
    holder_type* h = new (storage) holder_type(static_cast<P&&>(p), cls);
    instance->set_instance_fast(h, h->naked_ptr(), static_id, h->pointee_const());
#endif
}

// General path for polymorphic pointees or releasable movable_auto_ptr<T>.
template <class P, class T>
void make_instance_dispatch(lua_State* L, P& p, T* raw_p, mpl::false_)
{
    void const* initial_raw = static_cast<void const*>(raw_p);
    std::pair<class_id, void*> dynamic = get_dynamic_class(L, raw_p);

    class_rep* cls = get_pointee_class(L, p, dynamic.first);
    if (LUABIND_UNLIKELY(!cls))
        throw_unregistered_class();

    object_rep* instance = push_new_instance(L, cls);

    typedef pointer_holder<P> holder_type;

    void* storage = instance->allocate(sizeof(holder_type), alignof(holder_type));

#ifndef LUABIND_NO_EXCEPTIONS
    try
    {
        holder_type* h = new (storage) holder_type(
            static_cast<P&&>(p), dynamic.first, dynamic.second, cls, initial_raw);
        class_id const static_id = static_class_id(false ? raw_p : 0);
        instance->set_instance_fast(h, h->naked_ptr(), static_id, h->pointee_const());
    }
    catch (...)
    {
        instance->deallocate(storage);
        lua_pop(L, 1);
        throw;
    }
#else
    holder_type* h = new (storage) holder_type(
        static_cast<P&&>(p), dynamic.first, dynamic.second, cls, initial_raw);
    class_id const static_id = static_class_id(false ? raw_p : 0);
    instance->set_instance_fast(h, h->naked_ptr(), static_id, h->pointee_const());
#endif
}

// Create an appropriate instance holder for the given pointer-like object.
template <class P>
void make_instance(lua_State* L, P p)
{
    auto raw_p = get_pointer(p);
    typedef typename std::remove_pointer<decltype(raw_p)>::type pointee_t;
    typedef mpl::bool_<
        !is_releasable_auto_ptr<P>::value
        && !boost::is_polymorphic<pointee_t>::value
    > use_compact_t;
    make_instance_dispatch(L, p, raw_p, use_compact_t());
}

// Zero-malloc fast path for returning non-polymorphic C++ values to Lua.
template <class T>
void make_value_instance_aux(lua_State* L, T const& x, mpl::false_)
{
    class_id const static_id = registered_class<T>::id;
    class_map const* classes = get_class_map(L);
    class_rep* cls = classes ? classes->get(static_id) : 0;
    if (LUABIND_UNLIKELY(!cls))
        throw_unregistered_class();

    object_rep* instance = push_new_instance(L, cls);
    typedef compact_value_holder<T> holder_type;
    void* storage = instance->allocate(sizeof(holder_type), alignof(holder_type));

#ifndef LUABIND_NO_EXCEPTIONS
    try
    {
        holder_type* h = new (storage) holder_type(cls, x);
        instance->set_instance_fast(h, h->raw_ptr(), static_id, false);
    }
    catch (...)
    {
        instance->deallocate(storage);
        lua_pop(L, 1);
        throw;
    }
#else
    holder_type* h = new (storage) holder_type(cls, x);
    instance->set_instance_fast(h, h->raw_ptr(), static_id, false);
#endif
}

// Zero-malloc fast path for returning polymorphic C++ values to Lua.
template <class T>
void make_value_instance_aux(lua_State* L, T const& x, mpl::true_)
{
    class_id const static_id = registered_class<T>::id;
    class_id const dyn_id = get_class_id_map(L)->get_local(typeid(x));
    class_map const* classes = get_class_map(L);
    class_rep* cls = classes ? classes->get(dyn_id) : 0;
    if (!cls && classes)
        cls = classes->get(static_id);
    if (LUABIND_UNLIKELY(!cls))
        throw_unregistered_class();

    object_rep* instance = push_new_instance(L, cls);
    typedef poly_value_holder<T> holder_type;
    void* storage = instance->allocate(sizeof(holder_type), alignof(holder_type));

#ifndef LUABIND_NO_EXCEPTIONS
    try
    {
        holder_type* h = new (storage) holder_type(cls, dyn_id, x);
        instance->set_instance_fast(h, h->raw_ptr(), static_id, false);
    }
    catch (...)
    {
        instance->deallocate(storage);
        lua_pop(L, 1);
        throw;
    }
#else
    holder_type* h = new (storage) holder_type(cls, dyn_id, x);
    instance->set_instance_fast(h, h->raw_ptr(), static_id, false);
#endif
}

template <class T>
inline void make_value_instance(lua_State* L, T const& x)
{
    make_value_instance_aux(L, x, boost::is_polymorphic<T>());
}

}} // namespace luabind::detail

#endif // LUABIND_DETAIL_MAKE_INSTANCE_090310_HPP
