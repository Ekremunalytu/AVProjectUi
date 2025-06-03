#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include <iostream>

int main() {
    std::cout << "=== Quick CDR Test ===" << std::endl;
    
    try {
        CDR::CdrSanitizer sanitizer;
        std::cout << "CDR Sanitizer created successfully!" << std::endl;
        
        // Test basic methods
        auto sanitizers = sanitizer.getAvailableSanitizers();
        std::cout << "Available sanitizers: " << sanitizers.size() << std::endl;
        for (const auto& s : sanitizers) {
            std::cout << "  - " << s << std::endl;
        }
        
        auto fileTypes = sanitizer.getSupportedFileTypes();
        std::cout << "Supported file types: " << fileTypes.size() << std::endl;
        for (const auto& ft : fileTypes) {
            std::cout << "  - " << ft << std::endl;
        }
        
        std::cout << "CDR Test completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
