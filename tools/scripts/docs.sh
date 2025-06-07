#!/usr/bin/env bash
# AVProjectUi Documentation Management Script
# Unified script for all documentation operations

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
DOCS_DIR="$PROJECT_ROOT/docs"
HTML_DIR="$DOCS_DIR/html"
TOOLS_DIR="$PROJECT_ROOT/tools/scripts"

# Default values
PORT=8080
OPEN_BROWSER=true
VERBOSE=false

print_header() {
    echo -e "${CYAN}📚 AVProjectUi Documentation Manager${NC}"
    echo -e "${CYAN}═══════════════════════════════════════${NC}"
}

print_usage() {
    print_header
    echo
    echo -e "${CYAN}Usage:${NC}"
    echo "  $0 [command] [options]"
    echo
    echo -e "${CYAN}Commands:${NC}"
    echo "  generate, gen, g     Generate documentation using Doxygen"
    echo "  serve, s             Serve existing documentation"
    echo "  build-serve, bs      Generate and serve documentation"
    echo "  open, o              Open documentation in browser"
    echo "  clean, c             Clean generated documentation"
    echo "  status, st           Show documentation status"
    echo "  help, h              Show this help message"
    echo
    echo -e "${CYAN}Options:${NC}"
    echo "  --port PORT, -p PORT Port number for server (default: 8080)"
    echo "  --no-browser         Don't open browser automatically"
    echo "  --verbose, -v        Verbose output"
    echo
    echo -e "${CYAN}Examples:${NC}"
    echo "  $0 generate          # Generate documentation"
    echo "  $0 serve             # Serve documentation"
    echo "  $0 bs --port 9090    # Generate and serve on port 9090"
    echo "  $0 serve --no-browser # Serve without opening browser"
}

check_dependencies() {
    local missing_deps=()
    
    if ! command -v doxygen &> /dev/null; then
        missing_deps+=("doxygen")
    fi
    
    if ! command -v python3 &> /dev/null; then
        missing_deps+=("python3")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo -e "${RED}❌ Missing dependencies:${NC}"
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        echo
        echo -e "${YELLOW}💡 Install missing dependencies and try again${NC}"
        exit 1
    fi
}

check_docs_exist() {
    if [ ! -f "$HTML_DIR/index.html" ]; then
        echo -e "${YELLOW}⚠️  Documentation not found. Run 'generate' first.${NC}"
        return 1
    fi
    return 0
}

generate_docs() {
    echo -e "${BLUE}📚 Generating documentation...${NC}"
    
    # Ensure directories exist
    mkdir -p "$HTML_DIR"
    
    # Generate main API documentation
    cd "$DOCS_DIR"
    if [ -f "Doxyfile.main" ]; then
        echo -e "${CYAN}🔧 Using unified Doxyfile configuration${NC}"
        doxygen Doxyfile.main
    elif [ -f "api/Doxyfile" ]; then
        echo -e "${CYAN}🔧 Using legacy API Doxyfile${NC}"
        cd api
        doxygen Doxyfile
        cd ..
    else
        echo -e "${RED}❌ No Doxyfile found!${NC}"
        exit 1
    fi
    
    # Generate UML diagrams if script exists
    if [ -f "$TOOLS_DIR/generate_comprehensive_uml.py" ]; then
        echo -e "${CYAN}🎨 Generating comprehensive UML diagrams...${NC}"
        cd "$PROJECT_ROOT"
        python3 "$TOOLS_DIR/generate_comprehensive_uml.py" || true
    fi
    
    echo -e "${GREEN}✅ Documentation generated successfully!${NC}"
    echo -e "${CYAN}📍 Output: $HTML_DIR${NC}"
}

serve_docs() {
    check_docs_exist || return 1
    
    echo -e "${BLUE}🚀 Starting documentation server...${NC}"
    echo -e "${CYAN}📍 Serving from: $HTML_DIR${NC}"
    echo -e "${CYAN}🌐 URL: http://localhost:$PORT${NC}"
    echo -e "${YELLOW}💡 Press Ctrl+C to stop the server${NC}"
    echo
    
    if [ "$OPEN_BROWSER" = true ]; then
        # Try to open browser after a brief delay
        (sleep 2 && open "http://localhost:$PORT" 2>/dev/null || \
         xdg-open "http://localhost:$PORT" 2>/dev/null || \
         echo -e "${YELLOW}⚠️  Could not open browser automatically${NC}") &
    fi
    
    # Use the enhanced documentation server if available
    if [ -f "$TOOLS_DIR/serve_docs.py" ]; then
        cd "$PROJECT_ROOT"
        python3 "$TOOLS_DIR/serve_docs.py" --port "$PORT" --docs-dir "$HTML_DIR" ${OPEN_BROWSER:+} ${OPEN_BROWSER:+"--no-browser"}
    else
        # Fallback to simple HTTP server
        cd "$HTML_DIR"
        python3 -m http.server "$PORT"
    fi
}

