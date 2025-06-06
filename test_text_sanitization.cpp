#include "security/cdr/CdrManager.h"
#include "security/cdr/CdrTypes.h"
#include <iostream>
#include <fstream>

int main() {
    std::cout << "Testing TEXT_DOCUMENT sanitization..." << std::endl;
    
    try {
        CDR::CdrManager manager;
        
        // Set up configuration
        CDR::CdrConfiguration config;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.autoSanitize = true;
        config.preserveOriginal = false;
        config.inputDirectory = "/Volumes/Crucial/AVProjectUi";
        config.outputDirectory = "/tmp/cdr_test_output";
        config.quarantineDirectory = "/tmp/cdr_test_quarantine";
        config.timeoutSeconds = 60;
        
        // Test file paths
        std::string inputFile = "/Volumes/Crucial/AVProjectUi/test_malicious_text.txt";
        std::string outputFile = "/tmp/cdr_test_output/sanitized_text.txt";
        
        std::cout << "Testing sanitizeTextDocument method..." << std::endl;
        std::cout << "Input: " << inputFile << std::endl;
        std::cout << "Output: " << outputFile << std::endl;
        
        // Create output directory
        system("mkdir -p /tmp/cdr_test_output");
        
        // Test TEXT_DOCUMENT sanitization
        CDR::SanitizationResult result = manager.sanitizeTextDocument(inputFile, outputFile, config);
        
        std::cout << "\n=== SANITIZATION RESULT ===" << std::endl;
        std::cout << "Success: " << (result.success ? "YES" : "NO") << std::endl;
        std::cout << "File Type: " << (int)result.fileType << std::endl;
        std::cout << "Original Size: " << result.originalSize << " bytes" << std::endl;
        std::cout << "Sanitized Size: " << result.sanitizedSize << " bytes" << std::endl;
        
        if (!result.errorMessage.empty()) {
            std::cout << "Error: " << result.errorMessage << std::endl;
        }
        
        std::cout << "\nActions Performed:" << std::endl;
        for (const auto& action : result.actionsPerformed) {
            std::cout << "  - " << action << std::endl;
        }
        
        std::cout << "\nThreats Detected:" << std::endl;
        for (const auto& threat : result.threatsDetected) {
            std::cout << "  - " << threat << std::endl;
        }
        
        if (result.requiresQuarantine) {
            std::cout << "\nQuarantine Required: " << result.quarantineReason << std::endl;
        }
        
        // Display sanitized content
        if (result.success) {
            std::cout << "\n=== SANITIZED CONTENT ===" << std::endl;
            std::ifstream sanitizedFile(outputFile);
            if (sanitizedFile.is_open()) {
                std::string line;
                while (std::getline(sanitizedFile, line)) {
                    std::cout << line << std::endl;
                }
                sanitizedFile.close();
            } else {
                std::cout << "Could not read sanitized file." << std::endl;
            }
        }
        
        // Test file type detection
        std::cout << "\n=== FILE TYPE DETECTION ===" << std::endl;
        CDR::FileType detectedType = manager.detectFileType(inputFile);
        std::cout << "Detected file type: " << (int)detectedType << std::endl;
        
        // Test if file type is supported
        bool isSupported = manager.isFileTypeSupported(inputFile);
        std::cout << "File type supported: " << (isSupported ? "YES" : "NO") << std::endl;
        
        return result.success ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
