#pragma once
#include <utl/allocator.hpp>
#include <utl/config.hpp>
#include <utl/iterator.hpp>

#include <string>

namespace utl {

template <typename CharType, typename Traits>
struct basic_string_const_iterator;

template <typename CharType, typename Traits>
struct basic_string_iterator
    : iterator_wrapper<CharType *, CharType, CharType &, CharType *, std::random_access_iterator_tag> {
    constexpr basic_string_iterator() noexcept = default;

    constexpr explicit basic_string_iterator(CharType *data) noexcept
        : m_data(data)
    {
    }

    constexpr operator basic_string_const_iterator<CharType, Traits>() const noexcept;

    CharType *m_data;
};

template <typename CharType, typename Traits>
struct basic_string_const_iterator
    : iterator_wrapper<const CharType *, CharType, const CharType &, const CharType *, std::random_access_iterator_tag> {

    constexpr basic_string_const_iterator() noexcept = default;

    constexpr explicit basic_string_const_iterator(const CharType *data) noexcept
        : m_data(data)
    {
    }

    constexpr basic_string_const_iterator(
        basic_string_iterator<CharType, Traits> iter) noexcept
        : m_data(iter.m_data)
    {
    }

    const CharType *m_data;
};

template <typename CharType, typename Traits = char_traits<CharType>, typename Allocator = allocator<CharType>>
class basic_string {
public:
    using value_type = CharType;
    using traits_type = Traits;
    using allocator_type = Allocator;
    using alloc_traits = allocator_traits<Allocator>;
    using size_type = typename alloc_traits::size_type;
    using defference_type = typename alloc_traits::difference_type;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using iterator = basic_string_iterator<value_type, traits_type>;
    using const_iterator = basic_string_const_iterator<value_type, traits_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    basic_string() noexcept
        : m_size(0)
        , m_buffer{0}
    {
    }

    reference operator[](size_type position) noexcept
    {
        if (m_onheap) {
            return this->m_data.data[position];
        }
        return m_buffer[position];
    }

    const_reference operator[](size_type position) const noexcept
    {
        if (m_onheap) {
            return this->m_data.data[position];
        }
        return m_buffer[position];
    }

    reference at(size_type position)
    {
        if (position < m_size)
            return operator[](position);
        UTL_THROW(std::out_of_range("basic_string::at"));
    }
    const_reference at(size_type position) const
    {
        if (position < m_size)
            return operator[](position);
        UTL_THROW(std::out_of_range("basic_string::at"));
    }

    size_type size() const noexcept
    {
        return m_size;
    }

    size_type length() const noexcept
    {
        return m_size;
    }

    pointer data()
    {
        return m_onheap ? m_data.data : m_buffer;
    }

    const_pointer data() const
    {
        return m_onheap ? m_data.data : m_buffer;
    }

private:
    struct HeapData {
        value_type *data;
        size_type cap;
    };

    static constexpr size_type max_buffer_size = 16;

    bool m_onheap = false;
    size_type m_size;

    union {
        value_type m_buffer[max_buffer_size / sizeof(value_type)];
        HeapData m_data;
    };
};

extern template class basic_string<char>;
extern template class basic_string<wchar_t>;
extern template class basic_string<char16_t>;
extern template class basic_string<char32_t>;

} // namespace utl