cmake_minimum_required(VERSION 3.25.2)
project(notmrw_src LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_executable(test Main.cpp)

install(TARGETS test  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
