# Modern C++ features — grouped by the ISO revision that introduced them.
# The project still builds as C++20; directory names are pedagogical, not toolchain flags.

# C++11
add_learning_test(test_lambdas tests/cpp11/test_lambdas.cpp instrumentation)
add_learning_test(test_auto_type_deduction tests/cpp11/test_auto_type_deduction.cpp instrumentation)
add_learning_test(test_uniform_initialization tests/cpp11/test_uniform_initialization.cpp instrumentation)
add_learning_test(test_delegating_constructors tests/cpp11/test_delegating_constructors.cpp instrumentation)

# C++14
add_learning_test(test_generic_lambdas tests/cpp14/test_generic_lambdas.cpp instrumentation)
add_learning_test(test_auto_return_type tests/cpp14/test_auto_return_type.cpp instrumentation)

# C++17
add_learning_test(test_structured_bindings tests/cpp17/test_structured_bindings.cpp instrumentation)
add_learning_test(test_optional_variant_any tests/cpp17/test_optional_variant_any.cpp instrumentation)
add_learning_test(test_string_view tests/cpp17/test_string_view.cpp instrumentation)
add_learning_test(test_if_constexpr tests/cpp17/test_if_constexpr.cpp instrumentation)

# C++20
add_learning_test(test_concepts tests/cpp20/test_concepts.cpp instrumentation)
add_learning_test(test_span tests/cpp20/test_span.cpp instrumentation)
add_learning_test(test_ranges tests/cpp20/test_ranges.cpp instrumentation)
add_learning_test(test_spaceship_and_designated tests/cpp20/test_spaceship_and_designated.cpp instrumentation)
