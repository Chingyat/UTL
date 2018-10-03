link_libraries(utl --coverage)

add_executable(test_vector test_vector.cxx)
add_test(test_vector test_vector)
