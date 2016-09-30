cmake_minimum_required(VERSION 3.3)
project(mapcode_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    mapcodelib/basics.h
    mapcodelib/mapcode_countrynames.h
    mapcodelib/mapcode_countrynames_short.h
    mapcodelib/mapcode_fast_encode.h
    mapcodelib/mapcode_fastalpha.h
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

add_executable(all ${SOURCE_FILES})

add_executable(unittest ${SOURCE_FILES_UNITTEST})

add_executable(mapcode ${SOURCE_FILES_UTILITY})
