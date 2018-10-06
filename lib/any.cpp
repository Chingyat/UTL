#include <utl/any.hpp>

namespace utl {

const char *bad_cast::what() const noexcept { return msg.c_str(); }

} // namespace utl
