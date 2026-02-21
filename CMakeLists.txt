cmake_minimum_required(VERSION 3.14)
project(cpp_learning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Optional: Boost for main.cpp if needed
find_package(Boost COMPONENTS system thread)
if(Boost_FOUND)
    add_executable(main main.cpp)
    target_link_libraries(main Boost::system Boost::thread)
endif()

# Add all test module subdirectories
add_subdirectory(test_shared_ptr)
add_subdirectory(test_deadlocks)
add_subdirectory(test_move_semantics)
add_subdirectory(test_concurrency)
add_subdirectory(test_templates)
add_subdirectory(test_raii)
add_subdirectory(test_design_patterns)
add_subdirectory(test_memory)
add_subdirectory(test_error_handling)
add_subdirectory(test_stl)
add_subdirectory(test_modern_cpp)
add_subdirectory(test_performance)
add_subdirectory(test_debugging)