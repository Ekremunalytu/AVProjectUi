#pragma once

#pragma once

// Filesystem compatibility layer for cross-platform support
// This header provides a unified interface for filesystem operations

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>

// For now, force using POSIX implementation on macOS due to compilation issues
#define HAS_STD_FILESYSTEM 0

#if HAS_STD_FILESYSTEM
    #include <filesystem>
    #define FS_NAMESPACE std::filesystem
#else
    // Fallback to POSIX/platform-specific implementations
    #include <sys/stat.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <cstring>
    #include <cerrno>
    #ifdef __APPLE__
        #include <copyfile.h>
    #endif
#endif

namespace CDR {
namespace FileSystem {

class Path {
public:
    Path() = default;
    explicit Path(const std::string& path) : path_(path) {}
    
    std::string string() const { return path_; }
    std::string filename() const;
    std::string extension() const;
    Path parent_path() const;
    bool empty() const { return path_.empty(); }
    
    Path operator/(const std::string& other) const;
    
private:
    std::string path_;
};

// Filesystem operation functions
bool exists(const std::string& path);
bool exists(const Path& path);
bool is_directory(const std::string& path);
bool is_directory(const Path& path);
bool is_regular_file(const std::string& path);
bool is_regular_file(const Path& path);
std::uintmax_t file_size(const std::string& path);
std::uintmax_t file_size(const Path& path);
bool create_directories(const std::string& path);
bool create_directories(const Path& path);
bool copy_file(const std::string& from, const std::string& to, bool overwrite = true);
bool copy_file(const Path& from, const Path& to, bool overwrite = true);
bool copy(const std::string& from, const std::string& to, bool overwrite = true);
bool copy(const Path& from, const Path& to, bool overwrite = true);
bool remove(const std::string& path);
bool remove(const Path& path);

// Directory iteration
class DirectoryIterator {
public:
    explicit DirectoryIterator(const std::string& path);
    ~DirectoryIterator();
    
    bool is_valid() const;
    std::string current() const;
    void next();
    bool has_next() const;
    
private:
    std::string base_path_;
    bool valid_;
    bool recursive_;
    
#if HAS_STD_FILESYSTEM
    FS_NAMESPACE::directory_iterator iter_;
    FS_NAMESPACE::directory_iterator end_;
#else
    DIR* dir_;
    struct dirent* entry_;
#endif
};

class RecursiveDirectoryIterator {
public:
    explicit RecursiveDirectoryIterator(const std::string& path);
    ~RecursiveDirectoryIterator();
    
    bool is_valid() const;
    std::string current() const;
    void next();
    bool has_next() const;
    
private:
    std::string base_path_;
    bool valid_;
    
#if HAS_STD_FILESYSTEM
    FS_NAMESPACE::recursive_directory_iterator iter_;
    FS_NAMESPACE::recursive_directory_iterator end_;
#else
    std::vector<DirectoryIterator*> iterators_;
    void push_directory(const std::string& path);
    void pop_empty_iterators();
#endif
};

// Exception class for filesystem errors
class FilesystemError : public std::exception {
public:
    explicit FilesystemError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
    
private:
    std::string message_;
};

} // namespace FileSystem
} // namespace CDR
