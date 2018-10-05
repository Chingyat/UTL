#include <utl/any.hpp>

namespace utl {

const char *bad_cast::what() const noexcept { return msg.c_str(); }
void any::Value::dummy_vfunc() {}
} // namespace utl
