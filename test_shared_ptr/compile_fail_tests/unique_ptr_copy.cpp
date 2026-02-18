#include "instrumentation.h"
#include <memory>

void test_unique_ptr_copy()
{
    std::unique_ptr<Tracked> u1(new Tracked("A"));
    std::unique_ptr<Tracked> u2 = u1;
}
