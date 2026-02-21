include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(instrumentation STATIC
    src/instrumentation.cpp
)

target_include_directories(instrumentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_reference_counting
    tests/test_reference_counting.cpp
)

target_link_libraries(test_reference_counting
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME ReferenceCountingTests COMMAND test_reference_counting)

add_executable(test_deallocation_lifetime
    tests/test_deallocation_lifetime.cpp
)

target_link_libraries(test_deallocation_lifetime
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME DeallocationLifetimeTests COMMAND test_deallocation_lifetime)

add_executable(test_aliasing_weak
    tests/test_aliasing_weak.cpp
)

target_link_libraries(test_aliasing_weak
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME AliasingWeakTests COMMAND test_aliasing_weak)

add_executable(test_anti_patterns
    tests/test_anti_patterns.cpp
)

target_link_libraries(test_anti_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME AntiPatternTests COMMAND test_anti_patterns)

add_executable(test_smart_pointer_contrast
    tests/test_smart_pointer_contrast.cpp
)

target_link_libraries(test_smart_pointer_contrast
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME SmartPointerContrastTests COMMAND test_smart_pointer_contrast)

add_executable(test_ownership_patterns
    tests/test_ownership_patterns.cpp
)

target_link_libraries(test_ownership_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME OwnershipPatternsTests COMMAND test_ownership_patterns)

add_executable(test_allocation_patterns
    tests/test_allocation_patterns.cpp
)

target_link_libraries(test_allocation_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME AllocationPatternsTests COMMAND test_allocation_patterns)

add_executable(test_structural_patterns
    tests/test_structural_patterns.cpp
)

target_link_libraries(test_structural_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME StructuralPatternsTests COMMAND test_structural_patterns)

add_executable(test_collection_patterns
    tests/test_collection_patterns.cpp
)

target_link_libraries(test_collection_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME CollectionPatternsTests COMMAND test_collection_patterns)

add_executable(test_scope_lifetime_patterns
    tests/test_scope_lifetime_patterns.cpp
)

target_link_libraries(test_scope_lifetime_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME ScopeLifetimePatternsTests COMMAND test_scope_lifetime_patterns)

add_executable(test_self_reference_patterns
    tests/test_self_reference_patterns.cpp
)

target_link_libraries(test_self_reference_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME SelfReferencePatternsTests COMMAND test_self_reference_patterns)

add_executable(test_polymorphism_patterns
    tests/test_polymorphism_patterns.cpp
)

target_link_libraries(test_polymorphism_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME PolymorphismPatternsTests COMMAND test_polymorphism_patterns)

add_executable(test_singleton_registry
    tests/test_singleton_registry.cpp
)

target_link_libraries(test_singleton_registry
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME SingletonRegistryTests COMMAND test_singleton_registry)

add_executable(test_interop_patterns
    tests/test_interop_patterns.cpp
)

target_link_libraries(test_interop_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME InteropPatternsTests COMMAND test_interop_patterns)

add_executable(test_conditional_lifetime
    tests/test_conditional_lifetime.cpp
)

target_link_libraries(test_conditional_lifetime
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME ConditionalLifetimeTests COMMAND test_conditional_lifetime)

add_executable(test_exercises_fill_in
    tests/test_exercises_fill_in.cpp
)

target_link_libraries(test_exercises_fill_in
    instrumentation
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME FillInExercisesTests COMMAND test_exercises_fill_in)

add_executable(test_asio_basics
    tests/test_asio_basics.cpp
)

# Asio is header-only, just need to find the include path
find_path(ASIO_INCLUDE_DIR_BASICS asio.hpp
    PATHS /usr/include /usr/local/include
)

if(ASIO_INCLUDE_DIR_BASICS)
    target_include_directories(test_asio_basics PRIVATE ${ASIO_INCLUDE_DIR_BASICS})
    target_compile_definitions(test_asio_basics PRIVATE ASIO_STANDALONE)
endif()

target_link_libraries(test_asio_basics
    instrumentation
    GTest::gtest
    GTest::gtest_main
    pthread
)

add_test(NAME AsioBasicsTests COMMAND test_asio_basics)

add_executable(test_multi_threaded_patterns
    tests/test_multi_threaded_patterns.cpp
)

# Asio is header-only, just need to find the include path
find_path(ASIO_INCLUDE_DIR asio.hpp
    PATHS /usr/include /usr/local/include
)

if(ASIO_INCLUDE_DIR)
    target_include_directories(test_multi_threaded_patterns PRIVATE ${ASIO_INCLUDE_DIR})
    target_compile_definitions(test_multi_threaded_patterns PRIVATE ASIO_STANDALONE)
endif()

target_link_libraries(test_multi_threaded_patterns
    instrumentation
    GTest::gtest
    GTest::gtest_main
    pthread
)

add_test(NAME MultiThreadedPatternsTests COMMAND test_multi_threaded_patterns)
