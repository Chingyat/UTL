link_libraries(utl --coverage)

add_executable(tester main.cxx test_any.cxx test_vector.cxx)
add_test(tester tester)
