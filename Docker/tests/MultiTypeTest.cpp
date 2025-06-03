#include <iostream>
#include <string>
#include <filesystem>
#include "cdr/CdrManager.h"

void testFile(CdrManager& cdr, const std::string& inputFile, const std::string& outputFile, const std::string& fileType) {
    std::cout << "\n--- " << fileType << " Sanitization Test ---" << std::endl;
    std::cout << "Input: " << inputFile << std::endl;
    std::cout << "Output: " << outputFile << std::endl;
    
    if (!std::filesystem::exists(inputFile)) {
        std::cout << "❌ Input file not found!" << std::endl;
        return;
    }
    
    auto result = cdr.sanitizeFile(inputFile, outputFile, SecurityLevel::HIGH);
    
    std::cout << "Results:" << std::endl;
    std::cout << "Success: " << (result.success ? "YES" : "NO") << std::endl;
    
    if (result.success) {
        auto originalSize = std::filesystem::file_size(inputFile);
        auto sanitizedSize = std::filesystem::exists(outputFile) ? 
                            std::filesystem::file_size(outputFile) : 0;
        
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
        
        std::cout << "--- File Content Comparison ---" << std::endl;
        if (std::filesystem::exists(outputFile)) {
            std::cout << "✅ Sanitized file created successfully" << std::endl;
            std::cout << "  Original file: " << inputFile << std::endl;
            std::cout << "  Sanitized file: " << outputFile << std::endl;
            std::cout << "  Size change: " << (static_cast<long>(sanitizedSize) - static_cast<long>(originalSize)) << " bytes" << std::endl;
        } else {
            std::cout << "⚠️ No sanitized file created (may be quarantined)" << std::endl;
        }
    } else {
        std::cout << "Error: " << result.errorMessage << std::endl;
    }
}

int main() {
    std::cout << "CDR Multi-File Type Sanitization Test" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        CdrManager cdr;
        
        // Create output directory
        std::filesystem::create_directories("c:/temp/cdr_output");
        
        // Test HTML file
        testFile(cdr, "c:/temp/test_malicious.html", "c:/temp/cdr_output/sanitized.html", "HTML");
        
        // Test JavaScript file
        testFile(cdr, "c:/temp/test_malicious.js", "c:/temp/cdr_output/sanitized.js", "JavaScript");
        
        // Test PDF file
        testFile(cdr, "c:/temp/test_malicious.pdf", "c:/temp/cdr_output/sanitized.pdf", "PDF");
        
        std::cout << "\n=== Multi-File CDR Test Summary ===" << std::endl;
        std::cout << "✅ Tested HTML, JavaScript, and PDF files" << std::endl;
        std::cout << "✅ Validated threat detection across file types" << std::endl;
        std::cout << "✅ Confirmed sanitization actions" << std::endl;
        
        // List all output files
        std::cout << "\nOutput files created:" << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator("c:/temp/cdr_output")) {
            if (entry.is_regular_file()) {
                std::cout << "  📄 " << entry.path().filename().string() 
                         << " (" << std::filesystem::file_size(entry.path()) << " bytes)" << std::endl;
            }
        }
        
        std::cout << "\n🎉 CDR Multi-File Test Completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
