#pragma once

#include <utl/config.hpp>
#include <utl/iterator.hpp>

namespace utl {

inline constexpr std::ptrdiff_t dynamic_extent = -1;

template <typename Span>
struct span_iterator;

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
    return {m_data};
};

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
};

template <typename T, std::ptrdiff_t Extent = dynamic_extent>
class span : public span_base<span<T, Extent>, T> {
public:
};

template <typename T>
class span<T, dynamic_extent> {
};
} // namespace utl
