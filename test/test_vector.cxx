#include <utl/vector.hpp>

#include <algorithm>
#include <cassert>
#include <string>

void test_ctor()
{
    utl::vector<std::string> v1(300, utl::allocator<std::string>());
    const std::string cs = "102345678901234456790";
    utl::vector<std::string> v2(300, cs, utl::allocator<std::string>());
    utl::vector<std::string> v3(std::move(v1));
    utl::vector v4(std::move(v2), utl::allocator<std::string>());
    utl::vector<std::string> v5;
}

void test_assign()
{
    utl::vector<std::string> v1(300, std::string("12345678901234567890"));
    utl::vector<std::string> v2{"Hello ", "World"};
    utl::vector<std::string> v3{"Hello ", "World"};
    utl::vector<std::string> v4{"Hello ", "World"};
    utl::vector<std::string> v5{"Hello ", "World"};
    utl::vector<std::string> v6(300, std::string("12345678901234567890"));
    utl::vector<std::string> v7(300, std::string("12345678901234567890"));
    utl::vector<std::string> v8(300, std::string("12345678901234567890"));
    utl::vector<std::string> v9(300, std::string("12345678901234567890"));

    v2 = v1;
    assert(std::equal(std::begin(v1), std::end(v1), std::begin(v2), std::end(v2)));
    v6 = v3;
    assert(std::equal(std::begin(v3), std::end(v3), std::begin(v6), std::end(v6)));
    v3 = std::move(v7);
    assert(std::equal(std::begin(v3), std::end(v3), std::begin(v8), std::end(v8)));
    v8 = std::move(v4);
    assert(std::equal(std::begin(v5), std::end(v5), std::begin(v8), std::end(v8)));
    v5 = {"1", "2", "3", "4", "5"};
    assert(v5.size() == 5);
    v1.assign(v6.begin(), v6.end());
    assert(std::equal(std::begin(v1), std::end(v1), std::begin(v8), std::end(v8)));
    v2.assign(8u, "XXXXXXXXXXX");
    assert(v2.size() == 8);
    assert(std::all_of(v2.begin(), v2.end(), [](const auto &x) { return x == "XXXXXXXXXXX"; }));
    v2.assign(100u, "12345678901234567890");
    assert(std::all_of(v2.begin(), v2.end(), [](const auto &x) { return x == "12345678901234567890"; }));
    v3.assign({"1", "2", "3", "4", "5"});
    assert(v3.size() == 5);
    v3.reserve(200u);
    v3.assign(100u, "12345678901234567890");
    assert(std::all_of(v3.begin(), v3.end(), [](const auto &x) { return x == "12345678901234567890"; }));
}

void test_iter()
{
    utl::vector<int> v{1, 2, 3, 4, 5};
    const int a1[5]{1, 2, 3, 4, 5};
    const int a2[5]{5, 4, 3, 2, 1};
    assert(std::equal(std::begin(a1), std::end(a1),
        std::begin(v), std::end(v)));
    assert(std::equal(std::begin(a2), std::end(a2),
        std::rbegin(v), std::rend(v)));
    assert(std::equal(std::begin(a1), std::end(a1),
        std::cbegin(v), std::cend(v)));
    assert(std::equal(std::begin(a2), std::end(a2),
        std::crbegin(v), std::crend(v)));
}

void test_cap()
{
    utl::vector<int> v{1, 2, 3, 4, 5};
    v.resize(1);
    assert(v.size() == 1);
    v.resize(100);
    assert(v.size() == 100);
    v.reserve(200);
    assert(v.capacity() >= 200);
    assert(v.size() == 100);
    v.shrink_to_fit();
    assert(v.size() == 100);
    assert(v.capacity() == 100);
    assert(!v.empty());
    v.resize(0);
    assert(v.empty());
    v.resize(100, 42);
    v.clear();
    assert(v.size() == 0);
}

template <typename T>
struct NonequalAllocator {
    using value_type = T;
    using propagate_on_container_move_assignment = std::false_type;
    using is_always_equal = std::false_type;
    explicit NonequalAllocator(int id) noexcept
        : m_id(id)
    {
    }
    NonequalAllocator(const NonequalAllocator &) noexcept = default;
    template <class U>
    NonequalAllocator(const NonequalAllocator<U> &x) noexcept
        : m_id(x.m_id)
    {
    }
    ~NonequalAllocator() = default;
    NonequalAllocator &operator=(const NonequalAllocator &) noexcept = default;

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

    friend bool operator==(const NonequalAllocator &lhs, const NonequalAllocator &rhs) noexcept
    {
        return lhs.m_id == rhs.m_id;
    }
    friend bool operator!=(const NonequalAllocator &lhs, const NonequalAllocator &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    int m_id;
};

void test_with_nonequal_allocator()
{
    utl::vector v1(100u, 42, NonequalAllocator<int>(1));
    utl::vector v2(34u, 5, NonequalAllocator<int>(3));
    auto v3 = v2;

    assert(std::equal(v3.begin(), v3.end(), v2.begin(), v2.end()));

    v2 = v1;

    assert(std::equal(v1.begin(), v1.end(), v2.begin(), v2.end()));

    v3 = std::move(v1);

    assert(std::equal(v1.begin(), v1.end(), v3.begin(), v3.end()));
}

int main()
{
    test_ctor();
    test_assign();
    test_iter();
    test_cap();
    test_with_nonequal_allocator();
}
