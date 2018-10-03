#pragma once
#include <utl/iterator.hpp>

#include <type_traits>

namespace utl {

template <typename T1, typename T2>
constexpr decltype(auto) max(const T1 &x, const T2 &y) noexcept
{
    return x < y ? y : x;
}

template <typename T1, typename T2>
constexpr decltype(auto) min(const T1 &x, const T2 &y) noexcept
{
    return x < y ? x : y;
}

template <typename BiIt1, typename BiIt2>
inline BiIt2 copy_backward(BiIt1 first, BiIt1 last, BiIt2 output_last)
{
    while (first != last)
        *--output_last = *--last;

    return output_last;
}

template <typename InputIterator, typename OutputIterator>
inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator output_first)
{
    while (first != last)
        *output_first++ = *first++;

    return output_first;
}

template <typename Tp, typename = std::enable_if_t<std::is_trivially_copyable_v<Tp>>>
inline Tp *copy_backward(const Tp *first, const Tp *last, Tp *output_last) noexcept
{
    auto const ret_val = output_last - (last - first);
    std::memmove(ret_val, first, (last - first) * sizeof(Tp));
    return ret_val;
}

template <typename Tp, typename = std::enable_if_t<std::is_trivially_copyable_v<Tp>>>
inline Tp *copy(const Tp *first, const Tp *last, Tp *output_first) noexcept
{
    const auto num = last - first;
    std::memmove(output_first, first, num * sizeof(Tp));
    return output_first + num;
}

template <typename Tp, typename V = std::enable_if_t<std::is_trivially_copyable_v<Tp>>>
inline Tp *copy(move_if_noexcept_iterator<Tp *> first, move_if_noexcept_iterator<Tp *> last, Tp *output_first) noexcept
{
    const auto num = last - first;
    std::memmove(output_first, first.data(), num * sizeof(Tp));
    return output_first + num;
}

template <typename Tp, typename V = std::enable_if_t<std::is_trivially_copyable_v<Tp>>>
inline Tp *copy_backward(move_if_noexcept_iterator<Tp *> first, move_if_noexcept_iterator<Tp *> last, Tp *output_last, V * = nullptr) noexcept
{
    auto const ret_val = output_last - (last - first);
    std::memmove(ret_val, first.data(), (last - first) * sizeof(Tp));
    return ret_val;
}
}
