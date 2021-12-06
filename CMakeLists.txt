cmake_minimum_required(VERSION 3.0)

project(agc_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(agc_test src/main.cpp)
add_executable(agc_test_neon src/main.cpp)

target_compile_definitions(agc_test_neon PUBLIC BUILD_WITH_NEON)

target_compile_options(agc_test_neon PUBLIC -O3 -mtune=cortex-a72 -mcpu=cortex-a72 -O3 -ftree-vectorize)


install(TARGETS agc_test DESTINATION bin)
install(TARGETS agc_test_neon DESTINATION bin)