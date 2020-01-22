#include "doctest.h"

#include <string>
#include <utl/utility.hpp>

TEST_CASE("test pair") {
  utl::pair<std::string, std::string> p1;
  CHECK(p1.first == "");
  CHECK(p1.second == "");

  std::string hello("hello");
  std::string world("world");

  utl::pair<std::string, std::string> p2(hello, world);
  CHECK(p2.first == hello);
  CHECK(p2.second == world);

  utl::pair<std::string, std::string> p3(std::move(hello), std::move(world));
  CHECK(p3.first == "hello");
  CHECK(p3.second == "world");

  utl::pair<std::string, std::string> p4(p3);
  CHECK(p4.first == p3.first);
  CHECK(p4.second == p3.second);

  p4 = p1;
  CHECK(p4.first == p1.first);
  CHECK(p4.second == p1.second);

  utl::pair<std::unique_ptr<int>, int> p5;
  auto p6 = std::move(p5);
  p5 = std::move(p6);
}
