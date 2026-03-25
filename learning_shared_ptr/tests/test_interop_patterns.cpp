#include "instrumentation.h"

#include <cstdio>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>

class InteropPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

struct FileCloser
{
    void operator()(FILE* file) const
    {
        if (file)
        {
            EventLog::instance().record("FileCloser::operator() closing FILE*");
            std::fclose(file);
        }
    }
};

TEST_F(InteropPatternsTest, FileHandleCustomDeleter)
{
    {
        FILE* file = std::tmpfile();

        std::shared_ptr<FILE> file_ptr(file, FileCloser());

        long use_count = file_ptr.use_count();

        bool not_null = (file_ptr.get() != nullptr);

        EXPECT_EQ(use_count, 1);
        EXPECT_TRUE(not_null);
    }

    auto events = EventLog::instance().events();
    bool closer_called = false;

    for (const auto& event : events)
    {
        if (event.find("FileCloser::operator()") != std::string::npos)
        {
            closer_called = true;
        }
    }
    // Q: What guarantees that `FileCloser::operator()` is called when the scope exits? What would happen if you used
    // `delete` as the deleter instead? A: R:

    EXPECT_TRUE(closer_called);
}

struct BufferDeleter
{
    void operator()(char* buffer) const
    {
        if (buffer)
        {
            EventLog::instance().record("BufferDeleter::operator() freeing buffer");
            std::free(buffer);
        }
    }
};

