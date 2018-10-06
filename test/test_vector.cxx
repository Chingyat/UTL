#include "doctest.h"

#include <utl/vector.hpp>

#include <algorithm>
#include <iostream>
#include <string>

TEST_SUITE("vector")
{
    TEST_CASE("constructors")
    {
        utl::vector<std::string> v1(300, utl::allocator<std::string>());
        const std::string cs = "102345678901234456790";
        utl::vector<std::string> v2(300, cs, utl::allocator<std::string>());
        utl::vector<std::string> v3(std::move(v1));
        utl::vector v4(std::move(v2), utl::allocator<std::string>());
        utl::vector<std::string> v5;
    }

    TEST_CASE("assignment operators")
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
        CHECK(std::equal(std::begin(v1), std::end(v1), std::begin(v2), std::end(v2)));
        v6 = v3;
        CHECK(std::equal(std::begin(v3), std::end(v3), std::begin(v6), std::end(v6)));
        v3 = std::move(v7);
        CHECK(std::equal(std::begin(v3), std::end(v3), std::begin(v8), std::end(v8)));
        v8 = std::move(v4);
        CHECK(std::equal(std::begin(v5), std::end(v5), std::begin(v8), std::end(v8)));
        v5 = {"1", "2", "3", "4", "5"};
        CHECK(v5.size() == 5);
        v1.assign(v6.begin(), v6.end());
        CHECK(std::equal(std::begin(v1), std::end(v1), std::begin(v8), std::end(v8)));
        v2.assign(8u, "XXXXXXXXXXX");
        CHECK(v2.size() == 8);
        CHECK(std::all_of(v2.begin(), v2.end(), [](const auto &x) { return x == "XXXXXXXXXXX"; }));
        v2.assign(100u, "12345678901234567890");
        CHECK(std::all_of(v2.begin(), v2.end(), [](const auto &x) { return x == "12345678901234567890"; }));
        v3.assign({"1", "2", "3", "4", "5"});
        CHECK(v3.size() == 5);
        v3.reserve(200u);
        v3.assign(100u, "12345678901234567890");
        CHECK(std::all_of(v3.begin(), v3.end(), [](const auto &x) { return x == "12345678901234567890"; }));
    }

    TEST_CASE("iterators")
    {
        utl::vector<int> v{1, 2, 3, 4, 5};
        const int a1[5]{1, 2, 3, 4, 5};
        const int a2[5]{5, 4, 3, 2, 1};
        CHECK(std::equal(std::begin(a1), std::end(a1),
            std::begin(v), std::end(v)));
        CHECK(std::equal(std::begin(a2), std::end(a2),
            std::rbegin(v), std::rend(v)));
        CHECK(std::equal(std::begin(a1), std::end(a1),
            std::cbegin(v), std::cend(v)));
        CHECK(std::equal(std::begin(a2), std::end(a2),
            std::crbegin(v), std::crend(v)));
    }

    TEST_CASE("capacity")
    {
        utl::vector<int> v{1, 2, 3, 4, 5};
        v.resize(1);
        CHECK(v.size() == 1);
        CHECK(v.capacity() >= 5);
        v.resize(100);
        CHECK(v.size() == 100);
        CHECK(v.capacity() >= 100);
        v.reserve(200);
        CHECK(v.capacity() >= 200);
        CHECK(v.size() == 100);
        v.shrink_to_fit();
        CHECK(v.size() == 100);
        CHECK(v.capacity() == 100);
        CHECK(!v.empty());
        v.resize(0);
        CHECK(v.empty());
        v.resize(100, 42);
        v.clear();
        CHECK(v.size() == 0);
        CHECK(v.max_size() >= v.size());
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

    TEST_CASE("with nonequal allocator")
    {
        utl::vector v1(100u, 42, NonequalAllocator<int>(1));
        utl::vector v2(34u, 5, NonequalAllocator<int>(3));
        auto v3 = v2;

        CHECK(std::equal(v3.begin(), v3.end(), v2.begin(), v2.end()));

        v2 = v1;

        CHECK(std::equal(v1.begin(), v1.end(), v2.begin(), v2.end()));

        v3 = std::move(v1);

        CHECK(std::equal(v1.begin(), v1.end(), v3.begin(), v3.end()));
    }

    TEST_CASE("member access")
    {
        SUBCASE("vector front and back")
        {
            utl::vector<std::unique_ptr<int>> v;
            v.push_back(std::make_unique<int>(34));

            SUBCASE("front and back should be the same element")
            {
                CHECK(v.back() == v.front());
            }

            v.emplace_back();

            CHECK(*v.front() == 34);
        }

        utl::vector<int> v{1, 2, 3, 4, 5};

        SUBCASE("vector at")
        {
            CHECK(v.at(1) == 2);
            CHECK(v.at(2) == 3);
#if !UTL_NO_EXCEPTIONS
            CHECK_THROWS_AS(v.at(v.size() + 1), std::out_of_range);
#endif
        }

        SUBCASE("vector []")
        {
            CHECK(v[0] == 1);
            CHECK(v[1] == 2);
            CHECK(v[4] == 5);
            v[v.size() + 100];
        }

        SUBCASE("vector data")
        {
            CHECK(v.data() == &v[0]);
            CHECK(std::equal(v.begin(), v.end(), v.data(), v.data() + v.size()));
        }
    }

    struct ThrowWhenCopy {
        int m_data;
        ThrowWhenCopy(int i) noexcept
            : m_data(i)
        {
        }
        ThrowWhenCopy(ThrowWhenCopy &&) noexcept = default;
        [[noreturn]] ThrowWhenCopy(const ThrowWhenCopy &) { UTL_THROW(std::runtime_error("ThrowWhenCopy")); }
        [[noreturn]] ThrowWhenCopy &operator=(ThrowWhenCopy const &) { UTL_THROW(std::runtime_error("=")); }
        ThrowWhenCopy &operator=(ThrowWhenCopy &&) = default;
        ~ThrowWhenCopy() = default;
    };

    TEST_CASE("modifiers")
    {
        utl::vector<int> iv{1, 2, 3, 4, 5};
        utl::vector<std::string> sv{"Hello", "world"};
        utl::vector<std::unique_ptr<int>> pv;
        pv.push_back(std::make_unique<int>(32));
        pv.push_back(std::unique_ptr<int>());
        utl::vector<std::iostream> iov;
        utl::vector<ThrowWhenCopy> tv;
        tv.reserve(10);
        tv.emplace_back(1);
        tv.emplace_back(2);
        tv.emplace_back(3);

        SUBCASE("clear")
        {
            iv.clear();
            CHECK(iv.empty());
            sv.clear();
            CHECK(sv.empty());
            pv.clear();
            CHECK(pv.empty());
            CHECK(iov.empty());
            iov.clear();
            CHECK(iov.empty());
            tv.clear();
            CHECK(tv.empty());
        }

        SUBCASE("insert")
        {
            int i = -1;
            CHECK(*iv.insert(iv.cbegin() + 2, i) == -1);
            CHECK(iv[2] == -1);

#if !UTL_NO_EXCEPTIONS
            ThrowWhenCopy t(34);
            CHECK(tv.size() == 3);
            CHECK_THROWS(tv.insert(tv.cend(), t));
            CHECK(tv.size() == 3);
            CHECK(tv[0].m_data == 1);
            CHECK(tv[1].m_data == 2);
            CHECK(tv[2].m_data == 3);
#endif
        }
    }
}
