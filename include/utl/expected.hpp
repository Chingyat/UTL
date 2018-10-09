#pragma once
#include <utl/config.hpp>

#include <exception>

namespace utl {

struct noncopyable {
  noncopyable(const noncopyable &) = delete;
  noncopyable(noncopyable &&) = delete;

  noncopyable& operator=(const noncopyable &) = delete;

  noncopyable& operator=(noncopyable &&) = delete;
};

template <typename T>
struct expected {
  union {
    T m_value;
    std::exception_ptr m_exception;
  };

  bool filled;

  expected(const T &v) noexcept try
      : m_value(v)
      , filled(true)
      {}
  catch (...) {
      new (&m_exxeption) std::exception_ptr(current_exception());
      filled = false;
  }

  ~expected() {
    if (filled) {
      m_value.~T();
    } else {
      m_exception.~exception_ptr();
    }
  }      
};
}

