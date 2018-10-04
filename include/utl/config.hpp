#pragma once
#include <cstdlib>
#include <cstring>
#include <initializer_list>

#ifndef UTL_NO_EXCEPTIONS
#    define UTL_NO_EXCEPTIONS 0
#endif

#if UTL_NO_EXCEPTIONS

#    define UTL_THROW(...) std::abort()
#    define UTL_RETHROW assert(false)
#    define UTL_TRY
#    define UTL_CATCH(...) if constexpr (0)

#else

#    define UTL_THROW throw
#    define UTL_RETHROW throw
#    define UTL_TRY try
#    define UTL_CATCH catch

#endif

namespace utl {

using std::initializer_list;
}
