#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::cout << "CDR Real File Sanitization Test" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        // Create output directory
        fs::create_directories("c:\\temp\\cdr_output");
        
        // Initialize CDR Sanitizer
        CDR::CdrSanitizer sanitizer;
        
        // Configuration for high security
        CDR::CdrConfiguration config;
        config.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.blockOfficeMacros = true;
        config.blockPdfScripts = true;
        config.blockAllScripts = false; // Allow script analysis but sanitize
        
        // Test HTML sanitization
        std::string inputHtml = "c:\\temp\\test_malicious.html";
        std::string outputHtml = "c:\\temp\\cdr_output\\sanitized.html";
        
        std::cout << "\n--- HTML Sanitization Test ---" << std::endl;
        std::cout << "Input: " << inputHtml << std::endl;
        std::cout << "Output: " << outputHtml << std::endl;
        
        CDR::SanitizationResult result = sanitizer.sanitizeHtmlFile(inputHtml, outputHtml, config);
        
        std::cout << "\nResults:" << std::endl;
        std::cout << "Success: " << (result.success ? "YES" : "NO") << std::endl;
        std::cout << "Original size: " << result.originalSize << " bytes" << std::endl;
        std::cout << "Sanitized size: " << result.sanitizedSize << " bytes" << std::endl;
        std::cout << "Requires quarantine: " << (result.requiresQuarantine ? "YES" : "NO") << std::endl;
        
        if (!result.threatsDetected.empty()) {
            std::cout << "\nThreats detected:" << std::endl;
            for (const auto& threat : result.threatsDetected) {
                std::cout << "  • " << threat << std::endl;
            }
        }
        
        if (!result.actionsPerformed.empty()) {
            std::cout << "\nActions performed:" << std::endl;
            for (const auto& action : result.actionsPerformed) {
                std::cout << "  • " << action << std::endl;
            }
        }
        
        if (!result.errorMessage.empty()) {
            std::cout << "\nError: " << result.errorMessage << std::endl;
        }
        
        if (!result.sanitizationDetails.empty()) {
            std::cout << "\nDetails: " << result.sanitizationDetails << std::endl;
        }
        
        // Show file comparison
        if (result.success && fs::exists(outputHtml)) {
            std::cout << "\n--- File Content Comparison ---" << std::endl;
            std::cout << "✓ Sanitized file created successfully" << std::endl;
            std::cout << "  Original file: " << inputHtml << std::endl;
            std::cout << "  Sanitized file: " << outputHtml << std::endl;
            std::cout << "  Size reduction: " << (result.originalSize - result.sanitizedSize) << " bytes" << std::endl;
        }
        
        std::cout << "\n🎉 CDR Real File Test Completed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
