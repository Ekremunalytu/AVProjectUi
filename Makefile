# Simple Makefile helper - redirects to sub-tests
.PHONY: clean build test build-and-test all help coverage benchmark

# Default variables
BUILD_DIR ?= build
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug

# Default target
all: build-and-test

# Create CMake configuration
configure:
	@echo "Configuring CMake project..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS)

# Coverage enabled configuration
configure-coverage:
	@echo "Configuring CMake project with coverage..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. $(CMAKE_FLAGS) -DENABLE_COVERAGE=ON

# Build project
build: configure
	@echo "Building project..."
	@cd $(BUILD_DIR) && cmake --build .

# Run tests only
test:
	@echo "Running tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Build and test
build-and-test: build
	@echo "Running all tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Detailed test output
test-verbose:
	@echo "Running tests with detailed output..."
	@cd $(BUILD_DIR)/tests && ./DashboardTest -v2 && ./DbManagerTest -v2 && ./MainWindowTest -v2

# Run specific test
test-dashboard:
	@echo "Running Dashboard tests..."
	@cd $(BUILD_DIR)/tests && ./DashboardTest -v2

test-dbmanager:
	@echo "Running DbManager tests..."
	@cd $(BUILD_DIR)/tests && ./DbManagerTest -v2

test-mainwindow:
	@echo "Running MainWindow tests..."
	@cd $(BUILD_DIR)/tests && ./MainWindowTest -v2

# Generate test coverage
coverage: configure-coverage build
	@echo "Generating test coverage report..."
	@cd $(BUILD_DIR) && make coverage
	@echo "Coverage report ready: $(BUILD_DIR)/coverage-report/index.html"

# Run benchmark tests
benchmark:
	@echo "Running benchmark tests..."
	@cd $(BUILD_DIR)/tests && ./DbManagerTest -silent -functions testSha256Exists
	@cd $(BUILD_DIR)/tests && ./DashboardTest -silent -functions testInitialization

# Clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)/*

# Help information
help:
	@echo "Available targets:"
	@echo "  configure    - Create CMake configuration"
	@echo "  build        - Build project"
	@echo "  test         - Run tests"
	@echo "  test-verbose - Run tests with detailed output"
	@echo "  test-dashboard - Run only Dashboard tests"
	@echo "  test-dbmanager - Run only DbManager tests"
	@echo "  test-mainwindow - Run only MainWindow tests"
	@echo "  build-and-test - Build and run tests"
	@echo "  coverage     - Build and run test coverage reporting"
	@echo "  benchmark    - Run performance tests"
	@echo "  clean        - Clean build directory"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Documentation targets:"
	@echo "  docs         - Generate documentation"
	@echo "  docs-serve   - Generate and serve documentation"
	@echo "  serve-docs   - Serve existing documentation"
	@echo "  docs-open    - Open documentation in browser"
	@echo "  docs-clean   - Clean generated documentation"
	@echo "  docs-status  - Show documentation status"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=<directory>   - Build directory (default: build)"
	@echo "  CMAKE_FLAGS=<flags> - Extra flags for CMake"

# === OPTIMIZATION COMMANDS ===

# Cleanup build artifacts
cleanup:
	@echo "🧹 Cleaning build artifacts..."
	@./scripts/cleanup_build.sh

# Install git hooks
install-hooks:
	@echo "🔗 Installing git hooks..."
	@cp scripts/pre-commit .git/hooks/pre-commit
	@chmod +x .git/hooks/pre-commit
	@echo "✅ Pre-commit hook installed"

# Documentation targets
.PHONY: docs docs-serve docs-clean docs-status docs-open

# Generate documentation
docs:
	@echo "📚 Generating documentation..."
	@./scripts/docs.sh generate

# Generate and serve documentation  
docs-serve:
	@echo "📚 Generating and serving documentation..."
	@./scripts/docs.sh build-serve

# Serve existing documentation
serve-docs:
	@echo "🚀 Serving documentation..."
	@./scripts/docs.sh serve

# Open documentation in browser
docs-open:
	@echo "🔗 Opening documentation..."
	@./scripts/docs.sh open

# Clean documentation
docs-clean:
	@echo "🧹 Cleaning documentation..."
	@./scripts/docs.sh clean

# Show documentation status
docs-status:
	@echo "📊 Documentation status..."
	@./scripts/docs.sh status

# Code analysis
analyze:
	@echo "🔍 Starting code analysis..."
	@echo "📊 Line counts:"
	@find . -name "*.cpp" -o -name "*.h" | grep -v build | grep -v cmake-build-debug | xargs wc -l | sort -nr | head -10
	@echo ""
	@echo "📁 Directory sizes:"
	@du -sh */ | sort -hr
	@echo ""
	@echo "🔧 TODO/FIXME count:"
	@grep -r "TODO\|FIXME" --include="*.cpp" --include="*.h" . | wc -l

# Full optimization suite
optimize: cleanup install-hooks docs analyze
	@echo "🚀 Optimization completed!"
	@echo "💡 You can now rebuild your project: make build"

# Performance build (Release mode)
build-release:
	@echo "🏎️  Release mode build..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	@cd $(BUILD_DIR) && make -j$(shell nproc 2>/dev/null || echo 4)

# Development setup (first time setup)
setup-dev: install-hooks
	@echo "👨‍💻 Development environment setup..."
	@echo "📝 Recommended tools:"
	@echo "   - doxygen (docs): brew install doxygen"
	@echo "   - clang-format (formatting): brew install clang-format" 
	@echo "   - ccache (fast builds): brew install ccache"
	@echo "✅ Development setup completed!"