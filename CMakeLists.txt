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

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

set(SOURCE_FILES
    mapcodelib/basics.h
    mapcodelib/mapcode_alphabets.h
    mapcodelib/mapcode_countrynames.h
    mapcodelib/mapcode_countrynames_short.h
    mapcodelib/mapcode_fast_encode.h
    mapcodelib/mapcode_fastalpha.h
    mapcodelib/mapcode_territory_alphabets.h
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
