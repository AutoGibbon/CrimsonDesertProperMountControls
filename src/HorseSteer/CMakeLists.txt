cmake_minimum_required(VERSION 3.20)
project(HorseSteer C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(SAFETYHOOK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/safetyhook")

add_library(HorseSteer SHARED
    HorseSteer.cpp
    "${SAFETYHOOK_DIR}/safetyhook.cpp"
    "${SAFETYHOOK_DIR}/Zydis.c")
target_include_directories(HorseSteer PRIVATE "${SAFETYHOOK_DIR}")
target_compile_features(HorseSteer PRIVATE cxx_std_23)
target_compile_options(HorseSteer PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/EHa /utf-8>)
set_target_properties(HorseSteer PROPERTIES SUFFIX ".asi")
