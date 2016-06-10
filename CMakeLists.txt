cmake_minimum_required(VERSION 3.2.0)
project(sprint)

file(GLOB SOURCES "src/*.cpp")
 
add_executable(sprint ${SOURCES})
