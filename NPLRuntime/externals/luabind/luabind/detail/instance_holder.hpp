// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_INSTANCE_HOLDER_081024_HPP
# define LUABIND_INSTANCE_HOLDER_081024_HPP

# include <luabind/detail/inheritance.hpp>
# include <luabind/get_pointer.hpp>
# include <luabind/typeid.hpp>
# include <boost/type_traits/is_polymorphic.hpp>
# include <stdexcept>
# include <type_traits>
# include <utility>

namespace luabind { namespace detail {

class class_rep;

class instance_holder
{
public:
    instance_holder(class_rep* cls, bool pointee_const) noexcept
      : m_cls(cls)
      , m_pointee_const(pointee_const)
      , m_released(false)
    {}

    virtual ~instance_holder()
    {}

    virtual std::pair<void*, int> get(class_id target) const = 0;

    virtual void release() = 0;

    class_rep* get_class() const noexcept
    {
        return m_cls;
    }

    bool pointee_const() const noexcept
    {
        return m_pointee_const;
    }

protected:
    class_rep* m_cls;
    bool m_pointee_const;
    bool m_released;
};

namespace mpl = boost::mpl;

inline bool check_const_pointer_bool(void*) noexcept
{
    return false;
}

inline bool check_const_pointer_bool(void const*) noexcept
{
    return true;
}

template <class T>
void release_ownership(std::movable_auto_ptr<T>& p)
{
    p.release();
}

template <class P>
void release_ownership(P const&)
{
    throw std::runtime_error(
        "luabind: smart pointer does not allow ownership transfer");
}

template <class P>
struct is_releasable_auto_ptr : std::false_type
{};

template <class T>
struct is_releasable_auto_ptr<std::movable_auto_ptr<T> > : std::true_type
{};

template <class T>
inline void* compute_dynamic_ptr_aux(T const* p, mpl::true_) noexcept
{
    return p ? dynamic_cast<void*>(const_cast<T*>(p)) : 0;
}

template <class T>
inline void* compute_dynamic_ptr_aux(T const* p, mpl::false_) noexcept
{
    return const_cast<void*>(static_cast<void const*>(p));
}

template <class T>
inline void* compute_dynamic_ptr(T* p) noexcept
{
    return compute_dynamic_ptr_aux(p, boost::is_polymorphic<T>());
}

template <class T>
inline class_id static_class_id(T*)
{
    return registered_class<T>::id;
}

LUABIND_API std::pair<void*, int> cast_class_ptr(
    class_rep const* cls
  , void* naked_ptr
  , class_id src
  , class_id target
  , class_id dynamic_id
  , void const* dynamic_ptr
);

template <class P, class Pointee = void const>
class LUABIND_DETAIL_HIDDEN pointer_holder : public instance_holder
{
public:
    pointer_holder(
        P p_, class_id dynamic_id_, void* dynamic_ptr_, class_rep* cls
      , void const* initial_raw = 0
    )
      : instance_holder(cls, check_const_pointer_bool(false ? get_pointer(p_) : 0))
      , p(static_cast<P&&>(p_))
      , weak(0)
      , dynamic_id(dynamic_id_)
      , dynamic_ptr(dynamic_ptr_)
    {
        auto final_raw = get_pointer(this->p);
        if (initial_raw != 0 && static_cast<void const*>(final_raw) != initial_raw)
            dynamic_ptr = compute_dynamic_ptr(final_raw);
    }

    void* naked_ptr() const
    {
        return const_cast<void*>(static_cast<void const*>(
            weak ? weak : get_pointer(this->p)));
    }

    std::pair<void*, int> get(class_id target) const override
    {
        if (target == registered_class<P>::id)
            return std::pair<void*, int>(&this->p, 0);

        void* raw = naked_ptr();

        if (!raw)
            return std::pair<void*, int>((void*)0, 0);

        class_id const src_id = static_class_id(false ? get_pointer(this->p) : 0);
        if (LUABIND_LIKELY(target == src_id))
            return std::pair<void*, int>(raw, 0);

        return cast_class_ptr(
            get_class()
          , raw
          , src_id
          , target
          , dynamic_id
          , dynamic_ptr
        );
    }

