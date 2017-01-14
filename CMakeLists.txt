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
# Compiler directives (for internal use only):
#
# NO_POSIX_THREADS      - No multi-threaded unit testing - only effective for unit test.
# NO_FAST_ENCODE        - Drop fast encoding support - only for internal use.

set(MAPCODE_OPTIONS "")
set(MAPCODE_WARNING_OPTIONS "-Wall -Werror -Wextra")
set(MAPCODE_SANITIZER_COMPILER_OPTIONS "-fsanitize=address -fno-common -fno-optimize-sibling-calls -fno-omit-frame-pointer")
set(MAPCODE_SANITIZER_LINKER_OPTIONS "-fsanitize=address")

set(CMAKE_C_FLAGS_DEBUG "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} ${MAPCODE_SANITIZER_COMPILER_OPTIONS} -O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} ${MAPCODE_SANITIZER_COMPILER_OPTIONS} -O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} ${MAPCODE_SANITIZER_COMPILER_OPTIONS} -O0 -g -DDEBUG -std=c++11")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} ${MAPCODE_SANITIZER_COMPILER_OPTIONS} -O2 -g -DNDEBUG -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${MAPCODE_OPTIONS} ${MAPCODE_WARNING_OPTIONS} -O3 -DNDEBUG -std=c++11")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${MAPCODE_SANITIZER_LINKER_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${MAPCODE_SANITIZER_LINKER_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

set(SOURCE_FILES_MAPCODELIB
        mapcodelib/internal_data.h
        mapcodelib/internal_alphabet_recognizer.h
        mapcodelib/internal_iso3166_data.h
        mapcodelib/internal_territory_alphabets.h
        mapcodelib/internal_territory_names_af.h
        mapcodelib/internal_territory_names_ar.h
        mapcodelib/internal_territory_names_be.h
        mapcodelib/internal_territory_names_cn.h
        mapcodelib/internal_territory_names_cs.h
        mapcodelib/internal_territory_names_da.h
        mapcodelib/internal_territory_names_de.h
        mapcodelib/internal_territory_names_en.h
        mapcodelib/internal_territory_names_es.h
        mapcodelib/internal_territory_names_fi.h
        mapcodelib/internal_territory_names_fr.h
        mapcodelib/internal_territory_names_he.h
        mapcodelib/internal_territory_names_hi.h
        mapcodelib/internal_territory_names_hr.h
        mapcodelib/internal_territory_names_id.h
        mapcodelib/internal_territory_names_it.h
        mapcodelib/internal_territory_names_ja.h
        mapcodelib/internal_territory_names_ko.h
        mapcodelib/internal_territory_names_local.h
        mapcodelib/internal_territory_names_nl.h
        mapcodelib/internal_territory_names_no.h
        mapcodelib/internal_territory_names_pl.h
        mapcodelib/internal_territory_names_pt.h
        mapcodelib/internal_territory_names_ru.h
        mapcodelib/internal_territory_names_sv.h
        mapcodelib/internal_territory_names_sw.h
        mapcodelib/internal_territory_names_tr.h
        mapcodelib/internal_territory_names_uk.h
        mapcodelib/internal_territory_search.h
        mapcodelib/mapcode_alphabets.h
        mapcodelib/mapcode_legacy.c
        mapcodelib/mapcode_legacy.h
        mapcodelib/mapcode_territories.h
        mapcodelib/mapcoder.c
        mapcodelib/mapcoder.h)

set(SOURCE_FILES_TEST
        test/decode_test.h
        test/unittest.c)

set(SOURCE_FILES_UTILITY
        utility/mapcode.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_library(M_LIB m)

add_library(mapcodelib ${SOURCE_FILES_MAPCODELIB})
target_include_directories(mapcodelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mapcodelib Threads::Threads)
target_link_libraries(mapcodelib ${M_LIB})

add_executable(unittest ${SOURCE_FILES_TEST})
target_link_libraries(unittest LINK_PUBLIC mapcodelib)
target_link_libraries(unittest LINK_PUBLIC Threads::Threads)
target_link_libraries(unittest LINK_PUBLIC ${M_LIB})

add_executable(mapcode ${SOURCE_FILES_UTILITY})
target_link_libraries(mapcode LINK_PUBLIC mapcodelib)
target_link_libraries(mapcode LINK_PUBLIC Threads::Threads)
target_link_libraries(mapcode LINK_PUBLIC ${M_LIB})

install(TARGETS mapcode DESTINATION /usr/local/bin)