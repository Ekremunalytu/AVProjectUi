#!/bin/bash

# AVProjectUi Build Cleanup Script
# This script cleans build artifacts and recovers disk space

echo "🧹 Starting AVProjectUi Build Cleanup..."

# Show current status
echo "📊 Status before cleanup:"
if [ -d "build" ]; then
    build_size=$(du -sh build | cut -f1)
    echo "   build/ directory: $build_size"
fi

if [ -d "cmake-build-debug" ]; then
    debug_size=$(du -sh cmake-build-debug | cut -f1)
    echo "   cmake-build-debug/ directory: $debug_size"
fi

# Clean build directories
echo ""
echo "🗑️  Cleaning build directories..."
rm -rf build/
rm -rf cmake-build-debug/

# Clean app bundles
echo "📱 Cleaning app bundles..."
find . -name "*.app" -type d -exec rm -rf {} + 2>/dev/null

# Clean autogen files
echo "🤖 Cleaning autogen files..."
find . -name "*_autogen" -type d -exec rm -rf {} + 2>/dev/null

# Clean CMake cache files
echo "💾 Cleaning CMake cache files..."
find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null
find . -name "CMakeCache.txt" -exec rm -f {} + 2>/dev/null

# Qt generated files
echo "🔧 Cleaning Qt generated files..."
find . -name "moc_*.cpp" -exec rm -f {} + 2>/dev/null
find . -name "ui_*.h" -exec rm -f {} + 2>/dev/null
find . -name "qrc_*.cpp" -exec rm -f {} + 2>/dev/null

# macOS specific files
echo "🍎 Cleaning macOS specific files..."
find . -name ".DS_Store" -exec rm -f {} + 2>/dev/null

# Temporary files
echo "📄 Cleaning temporary files..."
find . -name "*.tmp" -exec rm -f {} + 2>/dev/null
find . -name "*.temp" -exec rm -f {} + 2>/dev/null

echo ""
echo "✅ Cleanup completed!"
echo "💡 To rebuild the project: make clean && make"
echo ""
