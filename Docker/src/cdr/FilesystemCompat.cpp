#include "cdr/FilesystemCompat.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

#if !HAS_STD_FILESYSTEM
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#ifdef __APPLE__
#include <copyfile.h>
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

#else // Fallback POSIX implementation

bool exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool exists(const Path& path) {
    return exists(path.string());
}

bool is_directory(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool is_directory(const Path& path) {
    return is_directory(path.string());
}

bool is_regular_file(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

bool is_regular_file(const Path& path) {
    return is_regular_file(path.string());
}

std::uintmax_t file_size(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return static_cast<std::uintmax_t>(-1);
    }
    return static_cast<std::uintmax_t>(st.st_size);
}

std::uintmax_t file_size(const Path& path) {
    return file_size(path.string());
}

bool create_directories(const std::string& path) {
    if (path.empty()) return false;
    if (exists(path)) return is_directory(path);
    
    // Create parent directories first
    Path p(path);
    Path parent = p.parent_path();
    if (!parent.empty() && !exists(parent)) {
        if (!create_directories(parent)) {
            return false;
        }
    }
    
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
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

#else // Fallback POSIX implementation

DirectoryIterator::DirectoryIterator(const std::string& path) 
    : base_path_(path), valid_(false), recursive_(false), dir_(nullptr), entry_(nullptr) {
    dir_ = opendir(path.c_str());
    if (dir_) {
        next(); // Move to first entry
    }
}

DirectoryIterator::~DirectoryIterator() {
    if (dir_) {
        closedir(dir_);
    }
}

std::string DirectoryIterator::current() const {
    if (!valid_ || !entry_) return "";
    return base_path_ + "/" + entry_->d_name;
}

void DirectoryIterator::next() {
    valid_ = false;
    if (!dir_) return;
    
    while ((entry_ = readdir(dir_)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry_->d_name, ".") == 0 || strcmp(entry_->d_name, "..") == 0) {
            continue;
        }
        valid_ = true;
        break;
    }
}

bool DirectoryIterator::has_next() const {
    return valid_ && entry_ != nullptr;
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