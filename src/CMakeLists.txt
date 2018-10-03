add_executable(example1 example1.cxx)
target_link_libraries(example1 PRIVATE utl --coverage)

add_test(example1 example1)