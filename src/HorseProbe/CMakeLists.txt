cmake_minimum_required(VERSION 3.20)
project(HorseProbe CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_library(HorseProbe SHARED HorseProbe.cpp)
target_compile_features(HorseProbe PRIVATE cxx_std_20)
target_compile_options(HorseProbe PRIVATE /W4 /EHa /utf-8)
set_target_properties(HorseProbe PROPERTIES SUFFIX ".asi")
