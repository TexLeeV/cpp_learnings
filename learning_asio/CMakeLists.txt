# Asio test suite (Boost.Asio) — authored to a C++11 minimum.
#
# Each target is pinned to C++11 (overriding the project-wide C++20) so the
# learner-facing code stays within classic, callback-based Asio idioms. Boost
# is header-only here: Asio and boost::system need no compiled libraries.

find_package(Boost REQUIRED)

set(asio_tests
    test_io_context_basics
    test_timers
    test_post_dispatch_strand
    test_timer_lifetime_uaf
    test_multi_timer_ownership
)

foreach(asio_test ${asio_tests})
    add_learning_test(${asio_test} tests/${asio_test}.cpp instrumentation Boost::headers Threads::Threads)
    set_target_properties(${asio_test} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endforeach()
