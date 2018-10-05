cmake_minimum_required(VERSION 3.4)

project(UTL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UTL_NO_EXCEPTIONS "use  -fno-exceptions" OFF)

if(CMAKE_COMPILER_IS_GNUCC)

endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_compile_options(-Weverything
                      -Wno-c++98-compat
                      -Wno-c++98-compat-pedantic
                      -Wno-sign-conversion
                      -Wno-missing-prototypes
                      -Wno-shadow-field-in-constructor
                      -Wno-padded
                      -ferror-limit=10)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

  if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0)
  endif()
  add_compile_options(-Wall -Wextra -pedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  add_compile_options(/W4)
  if(UTL_NO_EXCEPTIONS)
    add_compile_options( /EH-)
  endif()
endif()

if(UNIX)
  if(UTL_NO_EXCEPTIONS)
    add_compile_options(-fno-exceptions
                        -Wno-return-type
                        -Wno-empty-body)
  endif()
endif()

add_library(utl lib/any.cpp)
target_include_directories(utl PUBLIC include)
target_compile_features(utl PUBLIC cxx_std_17)

if(UTL_NO_EXCEPTIONS)
  target_compile_definitions(utl INTERFACE UTL_NO_EXCEPTIONS=1)
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
