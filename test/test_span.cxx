#include "doctest.h"

#include <utl/span.hpp>

TEST_CASE("dynamic span")
{
    int x[] = {1, 2, 4, 5};
    utl::span<int> si = x;

    CHECK(std::equal(si.begin(), si.end(), std::begin(x), std::end(x));
}
