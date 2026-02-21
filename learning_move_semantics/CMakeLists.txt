# cmake_minimum_required(VERSION 3.14)
# project(test_move_semantics LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# enable_testing()

# find_package(GTest REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(move_instrumentation STATIC
    src/instrumentation.cpp
)

target_include_directories(move_instrumentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_rvalue_references
    tests/test_rvalue_references.cpp
)

target_link_libraries(test_rvalue_references
    move_instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME RvalueReferencesTests COMMAND test_rvalue_references)

add_executable(test_move_assignment
    tests/test_move_assignment.cpp
)

target_link_libraries(test_move_assignment
    move_instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME MoveAssignmentTests COMMAND test_move_assignment)

add_executable(test_std_move
    tests/test_std_move.cpp
)

target_link_libraries(test_std_move
    move_instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME StdMoveTests COMMAND test_std_move)

add_executable(test_perfect_forwarding
    tests/test_perfect_forwarding.cpp
)

target_link_libraries(test_perfect_forwarding
    move_instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME PerfectForwardingTests COMMAND test_perfect_forwarding)

add_executable(test_move_only_types
    tests/test_move_only_types.cpp
)

target_link_libraries(test_move_only_types
    move_instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME MoveOnlyTypesTests COMMAND test_move_only_types)