open_docs() {
    check_docs_exist || return 1
    
    local index_file="$HTML_DIR/index.html"
    echo -e "${BLUE}🔗 Opening documentation...${NC}"
    
    if command -v open &> /dev/null; then
        open "$index_file"
    elif command -v xdg-open &> /dev/null; then
        xdg-open "$index_file"
    else
        echo -e "${YELLOW}⚠️  Could not open browser automatically${NC}"
        echo -e "${CYAN}📍 Open manually: file://$index_file${NC}"
    fi
}

clean_docs() {
    echo -e "${BLUE}🧹 Cleaning documentation...${NC}"
    
    if [ -d "$HTML_DIR" ]; then
        rm -rf "$HTML_DIR"
        echo -e "${GREEN}✅ Cleaned: $HTML_DIR${NC}"
    fi
    
    # Clean any generated diagrams in docs root
    cd "$DOCS_DIR"
    rm -f *.png *.svg *.pdf 2>/dev/null || true
    
    echo -e "${GREEN}✅ Documentation cleaned!${NC}"
}

show_status() {
    echo -e "${BLUE}📊 Documentation Status${NC}"
    echo -e "${CYAN}═══════════════════════${NC}"
    echo
    
    # Check if documentation exists
    if [ -f "$HTML_DIR/index.html" ]; then
        echo -e "${GREEN}✅ Documentation generated${NC}"
        
        # Show file count and size
        local file_count=$(find "$HTML_DIR" -type f | wc -l)
        local dir_size=$(du -sh "$HTML_DIR" 2>/dev/null | cut -f1)
        echo -e "${CYAN}📁 Files: $file_count${NC}"
        echo -e "${CYAN}💾 Size: $dir_size${NC}"
        
        # Show last generated time
        local last_modified=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$HTML_DIR/index.html" 2>/dev/null || stat -c "%y" "$HTML_DIR/index.html" 2>/dev/null | cut -d'.' -f1)
        echo -e "${CYAN}🕒 Generated: $last_modified${NC}"
    else
        echo -e "${YELLOW}⚠️  Documentation not generated${NC}"
        echo -e "${CYAN}💡 Run: $0 generate${NC}"
    fi
    
    echo
    
    # Check dependencies
    echo -e "${BLUE}🔧 Dependencies${NC}"
    if command -v doxygen &> /dev/null; then
        local doxy_version=$(doxygen --version 2>/dev/null || echo "unknown")
        echo -e "${GREEN}✅ Doxygen: $doxy_version${NC}"
    else
        echo -e "${RED}❌ Doxygen: not found${NC}"
    fi
    
    if command -v python3 &> /dev/null; then
        local py_version=$(python3 --version 2>&1 | cut -d' ' -f2)
        echo -e "${GREEN}✅ Python3: $py_version${NC}"
    else
        echo -e "${RED}❌ Python3: not found${NC}"
    fi
    
    if command -v dot &> /dev/null; then
        local dot_version=$(dot -V 2>&1 | head -n1 | cut -d' ' -f5)
        echo -e "${GREEN}✅ Graphviz: $dot_version${NC}"
    else
        echo -e "${YELLOW}⚠️  Graphviz: not found (UML diagrams disabled)${NC}"
    fi
}

# Parse command line arguments
COMMAND=""
while [[ $# -gt 0 ]]; do
    case $1 in
        generate|gen|g)
            COMMAND="generate"
            shift
            ;;
        serve|s)
            COMMAND="serve"
            shift
            ;;
        build-serve|bs)
            COMMAND="build-serve"
            shift
            ;;
        open|o)
            COMMAND="open"
            shift
            ;;
        clean|c)
            COMMAND="clean"
            shift
            ;;
        status|st)
            COMMAND="status"
            shift
            ;;
        help|h|--help|-h)
            print_usage
            exit 0
            ;;
        --port|-p)
            PORT="$2"
            shift 2
            ;;
        --no-browser)
            OPEN_BROWSER=false
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        *)
            echo -e "${RED}❌ Unknown option: $1${NC}"
            echo
            print_usage
            exit 1
            ;;
    esac
done

# Set verbose mode
if [ "$VERBOSE" = true ]; then
    set -x
fi

# Check dependencies for commands that need them
case "$COMMAND" in
    generate|build-serve|serve)
        check_dependencies
        ;;
esac

# Execute command
case "$COMMAND" in
    generate)
        generate_docs
        ;;
    serve)
        serve_docs
        ;;
    build-serve)
        generate_docs
        echo
        serve_docs
        ;;
    open)
        open_docs
        ;;
    clean)
        clean_docs
        ;;
    status)
        show_status
        ;;
    "")
        echo -e "${RED}❌ No command specified${NC}"
        echo
        print_usage
        exit 1
        ;;
    *)
        echo -e "${RED}❌ Unknown command: $COMMAND${NC}"
        echo
        print_usage
        exit 1
        ;;
esac
