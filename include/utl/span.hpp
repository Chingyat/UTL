#pragma once

#include <utl/config.hpp>
#include <utl/iterator.hpp>

#include <array>

namespace utl {

#ifdef HAVE_CPP17
inline constexpr std::size_t dynamic_extent = -1;

using std::byte;

using std::data;
using std::size;

#else
constexpr std::size_t dynamic_extent = -1;

using byte = unsigned char;

template <class C>
constexpr auto size(const C &c) -> decltype(c.size())
{
    return c.size();
}

template <class T, std::size_t N>

constexpr std::size_t size(const T (&)[N]) noexcept
{
    return N;
}

template <class C>

constexpr auto data(C &c) -> decltype(c.data())
{
    return c.data();
}

template <class C>
constexpr auto data(const C &c) -> decltype(c.data())
{
    return c.data();
}

template <class T, std::size_t N>
constexpr T *data(T (&array)[N]) noexcept
{
    return array;
}

template <class E>
constexpr const E *data(std::initializer_list<E> il) noexcept
{
    return il.begin();
}
#endif

template <typename T, size_t Extent = dynamic_extent>
class span;

namespace detail {
    template <typename T, std::size_t E>
    struct span_storage {
        typedef T element_type;
        T *p_ {};
        std::size_t dummy_;
        constexpr static std::size_t s_ = E;
    };

    template <typename T>
    struct span_storage<T, dynamic_extent> {
        typedef T element_type;
        T *p_ {};
        std::size_t s_ {};
    };

    template <typename T>
    struct is_span : std::false_type {
    };

    template <typename T, std::size_t E>
    struct is_span<span<T, E>> : std::true_type {
    };

    template <typename T>
    struct is_std_array : std::false_type {
    };

    template <typename T, std::size_t S>
    struct is_std_array<std::array<T, S>> : std::true_type {
    };
} // namespace detail

template <typename T, size_t Extent>
class span {
public:
    typedef T element_type;
    typedef std::remove_cv_t<T> value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;

    struct iterator : iterator_wrapper<iterator, value_type, reference, pointer, std::random_access_iterator_tag, difference_type> {
        using base_type = iterator_wrapper<iterator, value_type, reference, pointer, std::random_access_iterator_tag, difference_type>;

        pointer m_data;

        explicit constexpr iterator(pointer p) noexcept
            : m_data(p)
        {
        }
    };
    struct const_iterator : iterator_wrapper<const_iterator, value_type, const_reference, const_pointer, std::random_access_iterator_tag, difference_type> {
        using base_type = iterator_wrapper<const_iterator, value_type, const_reference, const_pointer, std::random_access_iterator_tag, difference_type>;

        const_pointer m_data;

        explicit constexpr const_iterator(const_pointer p) noexcept
            : m_data(p)
        {
        }
    };

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static constexpr std::size_t extent = Extent;

    template <std::size_t E = Extent, typename = std::enable_if_t<E == -1 || E == 0>>
    constexpr span() noexcept {}

    constexpr span(pointer ptr, size_type count)
        : s_ {ptr, count}
    {
    }

    constexpr span(pointer first, pointer last)
        : span(first, static_cast<std::size_t>(last - first))
    {
    }

    template <std::size_t N>
    constexpr span(element_type (&arr)[N]) noexcept
        : span(arr, N)
    {
    }

    template <std::size_t N, typename = std::enable_if_t<Extent == dynamic_extent || N == Extent>>
    constexpr span(std::array<value_type, N> &arr) noexcept
        : span(arr.data(), arr.size())
    {
    }

    template <std::size_t N>
    constexpr span(const std::array<value_type, N> &arr) noexcept
        : span(arr.data(), arr.size())
    {
    }

    template <class Container, typename = std::enable_if_t<!detail::is_span<Container>::value && !detail::is_std_array<Container>::value && !std::is_array<Container>::value && std::is_convertible<std::remove_pointer_t<decltype(utl::data(std::declval<Container &>()))> (*)[], element_type (*)[]>::value>,
        typename = decltype(utl::data(std::declval<Container &>())),
        typename = decltype(utl::size(std::declval<Container &>()))>
    constexpr span(Container &cont)
        : span(utl::data(cont), utl::size(cont))
    {
    }

