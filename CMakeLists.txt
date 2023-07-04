cmake_minimum_required(VERSION 3.20)


project(implementation VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(generator)



