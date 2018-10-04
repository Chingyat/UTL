#pragma once
#include <utl/config.hpp>

#include <memory>
#include <new>
#include <type_traits>

namespace utl {
template <typename T>
class allocator {
public:
    using value_type = T;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;
    allocator() noexcept = default;
    allocator(const allocator &) noexcept = default;
    template <class U>
    allocator(const allocator<U> &) noexcept
    {
    }
    ~allocator() = default;
    allocator &operator=(const allocator &) noexcept = default;

    T *allocate(size_t n)
    {
        auto ret = operator new[](n * sizeof(T), static_cast<std::align_val_t>(alignof(T)), std::nothrow);
        if (!ret)
            UTL_THROW(std::bad_alloc());
        return reinterpret_cast<T *>(ret);
    }
    void deallocate(T *p, [[maybe_unused]] size_t sz)
    {
        operator delete[](p, static_cast<std::align_val_t>(alignof(T)));
    }

    friend constexpr bool operator==(const allocator &, const allocator &) noexcept
    {
        return true;
    }
    friend constexpr bool operator!=(const allocator &, const allocator &) noexcept
    {
        return false;
    }
};

using std::allocator_traits;
} // namespace utl