TEST_F(InteropPatternsTest, MallocBufferCustomDeleter)
{
    {
        char* buffer = static_cast<char*>(std::malloc(1024));

        std::shared_ptr<char> buffer_ptr(buffer, BufferDeleter());

        long use_count = buffer_ptr.use_count();

        bool not_null = (buffer_ptr.get() != nullptr);

        EXPECT_EQ(use_count, 1);
        EXPECT_TRUE(not_null);
    }

    auto events = EventLog::instance().events();
    bool deleter_called = false;

    for (const auto& event : events)
    {
        if (event.find("BufferDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
    }
    // Q: Why is a custom deleter necessary for malloc-allocated memory? What would happen if shared_ptr used its
    // default deleter? A: R:

    EXPECT_TRUE(deleter_called);
}

void c_api_function(Tracked* raw_ptr)
{
    if (raw_ptr)
    {
        EventLog::instance().record("c_api_function called with raw pointer");
    }
}

TEST_F(InteropPatternsTest, SafeGetUsageForCAPI)
{
    auto shared = std::make_shared<Tracked>("Shared");

    long before_call = shared.use_count();

    c_api_function(shared.get());

    long after_call = shared.use_count();
    // Q: Why does `use_count` remain at 1 before and after the C API call? What guarantee must hold for this pattern to
    // be safe? A: R:

    auto events = EventLog::instance().events();
    bool api_called = false;

    for (const auto& event : events)
    {
        if (event.find("c_api_function") != std::string::npos)
        {
            api_called = true;
        }
    }

    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    EXPECT_TRUE(api_called);
}

struct ResourceHandle
{
    int fd;
    explicit ResourceHandle(int descriptor) : fd(descriptor)
    {
        EventLog::instance().record("ResourceHandle created");
    }

    ~ResourceHandle()
    {
        EventLog::instance().record("ResourceHandle destroyed");
    }
};

struct ResourceHandleDeleter
{
    void operator()(ResourceHandle* handle) const
    {
        EventLog::instance().record("ResourceHandleDeleter::operator() closing handle");
        delete handle;
    }
};

TEST_F(InteropPatternsTest, RAIIWrapperForCResource)
{
    {
        std::shared_ptr<ResourceHandle> handle(new ResourceHandle(42), ResourceHandleDeleter());

        long use_count = handle.use_count();

        EXPECT_EQ(use_count, 1);
    }

    auto events = EventLog::instance().events();
    bool handle_created = false;
    bool handle_destroyed = false;
    bool deleter_called = false;

    for (const auto& event : events)
    {
        if (event.find("ResourceHandle created") != std::string::npos)
        {
            handle_created = true;
        }
        if (event.find("ResourceHandle destroyed") != std::string::npos)
        {
            handle_destroyed = true;
        }
        if (event.find("ResourceHandleDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
    }
    // Q: What is the order of EventLog entries? Which appears first: "ResourceHandleDeleter::operator()" or
    // "ResourceHandle destroyed"? A: R:

    EXPECT_TRUE(handle_created);
    EXPECT_TRUE(handle_destroyed);
    EXPECT_TRUE(deleter_called);
}

Tracked* create_tracked_c_style(const char* name)
{
    return new Tracked(name);
}

void destroy_tracked_c_style(Tracked* ptr)
{
    EventLog::instance().record("destroy_tracked_c_style called");
    delete ptr;
}

TEST_F(InteropPatternsTest, BridgingCAndCppOwnership)
{
    {
        Tracked* raw = create_tracked_c_style("Bridged");

        std::shared_ptr<Tracked> cpp_owned(raw, [](Tracked* ptr) { destroy_tracked_c_style(ptr); });

        long use_count = cpp_owned.use_count();
        // Q: Why is a lambda deleter necessary here? What would happen if you used the default deleter?
        // A:
        // R:

        EXPECT_EQ(use_count, 1);
    }

    auto events = EventLog::instance().events();
    bool c_destroy_called = false;

    for (const auto& event : events)
    {
        if (event.find("destroy_tracked_c_style") != std::string::npos)
        {
            c_destroy_called = true;
        }
    }

    EXPECT_TRUE(c_destroy_called);
}

TEST_F(InteropPatternsTest, NullptrSafetyInCAPI)
{
    std::shared_ptr<Tracked> null_shared;

    Tracked* raw = null_shared.get();

    bool is_null = (raw == nullptr);

    long use_count = null_shared.use_count();
    // Q: Why does a default-constructed shared_ptr return `use_count() == 0` instead of 1? What does this reveal about
    // its internal state? A: R:

    EXPECT_TRUE(is_null);
    EXPECT_EQ(use_count, 0);
}

struct CStyleArray
{
    Tracked* elements;
    size_t count;

    explicit CStyleArray(size_t n) : elements(nullptr), count(n)
    {
        elements = new Tracked[3]{Tracked("E1"), Tracked("E2"), Tracked("E3")};
        EventLog::instance().record("CStyleArray allocated");
    }
};

struct CStyleArrayDeleter
{
    void operator()(CStyleArray* arr) const
    {
        if (arr)
        {
            EventLog::instance().record("CStyleArrayDeleter::operator() freeing array");
            delete[] arr->elements;
            delete arr;
        }
    }
};

TEST_F(InteropPatternsTest, CStyleArrayWrapping)
{
    {
        std::shared_ptr<CStyleArray> arr(new CStyleArray(3), CStyleArrayDeleter());

        long use_count = arr.use_count();
        size_t count = arr->count;

        EXPECT_EQ(use_count, 1);
        EXPECT_EQ(count, 3);
    }

    auto events = EventLog::instance().events();
    bool array_allocated = false;
    bool array_freed = false;

    for (const auto& event : events)
    {
        if (event.find("CStyleArray allocated") != std::string::npos)
        {
            array_allocated = true;
        }
        if (event.find("CStyleArrayDeleter::operator()") != std::string::npos)
        {
            array_freed = true;
        }
    }
    // Q: The custom deleter calls both `delete[] arr->elements` and `delete arr`. Why are two delete operations
    // necessary? A: R:

    EXPECT_TRUE(array_allocated);
    EXPECT_TRUE(array_freed);
}

TEST_F(InteropPatternsTest, GetWithTemporarySharedPtr)
{
    Tracked* raw = nullptr;

    {
        auto temp = std::make_shared<Tracked>("Temp");

        raw = temp.get();
    }

    bool would_be_dangling = true;
    // Q: After the scope exits, what state is `raw` in? What observable signal would confirm the Tracked object was
    // destroyed? A: R:

    EXPECT_TRUE(would_be_dangling);
}

void c_callback(void* user_data)
{
    if (user_data)
    {
        Tracked* tracked = static_cast<Tracked*>(user_data);
        EventLog::instance().record("c_callback invoked with user_data");
    }
}

TEST_F(InteropPatternsTest, PassingRawPointerToCallback)
{
    auto shared = std::make_shared<Tracked>("Shared");

    c_callback(shared.get());

    long use_count = shared.use_count();
    // Q: What assumption must hold about `c_callback`'s behavior for this pattern to be safe? What would break if the
    // callback stored the pointer? A: R:

    auto events = EventLog::instance().events();
    bool callback_invoked = false;

    for (const auto& event : events)
    {
        if (event.find("c_callback invoked") != std::string::npos)
        {
            callback_invoked = true;
        }
    }

    EXPECT_EQ(use_count, 1);
    EXPECT_TRUE(callback_invoked);
}

TEST_F(InteropPatternsTest, SharedPtrFromGetDangerousPattern)
{
    auto original = std::make_shared<Tracked>("Original");

    Tracked* raw = original.get();
    // Q: If you created `std::shared_ptr<Tracked> another(raw)`, what would `original.use_count()` return and why?
    // A:
    // R:

    long original_count = original.use_count();

    EXPECT_EQ(original_count, 1);
}
