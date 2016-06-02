cmake_minimum_required(VERSION 3.5.0)
project(sprint)
 
include_directories(lib)

file(GLOB SOURCES "src/*.cpp")
 
find_package (Threads)
add_executable(sprint ${SOURCES})
target_link_libraries (sprint ${CMAKE_THREAD_LIBS_INIT})
