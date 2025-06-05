#!/bin/bash

# AVProjectUi Documentation Generator and Server
# This script provides easy access to documentation generation and serving

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DOCS_DIR="$PROJECT_ROOT/docs"
HTML_DIR="$DOCS_DIR/html"

# Functions
print_header() {
    echo -e "${PURPLE}================================${NC}"
    echo -e "${PURPLE}  AVProjectUi Documentation${NC}"
    echo -e "${PURPLE}================================${NC}"
    echo
}

print_usage() {
    echo -e "${CYAN}Usage:${NC}"
    echo "  $0 [command] [options]"
    echo
    echo -e "${CYAN}Commands:${NC}"
    echo "  generate, gen, g     Generate documentation using Doxygen"
    echo "  serve, s             Serve existing documentation"
    echo "  build-serve, bs      Generate documentation and serve it"
    echo "  open, o              Open documentation in browser (if exists)"
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
    
    # Check for doxygen
    if ! command -v doxygen &> /dev/null; then
        missing_deps+=("doxygen")
    fi
    
    # Check for python3
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
    echo -e "${BLUE}🔨 Generating documentation...${NC}"
    
    if [ ! -f "$DOCS_DIR/Doxyfile" ]; then
        echo -e "${RED}❌ Doxyfile not found at: $DOCS_DIR/Doxyfile${NC}"
        exit 1
    fi
    
    cd "$DOCS_DIR"
    
    if [ "$VERBOSE" = "true" ]; then
        doxygen Doxyfile
    else
        echo "  Running Doxygen..."
        doxygen Doxyfile > /dev/null 2>&1
    fi
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Documentation generated successfully${NC}"
        echo -e "   📁 Output directory: $HTML_DIR"
        return 0
    else
        echo -e "${RED}❌ Documentation generation failed${NC}"
        exit 1
    fi
}

serve_docs() {
    local port_arg=""
    local browser_arg=""
    
    if [ ! -z "$PORT" ]; then
        port_arg="--port $PORT"
    fi
    
    if [ "$NO_BROWSER" = "true" ]; then
        browser_arg="--no-browser"
    fi
    
    echo -e "${BLUE}🚀 Starting documentation server...${NC}"
    
    python3 "$SCRIPT_DIR/serve_docs.py" $port_arg $browser_arg
}

open_docs() {
    if ! check_docs_exist; then
        return 1
    fi
    
    local index_file="file://$HTML_DIR/index.html"
    echo -e "${BLUE}🔗 Opening documentation in browser...${NC}"
    echo -e "   📖 URL: $index_file"
    
    if command -v open &> /dev/null; then
        # macOS
        open "$index_file"
    elif command -v xdg-open &> /dev/null; then
        # Linux
        xdg-open "$index_file"
    elif command -v start &> /dev/null; then
        # Windows
        start "$index_file"
    else
        echo -e "${YELLOW}⚠️  Could not detect system browser opener${NC}"
        echo -e "${CYAN}💡 Please open manually: $index_file${NC}"
        return 1
    fi
}

clean_docs() {
    echo -e "${BLUE}🧹 Cleaning generated documentation...${NC}"
    
    if [ -d "$HTML_DIR" ]; then
        rm -rf "$HTML_DIR"
        echo -e "${GREEN}✅ Documentation cleaned${NC}"
    else
        echo -e "${YELLOW}⚠️  No documentation to clean${NC}"
    fi
}

show_status() {
    echo -e "${CYAN}📊 Documentation Status:${NC}"
    echo
    
    # Check dependencies
    echo -e "${BLUE}Dependencies:${NC}"
    if command -v doxygen &> /dev/null; then
        echo -e "  ✅ Doxygen: $(doxygen --version)"
    else
        echo -e "  ❌ Doxygen: Not found"
    fi
    
    if command -v python3 &> /dev/null; then
        echo -e "  ✅ Python3: $(python3 --version)"
    else
        echo -e "  ❌ Python3: Not found"
    fi
    
    echo
    
    # Check files
    echo -e "${BLUE}Files:${NC}"
    if [ -f "$DOCS_DIR/Doxyfile" ]; then
        echo -e "  ✅ Doxyfile: Found"
    else
        echo -e "  ❌ Doxyfile: Not found"
    fi
    
    if [ -f "$HTML_DIR/index.html" ]; then
        echo -e "  ✅ Generated docs: Found"
        
        # Count files
        local file_count=$(find "$HTML_DIR" -name "*.html" | wc -l)
        echo -e "     📄 HTML files: $file_count"
        
        # Check modification time
        local mod_time=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$HTML_DIR/index.html" 2>/dev/null || stat -c "%y" "$HTML_DIR/index.html" 2>/dev/null || echo "Unknown")
        echo -e "     🕒 Last generated: $mod_time"
    else
        echo -e "  ❌ Generated docs: Not found"
    fi
    
    echo
    
    # Check server script
    echo -e "${BLUE}Server:${NC}"
    if [ -f "$SCRIPT_DIR/serve_docs.py" ]; then
        echo -e "  ✅ Server script: Found"
        if [ -x "$SCRIPT_DIR/serve_docs.py" ]; then
            echo -e "     ✅ Executable: Yes"
        else
            echo -e "     ⚠️  Executable: No"
        fi
    else
        echo -e "  ❌ Server script: Not found"
    fi
}

# Parse arguments
COMMAND=""
PORT=""
NO_BROWSER="false"
VERBOSE="false"

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
        help|h)
            COMMAND="help"
            shift
            ;;
        --port|-p)
            PORT="$2"
            shift 2
            ;;
        --no-browser)
            NO_BROWSER="true"
            shift
            ;;
        --verbose|-v)
            VERBOSE="true"
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

# Main execution
print_header

case "$COMMAND" in
    generate)
        check_dependencies
        generate_docs
        ;;
    serve)
        check_dependencies
        if check_docs_exist; then
            serve_docs
        fi
        ;;
    build-serve)
        check_dependencies
        generate_docs && serve_docs
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
    help|"")
        print_usage
        ;;
    *)
        echo -e "${RED}❌ Unknown command: $COMMAND${NC}"
        echo
        print_usage
        exit 1
        ;;
esac