    void release() override
    {
        weak = const_cast<void*>(static_cast<void const*>(
            get_pointer(this->p)));
        release_ownership(this->p);
    }

private:
    mutable P p;
    void* weak;
    class_id dynamic_id;
    void* dynamic_ptr;
};

// Compact 24+sizeof(P) holder for non-polymorphic pointers and stack-object wrappers
// (such as ParaScripting::get_pointer(T const&), shared_ptr<NonPoly>, NonPoly*).
// Fits types up to 32B inline in object_rep::m_instance_buffer (56B) with 0 mallocs,
// and always resolves naked_ptr() from this->p in final storage rather than a stack arg.
template <class P>
class LUABIND_DETAIL_HIDDEN compact_pointer_holder : public instance_holder
{
public:
    compact_pointer_holder(P p_, class_rep* cls)
      : instance_holder(cls, check_const_pointer_bool(false ? get_pointer(p_) : 0))
      , p(static_cast<P&&>(p_))
    {}

    void* naked_ptr() const
    {
        return const_cast<void*>(static_cast<void const*>(get_pointer(this->p)));
    }

    std::pair<void*, int> get(class_id target) const override
    {
        if (target == registered_class<P>::id)
            return std::pair<void*, int>(&this->p, 0);

        void* raw = naked_ptr();
        if (!raw)
            return std::pair<void*, int>((void*)0, 0);

        class_id const src_id = static_class_id(false ? get_pointer(this->p) : 0);
        if (LUABIND_LIKELY(target == src_id))
            return std::pair<void*, int>(raw, 0);

        return cast_class_ptr(
            get_class()
          , raw
          , src_id
          , target
          , src_id
          , raw
        );
    }

    void release() override
    {
        release_ownership(this->p);
    }

private:
    mutable P p;
};

// Zero-malloc inline value holder for non-polymorphic C++ value types
// (e.g. ParaObject, ParaUIObject, ParaAttributeObject, ParaCharacter, ParaAssetObject).
// Stores T directly inside object_rep::m_instance_buffer with 0 heap allocations!
template <class T>
class LUABIND_DETAIL_HIDDEN compact_value_holder : public instance_holder
{
public:
    template <class... Args>
    explicit compact_value_holder(class_rep* cls, Args&&... args)
        noexcept(std::is_nothrow_constructible<T, Args&&...>::value)
      : instance_holder(cls, false)
      , m_value(std::forward<Args>(args)...)
    {}

    T* raw_ptr() noexcept
    {
        return &m_value;
    }

    std::pair<void*, int> get(class_id target) const override
    {
        class_id const src_id = registered_class<T>::id;
        void* ptr = const_cast<T*>(&m_value);
        if (LUABIND_LIKELY(target == src_id))
            return std::pair<void*, int>(ptr, 0);
        if (target == registered_class<std::movable_auto_ptr<T> >::id)
            return std::pair<void*, int>((void*)0, -1);

        return cast_class_ptr(
            get_class()
          , ptr
          , src_id
          , target
          , src_id
          , ptr
        );
    }

    void release() override
    {
        m_released = true;
    }

private:
    mutable T m_value;
};

// Zero-malloc inline value holder for polymorphic C++ value types.
template <class T>
class LUABIND_DETAIL_HIDDEN poly_value_holder : public instance_holder
{
public:
    template <class... Args>
    explicit poly_value_holder(class_rep* cls, class_id dyn_id, Args&&... args)
      : instance_holder(cls, false)
      , m_dynamic_id(dyn_id)
      , m_value(std::forward<Args>(args)...)
    {}

    T* raw_ptr() noexcept
    {
        return &m_value;
    }

    std::pair<void*, int> get(class_id target) const override
    {
        class_id const src_id = registered_class<T>::id;
        void* ptr = const_cast<T*>(&m_value);
        if (LUABIND_LIKELY(target == src_id))
            return std::pair<void*, int>(ptr, 0);

        void* dyn_ptr = dynamic_cast<void*>(const_cast<T*>(&m_value));
        return cast_class_ptr(
            get_class()
          , ptr
          , src_id
          , target
          , m_dynamic_id
          , dyn_ptr
        );
    }

    void release() override
    {
        m_released = true;
    }

private:
    class_id m_dynamic_id;
    mutable T m_value;
};

}} // namespace luabind::detail

#endif // LUABIND_INSTANCE_HOLDER_081024_HPP
