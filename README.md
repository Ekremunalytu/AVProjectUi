# AVProjectUi - Advanced Antivirus Project

AVProjectUi is a comprehensive antivirus solution that combines multiple scanning technologies to provide robust malware detection and analysis capabilities. The project integrates static analysis, dynamic sandbox execution, CDR (Content Disarm and Reconstruction), and cloud-based scanning services.

## 🚀 Features

- **Multi-Scanner Architecture**: Supports various scanning technologies
  - CDR Scanner for content disarmament and reconstruction
  - Sandbox Scanner for dynamic behavioral analysis
  - VirusTotal integration for cloud-based detection

- **Docker Integration**: Isolated execution environments for safe analysis
  - Container-based sandboxing
  - CDR processing in isolated environments
  - Automated resource management

- **Database Management**: Comprehensive data storage and retrieval
  - Scan history tracking
  - Malware signature database
  - Configuration management

- **Modern UI**: Intuitive Qt-based user interface
  - Real-time scan progress monitoring
  - Detailed analysis reports
  - Configuration management

## 🏗️ Requirements

- Qt 6.x
- CMake 3.16 or higher
- Docker Engine (for sandbox and CDR functionality)
- C++17 compatible compiler
- Doxygen (for documentation generation)
- Python 3 (for documentation server)

## 🔧 Building

For detailed requirements and build instructions, see [`docs/REQUIREMENTS.md`](docs/REQUIREMENTS.md).

### Quick Start

```bash
# Build the project
make build

# Build and run tests
make build-and-test

# Generate and serve documentation
make docs-serve
```

### Manual Build

```bash
mkdir build
cd build
cmake ..
make
```

## 📚 Documentation

The project includes comprehensive API documentation generated with Doxygen.

### Generate Documentation

```bash
# Generate documentation only
make docs

# Generate and serve documentation with HTTP server
make docs-serve

# Serve existing documentation
make serve-docs

# Open documentation in browser
make docs-open

# Check documentation status
make docs-status

# Clean generated documentation
make docs-clean
```

### Documentation Features

- **Complete API Reference**: Detailed documentation for all classes and methods
- **Code Examples**: Usage examples for all major components
- **Architecture Overview**: Detailed description of the modular design
- **Interactive Navigation**: Search functionality and cross-references
- **Visual Diagrams**: Class hierarchies and dependency graphs

### Using Documentation Script

You can also use the unified documentation script directly:

```bash
# Unified documentation management
./tools/scripts/docs.sh help

# Quick commands
./tools/scripts/docs.sh generate     # Generate docs
./tools/scripts/docs.sh serve        # Serve docs  
./tools/scripts/docs.sh build-serve  # Generate and serve
./tools/scripts/docs.sh status       # Show status
./tools/scripts/docs.sh clean        # Clean docs
```

## 🧪 Testing

```bash
# Run all tests
make test

# Run specific test suites
make test-dashboard
make test-dbmanager
make test-mainwindow

# Verbose test output
make test-verbose

# Generate coverage report
make coverage
```

## 🐳 Docker Usage

### CDR Scanner Example

```cpp
#include "Scanner/CDRScanner.h"

CDRScanner scanner;
scanner.selectFile();
if (scanner.scanFile("/path/to/file")) {
    QString results = scanner.getResults();
    // Process results
}
```

### Sandbox Analysis Example

```cpp
#include "Scanner/SandboxScanner.h"

SandboxScanner sandbox;
sandbox.setSandboxTimeout(300); // 5 minutes
sandbox.setMonitoringLevel(2);  // Deep monitoring

if (sandbox.scanFile("/path/to/suspicious/file")) {
    QString behaviorSummary = sandbox.getBehaviorSummary();
    QString technicalReport = sandbox.getAnalysisReport();
}
```

### VirusTotal Integration Example

```cpp
#include "Network/VirusTotal/VirusTotalManager.h"

VirusTotalManager vtManager("your-api-key");
if (vtManager.scanFile("/path/to/file")) {
    QString status = vtManager.getSubmissionStatus();
    QString results = vtManager.getResults();
}
```

## 🏗️ Architecture

The project follows a modular architecture with clear separation of concerns:

### Core Modules

- **Core**: Application configuration and shared utilities
- **Interface**: Abstract interfaces for scanners and services
- **Scanner**: Implementation of various scanning technologies
- **Database**: Data persistence and management
- **Docker**: Container management and isolation
- **Network**: External service integration (VirusTotal)
- **Sandbox**: Dynamic analysis environment management
- **UI**: User interface components and widgets

### Design Patterns

- **Strategy Pattern**: Pluggable scanner implementations
- **Singleton Pattern**: Configuration management
- **Observer Pattern**: Real-time status updates
- **Factory Pattern**: Scanner instantiation
- **PIMPL Idiom**: Implementation hiding and ABI stability

## 🛠️ Development

### First Time Setup

```bash
# Install development tools and hooks
make setup-dev

# Check documentation dependencies
make docs-status
```

### Build Optimization

```bash
# Clean build artifacts
make cleanup

# Release build
make build-release

# Full optimization suite
make optimize
```

### Available Make Targets

Run `make help` to see all available targets, including:

- **Build Targets**: configure, build, test, coverage
- **Documentation Targets**: docs, docs-serve, docs-open, docs-clean
- **Development Targets**: setup-dev, analyze, optimize

## 📊 Project Status & Architecture

- **Project Status**: See [`docs/user/PROJECT_STATUS.md`](docs/user/PROJECT_STATUS.md)
- **Architecture**: See [`docs/architecture/project_structure.md`](docs/architecture/project_structure.md)
- **Optimization Examples**: See [`docs/architecture/OPTIMIZATION_EXAMPLE.md`](docs/architecture/OPTIMIZATION_EXAMPLE.md)
- **Requirements**: See [`docs/user/REQUIREMENTS.md`](docs/user/REQUIREMENTS.md)

## 📄 Project Structure

```
AVProjectUi/
├── Core/               # Application configuration
├── Database/           # Data persistence layer
├── Docker/             # Container management
├── Interface/          # Abstract interfaces
├── Network/            # External service integration
├── Sandbox/            # Dynamic analysis
├── Scanner/            # Scanning implementations
├── UI/                 # User interface
├── docs/               # Documentation
├── scripts/            # Build and utility scripts
└── test_files/         # Test data
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Add comprehensive tests
4. Update documentation
5. Submit a pull request

## 📝 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 📞 Contact

- **Project Lead**: Ekrem Ünal
- **Email**: contact@avprojectui.com
- **Documentation**: Generated with Doxygen

---

**Date**: 2024-2025  
**Version**: 1.0
