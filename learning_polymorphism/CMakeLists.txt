# Polymorphism test suite

add_learning_test(test_virtual_dispatch     tests/test_virtual_dispatch.cpp     instrumentation)
add_learning_test(test_virtual_destructors  tests/test_virtual_destructors.cpp  instrumentation)
add_learning_test(test_abstract_interfaces  tests/test_abstract_interfaces.cpp  instrumentation)
add_learning_test(test_multiple_inheritance tests/test_multiple_inheritance.cpp instrumentation)
add_learning_test(test_rtti_and_casts       tests/test_rtti_and_casts.cpp       instrumentation)
add_learning_test(test_static_polymorphism  tests/test_static_polymorphism.cpp  instrumentation)
