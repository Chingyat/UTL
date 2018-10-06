#pragma once
#include <utl/config.hpp>

#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

namespace utl {

class bad_cast : public std::exception {
public:
    bad_cast(const std::type_index &from, const std::type_index &to)
        : std::exception()
        , msg(std::string("cannot cast ") + from.name() + " to " + to.name())
    {
    }

    const char *what() const noexcept final;

    std::string msg;
};

class any {

    struct Value {

        virtual std::unique_ptr<Value> clone() const = 0;

        virtual std::type_index type() const noexcept = 0;

        virtual ~Value() = default;

        virtual void *get(std::type_index const &t) const noexcept = 0;

        virtual void dummy_vfunc();
    };

    template <class T>
    struct ValueImpl : Value {
        T mutable m_data;

        template <class... Args>
        ValueImpl(Args &&... args)
            : m_data(std::forward<Args>(args)...)
        {
        }

        std::unique_ptr<Value> clone() const final
        {
            return std::make_unique<ValueImpl>(m_data);
        }

        std::type_index type() const noexcept final
        {
            return typeid(T);
        }

        void *get(std::type_index const &t) const noexcept final
        {
            if (t == type()) {
                return &m_data;
            }
            return nullptr;
        }
    };

public:
    template <class T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any>>>
    any(T &&x)
        : m_value(new ValueImpl<std::decay_t<T>>(std::forward<T>(x)))
    {
    }

    template <class T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any>>>
    any &operator=(T &&x)
    {
        m_value = std::make_unique<ValueImpl<std::decay_t<T>>>(std::forward<T>(x));
        return *this;
    }

    any(const any &other)
        : m_value(other.m_value->clone())
    {
    }

    any(any &&other) noexcept = default;

    any &operator=(const any &rhs)
    {
        m_value = rhs.m_value->clone();
        return *this;
    }

    any &operator=(any &&rhs) noexcept = default;

    template <class T>
    T *get() noexcept
    {
        auto *p = m_value->get(typeid(T));
        return reinterpret_cast<T *>(p);
    }

    template <class T>
    const T *get() const noexcept
    {
        auto *p = m_value->get(typeid(T));
        return reinterpret_cast<T *>(p);
    }

    std::type_index type() const noexcept
    {
        return m_value->type();
    }

private:
    std::unique_ptr<Value> m_value;
};

template <class Tp, bool IsPtr = std::is_pointer_v<Tp>>
Tp any_cast(const any &a) noexcept(IsPtr)
{
    using T = std::remove_pointer_t<Tp>;
    const auto p = a.template get<T>();

    if constexpr (IsPtr)
        return p;
    else if (p)
        return *p;
    else
        UTL_THROW(bad_cast(a.type(), typeid(Tp)));
}

template <class Tp, typename = std::enable_if_t<std::is_pointer_v<Tp>>>
Tp any_cast(any &a) noexcept
{
    return a.template get<std::remove_pointer_t<Tp>>();
}

} // namespace utl
