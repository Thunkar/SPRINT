cmake_minimum_required(VERSION 3.5.0)
project(sprint)
 
include_directories(include)

file(GLOB SOURCES "src/*.cpp")
 
add_executable(sprint ${SOURCES})