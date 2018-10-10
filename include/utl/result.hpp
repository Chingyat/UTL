#pragma once
#include <utl/config.hpp>

#include <exception>

namespace utl {

struct noncopyable {
    noncopyable(const noncopyable &) = delete;
    noncopyable(noncopyable &&) = delete;

    noncopyable &operator=(const noncopyable &) = delete;

    noncopyable &operator=(noncopyable &&) = delete;
};

template <typename T>
struct result : noncopyable {
    union {
        T m_value;
        std::exception_ptr m_exception;
    };

    bool filled;

    result(const T &v) noexcept try
        : m_value(v),
          filled(true) {
    } catch (...) {
        new (&m_exception) std::exception_ptr(current_exception());
        filled = false;
    }

    ~result()
    {
        if (filled) {
            m_value.~T();
        } else {
            m_exception.~exception_ptr();
        }
    }
};
} // namespace utl
