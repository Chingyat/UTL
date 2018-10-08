#pragma once

#include <utl/config.hpp>

#include <type_traits>

namespace utl {

enum class nullopt_t { nullopt };
constexpr nullopt_t nullopt = nullopt_t::nullopt;

enum class in_place_t { in_place };
constexpr auto in_place = in_place_t::in_place;

struct bad_optional_access : public std::exception {
    bad_optional_access() = default;

    const char *what() const noexcept final;
};

template <typename T>
constexpr bool is_nothrow_movable_v = std::is_nothrow_move_constructible_v<T> &&std::is_nothrow_move_assignable_v<T>;

template <typename T>
class optional_base {
public:
    using value_type = T;
    using reference = T &;
    using rvalue_reference = T &&;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;

    constexpr bool has_value() const noexcept
    {
        return m_has_value;
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    constexpr const_pointer data() const noexcept
    {
        return reinterpret_cast<const_pointer>(&m_data);
    }

    constexpr pointer data() noexcept
    {
        return reinterpret_cast<pointer>(&m_data);
    }

    constexpr pointer operator->() noexcept
    {
        return data();
    }

    constexpr const_pointer operator->() const noexcept
    {
        return data();
    }

    constexpr reference operator*() & noexcept
    {
        return *data();
    }

    constexpr const_reference operator*() const &noexcept
    {
        return *data();
    }

    constexpr rvalue_reference operator*() && noexcept
    {
        return static_cast<rvalue_reference>(*data());
    }

    constexpr T &&operator*() const &&noexcept
    {
        return static_cast<T &&>(*data());
    }

    constexpr reference value() &
    {
        if (has_value())
            return *data();
        UTL_THROW(bad_optional_access());
    }

    constexpr const_reference value() const &
    {
        if (has_value())
            return *data();
        UTL_THROW(bad_optional_access());
    }

    constexpr rvalue_reference value() &&
    {
        if (has_value())
            return static_cast<rvalue_reference>(*data());
        UTL_THROW(bad_optional_access());
    }

    constexpr const T &&value() const &&
    {
        if (has_value())
            return static_cast<const T &&>(*data());
        UTL_THROW(bad_optional_access());
    }

protected:
    constexpr optional_base() noexcept
        : m_has_value(false)
    {
    }
    constexpr optional_base(nullopt_t) noexcept
        : optional_base()
    {
    }

    constexpr optional_base(const optional_base &other)
        : m_has_value(other.has_value())
    {
        if (has_value()) {
            new (&m_data) T(*other);
        }
    }

    constexpr optional_base(optional_base &&other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_has_value(other.has_value())
    {
        if (has_value())
            new (&m_data) T(std::move(*other));
    }

    ~optional_base()
    {
        if (has_value())
            data()->~T();
    }

    constexpr optional_base(const_reference value)
        : m_has_value(true)
    {
        new (&m_data) T(value);
    }

    constexpr optional_base(rvalue_reference value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_has_value(true)
    {
        new (&m_data) T(std::move(value));
    }

    template <typename... Args>
    constexpr optional_base(in_place_t, Args &&... args)
        : m_has_value(true)
    {
        new (&m_data) T(std::forward<Args>(args)...);
    }

    optional_base &operator=(nullopt_t) noexcept
    {
        if (has_value())
            destroy();
        m_has_value = false;
        return *this;
    }

    optional_base &operator=(const optional_base &rhs)
    {
        if (has_value()) {
            if (rhs.has_value())
                **this = *rhs;
        } else {
            m_has_value = true;
            if (rhs.has_value())
                new (&m_data) T(*rhs);
        }
        return *this;
    }

    optional_base &operator=(optional_base &&rhs) noexcept(is_nothrow_movable_v<T>)
    {
        if (has_value()) {
            if (rhs.has_value())
                **this = std::move(*rhs);
        } else {
            m_has_value = true;
            if (rhs.has_value())
                new (&m_data) T(std::move(*rhs));
        }
        return *this;
    }

    void assign(const_reference value)
    {
        if (has_value())
            **this = value;
        else {
            m_has_value = true;
            new (&m_data) T(value);
        }
    }

    void assign(rvalue_reference value) noexcept(is_nothrow_movable_v<T>)
    {
        if (has_value())
            **this = std::move(value);
        else {
            m_has_value = true;
            new (&m_data) T(std::move(value));
        }
    }

private:
    void destroy() noexcept
    {
        if constexpr (std::is_destructible_v<T>)
            data()->~T();
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> m_data;
    bool m_has_value;
};

template <typename T>
class optional : public optional_base<T> {
    using base = optional_base<T>;

public:
    constexpr optional() noexcept = default;
    constexpr optional(nullopt_t) noexcept
        : base()
    {
    }

    constexpr optional(const optional &other) = default;
    constexpr optional(optional &&other) = default;

    optional &operator=(const optional &rhs) = default;
    optional &operator=(optional &&rhs) = default;

    constexpr optional(const T &value)
        : base(value)
    {
    }
    constexpr optional(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : base(std::move(value))
    {
    }

    template <typename... Args>
    constexpr optional(in_place_t, Args &&... args)
        : base(in_place, std::forward<Args>(args)...)
    {
    }

    optional &operator=(const T &value)
    {
        base::assign(value);
        return *this;
    }

    optional &operator=(T &&value) noexcept(is_nothrow_movable_v<T>)
    {
        base::assign(std::move(value));
        return *this;
    }

    template <typename U>
    constexpr T value_or(U &&default_value) const &
    {
        return base::has_value() ? *base::data() : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    constexpr T value_or(U &&default_value) &&
    {
        return base::has_value() ? *base::data() : static_cast<T>(std::forward<U>(default_value));
    }
};

} // namespace utl
