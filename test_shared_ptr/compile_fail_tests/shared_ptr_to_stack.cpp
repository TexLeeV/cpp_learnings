#include "instrumentation.h"
#include <memory>

void test_shared_ptr_to_stack()
{
    Tracked stack_obj("C");
    std::shared_ptr<Tracked> p(&stack_obj);
}
