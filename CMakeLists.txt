cmake_minimum_required(VERSION 3.5)

project(no-tomorrow-clocksynth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(NO_TMRW_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/argparse/include")

add_subdirectory(src "${NO_TMRW_BINARY_DIR}")


