#include <utl/vector.hpp>
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

int main()
{
    test_ctor();
}

