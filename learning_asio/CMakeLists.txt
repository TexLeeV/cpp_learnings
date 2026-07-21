# Asio learning suite (standalone Asio — not std::asio; there is no std::asio yet)
#
# Fetches header-only Asio and exposes target `asio::asio`.

include(FetchContent)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
  GIT_SHALLOW TRUE
)

# Asio's repo has no top-level CMakeLists.txt; MakeAvailable only populates sources.
FetchContent_MakeAvailable(asio)

if(NOT TARGET asio::asio)
  add_library(asio INTERFACE)
  add_library(asio::asio ALIAS asio)
  target_include_directories(asio INTERFACE "${asio_SOURCE_DIR}/asio/include")
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
  target_link_libraries(asio INTERFACE Threads::Threads)
endif()

add_learning_test(test_io_context tests/test_io_context.cpp instrumentation asio::asio)
add_learning_test(test_timers tests/test_timers.cpp instrumentation asio::asio)
add_learning_test(test_async_composition tests/test_async_composition.cpp instrumentation asio::asio)
