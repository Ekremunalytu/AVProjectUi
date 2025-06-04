# Code Optimization Examples

## 1. CdrSanitizer.cpp Refactoring Example

### Current State (1636 lines)
```cpp
// All sanitization logic in a single file
class CdrSanitizer {
    // PDF sanitization (400+ lines)
    SanitizationResult sanitizePdfDocument(...);
    
    // Office sanitization (500+ lines)  
    SanitizationResult sanitizeOfficeDocument(...);
    
    // HTML sanitization (300+ lines)
    SanitizationResult sanitizeHtmlDocument(...);
    
    // File type detection (200+ lines)
    FileType detectFileType(...);
    
    // Utility functions (200+ lines)
};
```

### Recommended Structure
```cpp
// 1. Main CdrSanitizer class (orchestration only)
class CdrSanitizer {
    std::unique_ptr<PdfSanitizer> pdfSanitizer_;
    std::unique_ptr<OfficeSanitizer> officeSanitizer_;
    std::unique_ptr<HtmlSanitizer> htmlSanitizer_;
    std::unique_ptr<FileTypeDetector> detector_;
    
    SanitizationResult sanitize(const std::string& filePath) {
        auto type = detector_->detectFileType(filePath);
        switch(type) {
            case FileType::PDF_DOCUMENT:
                return pdfSanitizer_->sanitize(filePath);
            case FileType::OFFICE_DOCUMENT:
                return officeSanitizer_->sanitize(filePath);
            // ...
        }
    }
};

// 2. Specialized sanitizer classes
class PdfSanitizer {
    SanitizationResult sanitize(const std::string& filePath);
    // PDF-specific methods...
};

class OfficeSanitizer {
    SanitizationResult sanitize(const std::string& filePath);
    // Office-specific methods...
};
```

## 2. Build Artifacts Cleanup

### Problem
```bash
# Large build directories:
build/              # ~50-100MB
cmake-build-debug/  # ~100-200MB  
*.app/             # ~10-50MB
*_autogen/         # ~10MB
```

### Solution Scripts
```bash
#!/bin/bash
# scripts/clean_build.sh
echo "🧹 Cleaning build artifacts..."
rm -rf build/
rm -rf cmake-build-debug/
rm -rf *.app/
find . -name "*_autogen" -type d -exec rm -rf {} +
find . -name "CMakeFiles" -type d -exec rm -rf {} +
echo "✅ Cleanup completed!"
```

## 3. Git Hooks Optimization

### Pre-commit Hook Example
```bash
#!/bin/bash
# .git/hooks/pre-commit
echo "🔍 Running pre-commit checks..."

# 1. Code formatting check
if command -v clang-format >/dev/null 2>&1; then
    echo "📝 Checking code formatting..."
    git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h)$' | \
    while read file; do
        clang-format -i "$file"
        git add "$file"
    done
fi

# 2. Large file check
echo "📏 Checking for large files..."
git diff --cached --name-only | while read file; do
    if [ -f "$file" ]; then
        size=$(wc -l < "$file" 2>/dev/null || echo 0)
        if [ "$size" -gt 1000 ]; then
            echo "⚠️  Large file detected: $file ($size lines)"
            echo "💡 Consider splitting this file into smaller modules"
        fi
    fi
done

echo "✅ Pre-commit checks completed!"
```

## 4. Documentation Generation

### Doxygen Configuration
```cmake
# CMakeLists.txt addition
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()
```

## 5. CMake Modularization

### Current State
```cmake
# All targets in a single CMakeLists.txt (144 lines)
```

### Recommended Structure
```cmake
# Main CMakeLists.txt
cmake_minimum_required(VERSION 3.19)
project(AvProjectUi)

# Add sub-modules
add_subdirectory(Core)
add_subdirectory(Database)  
add_subdirectory(Docker)
add_subdirectory(Sandbox)
add_subdirectory(Scanner)
add_subdirectory(UI)

# Main executable
qt_add_executable(AvProjectUi main.cpp)
target_link_libraries(AvProjectUi 
    PRIVATE 
        Core
        Database
        Docker  
        Sandbox
        Scanner
        UI
)

# Docker/CMakeLists.txt
add_library(Docker STATIC
    src/docker/DockerManager.cpp
    src/cdr/CdrManager.cpp
    # ...
)
target_include_directories(Docker PUBLIC include)
```
