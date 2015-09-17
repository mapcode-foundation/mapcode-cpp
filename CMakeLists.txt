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
    unitttest/decode_test.h
    unitttest/test_territories.c
    unitttest/unittest.c
    utility/mapcode.cpp)

add_executable(mapcode_cpp ${SOURCE_FILES})