#!/bin/bash
# Copyright (c) 2018 Chingyat <tsingyat@outlook.com>


set -euo pipefail
trap "echo 'error: Script failed '" ERR

toplevel=$(git rev-parse --show-toplevel)

find ${toplevel}/include -type f -name \*.hpp -print -exec clang-format -i {} \;
find ${toplevel}/src -type f -name \*.cpp -print -exec clang-format -i {} \;
find ${toplevel}/test -type f -name \*.cxx -print -exec clang-format -i {} \;

find ${toplevel}/ -type f -name CMakeLists.txt -print -exec cmake-format -i {} \;
find ${toplevel}/cmake -type f -name \*.cmake -print -exec cmake-format -i {} \;
