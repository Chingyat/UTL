#pragma once

#include <utl/compressed_pair.hpp>
#include <utl/config.hpp>
#include <cassert>

namespace utl {

using nullptr_t = decltype(nullptr);

template <typename T> struct default_delete {
  constexpr default_delete() noexcept = default;

  template <class U, typename = typename std::enable_if<
                         std::is_convertible<U *, T *>::value>::type>
  default_delete(const default_delete<U> &) noexcept {}

  void operator()(T *x) const { delete x; }
};

template <typename T, typename Deleter, typename = void> struct pointer_type {
  using type = T *;
};

template <typename T, typename Deleter>
struct pointer_type<T, Deleter, std::void_t<typename Deleter::pointer>> {
  using type = typename Deleter::pointer;
};

template <class T, class Deleter = default_delete<T>> class unique_ptr {
public:
  using pointer = typename pointer_type<T, Deleter>::type;
  using element_type = T;
  using deleter_type = Deleter;

  compressed_pair<pointer, deleter_type> _p;

  template <typename D = Deleter,
            typename = std::enable_if_t<std::is_default_constructible<D>{} &&
                                        !std::is_pointer<D>{}>>
  constexpr unique_ptr() noexcept : _p(nullptr) {}

  template <typename D = Deleter,
            typename = std::enable_if_t<std::is_default_constructible<D>{} &&
                                        !std::is_pointer<D>{}>>
  constexpr unique_ptr(std::nullptr_t) noexcept : _p(nullptr) {}

  template <typename D = Deleter,
            typename = std::enable_if_t<std::is_default_constructible<D>{} &&
                                        !std::is_pointer<D>{}>>
  explicit unique_ptr(pointer p) noexcept : _p(p) {
    static_assert(!std::is_pointer<deleter_type>(), "");
  }

  unique_ptr(pointer p, Deleter &d) noexcept
      : _p(p, std::forward<decltype(d)>(d)) {}

  unique_ptr(pointer p, std::remove_reference_t<Deleter> &&d) noexcept
      : _p(p, std::forward<decltype(d)>(d)) {}

  unique_ptr(unique_ptr &&u) noexcept : _p(std::move(u._p)) { u.release(); }

  template <class U, class E>
  unique_ptr(unique_ptr<U, E> &&u) noexcept
      : _p(u.release(), u.get_deleter()) {}

  // template <class U>
  // unique_ptr(std::auto_ptr<U> &&u) noexcept;

  ~unique_ptr() {
    if (get()) {
      get_deleter()(get());
    }
  }

  pointer get() const noexcept { return _p.first(); }

  deleter_type &get_deleter() noexcept { return _p.second(); }

  const deleter_type &get_deleter() const noexcept { return _p.second(); }

  unique_ptr &operator=(unique_ptr &&r) noexcept {
    reset(r.release());
    get_deleter() = std::forward<Deleter>(r.get_deleter());
    return *this;
  }

  template <class U, class E>
  typename std::enable_if<
      !std::is_array<U>::value &&
          std::is_convertible<typename unique_ptr<U, E>::pointer,
                              pointer>::value &&
          std::is_assignable<Deleter &, E &&>::value,
      unique_ptr &>::type
  operator=(unique_ptr<U, E> &&r) noexcept {
    reset(r.release());
    get_deleter() = std::forward<E>(r.get_deleter());

    return *this;
  }

  unique_ptr &operator=(std::nullptr_t) noexcept { reset(); }

  pointer release() noexcept {
    pointer r = get();
    _p.first() = nullptr;
    return r;
  }

  void reset(pointer ptr = pointer()) noexcept {
    pointer old_ptr = get();
    _p.first() = ptr;
    if (old_ptr)
      get_deleter()(old_ptr);
  }

  void swap(unique_ptr &other) noexcept {
    using std::swap;
    swap(_p, other._p);
  }

  operator bool() const noexcept { return get(); }

  typename std::add_lvalue_reference<T>::type operator*() const {
    assert(get());
    return *get();
  }

  pointer operator->() const noexcept {
    assert(get());
    return get();
  }
};

template <class T, class Deleter> class unique_ptr<T[], Deleter> {};

template <class T1, class D1, class T2, class D2>
bool operator==(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  return x.get() == y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator!=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  return x.get() != y.get();
}

template <class T1, class D1, class T2, class D2>
bool operator<(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  using CT =
      typename std::common_type<decltype(x.get()), decltype(y.get())>::type;

  return std::less<CT>()(x.get(), y.get());
}

template <class T1, class D1, class T2, class D2>
bool operator<=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  return !(y < x);
}

template <class T1, class D1, class T2, class D2>
bool operator>(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  return y < x;
}

template <class T1, class D1, class T2, class D2>
bool operator>=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y) {
  return !(x < y);
}

template <class T, class D>
bool operator==(const unique_ptr<T, D> &x, nullptr_t) noexcept {
  return !x;
}

template <class T, class D>
bool operator==(nullptr_t, const unique_ptr<T, D> &x) noexcept {
  return !x;
}

template <class T, class D>
bool operator!=(const unique_ptr<T, D> &x, nullptr_t) noexcept {
  return x;
}

template <class T, class D>
bool operator!=(nullptr_t, const unique_ptr<T, D> &x) noexcept {
  return x;
}

template <class T, class D>
bool operator<(const unique_ptr<T, D> &x, nullptr_t) {
  return std::less<typename unique_ptr<T, D>::pointer>()(x.get(), nullptr);
}

template <class T, class D>
bool operator<(nullptr_t, const unique_ptr<T, D> &y) {
  return std::less<typename unique_ptr<T, D>::pointer>()(nullptr, y.get());
}

template <class T, class D>
bool operator<=(const unique_ptr<T, D> &x, nullptr_t) {
  return !(nullptr < x);
}

template <class T, class D>
bool operator<=(nullptr_t, const unique_ptr<T, D> &y) {
  return !(y < nullptr);
}

template <class T, class D>
bool operator>(const unique_ptr<T, D> &x, nullptr_t) {
  return nullptr < x;
}

template <class T, class D>
bool operator>(nullptr_t, const unique_ptr<T, D> &y) {
  return y < nullptr;
}

template <class T, class D>
bool operator>=(const unique_ptr<T, D> &x, nullptr_t) {
  return !(x < nullptr);
}

template <class T, class D>
bool operator>=(nullptr_t, const unique_ptr<T, D> &y) {
  return !(nullptr < y);
}

template <typename T, typename D>
void swap(unique_ptr<T, D> &x, unique_ptr<T, D> &y) noexcept {
  x.swap(y);
}

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args &&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace utl
