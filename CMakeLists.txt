cmake_minimum_required(VERSION 3.23)
project(hello_signal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
enable_testing()

find_package(Boost REQUIRED system thread)

add_executable(main main.cpp)

add_subdirectory(test_shared_ptr)