#pragma once
#include <typeinfo>
#include <utl/config.hpp>
#include <utl/unique_ptr.hpp>

namespace utl {
using std::bad_cast;

class bad_any_cast : public bad_cast {
public:
  const char *what() const noexcept;
};

class any {

  struct Value {

    virtual Value *clone() const = 0;

    virtual const std::type_info &type() const noexcept = 0;

    virtual ~Value() = default;
  };

  template <typename T> struct ValueImpl : Value {
    T m_data;

    template <typename... Args>
    ValueImpl(Args &&... args) : m_data(std::forward<Args>(args)...) {}

    Value *clone() const final { return new ValueImpl(m_data); }

    const std::type_info &type() const noexcept final { return typeid(T); }
  };

public:
  constexpr any() noexcept = default;

  template <typename T,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any>>>
  any(T &&x) : m_value(new ValueImpl<std::decay_t<T>>(std::forward<T>(x))) {}

  template <typename T,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any>>>
  any &operator=(T &&x) {
    m_value = make_unique<ValueImpl<std::decay_t<T>>>(std::forward<T>(x));
    return *this;
  }

  any(const any &other)
      : m_value(other.empty() ? nullptr : other.m_value->clone()) {}

  any(any &&other) noexcept = default;

  any &operator=(const any &rhs) {
    m_value.reset(rhs.empty() ? nullptr : rhs.m_value->clone());
    return *this;
  }

  any &operator=(any &&rhs) noexcept = default;

  template <typename T> T *get() noexcept {
    if (typeid(T) == type())
      return &static_cast<ValueImpl<T> *>(m_value.get())->m_data;
    return nullptr;
  }

  template <typename T> const T *get() const noexcept {
    if (typeid(T) == type())
      return &static_cast<ValueImpl<std::decay_t<T>> *>(m_value.get())->m_data;
    return nullptr;
  }

  const std::type_info &type() const noexcept { return m_value->type(); }

  bool empty() const noexcept { return m_value == nullptr; }

private:
  utl::unique_ptr<Value> m_value;
};

template <typename Tp, bool IsPtr = std::is_pointer_v<Tp>>
Tp any_cast(const any &a) noexcept(IsPtr) {
  using T = std::remove_pointer_t<Tp>;
  const auto p = a.get<T>();

  if constexpr (IsPtr)
    return p;
  else if (p)
    return *p;
  else
    UTL_THROW(bad_any_cast());
}

template <typename Tp, typename = std::enable_if_t<std::is_pointer_v<Tp>>>
Tp any_cast(any &a) noexcept {
  return a.get<std::remove_pointer_t<Tp>>();
}

} // namespace utl
