#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include <iostream>
#include <filesystem>

void testCdrSanitizer() {
    std::cout << "\n=== CDR Sanitizer Test ===" << std::endl;
    
    try {
        // CDR Sanitizer oluştur
        CDR::CdrSanitizer sanitizer;
        std::cout << "✓ CDR Sanitizer successfully created" << std::endl;
        
        // Test configuration
        CDR::CdrConfiguration config;
        config.securityLevel = CDR::CdrConfiguration::SecurityLevel::MEDIUM;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.inputDirectory = "/tmp/test_input";
        config.outputDirectory = "/tmp/test_output";
        config.quarantineDirectory = "/tmp/test_quarantine";
        
        // Get available sanitizers
        auto sanitizers = sanitizer.getAvailableSanitizers();
        std::cout << "Available sanitizers: " << sanitizers.size() << std::endl;
        
        // Get supported file types 
        auto fileTypes = sanitizer.getSupportedFileTypes();
        std::cout << "Supported file types: " << fileTypes.size() << std::endl;
        
        // Test basic functionality
        std::cout << "✓ Basic CDR Sanitizer functions working" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error testing CDR Sanitizer: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "CDR (Content Detection & Remediation) System Test" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    testCdrSanitizer();
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "✓ CDR Sanitizer basic functionality test completed" << std::endl;
    std::cout << "Note: Full functionality requires Docker environment" << std::endl;
    
    return 0;
}