#include "instrumentation.h"
#include <memory>

void test_shared_from_raw()
{
    Tracked* raw = new Tracked("B");
    std::shared_ptr<Tracked> p1(raw);
    std::shared_ptr<Tracked> p2(raw);
}
