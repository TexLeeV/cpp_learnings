#include "instrumentation.h"
#include <memory>

class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

void test_deleted_copy_constructor()
{
    NonCopyable obj;
    std::shared_ptr<NonCopyable> p1 = std::make_shared<NonCopyable>(obj);
}
