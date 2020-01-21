#include "doctest.h"
#include <cxxabi.h>
#include <iostream>
#include <memory>
#include <utl/unique_ptr.hpp>

struct A {};

struct B : A {};

TEST_CASE("test unique_ptr") {
  const utl::unique_ptr<int> p1;
  const utl::unique_ptr<int> p2(nullptr);
  utl::unique_ptr<int> p3(new int(3));

  utl::unique_ptr<A> p4;
  utl::unique_ptr<B> p5{new B};
  utl::unique_ptr<A> p6 = std::move(p5);

  CHECK(!p4);
  CHECK(p4 == nullptr);
  CHECK(p5 == nullptr);
  CHECK(p6 != nullptr);
  CHECK(p6);

  utl::unique_ptr<int, void (*)(int *)> p7(new int, [](int *x) { delete x; });
  utl::unique_ptr<int, void (*)(int *)> p8(new int, [](int *x) { delete x; });
  
  using std::swap;

  swap(p7, p8);

  //   utl::unique_ptr<int, void (*)(int *)> p8(new int);
}
