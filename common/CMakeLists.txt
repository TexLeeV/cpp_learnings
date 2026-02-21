add_library(instrumentation STATIC
    src/instrumentation.cpp
)

target_include_directories(instrumentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_library(move_instrumentation STATIC
    src/move_instrumentation.cpp
)

target_include_directories(move_instrumentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(move_instrumentation PUBLIC
    instrumentation
)
