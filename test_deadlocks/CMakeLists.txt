# Deadlock test suite - requires pthread for threading support
# Inherits settings from parent CMakeLists.txt

# Include instrumentation from test_shared_ptr
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../test_shared_ptr/src)

# Test 1: Mutex Ordering Deadlocks
add_executable(test_mutex_ordering_deadlocks
    test_mutex_ordering_deadlocks.cpp
)

target_link_libraries(test_mutex_ordering_deadlocks
    instrumentation
    GTest::GTest
    GTest::Main
    Threads::Threads
)

add_test(NAME MutexOrderingDeadlocksTests COMMAND test_mutex_ordering_deadlocks)

# Test 2: Circular Reference Deadlocks
add_executable(test_circular_reference_deadlocks
    test_circular_reference_deadlocks.cpp
)

target_link_libraries(test_circular_reference_deadlocks
    instrumentation
    GTest::GTest
    GTest::Main
    Threads::Threads
)

add_test(NAME CircularReferenceDeadlocksTests COMMAND test_circular_reference_deadlocks)

# Test 3: Condition Variable Deadlocks
add_executable(test_condition_variable_deadlocks
    test_condition_variable_deadlocks.cpp
)

target_link_libraries(test_condition_variable_deadlocks
    instrumentation
    GTest::GTest
    GTest::Main
    Threads::Threads
)

add_test(NAME ConditionVariableDeadlocksTests COMMAND test_condition_variable_deadlocks)

# Test 4: Ownership Transfer Deadlocks
add_executable(test_ownership_transfer_deadlocks
    test_ownership_transfer_deadlocks.cpp
)

target_link_libraries(test_ownership_transfer_deadlocks
    instrumentation
    GTest::GTest
    GTest::Main
    Threads::Threads
)

add_test(NAME OwnershipTransferDeadlocksTests COMMAND test_ownership_transfer_deadlocks)
