#include "doctest.h"

#include <utl/span.hpp>
#include <utl/vector.hpp>

TEST_CASE("dynamic span") {
  int x[] = {1, 2, 4, 5};
  // utl::span<int> si = x;

  utl::span<int> s0;
  CHECK(s0.data() == nullptr);
  CHECK(s0.size() == 0);

  utl::span<int, 0> s1;
  CHECK(s1.data() == nullptr);
  CHECK(s1.size() == 0);

  utl::span<int, 1> s2(x, 1);
  CHECK(s2.size() == 1);
  CHECK(s2.data() == x);

  utl::span<int, 1> s3(x, x + 1);
  CHECK(s3.size() == 1);
  CHECK(s3.data() == x);

  utl::span<int, 4> s4(x);
  CHECK(s4.size() == 4);
  CHECK(s4.data() == x);

  std::array<int, 4> x1{1, 2, 3, 4};

  utl::span<int, 4> s5(x1);
  CHECK(s5.size() == 4);
  CHECK(s5.data() == x1.data());

  utl::span<const int, 4> s6(s5);
  CHECK(s6.data() == s5.data());

  s2 = s3;
  CHECK(s2.data() == s3.data());
  CHECK(s2.size() == s3.size());

  utl::vector<std::string> vec{"hello", "world"};

  utl::span<std::string, 2> s7(vec);
  CHECK(s7.data() == vec.data());
  utl::span<std::string> s8(vec);
  CHECK(s8.data() == vec.data());
  CHECK(s8.size() == 2);

  utl::span<std::string const> s9(s8);
  CHECK(s9.data() == s8.data());
  CHECK(s9.size() == 2);
  CHECK(s9.extent == utl::dynamic_extent);

  s0 = s4;
  CHECK(s0.data() == s4.data());
  CHECK(s0.size() == s4.size());

  CHECK(s0.size() == s4.size());
  CHECK(s0.data() == s4.data());

  CHECK(std::equal(s4.begin(), s4.end(), std::begin(x), std::end(x)));

  auto s10 = s4.first(2);
  CHECK(s10.data() == s4.data());
  CHECK(s10.size() == 2);

  auto s11 = s4.first<2>();
  CHECK(s11.data() == s4.data());
  CHECK(s11.size() == 2);
  CHECK(s11.extent == 2);

  auto s12 = s4.last<1>();
  CHECK(s12.data() == s4.data() + 3);
  CHECK(s12.size() == 1);

  auto s13 = s4.last(2);
  CHECK(s13.data() == s4.data() + 2);
  CHECK(s13.size() == 2);

  auto s14 = s4.subspan(1, 2);
  CHECK(s14.data() == s4.data() + 1);
  CHECK(s14.size() == 2);

  auto s15 = s4.subspan(1);
  CHECK(s15.data() == s4.data() + 1);
  CHECK(s15.size() == 3);

  auto s16 = s4.subspan<1>();
  CHECK(s16.data() == s4.data() + 1);
  CHECK(s16.size() == 3);

  for (size_t i = 0; i != s16.size(); ++i) {
    CHECK(s16[i] == s4[i + 1]);
  }

  auto bytes = utl::as_bytes(s4);
  CHECK(bytes.size() == s4.size_bytes());

  auto wb = utl::as_writable_bytes(s4);
  CHECK(bytes.size() == wb.size());
}
