#include "doctest.h"

#include <utl/string.hpp>

TEST_SUITE("string")
{
    TEST_CASE("constructors")
    {
        SUBCASE("default constructed string has [0] == '\0'")
        {
            utl::basic_string<wchar_t> ws;
            CHECK(ws[0] == L'\0');

            utl::basic_string<char> cs;
            CHECK(cs[0] == '\0');

            utl::basic_string<char16_t> u16s;
            CHECK(u16s[0] == u'\0');

            utl::basic_string<char32_t> u32s;
            CHECK(u32s[0] == U'\0');
        }
    }
}
