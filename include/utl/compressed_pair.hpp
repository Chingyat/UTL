#pragma once
#include <utl/config.hpp>

#include <type_traits>
#include <utility>

/// Synopsis

// template <class T1, class T2>
// class compressed_pair
// {
// public:
//     typedef T1                                                 first_type;
//     typedef T2                                                 second_type;
//     typedef typename call_traits<first_type>::param_type first_param_type;
//     typedef typename call_traits<second_type>::param_type second_param_type;
//     typedef typename call_traits<first_type>::reference first_reference;
//     typedef typename call_traits<second_type>::reference second_reference;
//     typedef typename call_traits<first_type>::const_reference
//     first_const_reference; typedef typename
//     call_traits<second_type>::const_reference second_const_reference;

//              compressed_pair() : base() {}
//              compressed_pair(first_param_type x, second_param_type y);
//     explicit compressed_pair(first_param_type x);
//     explicit compressed_pair(second_param_type y);

//     compressed_pair& operator=(const compressed_pair&);

//     first_reference       first();
//     first_const_reference first() const;

//     second_reference       second();
//     second_const_reference second() const;

//     void swap(compressed_pair& y);
// };

namespace utl {

template <typename T1, typename T2> struct compressed_pair;

template <typename Tag, typename T, typename = void> struct compressed_element {
  typedef T value_type;
  typedef T &reference;
  typedef const T &const_reference;

  T x_;

  constexpr compressed_element() noexcept(
      std::is_nothrow_default_constructible<T>::value) = default;

  template <typename... Args>
  constexpr compressed_element(Args &&... args) noexcept(
      noexcept(T(std::forward<Args>(args)...)))
      : x_(std::forward<Args>(args)...) {}

  reference get() noexcept { return x_; }

  const_reference get() const noexcept { return x_; }
};

template <typename Tag, typename T>
struct compressed_element<
    Tag, T, typename std::enable_if<std::is_empty<T>::value>::type> : T {
  typedef T value_type;
  typedef T &reference;
  typedef const T &const_reference;

  template <typename... Args>
  constexpr compressed_element(Args &&... args) noexcept(
      noexcept(T(std::forward<Args>(args)...)))
      : T(std::forward<Args>(args)...) {}

  reference get() noexcept { return *this; }

  const_reference get() const noexcept { return *this; }
};

template <typename T1, typename T2>
struct compressed_pair : private compressed_element<struct _tag1, T1>,
                         private compressed_element<struct _tag2, T2> {
  using first_type = typename compressed_element<_tag1, T1>::value_type;

  using second_type = typename compressed_element<_tag2, T2>::value_type;

  constexpr compressed_pair() = default;

  constexpr explicit compressed_pair(T1 const &x)
      : compressed_element<_tag1, T1>(x) {}

  constexpr explicit compressed_pair(T1 &&x)
      : compressed_element<_tag1, T1>(std::move(x)) {}

  template <typename X1, typename X2>
  constexpr compressed_pair(X1 &&x1, X2 &&x2)
      : compressed_element<_tag1, T1>(std::forward<X1>(x1)),
        compressed_element<_tag2, T2>(std::forward<X2>(x2)) {}

  T1 &first() noexcept { return compressed_element<_tag1, T1>::get(); }
  const T1 &first() const noexcept {
    return compressed_element<_tag1, T1>::get();
  }

  T2 &second() noexcept { return compressed_element<_tag2, T2>::get(); }
  const T2 &second() const noexcept {
    return compressed_element<_tag2, T2>::get();
  }

  void swap(compressed_pair &other) {
    using std::swap;
    swap(first(), other.first());
    swap(second(), other.second());
  }
};

template <typename T1, typename T2>
void swap(compressed_pair<T1, T2> &x, compressed_pair<T1, T2> &y) {
  x.swap(y);
}

} // namespace utl
