# Copyright (C) 2014-2015 Stichting Mapcode Foundation (http://www.mapcode.com)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.3)
project(mapcode_cpp)

# The debug configuration adds the "address sanitizer" to check for out of bounds behavior and such.
# You may wish to set the following environment variables during runtime as well:
#
# export ASAN_OPTIONS=debug=true:strict_string_checks=1:detect_stack_use_after_return=true:detect_invalid_pointer_pairs=99999:
#   detect_container_overflow=true:detect_odr_violation=2:check_initialization_order=true

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DNO_POSIX_THREADS -DDEBUG -Wall -fsanitize=address -fno-common -fno-optimize-sibling-calls -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")

set(SOURCE_FILES
        mapcodelib/basics.h
        mapcodelib/mapcode_alphabets.h
        mapcodelib/mapcode_countrynames.h
        mapcodelib/mapcode_countrynames_local.h
        mapcodelib/mapcode_countrynames_short.h
        mapcodelib/mapcode_fast_encode.h
        mapcodelib/mapcode_fastalpha.h
        mapcodelib/mapcode_legacy.h
        mapcodelib/mapcode_territory_alphabets.h
        mapcodelib/mapcode_territories.h
        mapcodelib/mapcoder.c
        mapcodelib/mapcoder.h
        unittest/decode_test.h
        unittest/test_territories.c
        unittest/unittest.c
        utility/mapcode.cpp)

set(SOURCE_FILES_UNITTEST
        unittest/unittest.c)

set(SOURCE_FILES_UTILITY
        utility/mapcode.cpp)

add_executable(fullset ${SOURCE_FILES})

add_executable(unittest ${SOURCE_FILES_UNITTEST})

add_executable(mapcode ${SOURCE_FILES_UTILITY})
