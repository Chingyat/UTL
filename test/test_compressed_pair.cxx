#include "doctest.h"
#include <iostream>

#include <utl/compressed_pair.hpp>

struct A {};
struct B {};

TEST_CASE("test compressed pair") {
  utl::compressed_pair<A, B> p1;
  utl::compressed_pair<B, char> p2;
  utl::compressed_pair<char, A> p3;

  p1.first();
  p2.first();
  p1.second();
  p2.second();

  CHECK(sizeof(p1) == 1);
  CHECK(sizeof(p2) == 1);
}
