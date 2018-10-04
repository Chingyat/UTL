#pragma once
#include <utl/config.hpp>

#include <cassert>
#include <iterator>

namespace utl {

using std::iterator_traits;

template <typename It, typename Val, typename Ref, typename Ptr, typename Cat, typename Diff = std::ptrdiff_t>
class iterator_wrapper {
public:
    using value_type = Val;
    using reference = Ref;
    using pointer = Ptr;
    using iterator_category = Cat;
    using difference_type = Diff;

    static_assert(std::is_signed_v<Diff>);

    auto &data() noexcept { return static_cast<It *>(this)->m_data; }

    auto data() const noexcept { return static_cast<const It *>(this)->m_data; }

    reference operator*() const noexcept
    {
        assert(this->data());
        return static_cast<Ref>(*this->data());
    }

    reference operator[](difference_type idx) const noexcept
    {
        return static_cast<Ref>(this->data()[idx]);
    }

    pointer operator->() const noexcept { return static_cast<Ptr>(this->data()); }

    It &operator++() noexcept
    {
        advance();
        return static_cast<It &>(*this);
    }

    It operator++(int) noexcept
    {
        const It retval = static_cast<const It &>(*this);
        advance();
        return retval;
    }

    It &operator--() noexcept
    {
        advance(-1);
        return static_cast<It &>(*this);
    }

    It operator--(int) noexcept
    {
        const It retval = static_cast<const It &>(*this);
        advance(-1);
        return retval;
    }

    friend It operator+(const iterator_wrapper &it, difference_type diff) noexcept
    {
        return It{it.data() + diff};
    }

    friend It operator+(difference_type diff, const iterator_wrapper &it) noexcept
    {
        return It{it.data() + diff};
    }

    friend It operator-(const iterator_wrapper &it, difference_type diff) noexcept
    {
        return It{it.data() - diff};
    }

    It &operator+=(difference_type diff) noexcept
    {
        advance(diff);
        return static_cast<It &>(*this);
    }

    It &operator-=(difference_type diff) noexcept
    {
        advance(-diff);
        return static_cast<It &>(*this);
    }

    friend bool operator==(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() == rhs.data();
    }

    friend bool operator!=(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() != rhs.data();
    }

    friend bool operator<(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() < rhs.data();
    }

    friend bool operator>(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() > rhs.data();
    }

    friend bool operator<=(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() <= rhs.data();
    }

    friend bool operator>=(const iterator_wrapper &lhs, const iterator_wrapper &rhs) noexcept
    {
        return lhs.data() >= rhs.data();
    }

    friend difference_type operator-(const iterator_wrapper &lhs, iterator_wrapper const &rhs) noexcept
    {
        return lhs.data() - rhs.data();
    }

    void advance(difference_type diff = 1) noexcept
    {
        assert(this->data() != nullptr);
        this->data() += diff;
    }
};

template <typename InputIterator>
class move_if_noexcept_iterator;

template <typename InputIterator>
using move_if_noexcept_iterator_base = iterator_wrapper<
    move_if_noexcept_iterator<InputIterator>,
    typename iterator_traits<InputIterator>::value_type,
    std::conditional_t<
        std::is_nothrow_move_constructible_v<typename iterator_traits<InputIterator>::value_type> || !std::is_copy_constructible_v<typename iterator_traits<InputIterator>::value_type>,
        std::add_rvalue_reference_t<typename iterator_traits<InputIterator>::value_type>,
        std::add_lvalue_reference_t<std::add_const_t<typename iterator_traits<InputIterator>::value_type>>>,
    typename iterator_traits<InputIterator>::pointer,
    typename iterator_traits<InputIterator>::iterator_category,
    typename iterator_traits<InputIterator>::difference_type>;

template <typename InputIterator>
class move_if_noexcept_iterator
    : public move_if_noexcept_iterator_base<InputIterator> {
public:
    move_if_noexcept_iterator() noexcept = default;

    explicit move_if_noexcept_iterator(InputIterator it) noexcept
        : m_data(it)
    {
    }

    InputIterator m_data;
};
} // namespace utl
