#!/bin/bash

# AVProjectUi - Project Quick Tools
# Bu script proje yönetimi için hızlı araçlar sağlar

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

show_help() {
    echo -e "${BLUE}AVProjectUi - Quick Tools${NC}"
    echo ""
    echo -e "${YELLOW}Usage:${NC}"
    echo "  $0 [command]"
    echo ""
    echo -e "${YELLOW}Commands:${NC}"
    echo "  status       Show project status"
    echo "  cleanup      Clean build artifacts"
    echo "  docs         Generate and serve documentation"
    echo "  test         Run all tests"
    echo "  format       Format code (if available)"
    echo "  help         Show this help"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  $0 status    # Project overview"
    echo "  $0 cleanup   # Clean build files"
    echo "  $0 docs      # Generate docs and serve"
}

show_status() {
    echo -e "${BLUE}📊 AVProjectUi Project Status${NC}"
    echo ""
    
    # Build directory
    if [ -d "$PROJECT_ROOT/build" ]; then
        build_size=$(du -sh "$PROJECT_ROOT/build" 2>/dev/null | cut -f1)
        echo -e "${GREEN}✓${NC} Build directory: $build_size"
    else
        echo -e "${YELLOW}-${NC} Build directory: Not found"
    fi
    
    # Documentation
    if [ -f "$PROJECT_ROOT/docs/html/index.html" ]; then
        echo -e "${GREEN}✓${NC} Documentation: Generated"
    else
        echo -e "${YELLOW}-${NC} Documentation: Not generated"
    fi
    
    # Tests
    if [ -d "$PROJECT_ROOT/tests" ]; then
        test_count=$(find "$PROJECT_ROOT/tests" -name "*.cpp" 2>/dev/null | wc -l | tr -d ' ')
        echo -e "${GREEN}✓${NC} Unit tests: $test_count files"
    else
        echo -e "${YELLOW}-${NC} Tests directory: Not found"
    fi
    
    # Source code stats
    echo ""
    echo -e "${BLUE}📈 Code Statistics:${NC}"
    if [ -d "$PROJECT_ROOT/src" ]; then
        cpp_count=$(find "$PROJECT_ROOT/src" -name "*.cpp" -o -name "*.h" 2>/dev/null | wc -l | tr -d ' ')
        echo "  C++: $cpp_count files"
    else
        echo "  C++: src directory not found"
    fi
    
    py_count=$(find "$PROJECT_ROOT" -maxdepth 3 -name "*.py" 2>/dev/null | wc -l | tr -d ' ')
    echo "  Python: $py_count files"
    sh_count=$(find "$PROJECT_ROOT" -maxdepth 3 -name "*.sh" 2>/dev/null | wc -l | tr -d ' ')
    echo "  Shell: $sh_count files"
}

quick_cleanup() {
    echo -e "${YELLOW}🧹 Quick Cleanup...${NC}"
    cd "$PROJECT_ROOT"
    
    if [ -f "tools/scripts/cleanup_build.sh" ]; then
        ./tools/scripts/cleanup_build.sh
    else
        echo "Cleanup script not found, doing basic cleanup..."
        rm -rf build/ cmake-build-debug/ *.app 2>/dev/null
    fi
    
    echo -e "${GREEN}✓ Cleanup completed${NC}"
}

quick_docs() {
    echo -e "${YELLOW}📚 Generating and serving documentation...${NC}"
    cd "$PROJECT_ROOT"
    
    if [ -f "tools/scripts/docs.sh" ]; then
        ./tools/scripts/docs.sh build-serve
    else
        echo -e "${RED}❌ Documentation script not found${NC}"
        exit 1
    fi
}

quick_test() {
    echo -e "${YELLOW}🧪 Running tests...${NC}"
    cd "$PROJECT_ROOT"
    
    if [ -f "Makefile" ]; then
        make test
    else
        echo -e "${RED}❌ Makefile not found${NC}"
        exit 1
    fi
}

# Main command handling
case "${1:-help}" in
    "status"|"st")
        show_status
        ;;
    "cleanup"|"clean")
        quick_cleanup
        ;;
    "docs"|"doc")
        quick_docs
        ;;
    "test"|"t")
        quick_test
        ;;
    "help"|"h"|"")
        show_help
        ;;
    *)
        echo -e "${RED}❌ Unknown command: $1${NC}"
        echo ""
        show_help
        exit 1
        ;;
esac
