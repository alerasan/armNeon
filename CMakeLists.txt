cmake_minimum_required(VERSION 3.0)

project(agc_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(agc_test src/main.cpp)

add_compile_options(-mfpu=neon mtune=cortex-a53)

install(TARGETS agc_test DESTINATION bin)
