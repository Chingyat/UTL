#pragma once
#include <utl/algorithm.hpp>
#include <utl/allocator.hpp>
#include <utl/config.hpp>
#include <utl/iterator.hpp>

#include <cassert>
#include <cstring>
#include <iterator>
#include <memory>

namespace utl {

template <typename Tp>
class vector_const_iterator;

template <typename Tp>
class vector_iterator
    : public iterator_wrapper<vector_iterator<Tp>, Tp, Tp &, Tp *, std::random_access_iterator_tag> {
public:
    constexpr vector_iterator() noexcept = default;

    explicit vector_iterator(Tp *data) noexcept
        : m_data(data)
    {
    }

    constexpr operator vector_const_iterator<Tp>() const noexcept;

    Tp *m_data;
};

template <typename Tp>
class vector_const_iterator
    : public iterator_wrapper<vector_const_iterator<Tp>, Tp, const Tp &, const Tp *, std::random_access_iterator_tag> {
public:
    constexpr vector_const_iterator() noexcept = default;

    explicit vector_const_iterator(const Tp *data) noexcept
        : m_data(const_cast<Tp *>(data))
    {
    }

    constexpr explicit vector_const_iterator(vector_iterator<Tp> it) noexcept
        : vector_const_iterator(it.m_data)
    {
    }

    Tp *m_data;
};

template <typename Tp>
constexpr vector_iterator<Tp>::operator vector_const_iterator<Tp>() const noexcept
{
    return vector_const_iterator<Tp>{ *this };
}

template <typename Tp, typename Allocator = allocator<Tp>>
class vector {
public:
    // types:
    using value_type = Tp;
    using allocator_type = Allocator;
    using alloc_traits = allocator_traits<allocator_type>;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using reference = value_type &;
    using rvalue_reference = value_type &&;
    using const_reference = const value_type &;
    using size_type = size_t; // see 26.2
    using difference_type = std::ptrdiff_t; // see 26.2
    using iterator = vector_iterator<value_type>; // see 26.2
    using const_iterator = vector_const_iterator<value_type>; // see 26.2
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    static void destroy(pointer data, size_type count, allocator_type &allocator) noexcept
    {
        if (!std::is_trivially_destructible_v<value_type> && !std::is_fundamental_v<value_type>)

            for (size_type i = 0; i != count; ++i)
                alloc_traits::destroy(allocator, &data[i]);
    }

    static void destroy_and_dealloc(pointer data, size_type count, size_type cap, allocator_type &allocator) noexcept
    {
        destroy(data, count, allocator);
        alloc_traits::deallocate(allocator, data, cap);
    }

    static void construct(pointer data, size_type count, std::tuple<>, allocator_type &allocator)
    {
        if constexpr (std::is_trivially_default_constructible_v<value_type>)
            return;
        if constexpr (std::is_fundamental_v<value_type>) {
            std::memset(data, 0, count * sizeof(value_type));
            return;
        }
        size_type i = 0;
        UTL_TRY
        {
            while (i < count) {
                alloc_traits::construct(allocator, data + i);
                ++i;
            }
        }
        UTL_CATCH(...)
        {
            destroy(data, i, allocator);
            UTL_RETHROW;
        }
    }

    template <typename Arg>
    static auto construct(pointer data, size_type count, Arg &&arg, allocator_type &allocator)
        -> std::void_t<decltype(std::tuple_size<std::decay_t<Arg>>::value)>
    {
        size_type i = 0;
        UTL_TRY
        {
            while (i < count) {
                std::apply([&allocator, ptr = data + i](auto &&... args) {
                    alloc_traits::construct(allocator, ptr, std::forward<decltype(args)>(args)...);
                },
                    std::forward<Arg>(arg));
                ++i;
            }
        }
        UTL_CATCH(...)
        {
            destroy(data, i, allocator);
            UTL_RETHROW;
        }
    }

