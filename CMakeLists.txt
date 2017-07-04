cmake_minimum_required(VERSION 2.0)
add_library(GlobalGrid GlobalGrid.cpp)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I. -std=c++11 -g")
target_link_libraries(GlobalGrid pthread dl rt)
