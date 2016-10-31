# Copyright (C) 2014-2016 Stichting Mapcode Foundation (http://www.mapcode.com)
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
#
# Compiler directives, only for internal use:
#
# NO_POSIX_THREADS      - No multi-threaded unit testing.
# NO_FAST_ENCODE        - Drop fast encoding support.

set(MAPCODE_OPTIONS "")
set(MAPCODE_WARNING_OPTIONS "-Wall -Werror")
set(MAPCODE_ADDRESS_SANITIZER_COMPILER_OPTIONS "-fsanitize=address -fno-common -fno-optimize-sibling-calls -fno-omit-frame-pointer")
set(MAPCODE_ADDRESS_SANITIZER_LINKER_OPTIONS "-fsanitize=address")

set(CMAKE_C_FLAGS_DEBUG "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} ${MAPCODE_ADDRESS_SANITIZER_COMPILER_OPTIONS} -O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O2 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O0 -g -DDEBUG -std=c++11")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O2 -g -DDEBUG -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O3 -DNDEBUG -std=c++11")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${MAPCODE_ADDRESS_SANITIZER_LINKER_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

set(SOURCE_DOC_FILES
        README.md
        utility/README.md
        unittest/README.md
        docs/logo_CLion.png
        docs/mapcode_library_c.docx
        docs/mapcode_library_c.pdf)

set(SOURCE_FILES_MAPCODELIB
        mapcodelib/internal_data.h
        mapcodelib/internal_alphabet_recognizer.h
        mapcodelib/internal_iso3166_data.h
        mapcodelib/internal_territory_alphabets.h
        mapcodelib/internal_territory_names_english.h
        mapcodelib/internal_territory_names_local.h
        mapcodelib/internal_territory_search.h
        mapcodelib/mapcode_alphabets.h
        mapcodelib/mapcode_legacy.c
        mapcodelib/mapcode_legacy.h
        mapcodelib/mapcode_territories.h
        mapcodelib/mapcoder.c
        mapcodelib/mapcoder.h)

set(SOURCE_FILES_UNITTEST
        unittest/decode_test.h
        unittest/unittest.c)

set(SOURCE_FILES_UTILITY
        utility/mapcode.cpp)

add_library(mapcodelib ${SOURCE_FILES_MAPCODELIB})
target_include_directories(mapcodelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(unittest ${SOURCE_FILES_UNITTEST})
target_link_libraries(unittest LINK_PUBLIC mapcodelib)

add_executable(mapcode ${SOURCE_FILES_UTILITY})
target_link_libraries(mapcode LINK_PUBLIC mapcodelib)
