cmake_minimum_required(VERSION 3.10)
project(HMCrypt)

execute_process(
    COMMAND git describe --tags --always
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_definitions(VERSION_STR="${GIT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hmcrypt main.cpp)

