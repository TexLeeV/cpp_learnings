function(add_learning_test TEST_NAME SOURCE_FILE)
    add_executable(${TEST_NAME} ${SOURCE_FILE})
    target_link_libraries(${TEST_NAME}
        GTest::gtest
        GTest::gtest_main
        ${ARGN}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()
