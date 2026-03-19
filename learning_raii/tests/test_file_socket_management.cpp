// Test Suite: File Handle and Socket Management
// Estimated Time: 2 hours
// Difficulty: Easy

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>

class FileSocketManagementTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
    
    void TearDown() override
    {
        std::remove("test_file.txt");
    }
};

// ============================================================================
// Scenario 1: File Handle RAII with std::fstream (Easy)
// ============================================================================

TEST_F(FileSocketManagementTest, FileHandleRAII)
{
    // Q: What resource does std::fstream manage?
    // A:
    // R:
    
    {
        std::ofstream file("test_file.txt");
        
        // Q: When is the file opened?
        // A:
        // R:
        
        EXPECT_TRUE(file.is_open());
        
        file << "RAII test content\n";
        
        // Q: When will the file be closed?
        // A:
        // R:
    }
    
    // Q: Is the file closed now?
    // A:
    // R:
    
    // Verify file was written
    std::ifstream read_file("test_file.txt");
    std::string content;
    std::getline(read_file, content);
    
    EXPECT_EQ(content, "RAII test content");
}

// ============================================================================
// Scenario 2: File Handle with Exception (Moderate)
// ============================================================================

TEST_F(FileSocketManagementTest, FileHandleWithException)
{
    // Q: What happens to the file handle if an exception is thrown?
    // A:
    // R:
    
    try
    {
        std::ofstream file("test_file.txt");
        EXPECT_TRUE(file.is_open());
        
        file << "Before exception\n";
        
        throw std::runtime_error("Test exception");
        
        file << "After exception\n";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Q: Was the file properly closed despite the exception?
    // A:
    // R:
    
    // Verify file was written and closed
    std::ifstream read_file("test_file.txt");
    std::string content;
    std::getline(read_file, content);
    
    EXPECT_EQ(content, "Before exception");
}

// ============================================================================
// Scenario 3: Custom File Handle Wrapper (Moderate)
// ============================================================================

class FileHandle
{
public:
    explicit FileHandle(const std::string& filename)
    : file_(nullptr)
    , filename_(filename)
    {
        file_ = std::fopen(filename.c_str(), "w");
        
        if (file_)
        {
            EventLog::instance().record("FileHandle::open " + filename_);
        }
        else
        {
            EventLog::instance().record("FileHandle::open_failed " + filename_);
        }
    }
    
    ~FileHandle()
    {
        if (file_)
        {
            std::fclose(file_);
            EventLog::instance().record("FileHandle::close " + filename_);
        }
    }
    
    void write(const std::string& data)
    {
        if (file_)
        {
            std::fputs(data.c_str(), file_);
        }
    }
    
    bool is_open() const
    {
        return file_ != nullptr;
    }
    
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
private:
    FILE* file_;
    std::string filename_;
};

TEST_F(FileSocketManagementTest, CustomFileHandleWrapper)
{
    // Q: Why do we delete the copy constructor and copy assignment?
    // A:
    // R:
    
    {
        FileHandle handle("test_file.txt");
        
        EXPECT_TRUE(handle.is_open());
        EXPECT_EQ(EventLog::instance().count_events("FileHandle::open"), 1);
        
        handle.write("Custom wrapper\n");
    }
    
    // Q: What observable signal confirms the file was closed?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("FileHandle::close"), 1);
    
    // Verify content
    std::ifstream read_file("test_file.txt");
    std::string content;
    std::getline(read_file, content);
    
    EXPECT_EQ(content, "Custom wrapper");
}

// ============================================================================
// Scenario 4: Move-Only File Handle (Hard)
// ============================================================================

class MovableFileHandle
{
public:
    explicit MovableFileHandle(const std::string& filename)
    : file_(nullptr)
    , filename_(filename)
    {
        file_ = std::fopen(filename.c_str(), "w");
        
        if (file_)
        {
            EventLog::instance().record("MovableFileHandle::open " + filename_);
        }
    }
    
    ~MovableFileHandle()
    {
        close();
    }
    
    MovableFileHandle(MovableFileHandle&& other) noexcept
    : file_(other.file_)
    , filename_(std::move(other.filename_))
    {
        other.file_ = nullptr;
        EventLog::instance().record("MovableFileHandle::move_ctor");
    }
    
    MovableFileHandle& operator=(MovableFileHandle&& other) noexcept
    {
        if (this != &other)
        {
            close();
            file_ = other.file_;
            filename_ = std::move(other.filename_);
            other.file_ = nullptr;
            EventLog::instance().record("MovableFileHandle::move_assign");
        }
        return *this;
    }
    
    void write(const std::string& data)
    {
        if (file_)
        {
            std::fputs(data.c_str(), file_);
        }
    }
    
    bool is_open() const
    {
        return file_ != nullptr;
    }
    
    MovableFileHandle(const MovableFileHandle&) = delete;
    MovableFileHandle& operator=(const MovableFileHandle&) = delete;
    
private:
    void close()
    {
        if (file_)
        {
            std::fclose(file_);
            EventLog::instance().record("MovableFileHandle::close " + filename_);
            file_ = nullptr;
        }
    }
    
    FILE* file_;
    std::string filename_;
};

TEST_F(FileSocketManagementTest, MoveOnlyFileHandle)
{
    // Q: Why would we want a movable file handle?
    // A:
    // R:
    
    MovableFileHandle handle1("test_file.txt");
    EXPECT_TRUE(handle1.is_open());
    
    EventLog::instance().clear();
    
    // TODO: Move handle1 to handle2
    MovableFileHandle handle2(std::move(handle1));
    
    // Q: What is the state of handle1 after the move?
    // A:
    // R:
    
    EXPECT_FALSE(handle1.is_open());
    EXPECT_TRUE(handle2.is_open());
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
    
    // Q: How many times will the file be closed?
    // A:
    // R:
}

// ============================================================================
// Scenario 5: RAII with Multiple Resources (Hard)
// ============================================================================

class MultiResourceManager
{
public:
    MultiResourceManager(const std::string& file1, const std::string& file2)
    : handle1_(file1)
    , handle2_(file2)
    {
        EventLog::instance().record("MultiResourceManager::ctor");
    }
    
    ~MultiResourceManager()
    {
        EventLog::instance().record("MultiResourceManager::dtor");
    }
    
    void write_both(const std::string& data)
    {
        handle1_.write(data);
        handle2_.write(data);
    }
    
private:
    FileHandle handle1_;
    FileHandle handle2_;
};

TEST_F(FileSocketManagementTest, MultipleResourcesRAII)
{
    std::remove("file1.txt");
    std::remove("file2.txt");
    
    // Q: In what order are handle1_ and handle2_ constructed?
    // A:
    // R:
    
    {
        MultiResourceManager manager("file1.txt", "file2.txt");
        
        EXPECT_EQ(EventLog::instance().count_events("FileHandle::open"), 2);
        
        manager.write_both("data\n");
    }
    
    // Q: In what order are handle1_ and handle2_ destroyed?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("FileHandle::close"), 2);
    
    // Q: What happens if handle1_ construction succeeds but handle2_ construction throws?
    // A:
    // R:
    
    std::remove("file1.txt");
    std::remove("file2.txt");
}
