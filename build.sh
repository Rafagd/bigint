#!/bin/bash

set -e

mkdir -p cmake_build
cd cmake_build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
ninja
ninja test || cat Testing/Temporary/LastTest.log
#ninja test_memcheck
