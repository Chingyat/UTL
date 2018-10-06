#include "doctest.h"

#include <utl/any.hpp>

#include <string>

TEST_CASE("any")
{
    utl::any a = 12;

    int *i = utl::any_cast<int *>(a);
    CHECK(*i == 12);

    CHECK(utl::any_cast<int>(a) == 12);

#if !UTL_NO_EXCEPTIONS
    CHECK_THROWS_AS(utl::any_cast<float>(a), utl::bad_cast);
#endif

    a = 12.6;

    CHECK(utl::any_cast<double>(a) == 12.6);

    auto p = utl::any_cast<double *>(a);
    CHECK(*p == 12.6);

    const utl::any b = std::string("Hello");
    CHECK(utl::any_cast<std::string>(b) == "Hello");
    CHECK(*utl::any_cast<const std::string *>(b) == "Hello");

    auto x = a;

    auto y = std::move(a);

    a = b;

    y = std::move(a);

    CHECK(utl::any_cast<std::string>(y) == "Hello");

    utl::any xxx;

    utl::any yyy = xxx;

    y = xxx;
}
