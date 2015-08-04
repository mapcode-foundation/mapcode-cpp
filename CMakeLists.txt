cmake_minimum_required(VERSION 3.2)
project(mapcode_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    mapcodelib/basics.h
    mapcodelib/dividemaps.h
    mapcodelib/mapcode_countrynames.h
    mapcodelib/mapcode_countrynames_short.h
    mapcodelib/mapcoder.c
    mapcodelib/mapcoder.h
    utility/mapcode.cpp)

add_executable(mapcode_cpp ${SOURCE_FILES})