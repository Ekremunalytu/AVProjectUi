#include "cdr/FilesystemCompat.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

#if !HAS_STD_FILESYSTEM
#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
// Windows stat constants
#ifndef _S_IFDIR
    #define _S_IFDIR 0x4000
#endif
#ifndef _S_IFREG
    #define _S_IFREG 0x8000
#endif
#else
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#ifdef __APPLE__
#include <copyfile.h>
#endif
#endif
#endif

namespace CDR {
namespace FileSystem {

// Path implementation
std::string Path::filename() const {
    if (path_.empty()) return "";
    
    size_t pos = path_.find_last_of("/\\");
    if (pos == std::string::npos) return path_;
    return path_.substr(pos + 1);
}

std::string Path::extension() const {
    std::string fname = filename();
    size_t pos = fname.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return "";
    return fname.substr(pos);
}

Path Path::parent_path() const {
    if (path_.empty()) return Path();
    
    auto pos = path_.find_last_of("/\\");
    if (pos == std::string::npos) {
        return Path();
    }
    if (pos == 0) {
        return Path("/");
    }
    return Path(path_.substr(0, pos));
}

Path Path::operator/(const std::string& other) const {
    if (path_.empty()) return Path(other);
    if (other.empty()) return *this;
    
    std::string result = path_;
    if (result.back() != '/' && result.back() != '\\') {
        result += '/';
    }
    result += other;
    return Path(result);
}

// Filesystem operation implementations
#if HAS_STD_FILESYSTEM

bool exists(const std::string& path) {
    try {
        return FS_NAMESPACE::exists(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool exists(const Path& path) {
    return exists(path.string());
}

bool is_directory(const std::string& path) {
    try {
        return FS_NAMESPACE::is_directory(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool is_directory(const Path& path) {
    return is_directory(path.string());
}

bool is_regular_file(const std::string& path) {
    try {
        return FS_NAMESPACE::is_regular_file(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool is_regular_file(const Path& path) {
    return is_regular_file(path.string());
}

std::uintmax_t file_size(const std::string& path) {
    try {
        return FS_NAMESPACE::file_size(path);
    } catch (const std::exception&) {
        return static_cast<std::uintmax_t>(-1);
    }
}

std::uintmax_t file_size(const Path& path) {
    return file_size(path.string());
}

bool create_directories(const std::string& path) {
    try {
        return FS_NAMESPACE::create_directories(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool create_directories(const Path& path) {
    return create_directories(path.string());
}

bool copy_file(const std::string& from, const std::string& to, bool overwrite) {
    try {
        auto options = overwrite ? FS_NAMESPACE::copy_options::overwrite_existing 
                                 : FS_NAMESPACE::copy_options::none;
        return FS_NAMESPACE::copy_file(from, to, options);
    } catch (const std::exception&) {
        return false;
    }
}

bool copy_file(const Path& from, const Path& to, bool overwrite) {
    return copy_file(from.string(), to.string(), overwrite);
}

bool copy(const std::string& from, const std::string& to, bool overwrite) {
    try {
        auto options = overwrite ? FS_NAMESPACE::copy_options::overwrite_existing 
                                 : FS_NAMESPACE::copy_options::none;
        FS_NAMESPACE::copy(from, to, options);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool copy(const Path& from, const Path& to, bool overwrite) {
    return copy(from.string(), to.string(), overwrite);
}

bool remove(const std::string& path) {
    try {
        return FS_NAMESPACE::remove(path);
    } catch (const std::exception&) {
        return false;
    }
}

bool remove(const Path& path) {
    return remove(path.string());
}

#else // Fallback implementation for platforms without std::filesystem

bool exists(const std::string& path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#endif
}

bool exists(const Path& path) {
    return exists(path.string());
}

bool is_directory(const std::string& path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
#endif
}

bool is_directory(const Path& path) {
    return is_directory(path.string());
}

bool is_regular_file(const std::string& path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
#endif
}

bool is_regular_file(const Path& path) {
    return is_regular_file(path.string());
}

std::uintmax_t file_size(const std::string& path) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return static_cast<std::uintmax_t>(-1);
    }
    
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return static_cast<std::uintmax_t>(-1);
    }
    
    CloseHandle(hFile);
    return static_cast<std::uintmax_t>(fileSize.QuadPart);
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return static_cast<std::uintmax_t>(-1);
    }
    return static_cast<std::uintmax_t>(st.st_size);
#endif
}

std::uintmax_t file_size(const Path& path) {
    return file_size(path.string());
}


// Helper function for recursive directory creation (POSIX)
static bool create_directories_recursive_impl(const std::string& path_s) {
    if (path_s.empty()) {
        return true; // Successfully "created" an empty path
    }

    std::string path = path_s;
    // Normalize path separators to '/'
    for (char &c : path) {
        if (c == '\\') {
            c = '/';
        }
    }
    // Remove trailing slash if not root and path is not empty
    if (path.length() > 1 && path.back() == '/') {
        path.pop_back();
    }
    if (path.empty()) { // Original path was just "/" or "\"
        return true;
    }

    char tmp[1024]; // Using a fixed-size buffer; ensure it's large enough or use dynamic allocation.
    strncpy(tmp, path.c_str(), sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0'; // Ensure null termination

    char *p = tmp;
    // Skip leading drive letter or first slash to correctly form paths like "C:/Users" or "/usr/bin"
    #if defined(_WIN32) || defined(_WIN64)
    if (strlen(tmp) >= 2 && tmp[1] == ':' && (tmp[0] >= 'a' && tmp[0] <= 'z' || tmp[0] >= 'A' && tmp[0] <= 'Z')) {
        p = tmp + 2; // Skip "C:"
        if (*p == '/') p++; // Skip "C:/"
    } else if (*p == '/') {
        p++; // Skip leading "/"
    }
    #else
    if (*p == '/') {
        p++; // Skip leading "/"
    }
    #endif


    // Iterate over path components
    while (*p) {
        char *slash = strchr(p, '/');
        if (slash) {
            *slash = '\0'; // Temporarily terminate at this component
        }

        // Check current cumulative path (tmp)
#ifdef _WIN32
        DWORD dwAttrib = GetFileAttributesA(tmp);
        if (dwAttrib == INVALID_FILE_ATTRIBUTES) { // If path component does not exist
            if (!CreateDirectoryA(tmp, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
                if (slash) *slash = '/'; // Restore slash before returning
                return false; // Failed to create directory
            }
        } else if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) { // Exists but is not a directory
            if (slash) *slash = '/'; // Restore slash
            return false;
        }
#else
        struct stat st;
        if (stat(tmp, &st) != 0) { // If path component does not exist
            #if defined(__MINGW32__) || defined(__MINGW64__)
            if (mkdir(tmp) != 0 && errno != EEXIST) {
            #else
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
            #endif
                if (slash) *slash = '/'; // Restore slash before returning
                return false; // Failed to create directory
            }
        } else if (!S_ISDIR(st.st_mode)) { // Exists but is not a directory
            if (slash) *slash = '/'; // Restore slash
            return false;
        }
#endif

        if (slash) {
            *slash = '/'; // Restore slash
            p = slash + 1;  // Move to next component
        } else {
            break; // No more slashes, processed full path
        }
    }
    return true;
}

bool create_directories(const std::string& path_str) {
    if (CDR::FileSystem::exists(path_str)) {
        return CDR::FileSystem::is_directory(path_str);
    }
    return create_directories_recursive_impl(path_str);
}

bool create_directories(const Path& path) {
    return create_directories(path.string());
}

bool copy_file(const std::string& from, const std::string& to, bool overwrite) {
    if (!exists(from) || !is_regular_file(from)) {
        return false;
    }
    
    if (exists(to) && !overwrite) {
        return false;
    }
    
#ifdef __APPLE__
    // Use macOS copyfile API for better performance and metadata preservation
    return copyfile(from.c_str(), to.c_str(), nullptr, COPYFILE_ALL) == 0;
#else
    // Generic file copy using streams
    std::ifstream src(from, std::ios::binary);
    if (!src) return false;
    
    std::ofstream dst(to, std::ios::binary);
    if (!dst) return false;
    
    dst << src.rdbuf();
    return dst.good() && src.good();
#endif
}

bool copy_file(const Path& from, const Path& to, bool overwrite) {
    return copy_file(from.string(), to.string(), overwrite);
}

bool copy(const std::string& from, const std::string& to, bool overwrite) {
    if (is_regular_file(from)) {
        return copy_file(from, to, overwrite);
    } else if (is_directory(from)) {
        // Recursive directory copy
        if (!create_directories(to)) {
            return false;
        }
        
        DirectoryIterator it(from);
        while (it.has_next()) {
            std::string current = it.current();
            Path currentPath(current);
            std::string filename = currentPath.filename();
            std::string dest = (Path(to) / filename).string();
            
            if (!copy(current, dest, overwrite)) {
                return false;
            }
            it.next();
        }
        return true;
    }
    return false;
}

bool copy(const Path& from, const Path& to, bool overwrite) {
    return copy(from.string(), to.string(), overwrite);
}

bool remove(const std::string& path) {
    return unlink(path.c_str()) == 0 || rmdir(path.c_str()) == 0;
}

bool remove(const Path& path) {
    return remove(path.string());
}

#endif // HAS_STD_FILESYSTEM

// Directory iterator implementations
#if HAS_STD_FILESYSTEM

DirectoryIterator::DirectoryIterator(const std::string& path) 
    : base_path_(path), valid_(false), recursive_(false) {
    try {
        iter_ = FS_NAMESPACE::directory_iterator(path);
        end_ = FS_NAMESPACE::directory_iterator();
        valid_ = (iter_ != end_);
    } catch (const std::exception&) {
        valid_ = false;
    }
}

DirectoryIterator::~DirectoryIterator() = default;

std::string DirectoryIterator::current() const {
    if (!valid_ || iter_ == end_) return "";
    return iter_->path().string();
}

void DirectoryIterator::next() {
    if (valid_ && iter_ != end_) {
        try {
            ++iter_;
            valid_ = (iter_ != end_);
        } catch (const std::exception&) {
            valid_ = false;
        }
    }
}

bool DirectoryIterator::has_next() const {
    return valid_ && iter_ != end_;
}

bool DirectoryIterator::is_valid() const {
    return valid_;
}

RecursiveDirectoryIterator::RecursiveDirectoryIterator(const std::string& path)
    : base_path_(path), valid_(false) {
    try {
        iter_ = FS_NAMESPACE::recursive_directory_iterator(path);
        end_ = FS_NAMESPACE::recursive_directory_iterator();
        valid_ = (iter_ != end_);
    } catch (const std::exception&) {
        valid_ = false;
    }
}

RecursiveDirectoryIterator::~RecursiveDirectoryIterator() = default;

std::string RecursiveDirectoryIterator::current() const {
    if (!valid_ || iter_ == end_) return "";
    return iter_->path().string();
}

void RecursiveDirectoryIterator::next() {
    if (valid_ && iter_ != end_) {
        try {
            ++iter_;
            valid_ = (iter_ != end_);
        } catch (const std::exception&) {
            valid_ = false;
        }
    }
}

bool RecursiveDirectoryIterator::has_next() const {
    return valid_ && iter_ != end_;
}

#else // Fallback implementation for non-std::filesystem

DirectoryIterator::DirectoryIterator(const std::string& path) 
    : base_path_(path), valid_(false), recursive_(false) {
#ifdef _WIN32
    search_pattern_ = path + "\\*";
    first_call_ = true;
    hFind_ = FindFirstFileA(search_pattern_.c_str(), &findData_);
    if (hFind_ != INVALID_HANDLE_VALUE) {
        next(); // Move to first valid entry
    }
#else
    dir_ = opendir(path.c_str());
    if (dir_) {
        next(); // Move to first entry
    }
#endif
}

DirectoryIterator::~DirectoryIterator() {
#ifdef _WIN32
    if (hFind_ != INVALID_HANDLE_VALUE) {
        FindClose(hFind_);
    }
#else
    if (dir_) {
        closedir(dir_);
    }
#endif
}

std::string DirectoryIterator::current() const {
#ifdef _WIN32
    if (!valid_) return "";
    return base_path_ + "\\" + findData_.cFileName;
#else
    if (!valid_ || !entry_) return "";
    return base_path_ + "/" + entry_->d_name;
#endif
}

void DirectoryIterator::next() {
    valid_ = false;
#ifdef _WIN32
    if (hFind_ == INVALID_HANDLE_VALUE) return;
    
    do {
        BOOL result;
        if (first_call_) {
            first_call_ = false;
            result = TRUE; // First call already populated findData_
        } else {
            result = FindNextFileA(hFind_, &findData_);
        }
        
        if (!result) {
            break; // No more files
        }
        
        // Skip . and ..
        if (strcmp(findData_.cFileName, ".") == 0 || strcmp(findData_.cFileName, "..") == 0) {
            continue;
        }
        
        valid_ = true;
        break;
    } while (true);
#else
    if (!dir_) return;
    
    while ((entry_ = readdir(dir_)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry_->d_name, ".") == 0 || strcmp(entry_->d_name, "..") == 0) {
            continue;
        }
        valid_ = true;
        break;
    }
#endif
}

bool DirectoryIterator::has_next() const {
#ifdef _WIN32
    return valid_;
#else
    return valid_ && entry_ != nullptr;
#endif
}

bool DirectoryIterator::is_valid() const {
    return valid_;
}

RecursiveDirectoryIterator::RecursiveDirectoryIterator(const std::string& path)
    : base_path_(path), valid_(false) {
    push_directory(path);
    if (!iterators_.empty()) {
        valid_ = iterators_.back()->is_valid();
        if (!valid_) {
            next(); // Move to first valid entry
        }
    }
}

RecursiveDirectoryIterator::~RecursiveDirectoryIterator() {
    for (auto* it : iterators_) {
        delete it;
    }
}

std::string RecursiveDirectoryIterator::current() const {
    if (!valid_ || iterators_.empty()) return "";
    return iterators_.back()->current();
}

void RecursiveDirectoryIterator::next() {
    valid_ = false;
    pop_empty_iterators();
    
    while (!iterators_.empty()) {
        auto* current_iter = iterators_.back();
        if (!current_iter->has_next()) {
            pop_empty_iterators();
            continue;
        }
        
        std::string current_path = current_iter->current();
        current_iter->next();
        
        // If current path is a directory, push it to the stack
        if (is_directory(current_path)) {
            push_directory(current_path);
        }
        
        valid_ = true;
        break;
    }
}

bool RecursiveDirectoryIterator::has_next() const {
    return valid_ && !iterators_.empty();
}

bool RecursiveDirectoryIterator::is_valid() const {
    return valid_;
}

void RecursiveDirectoryIterator::push_directory(const std::string& path) {
    auto* iter = new DirectoryIterator(path);
    if (iter->is_valid()) {
        iterators_.push_back(iter);
    } else {
        delete iter;
    }
}

void RecursiveDirectoryIterator::pop_empty_iterators() {
    while (!iterators_.empty() && !iterators_.back()->has_next()) {
        delete iterators_.back();
        iterators_.pop_back();
    }
}

#endif // HAS_STD_FILESYSTEM

} // namespace FileSystem
} // namespace CDR