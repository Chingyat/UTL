#include "doctest.h"

#include <utl/any.hpp>

TEST_CASE("any")
{
    utl::any a = 12;

    int *i = utl::any_cast<int *>(a);
    CHECK(*i == 12);

    CHECK(utl::any_cast<int>(a) == 12);
}