    template <class Container, typename = std::enable_if_t<!detail::is_span<Container>::value && !detail::is_std_array<Container>::value && !std::is_array<Container>::value && std::is_convertible<std::remove_pointer_t<decltype(utl::data(std::declval<const Container &>()))> (*)[], element_type (*)[]>::value>,
        typename = decltype(utl::data(std::declval<const Container &>())),
        typename = decltype(utl::size(std::declval<const Container &>()))>
    constexpr span(const Container &cont)
        : span(utl::data(cont), utl::size(cont))
    {
    }

    template <typename U, std::size_t N, typename = std::enable_if_t<(Extent == dynamic_extent || N == Extent) && std::is_convertible<U (*)[], element_type (*)[]>::value>>
    constexpr span(const span<U, N> &s) noexcept
        : span(s.data(), s.size())
    {
    }

    constexpr span(const span &other) noexcept = default;

    constexpr span &operator=(const span &other) noexcept = default;

    constexpr iterator begin() const noexcept
    {
        return iterator {data()};
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator {data()};
    }

    constexpr iterator end() const noexcept
    {
        return iterator {data() + size()};
    }

    constexpr const_iterator cend() const noexcept
    {
        return const_iterator {data() + size()};
    }

    constexpr reverse_iterator rbegin() const noexcept
    {
        return reverse_iterator(begin());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return reverse_iterator(end());
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cend());
    }

    constexpr reference front() const
    {
        return *data();
    }

    constexpr reference back() const
    {
        return *(data() + size() - 1);
    }

    constexpr reference operator[](size_type idx) const
    {
        return *(data() + idx);
    }

    constexpr pointer data() const noexcept
    {
        return s_.p_;
    }

    constexpr size_type size() const noexcept
    {
        return s_.s_;
    }

    constexpr size_type size_bytes() const noexcept
    {
        return size() * sizeof(element_type);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    template <std::size_t Count>
    constexpr span<element_type, Count> first() const
    {
        return span<element_type, Count>(data(), Count);
    }

    constexpr span<element_type, dynamic_extent> first(std::size_t Count) const
    {
        return span<element_type, dynamic_extent>(data(), Count);
    }

    template <std::size_t Count>
    constexpr span<element_type, Count> last() const
    {
        return span<element_type, Count>(data() + size() - Count, Count);
    }

    constexpr span<element_type, dynamic_extent> last(std::size_t Count) const
    {
        return span<element_type, dynamic_extent>(data() + size() - Count, Count);
    }

    template <std::size_t Offset,
        std::size_t Count = dynamic_extent, typename R = span<element_type, std::conditional_t<Count != dynamic_extent, std::integral_constant<std::size_t, Count>, std::conditional_t<Extent != dynamic_extent, std::integral_constant<std::size_t, Extent - Offset>, std::integral_constant<std::size_t, dynamic_extent>>>::value>>
    constexpr R
        subspan() const
    {
        if (Count == dynamic_extent)
            return { data() + Offset, size() - Offset };
        else
            return { data() + Offset, Count };
    }

    constexpr span<element_type, dynamic_extent>
    subspan(std::size_t Offset,
        std::size_t Count = dynamic_extent) const
    {
        if (Count == dynamic_extent)
            return span<element_type, dynamic_extent>(data() + Offset, size() - Offset);
        return span<element_type, dynamic_extent>(data() + Offset, Count);
    }

private:
    detail::span_storage<T, Extent> s_;
};

template <class T, std::size_t N>
span<const byte,
    std::conditional_t<N == dynamic_extent, std::integral_constant<std::size_t, dynamic_extent>,
        std::integral_constant<std::size_t, sizeof(T) * N>>::value>
as_bytes(span<T, N> s) noexcept
{
    return {reinterpret_cast<const byte *>(s.data()), s.size_bytes()};
}

template <class T, std::size_t N>
span<byte, std::conditional_t<N == dynamic_extent, std::integral_constant<std::size_t, dynamic_extent>, std::integral_constant<std::size_t, sizeof(T) * N>>::value> as_writable_bytes(span<T, N> s) noexcept
{
    return { reinterpret_cast<byte*>(s.data()), s.size_bytes() };
}

template <std::size_t I, class T, std::size_t N>
constexpr T &get(span<T, N> s) noexcept
{
    return s[I];
}

} // namespace utl