    template <typename InputIterator, typename = std::enable_if_t<std::is_same_v<std::remove_cv_t<typename std::iterator_traits<InputIterator>::value_type>, value_type>>>
    static auto construct(pointer data, size_type count, InputIterator it, allocator_type &allocator)
    {
        size_type i = 0;
        UTL_TRY
        {
            while (i < count) {
                alloc_traits::construct(allocator, data + i, *it++);
                ++i;
            }
        }
        UTL_CATCH(...)
        {
            destroy(data, i, allocator);
            UTL_RETHROW;
        }
    }

    template <typename ValTp = value_type>
    static auto construct(pointer data, size_type count, move_if_noexcept_iterator<pointer> it, allocator_type &) noexcept
        -> std::enable_if_t<std::is_trivially_copyable_v<ValTp>>
    {
        std::memmove(data, it.data(), count * sizeof(value_type));
    }

    template <typename Arg>
    static pointer alloc_and_construct(size_type count, size_type cap, Arg &&arg, allocator_type &allocator, pointer hint = nullptr)
    {
        pointer const data = alloc_traits::allocate(allocator, cap, hint);
        UTL_TRY
        {
            construct(data, count, arg, allocator);
            return data;
        }
        UTL_CATCH(...)
        {
            alloc_traits::deallocate(allocator, data, cap);
            UTL_RETHROW;
        }
    }

    template <typename Arg>
    static pointer alloc_and_construct(size_type count, Arg &&arg, allocator_type &allocator, pointer hint = nullptr)
    {
        return alloc_and_construct(count, count, std::forward<Arg>(arg), allocator, hint);
    }

    template <typename... Args>
    static decltype(auto) forward_args(Args &&... args) noexcept
    {
        return std::tuple<Args &&...>(std::forward<Args>(args)...);
    }

    static void realloc(pointer &data, size_type count, size_type &cap, size_type new_cap, allocator_type &allocator)
    {
        assert((!data && !count) || (data && cap));

        if constexpr (std::is_trivially_copyable_v<value_type>) {
            const auto new_data = alloc_traits::allocate(allocator, new_cap);
            if (data)
                std::memcpy(new_data, data, count * sizeof(value_type));
            alloc_traits::deallocate(allocator, data, cap);
            data = new_data;
            cap = new_cap;
        } else {
            pointer const new_data = alloc_and_construct(count, new_cap, move_if_noexcept_iterator(data), allocator, data);
            destroy_and_dealloc(data, count, cap, allocator);
            data = new_data;
            cap = new_cap;
        }
    }

public:
    // 26.3.11.2, construct/copy/destroy
    vector() noexcept(noexcept(Allocator()))
        : vector(Allocator())
    {
    }

    explicit vector(const allocator_type &allocator) noexcept
        : m_alloc(allocator)
        , m_data(nullptr)
        , m_cap(0)
        , m_size(0)
    {
    }

    explicit vector(size_type num, const allocator_type &allocator = allocator_type())
        : m_alloc(allocator)
        , m_data(alloc_and_construct(num, forward_args(), m_alloc))
        , m_cap(num + 1)
        , m_size(num)
    {
    }

    vector(size_type num, const_reference val, const allocator_type &allocator = allocator_type())
        : m_alloc(allocator)
        , m_data(alloc_and_construct(num, forward_args(val), m_alloc))
        , m_cap(num)
        , m_size(num)
    {
    }

    template <typename InputIterator>
    vector(InputIterator first, InputIterator last, const allocator_type &allocator = allocator_type())
        : m_alloc(allocator)
    {
        if constexpr (std::is_same_v<typename iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
            m_cap = 0;
            m_size = 0;
            m_data = nullptr;
            UTL_TRY
            {
                while (first != last)
                    emplace_back(*first++);
            }
            UTL_CATCH(...)
            {
                destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);
            }
        } else {
            const size_type num = std::distance(first, last);
            assert(num >= 0);
            m_data = alloc_and_construct(num, first, m_alloc);
            m_cap = num;
            m_size = num;
        }
    }

    vector(const vector &other)
        : vector(other, alloc_traits::select_on_container_copy_construction(other.m_alloc))
    {
    }

