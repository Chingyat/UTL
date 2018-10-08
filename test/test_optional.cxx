#include "doctest.h"

#include <utl/optional.hpp>

template class utl::optional<int>;

TEST_CASE("optional")
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
#if !UTL_NO_EXCEPTIONS
    CHECK_THROWS_AS(y.value(), utl::bad_optional_access);
#endif
    CHECK(y.value_or(1.0) == 1);

    auto get_optional = [] {
        utl::optional o = 'a';
        return o;
    };

    CHECK(get_optional().value() == 'a');

    utl::optional<int> default_construction;
}
