#include <utl/optional.hpp>

namespace utl {

const char *bad_optional_access::what() const noexcept {
  return "deferencing null optional object";
}
} // namespace utl