    vector(vector &&other) noexcept
        : vector(std::move(other), other.get_allocator())
    {
    }

    vector(const vector &other, const allocator_type &allocator)
        : m_alloc(allocator)
        , m_data(alloc_and_construct(other.m_size, other.m_data, m_alloc))
        , m_cap(other.m_size)
        , m_size(other.m_size)
    {
    }

    vector(vector &&other, const allocator_type &allocator)
        : m_alloc(allocator)
    {
        if (alloc_traits::is_always_equal::value || m_alloc == other.get_allocator()) {
            m_data = other.m_data;
            m_cap = other.m_cap;
            m_size = other.m_size;
        } else {
            m_data = alloc_and_construct(other.m_size, other.m_data, m_alloc);
            m_size = other.m_size;
            m_cap = m_size;
        }
        other.m_data = nullptr;
        other.m_cap = 0;
        other.m_size = 0;
    }

    vector(initializer_list<value_type> il, const allocator_type &allocator = Allocator())
        : vector(il.begin(), il.end(), allocator)
    {
    }

    ~vector() { destroy_and_dealloc(m_data, m_size, m_cap, m_alloc); }

    vector &operator=(const vector &other)
    {
        constexpr bool copy_allocator = alloc_traits::propagate_on_container_copy_assignment::value;
        const bool reallocate = m_cap < other.m_size || (copy_allocator && m_alloc != other.get_allocator());

        if (reallocate) {
            destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);
            m_cap = 0;
            m_size = 0;
        }

        if constexpr (copy_allocator)
            m_alloc = other.get_allocator();

        if (reallocate) {
            m_data = alloc_traits::allocate(m_alloc, other.m_size, other.m_data);

            m_cap = other.m_size;
            m_size = 0;
        }

        const size_type common = utl::min(m_size, other.m_size);

        // CopyAssignable
        for (size_type i = 0; i != common; ++i)
            m_data[i] = other.m_data[i];

        // Erasable
        if (m_size > common) {
            destroy(m_data + common, m_size - common, m_alloc);
            m_size = common;
        }

        // CopyInsertible
        if (m_size < other.m_size) {
            construct(m_data + m_size, other.m_size - m_size, other.m_data + m_size, m_alloc);
            m_size = other.m_size;
        }

