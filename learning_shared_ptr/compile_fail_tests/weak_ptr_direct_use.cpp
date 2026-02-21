#include "instrumentation.h"
#include <memory>

void test_weak_ptr_direct_use()
{
    std::shared_ptr<Tracked> shared(new Tracked("D"));
    std::weak_ptr<Tracked> weak = shared;
    
    Tracked& ref = *weak;
}
