#include <utl/any.hpp>

namespace utl {
	const char* bad_any_cast::what() const noexcept
	{
		return "bad_any_cast";
	}
} // namespace utl
