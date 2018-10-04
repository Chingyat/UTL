#include <utl/vector.hpp>

#include <algorithm>
#include <cassert>
#include <string>

void test_ctor()
{
    utl::vector<std::string> v1(300, utl::allocator<std::string>());
    const std::string cs = "102345678901234456790";
    utl::vector<std::string> v2(300, cs, utl::allocator<std::string>());
    utl::vector<std::string> v3(std::move(v1));
    utl::vector v4(std::move(v2), utl::allocator<std::string>());
    utl::vector<std::string> v5;
}

void test_assign()
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
    assert(std::equal(std::begin(v1), std::end(v1), std::begin(v2), std::end(v2)));
    v6 = v3;
    assert(std::equal(std::begin(v3), std::end(v3), std::begin(v6), std::end(v6)));
    v3 = std::move(v7);
    assert(std::equal(std::begin(v3), std::end(v3), std::begin(v8), std::end(v8)));
    v8 = std::move(v4);
    assert(std::equal(std::begin(v5), std::end(v5), std::begin(v8), std::end(v8)));
    v5 = {"1", "2", "3", "4", "5"};
    assert(v5.size() == 5);
    v1.assign(v6.begin(), v6.end());
    assert(std::equal(std::begin(v1), std::end(v1), std::begin(v8), std::end(v8)));
    v2.assign(8u, "XXXXXXXXXXX");
    assert(v2.size() == 8);
    assert(std::all_of(v2.begin(), v2.end(), [](const auto &x) { return x == "XXXXXXXXXXX"; }));
    v3.assign({"1", "2", "3", "4", "5"});
    assert(v3.size() == 5);
}

int main()
{
    test_ctor();
    test_assign();
}
