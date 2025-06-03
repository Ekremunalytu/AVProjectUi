#include <iostream>
#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"

int main() {
    std::cout << "=== Simple CDR Test ===" << std::endl;
    
    try {
        // Create CDR Sanitizer
        CDR::CdrSanitizer sanitizer;
        std::cout << "✓ CDR Sanitizer created successfully" << std::endl;
        
        // Get available sanitizers
        auto sanitizers = sanitizer.getAvailableSanitizers();
        std::cout << "Available sanitizers: " << sanitizers.size() << std::endl;
        for (const auto& name : sanitizers) {
            std::cout << "  - " << name << std::endl;
        }
        
        // Get supported file types
        auto fileTypes = sanitizer.getSupportedFileTypes();
        std::cout << "Supported file types: " << fileTypes.size() << std::endl;
        for (const auto& type : fileTypes) {
            std::cout << "  - " << type << std::endl;
        }
        
        std::cout << "✓ Simple test completed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
