#pragma once

#include <utl/config.hpp>

#include <initializer_list>

namespace utl {

template <typename T>
constexpr typename std::remove_reference<T>::type &&move(T &&x) noexcept {
  return static_cast<typename std::remove_reference<T>::type &&>(x);
}

template <class T>
constexpr
    typename std::conditional<!std::is_nothrow_move_constructible<T>::value &&
                                  std::is_copy_constructible<T>::value,
                              const T &, T &&>::type
    move_if_noexcept(T &x) noexcept {
  return static_cast<T &&>(x);
}

template <typename T, typename U> constexpr T &&forward(U &&x) noexcept {
  return static_cast<T &&>(x);
}

template <class T> constexpr std::add_const_t<T> &as_const(T &t) noexcept {
  return t;
}

template <class T> void as_const(const T &&) = delete;

template <class T>
typename std::add_rvalue_reference<T>::type declval() noexcept;

template <typename T,
          typename std::enable_if<std::is_move_constructible<T>::value &&
                                      std::is_move_assignable<T>::value,
                                  int>::type = 0>
constexpr void
swap(T &a, T &b) noexcept(std::is_nothrow_move_constructible<T>::value
                              &&std::is_nothrow_move_assignable<T>::value) {
  T temp = utl::move(b);
  b = utl::move(a);
  a = utl::move(temp);
}

template <class T, class U = T> constexpr T exchange(T &obj, U &&new_value) {
  T old_value = utl::move(obj);
  obj = utl::forward<U>(new_value);
  return old_value;
}

template <class _Ty, class = void>
struct is_implicitly_default_constructible : std::false_type {
  // determine whether _Ty can be copy-initialized with {}
};

template <class _Ty> void implicitly_default_construct(const _Ty &);

template <class _Ty>
struct is_implicitly_default_constructible<
    _Ty, decltype(implicitly_default_construct<_Ty>({}), void())>
    : std::true_type {};

namespace _swappable {
using utl::move;

template <typename T, typename U, typename = void>
struct is_swappable_with : std::false_type {};

template <typename T, typename U>
struct is_swappable_with<T, U,
                         decltype(swap(utl::declval<T>(), utl::declval<U>()),
                                  swap(utl::declval<U>(), utl::declval<T>()),
                                  void())> : std::true_type {};

template <typename T, typename = void>
struct is_swappable : is_swappable_with<T, T> {};

template <typename T>
struct is_swappable<
    T, typename std::enable_if<
           std::is_void<typename std::decay<T>::type>::value ||
           std::is_function<typename std::decay<T>::type>::value>::type>
    : std::false_type {};

template <typename T, typename U>
struct swap_cannot_throw
    : std::bool_constant<
          noexcept(swap(utl::declval<T>(), utl::declval<U>())) &&noexcept(
              swap(utl::declval<U>(), utl::declval<T>()))> {};

template <typename T, typename U>
struct is_nothrow_swappable_with
    : std::conjunction<is_swappable_with<T, U>, swap_cannot_throw<T, U>> {};

template <typename T, typename = void>
struct is_nothrow_swappable : is_nothrow_swappable_with<T, T> {};

template <typename T>
struct is_nothrow_swappable<
    T, typename std::enable_if<
           std::is_void<typename std::decay<T>::type>::value ||
           std::is_function<typename std::decay<T>::type>::value>::type>
    : std::false_type {};

} // namespace _swappable

template <typename T, typename U>
struct is_swappable_with : _swappable::is_swappable_with<T, U> {};

template <typename T> struct is_swappable : _swappable::is_swappable<T> {};

template <typename T, typename U>
struct is_nothrow_swappable_with : _swappable::is_nothrow_swappable_with<T, U> {
};

template <typename T>
struct is_nothrow_swappable : _swappable::is_nothrow_swappable<T> {};

template <typename T1, typename T2> struct pair {
  typedef T1 first_type;
  typedef T2 second_type;

  first_type first;
  second_type second;

  template <bool Dummy = true,
            typename std::enable_if<
                Dummy && std::is_default_constructible<first_type>::value &&
                    std::is_default_constructible<second_type>::value &&
                    is_implicitly_default_constructible<first_type>::value &&
                    is_implicitly_default_constructible<second_type>::value,
                int>::type = 0>
  constexpr pair() {}

  template <bool Dummy = true,
            typename std::enable_if<
                Dummy && std::is_default_constructible<first_type>::value &&
                    std::is_default_constructible<second_type>::value &&
                    (!is_implicitly_default_constructible<first_type>::value ||
                     !is_implicitly_default_constructible<second_type>::value),
                int>::type = 0>
  explicit constexpr pair() {}

  template <
      bool Dummy = true,
      typename std::enable_if<
          Dummy && std::is_copy_constructible<first_type>::value &&
              std::is_copy_constructible<second_type>::value &&
              std::is_convertible<const first_type &, first_type>::value &&
              std::is_convertible<const second_type &, second_type>::value,
          int>::type = 0>
  constexpr pair(const T1 &x, const T2 &y) : first(x), second(y) {}

  template <
      bool Dummy = true,
      typename std::enable_if<
          Dummy && std::is_copy_constructible<first_type>::value &&
              std::is_copy_constructible<second_type>::value &&
              (!std::is_convertible<const first_type &, first_type>::value ||
               !std::is_convertible<const second_type &, second_type>::value),
          int>::type = 0>
  explicit constexpr pair(const T1 &x, const T2 &y) : first(x), second(y) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 &&>::value &&
                    std::is_constructible<second_type, U2 &&>::value &&
                    std::is_convertible<U1 &&, first_type>::value &&
                    std::is_convertible<U2 &&, second_type>::value,
                int>::type = 0>
  constexpr pair(U1 &&x, U2 &&y)
      : first(utl::forward<U1>(x)), second(utl::forward<U2>(y)) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 &&>::value &&
                    std::is_constructible<second_type, U2 &&>::value &&
                    (!std::is_convertible<U1 &&, first_type>::value ||
                     !std::is_convertible<U2 &&, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(U1 &&x, U2 &&y)
      : first(utl::forward<U1>(x)), second(utl::forward<U2>(y)) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 const &>::value &&
                    std::is_constructible<second_type, U2 const &>::value &&
                    std::is_convertible<U1 const &, first_type>::value &&
                    std::is_convertible<U2 const &, second_type>::value,
                int>::type = 0>
  constexpr pair(const pair<U1, U2> &p) : first(p.first), second(p.second) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 const &>::value &&
                    std::is_constructible<second_type, U2 const &>::value &&
                    (!std::is_convertible<U1 const &, first_type>::value ||
                     !std::is_convertible<U2 const &, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(const pair<U1, U2> &p)
      : first(p.first), second(p.second) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 &&>::value &&
                    std::is_constructible<second_type, U2 &&>::value &&
                    std::is_convertible<U1 &&, first_type>::value &&
                    std::is_convertible<U2 &&, second_type>::value,
                int>::type = 0>
  constexpr pair(pair<U1, U2> &&p)
      : first(std::move(p.first)), second(std::move(p.second)) {}

  template <typename U1, typename U2,
            typename std::enable_if<
                std::is_constructible<first_type, U1 &&>::value &&
                    std::is_constructible<second_type, U2 &&>::value &&
                    (!std::is_convertible<U1 &&, first_type>::value ||
                     !std::is_convertible<U2 &&, second_type>::value),
                int>::type = 0>
  explicit constexpr pair(pair<U1, U2> &&p)
      : first(std::move(p.first)), second(std::move(p.second)) {}

  pair(const pair &p) = default;

  pair(pair &&p) = default;

  pair &operator=(
      typename std::conditional<std::is_copy_assignable<first_type>::value &&
                                    std::is_copy_assignable<second_type>::value,
                                pair, struct not_a_type>::type const &other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  template <class U1, class U2,
            typename std::enable_if<
                std::is_assignable<first_type &, const U1 &>::value &&
                    std::is_assignable<second_type &, const U2 &>::value,
                int>::type = 0>
  constexpr pair &operator=(const pair<U1, U2> &other) {
    first = other.first;
    second = other.second;
    return *this;
  }

  constexpr pair &operator=(
      typename std::conditional<std::is_move_assignable<first_type>::value &&
                                    std::is_move_assignable<second_type>::value,
                                pair, struct not_a_type>::type
          &&other) noexcept(std::is_nothrow_move_assignable<first_type>::value
                                &&std::is_nothrow_move_assignable<
                                    second_type>::value) {
    first = utl::move(other).first;
    second = utl::move(other).second;
    return *this;
  }

  template <class U1, class U2,
            typename std::enable_if<
                std::is_assignable<first_type &, U1 &&>::value &&
                    std::is_assignable<second_type &, U2 &&>::value,
                int>::type = 0>
  constexpr pair &operator=(pair<U1, U2> &&other) {
    first = utl::move(other).first;
    second = utl::move(other).second;
  }

  constexpr void
  swap(pair &other) noexcept(is_nothrow_swappable<first_type>::value
                                 &&is_nothrow_swappable<second_type>::value) {
    using utl::move;
    swap(first, other.first);
    swap(second, other.second);
  }
};

} // namespace utl