        return *this;
    }

    vector &operator=(vector &&other) noexcept(
        alloc_traits::propagate_on_container_move_assignment::value || alloc_traits::is_always_equal::value)
    {
        constexpr bool move_allocator = alloc_traits::propagate_on_container_move_assignment::value;
        const bool can_take_ownership = move_allocator || m_alloc == other.get_allocator();
        const bool deallocate = can_take_ownership || m_cap < other.m_size;

        if (deallocate) {
            destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);
            m_data = nullptr;
            m_cap = 0;
            m_size = 0;
        }

        if constexpr (move_allocator) {
            m_alloc = std::move(other.m_alloc);
        }

        if (can_take_ownership) {
            using std::swap;
            swap(m_data, other.m_data);
            swap(m_cap, other.m_cap);
            swap(m_size, other.m_size);
            return *this;
        }

        if (m_cap < other.m_size) {
            m_data = alloc_traits::allocate(m_alloc, other.m_size, other.m_data);
            m_cap = other.m_size;
        }

        const size_type common = utl::min(m_size, other.m_size);

        // MoveAssignable
        for (size_type i = 0; i != common; ++i)
            m_data[i] = std::move_if_noexcept(other.m_data[i]);

        // Erasable
        if (m_size > common) {
            destroy(m_data + common, m_size - common, m_alloc);
            m_size = common;
        }

        // MoveInsertible
        if (m_size < other.m_size) {
            construct(m_data + m_size, other.m_size - m_size, move_if_noexcept_iterator(other.m_data + m_size), m_alloc);
            m_size = other.m_size;
        }

        return *this;
    }

    vector &operator=(initializer_list<value_type> il)
    {
        assign(il.begin(), il.end());
        return *this;
    }

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        if constexpr (std::is_same_v<typename iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
            size_type i;
            for (i = 0; first != last && i < m_size; ++i, ++first)
                m_data[i] = *first;

            if (i != m_size)
                resize(i);

            while (first != last)
                emplace_back(*first++);

        } else {
            const size_type count = std::distance(first, last);

            if (m_cap < count) {
                destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);
                m_data = alloc_traits::allocate(m_alloc, count);
                m_cap = count;
                m_size = 0;
            }

            const size_type common = utl::min(m_size, count);

            // CopyAssignable
            for (size_type i = 0; i != common; ++i)
                m_data[i] = *first++;

            // Erasable
            if (m_size > common) {
                destroy(m_data + common, m_size - common, m_alloc);
                m_size = common;
            }

            // CopyInsertible
            if (m_size < count) {
                construct(m_data + m_size, count - m_size, first + m_size, m_alloc);
                m_size = count;
            }
        }
    }

    void assign(size_type num, const_reference val)
    {
        if (m_cap < num) {
            destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);
            m_data = alloc_traits::allocate(m_alloc, num, m_data);
            m_cap = num;
            m_size = 0;
        }

        const size_type common = utl::min(m_size, num);

        // CopyAssignable
        for (size_type i = 0; i != common; ++i)
            m_data[i] = val;

        // Erasable
        if (m_size > common) {
            destroy(m_data + common, m_size - common, m_alloc);
            m_size = common;
        }

        // CopyInsertible
        if (m_size < num) {
            construct(m_data + m_size, num - m_size, forward_args(val), m_alloc);
            m_size = num;
        }
    }

    void assign(initializer_list<value_type> il) { assign(il.begin(), il.end()); }

    allocator_type get_allocator() const noexcept { return m_alloc; }

    // iterators:
    iterator begin() noexcept { return iterator{ m_data }; }
    const_iterator begin() const noexcept { return const_iterator{ m_data }; }
    iterator end() noexcept { return iterator{ m_data + m_size }; }
    const_iterator end() const noexcept { return const_iterator{ m_data + m_size }; }
    reverse_iterator rbegin() noexcept
    {
        return std::make_reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept
    {
        return std::make_reverse_iterator(end());
    }
    reverse_iterator rend() noexcept { return std::make_reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept
    {
        return std::make_reverse_iterator(begin());
    }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }
    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    // 26.3.11.3, capacity
    bool empty() const noexcept { return m_size == 0; }
    size_type size() const noexcept { return m_size; }
    size_type max_size() const noexcept
    {
        return alloc_traits::max_size(m_alloc);
    }

    size_type capacity() const noexcept { return m_cap; }

    void resize(size_type size) { resize_impl(size, std::tuple<>()); }

    void resize(size_type size, const_reference val) { resize_impl(size, forward_args(val)); }

    template <typename Arg>
    void resize_impl(size_type size, Arg &&arg)
    {
        reserve(size);
        if (m_size < size)
            construct(m_data + m_size, size - m_size, std::forward<Arg>(arg), m_alloc);
        if (m_size > size)
            destroy(m_data + size, m_size - size, m_alloc);
        m_size = size;
    }

    void reserve(size_type num)
    {
        if (num > m_cap) {
            auto new_cap = utl::max(m_cap * 2, num);
            realloc(m_data, m_size, m_cap, new_cap, m_alloc);
        }
    }

    void shrink_to_fit()
    {
        if (m_cap != m_size) {
            realloc(m_data, m_size, m_cap, m_size, m_alloc);
        }
    }

    // element access:
    reference operator[](size_type num) { return m_data[num]; }
    const_reference operator[](size_type num) const { return m_data[num]; }
    const_reference at(size_type num) const
    {
        if (num < m_size)
            return m_data[num];
        UTL_THROW(std::out_of_range("vector::at"));
    }
    reference at(size_type num)
    {
        if (num < m_size)
            return m_data[num];
        UTL_THROW(std::out_of_range("vector::at"));
    }
    reference front() { return m_data[0]; }
    const_reference front() const { return m_data[0]; }
    reference back() { return m_data[m_size - 1]; }
    const_reference back() const { return m_data[m_size - 1]; }

    // 26.3.11.4, data access
    pointer data() noexcept { return m_data; }
    const_pointer data() const noexcept { return m_data; }

    // 26.3.11.5, modifiers
    template <typename... Args>
    reference emplace_back(Args &&... args)
    {
        reserve(m_size + 1);
        alloc_traits::construct(m_alloc, &m_data[m_size], std::forward<Args>(args)...);
        ++m_size;
        return back();
    }

    void push_back(const_reference elem)
    {
        reserve(m_size + 1);
        alloc_traits::construct(m_alloc, &m_data[m_size], elem);
        ++m_size;
    }

    void push_back(rvalue_reference elem)
    {
        reserve(m_size + 1);
        alloc_traits::construct(m_alloc, &m_data[m_size], std::move(elem));
        ++m_size;
    }

    void pop_back() // noexcept
    {
        alloc_traits::destroy(m_alloc, &m_data[--m_size]);
    }

    template <typename... Args>
    iterator emplace(const_iterator position, Args &&... args)
    {
        auto ptr = insert_impl(position - begin(), 1, forward_args(std::forward<Args>(args)...));
        return iterator{ ptr };
    }

    template <typename Arg>
    pointer insert_impl(size_type idx, size_type count, Arg &&arg)
    {
        if (m_size + count > m_cap) {
            const auto new_cap = utl::max(m_size + count, m_size * 2);
            const pointer new_data = alloc_and_construct(idx, new_cap, move_if_noexcept_iterator(m_data), m_alloc, m_data);

            UTL_TRY
            {
                construct(new_data + idx, count, std::forward<Arg>(arg), m_alloc);
            }
            UTL_CATCH(...)
            {
                destroy_and_dealloc(new_data, idx, new_cap, m_alloc);
                UTL_RETHROW;
            }

            UTL_TRY
            {
                construct(new_data + idx + count, m_size - idx, move_if_noexcept_iterator(m_data + idx), m_alloc);
            }
            UTL_CATCH(...)
            {
                destroy_and_dealloc(new_data, idx + count, new_cap, m_alloc);
                UTL_RETHROW;
            }

            destroy_and_dealloc(m_data, m_size, m_cap, m_alloc);

            m_data = new_data;
            m_cap = new_cap;
            m_size = m_size + count;
            return m_data + idx;
        } else {
            const size_type num = utl::min(m_size - idx, count);
            const auto size = m_size;
            construct(m_data + size + count - num, num, move_if_noexcept_iterator(m_data + size - num), m_alloc);
            m_size += count;
            utl::copy_backward(move_if_noexcept_iterator(m_data + idx), move_if_noexcept_iterator(m_data + size - num), m_data + size + count - num);
            destroy(m_data + idx, num, m_alloc); // destruction might be unneeded
            UTL_TRY
            {
                construct(m_data + idx, count, std::forward<Arg>(arg), m_alloc);
            }
            UTL_CATCH(...)
            {
                utl::copy(move_if_noexcept_iterator(m_data + size - num), move_if_noexcept_iterator(m_data + idx), m_data + size + count - num);
                destroy(m_data + size + count - num, num, m_alloc);
                m_size -= count;
                UTL_RETHROW;
            }
            return m_data + idx;
        }
    }

    iterator insert(const_iterator position, const_reference elem)
    {
        auto *ptr = insert_impl(position - begin(), 1, forward_args(elem));
        return iterator{ ptr };
    }

    iterator insert(const_iterator position, rvalue_reference elem)
    {
        auto *ptr = insert_impl(position - begin(), 1, forward_args(std::move(elem)));
        return iterator{ ptr };
    }

    iterator insert(const_iterator position, size_type num, const_reference elem)
    {
        auto *ptr = insert_impl(position - begin(), num, forward_args(elem));
        return iterator{ ptr };
    }

    template <typename InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last)
    {
        if constexpr (std::is_same_v<std::input_iterator_tag, typename iterator_traits<InputIterator>::iterator_category>) {
            auto idx = position - begin();
            const auto idx_origin = idx;
            while (first != last)
                insert_impl(idx++, 1, forward_args(*first++));
            return iterator{ m_data + idx_origin };
        } else {
            const size_type num = std::distance(first, last);
            auto *const ptr = insert_impl(position - begin(), num, first);
            return iterator{ ptr };
        }
    }

    iterator insert(const_iterator position, initializer_list<value_type> il)
    {
        auto *const ptr = insert_impl(position - begin(), il.size(), il.begin());
        return iterator{ ptr };
    }

    iterator erase(const_iterator position)
    {
        return erase(position, position + 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        const size_type num = last - first;
        utl::copy(move_if_noexcept_iterator(last.data()), move_if_noexcept_iterator(cend().data()), first.data());
        destroy(m_data + m_size - num, num, m_alloc);
        m_size -= num;
        return iterator(first.data());
    }

    void swap(vector &other) noexcept(alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value)
    {
        using std::swap;
        if constexpr (alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value) {
            swap(m_alloc, other.m_alloc);
            swap(m_data, other.m_data);
            swap(m_size, other.m_size);
            swap(m_cap, other.m_cap);
        } else if (m_alloc == other.get_allocator()) {
            swap(m_data, other.m_data);
            swap(m_size, other.m_size);
            swap(m_cap, other.m_cap);
        } else if (m_size != other.m_size) {
            auto [t_more, t_less] = other.m_size > m_size ? std::tie(other, *this) : std::tie(*this, other);
            auto *const new_data = alloc_and_construct(t_more.m_size, move_if_noexcept_iterator(t_more.m_data), t_less.m_alloc, t_more.m_data);

            UTL_TRY
            {
                for (size_type i = 0; i != t_less.m_size; ++i)
                    t_more.m_data[i] = std::move_if_noexcept(t_less.m_data[i]);
            }
            UTL_CATCH(...)
            {
                destroy_and_dealloc(new_data, t_more.m_size, t_more.m_size, t_less.m_alloc);
                UTL_RETHROW;
            }

            destroy_and_dealloc(t_less.m_data, t_less.m_size, t_less.m_cap, t_less.m_alloc);

            t_less.m_data = new_data;
            destroy(t_more.m_data + t_less.m_size, t_more.m_size - t_less.m_size, t_more.m_alloc);
            swap(t_less.m_size, t_more.m_size);
            swap(t_less.m_cap, t_more.m_cap);
        } else {
            for (size_type i{ 0 }; i != m_size; ++i) {
                swap(m_data[i], other.m_data[i]);
            }
        }
    }

    void clear() noexcept
    {
        destroy(m_data, m_size, m_alloc);
        m_size = 0;
    }

private:
    allocator_type m_alloc;
    pointer m_data;
    size_type m_cap;
    size_type m_size;
};

template <typename InputIterator, typename Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
vector(InputIterator, InputIterator, Allocator = Allocator())
    ->vector<typename iterator_traits<InputIterator>::value_type, Allocator>;

template <typename Tp, typename Allocator>
inline bool operator==(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y)
{
    if (x.size() != y.size())
        return false;
    for (auto i = x.begin(), j = y.begin(), end = x.end(); i != end; ++i, ++j)
        if (*i != *j)
            return false;
    return true;
}

template <typename Tp, typename Allocator>
bool operator<(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y);

template <typename Tp, typename Allocator>
bool operator!=(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y);

template <typename Tp, typename Allocator>
bool operator>(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y);

template <typename Tp, typename Allocator>
bool operator>=(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y);

template <typename Tp, typename Allocator>
bool operator<=(const vector<Tp, Allocator> &x, const vector<Tp, Allocator> &y);

// 26.3.11.6, specialized algorithms
template <typename Tp, typename Allocator>
void swap(vector<Tp, Allocator> &x, vector<Tp, Allocator> &y) noexcept(noexcept(x.swap(y)))
{
    x.swap(y);
}

} // namespace utl
