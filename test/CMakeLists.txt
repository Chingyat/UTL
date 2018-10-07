link_libraries(utl --coverage)
add_compile_options(-Wno-float-equal -Wno-zero-as-null-pointer-constant)
add_executable(tester
               main.cxx
               test_any.cxx
               test_optional.cxx
               test_string.cxx
               test_vector.cxx)
add_test(tester tester)
