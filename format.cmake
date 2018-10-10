#!/usr/bin/cmake -P

find_package(Git REQUIRED)

find_program(clang_format
             NAMES clang-format-7.0.0 clang-format-7.0 clang-format-7)

find_program(cmake_format NAMES cmake-format)

execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
                OUTPUT_VARIABLE project_root)

string(STRIP ${project_root} project_root)

if(clang_format)
  execute_process(COMMAND ${GIT_EXECUTABLE} clang-format)
  execute_process(COMMAND find ${project_root} -regextype posix-extended
                          "-regex" ".*\\.(h|hpp|hxx|hh|ttc|cpp|cc|cxx|c++)"
                          "!" "-path" "*CMakeFiles*" -print -exec
                          ${clang_format} -i {} ";")
endif()

if(cmake_format)
  execute_process(COMMAND find ${project_root} "(" -name "*.cmake" -o -name
                          "CMakeLists.txt" ")" "!" -path "*CMakeFiles*" -print
                          -exec ${cmake_format} --first-comment-is-literal true
                          -i {} ";")
endif()
