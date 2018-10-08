#include "doctest.h"

#include <utl/optional.hpp>

#include <string>

#if 0
TEST_CASE("optional for trivially-copyable objects")
{
    utl::optional<int> x = 12;

    CHECK(x);
    CHECK(x.has_value());
    CHECK(*x == 12);
    CHECK(x.value() == 12);
    CHECK(x.value_or(1.0) == 12);
    CHECK(&*x == x.operator->());
    utl::optional<int> y;

    CHECK(!y.has_value());
    CHECK(!y);
#    if !UTL_NO_EXCEPTIONS
    CHECK_THROWS_AS(y.value(), utl::bad_optional_access);
#    endif
    CHECK(y.value_or(1.0) == 1);

    auto get_optional = [] {
        utl::optional o = 'a';
        return o;
    };

    CHECK(get_optional().value() == 'a');

    utl::optional<int> default_construction;
}
#endif

TEST_CASE("optional")
{

    SUBCASE("default construction")
    {
        utl::optional<std::string> o;

        CHECK(!o.has_value());
        CHECK(!o);
#if !UTL_NO_EXCEPTIONS
        CHECK_THROWS_AS(o.value(), utl::bad_optional_access);
#endif
        CHECK(o.value_or("Hello world") == "Hello world");
    }

    SUBCASE("construction from const T &")
    {
        const std::string str = "Hello";
        utl::optional<std::string> o(str);
        CHECK(o);
        CHECK(o.has_value());
        CHECK(o.value() == "Hello");
        CHECK(o.value_or("Bla") == "Hello");
    }

    SUBCASE("construction from const T &")
    {
        utl::optional<std::string> o(std::string("Hello"));
        CHECK(o);
        CHECK(o.has_value());
        CHECK(o.value() == "Hello");
        CHECK(o.value_or("Bla") == "Hello");
    }

    SUBCASE("construction from U&&")
    {
        utl::optional<std::string> o("Hello");
        CHECK(o);
        CHECK(o.has_value());
        CHECK(o.value() == "Hello");
        CHECK(o.value_or("Bla") == "Hello");
    }

    SUBCASE("construction from Args&&...")
    {
        utl::optional<std::string> o(utl::in_place_t{}, 10, 'a');
        CHECK(o);
        CHECK(o.has_value());
        CHECK(o.value() == "aaaaaaaaaa");
        CHECK(o.value_or("Bla") == "aaaaaaaaaa");
    }

    SUBCASE("copy construction")
    {
        const utl::optional<std::string> o1("Hello");
        const auto o2 = o1;
        CHECK(o2);
        CHECK(o2.has_value());
        CHECK(o2.value() == "Hello");
        CHECK(o2.value_or("Bla") == "Hello");
    }

    SUBCASE("move construction")
    {

        utl::optional<std::string> o1("Hello");
        const auto o2 = std::move(o1);
        CHECK(o2);
        CHECK(o2.has_value());
        CHECK(o2.value() == "Hello");
        CHECK(o2.value_or("Bla") == "Hello");
    }
}
