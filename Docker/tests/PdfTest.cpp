#include <iostream>
#include <string>
#include <filesystem>
#include "cdr/CdrManager.h"

int main() {
    std::cout << "CDR PDF Sanitization Test" << std::endl;
    std::cout << "=========================" << std::endl;
    
    try {
        CdrManager cdr;
        
        std::string inputPath = "c:/temp/test_malicious.pdf";
        std::string outputPath = "c:/temp/cdr_output/sanitized.pdf";
        
        // Create output directory
        std::filesystem::create_directories("c:/temp/cdr_output");
        
        std::cout << "--- PDF Sanitization Test ---" << std::endl;
        std::cout << "Input: " << inputPath << std::endl;
        std::cout << "Output: " << outputPath << std::endl;
        
        // Test with HIGH security level
        auto result = cdr.sanitizeFile(inputPath, outputPath, SecurityLevel::HIGH);
        
        std::cout << "Results:" << std::endl;
        std::cout << "Success: " << (result.success ? "YES" : "NO") << std::endl;
        
        if (result.success) {
            // Get file sizes
            auto originalSize = std::filesystem::file_size(inputPath);
            auto sanitizedSize = std::filesystem::exists(outputPath) ? 
                                std::filesystem::file_size(outputPath) : 0;
            
            std::cout << "Original size: " << originalSize << " bytes" << std::endl;
            std::cout << "Sanitized size: " << sanitizedSize << " bytes" << std::endl;
            std::cout << "Requires quarantine: " << (result.requiresQuarantine ? "YES" : "NO") << std::endl;
            
            std::cout << "Threats detected:" << std::endl;
            for (auto threat : result.threatsDetected) {
                std::cout << "  • " << static_cast<int>(threat) << std::endl;
            }
            
            std::cout << "Actions performed:" << std::endl;
            for (auto action : result.actionsPerformed) {
                std::cout << "  • " << static_cast<int>(action) << std::endl;
            }
        } else {
            std::cout << "Error: " << result.errorMessage << std::endl;
        }
        
        std::cout << "✅ CDR PDF Test Completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
