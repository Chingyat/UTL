#pragma once

#include <utl/config.hpp>
#include <utl/iterator.hpp>

#include <array>

namespace utl {

inline constexpr std::ptrdiff_t dynamic_extent = -1;

template <typename T, std::ptrdiff_t Extent = dynamic_extent>
class span;

template <typename Span, typename Container>
inline constexpr bool span_can_construct_from_v = std::is_convertible_v<
    std::remove_pointer_t<decltype(std::data(std::declval<Container &>()))> (*)[],
    typename Span::element_type (*)[]>;

template <typename Span>
struct span_const_iterator;

template <typename Span>
struct span_iterator
    : iterator_wrapper<
          span_iterator<Span>,
          typename Span::value_type,
          typename Span::reference,
          typename Span::pointer,
          std::random_access_iterator_tag> {
    typename Span::pointer m_data;

    constexpr span_iterator() = default;

    constexpr explicit span_iterator(typename Span::pointer data) noexcept
        : m_data(data)
    {
    }

    constexpr operator span_const_iterator<Span>() const noexcept;
};

template <typename Span>
struct span_const_iterator
    : iterator_wrapper<
          span_const_iterator<Span>,
          typename Span::value_type,
          typename Span::const_reference,
          typename Span::const_pointer,
          std::random_access_iterator_tag> {
    typename Span::pointer m_data;

    constexpr span_const_iterator() = default;

    constexpr explicit span_const_iterator(typename Span::pointer data) noexcept
        : m_data(data)
    {
    }

    constexpr span_const_iterator(const span_iterator<Span> it) noexcept
        : m_data(it.m_data)
    {
    }
};

template <typename Span>
constexpr span_iterator<Span>::operator span_const_iterator<Span>() const noexcept
{
    return span_const_iterator<Span>{m_data};
}

template <typename Span, typename T>
struct span_base {
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = span_iterator<Span>;
    using const_iterator = span_const_iterator<Span>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr pointer data() const noexcept { return static_cast<const Span *>(this)->m_data; }
    constexpr index_type size() const noexcept { return static_cast<const Span *>(this)->m_size; }
    constexpr index_type size_bytes() const noexcept { return size() * sizeof(element_type); }

    constexpr iterator begin() const noexcept { return iterator{data()}; }
    constexpr const_iterator cbegin() const noexcept { return const_iterator{data()}; }
    constexpr iterator end() const noexcept { return iterator{data() + size()}; }
    constexpr const_iterator cend() const noexcept { return const_iterator{data() + size()}; }
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }
    constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }
    constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

    constexpr reference operator[](index_type idx) const { return data()[idx]; }
    constexpr reference operator()(index_type idx) const { return data()[idx]; }

    constexpr bool empty() const noexcept { return size() == 0; }
};

template <typename Span, typename T>
struct dynamic_span : public span_base<Span, T> {
private:
    using base_type = span_base<Span, T>;

public:
    typename span_base<Span, T>::pointer m_data;
    typename span_base<Span, T>::index_type m_size;

    static constexpr std::ptrdiff_t extent = dynamic_extent;

    constexpr dynamic_span(typename base_type::pointer ptr, typename base_type::index_type count)
        : m_data(ptr)
        , m_size(count)
    {
    }
    constexpr dynamic_span(typename base_type::pointer first, typename base_type::pointer last)
        : dynamic_span(first, last - first)
    {
    }

    template <std::size_t N>
    constexpr dynamic_span(typename base_type::element_type (&arr)[N], std::enable_if_t<span_can_construct_from_v<dynamic_span, typename base_type::element_type[N]>> * = nullptr) noexcept
        : m_data(arr)
        , m_size(N)
    {
    }

    template <std::size_t N>
    constexpr dynamic_span(std::array<typename base_type::value_type, N> &arr, std::enable_if_t<span_can_construct_from_v<dynamic_span, std::array<typename base_type::value_type, N>>> * = nullptr) noexcept
        : m_data(arr.data())
        , m_size(arr.size())
    {
    }

    template <std::size_t N>
    constexpr dynamic_span(const std::array<typename base_type::value_type, N> &arr, std::enable_if_t<span_can_construct_from_v<dynamic_span, std::array<typename base_type::value_type, N>>> * = nullptr) noexcept
        : m_data(arr.data())
        , m_size(arr.size())
    {
    }

    template <typename Container>
    constexpr dynamic_span(Container &container, std::enable_if_t<span_can_construct_from_v<dynamic_span, Container>> * = nullptr)
        : dynamic_span(std::data(container), std::size(container))
    {
    }

    template <typename Container>
    constexpr dynamic_span(const Container &container, std::enable_if_t<span_can_construct_from_v<dynamic_span, const Container>> * = nullptr)
        : dynamic_span(std::data(container), std::size(container))
    {
    }

    template <typename U, std::ptrdiff_t N>
    constexpr dynamic_span(const span<U, N> &container, std::enable_if_t<span_can_construct_from_v<dynamic_span, span<U, N>>> * = nullptr) noexcept
        : dynamic_span(std::data(container), std::data(container) + std::size(container))
    {
    }

    constexpr dynamic_span(const dynamic_span &other) noexcept = default;
};

template <typename Span, typename T, std::ptrdiff_t Extent>
struct static_span : public span_base<Span, T> {
    typename span_base<Span, T>::pointer m_data;
    static constexpr typename span_base<Span, T>::index_type m_size = Extent;
};

template <typename T, std::ptrdiff_t Extent>
class span : public std::conditional_t<
                 Extent == dynamic_extent,
                 dynamic_span<span<T, dynamic_extent>, T>,
                 static_span<span<T, Extent>, T, Extent>> {
    using base_type = std::conditional_t<
        Extent == dynamic_extent,
        dynamic_span<span<T, dynamic_extent>, T>,
        static_span<span<T, Extent>, T, Extent>>;

public:
    using base_type::base_type;
};

} // namespace utl
