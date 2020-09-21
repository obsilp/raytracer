cmake_minimum_required(VERSION 3.16)
project(raytracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-pthread -O2")

find_package(glm REQUIRED)

file(
        GLOB_RECURSE SOURCE_FILES
        lib/*.h
        lib/*.cpp
        src/*.h
        src/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(
        "lib"
        "src"
)
