#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <cstdio>
#include <cstring>

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
        
        // TODO: Create shared_ptr<FILE> with FileCloser deleter
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count
        // use_count = ???
        
        bool not_null = false;
        // TODO: Check if file_ptr.get() is not nullptr
        // not_null = ???
        
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
        
        // TODO: Create shared_ptr<char> with BufferDeleter
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count
        // use_count = ???
        
        bool not_null = false;
        // TODO: Check if buffer_ptr.get() is not nullptr
        // not_null = ???
        
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
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    long before_call = 0;
    // TODO: Get use_count before call
    // before_call = ???
    
    // TODO: Call c_api_function with shared.get()
    // YOUR CODE HERE
    
    long after_call = 0;
    // TODO: Get use_count after call
    // after_call = ???
    
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
    explicit ResourceHandle(int descriptor)
    : fd(descriptor)
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
        // TODO: Create shared_ptr<ResourceHandle> with ResourceHandleDeleter
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count
        // use_count = ???
        
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
        
        // TODO: Create shared_ptr with lambda deleter that calls destroy_tracked_c_style
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count
        // use_count = ???
        
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
    // TODO: Create empty shared_ptr
    // YOUR CODE HERE
    
    // TODO: Get raw pointer from null_shared
    // YOUR CODE HERE
    
    bool is_null = false;
    // TODO: Check if raw is nullptr
    // is_null = ???
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_TRUE(is_null);
    EXPECT_EQ(use_count, 0);
}

struct CStyleArray
{
    Tracked* elements;
    size_t count;
    
    explicit CStyleArray(size_t n)
    : elements(nullptr)
    , count(n)
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
        // TODO: Create shared_ptr<CStyleArray> with CStyleArrayDeleter
        // YOUR CODE HERE
        
        long use_count = 0;
        size_t count = 0;
        // TODO: Get use_count and arr->count
        // use_count = ???
        // count = ???
        
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
    
    EXPECT_TRUE(array_allocated);
    EXPECT_TRUE(array_freed);
}

TEST_F(InteropPatternsTest, GetWithTemporarySharedPtr)
{
    Tracked* raw = nullptr;
    
    {
        // TODO: Create temporary shared_ptr
        // YOUR CODE HERE
        
        // TODO: Get raw pointer from temp
        // raw = ???
    }
    
    // WARNING: raw is now dangling!
    bool would_be_dangling = true;
    
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
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    // TODO: Call c_callback with shared.get()
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
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
    // TODO: Create original shared_ptr
    // YOUR CODE HERE
    
    // TODO: Get raw pointer
    // YOUR CODE HERE
    
    // WARNING: DO NOT create another shared_ptr from raw!
    // That would create two independent control blocks
    
    long original_count = 0;
    // TODO: Get use_count
    // original_count = ???
    
    EXPECT_EQ(original_count, 1);
}
