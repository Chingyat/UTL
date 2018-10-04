link_libraries(utl --coverage)

add_executable(test_vector main.cxx test_vector.cxx)
add_test(test_vector test_vector)
