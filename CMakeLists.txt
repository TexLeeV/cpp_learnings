cmake_minimum_required(VERSION 3.14)
project(cpp_learning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddLearningTest)

add_subdirectory(common)
add_subdirectory(examples)
add_subdirectory(profile_showcase)

add_subdirectory(learning_shared_ptr)
add_subdirectory(learning_deadlocks)
add_subdirectory(learning_move_semantics)
add_subdirectory(learning_concurrency)
add_subdirectory(learning_templates)
add_subdirectory(learning_raii)
add_subdirectory(learning_design_patterns)
add_subdirectory(learning_memory)
add_subdirectory(learning_error_handling)
add_subdirectory(learning_stl)
add_subdirectory(learning_modern_cpp)
add_subdirectory(learning_performance)
add_subdirectory(learning_debugging)
