#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "cdr/CdrManager.h"

int main() {
    std::cout << "CDR Comprehensive File Type Test" << std::endl;
    std::cout << "=================================" << std::endl;
    
    try {
        CdrManager cdr;
        
        // Create output directory
        std::filesystem::create_directories("c:/temp/cdr_output");
        
        // Test files
        std::vector<std::pair<std::string, std::string>> testFiles = {
            {"test_malicious.html", "c:/temp/test_malicious.html"},
            {"test_malicious.pdf", "c:/temp/test_malicious.pdf"},
            {"test_malicious.js", "c:/temp/test_malicious.js"}
        };
        
        // Security levels to test
        std::vector<std::pair<SecurityLevel, std::string>> securityLevels = {
            {SecurityLevel::LOW, "LOW"},
            {SecurityLevel::MEDIUM, "MEDIUM"},
            {SecurityLevel::HIGH, "HIGH"},
            {SecurityLevel::STRICT, "STRICT"},
            {SecurityLevel::PARANOID, "PARANOID"}
        };
        
        for (auto& [filename, filepath] : testFiles) {
            std::cout << "\n--- Testing: " << filename << " ---" << std::endl;
            
            if (!std::filesystem::exists(filepath)) {
                std::cout << "❌ File not found: " << filepath << std::endl;
                continue;
            }
            
            for (auto& [level, levelName] : securityLevels) {
                std::cout << "\nSecurity Level: " << levelName << std::endl;
                
                std::string outputPath = "c:/temp/cdr_output/" + levelName + "_" + filename;
                auto result = cdr.sanitizeFile(filepath, outputPath, level);
                
                std::cout << "Result: " << (result.success ? "SUCCESS" : "FAILED");
                
                if (result.success) {
                    std::cout << " (Threats: " << result.threatsDetected.size() 
                             << ", Actions: " << result.actionsPerformed.size() 
                             << ", Quarantine: " << (result.requiresQuarantine ? "YES" : "NO") << ")";
                    
                    // Show some threat details
                    if (!result.threatsDetected.empty()) {
                        std::cout << "\n  Threats: ";
                        for (size_t i = 0; i < std::min(size_t(3), result.threatsDetected.size()); ++i) {
                            std::cout << static_cast<int>(result.threatsDetected[i]) << " ";
                        }
                        if (result.threatsDetected.size() > 3) {
                            std::cout << "...";
                        }
                    }
                    
                    if (!result.actionsPerformed.empty()) {
                        std::cout << "\n  Actions: ";
                        for (size_t i = 0; i < std::min(size_t(3), result.actionsPerformed.size()); ++i) {
                            std::cout << static_cast<int>(result.actionsPerformed[i]) << " ";
                        }
                        if (result.actionsPerformed.size() > 3) {
                            std::cout << "...";
                        }
                    }
                } else {
                    std::cout << " - Error: " << result.errorMessage;
                }
                std::cout << std::endl;
            }
        }
        
        // Show summary
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "✅ Tested " << testFiles.size() << " file types" << std::endl;
        std::cout << "✅ Tested " << securityLevels.size() << " security levels" << std::endl;
        std::cout << "✅ Total test combinations: " << (testFiles.size() * securityLevels.size()) << std::endl;
        
        // Check output directory
        std::cout << "\nOutput files created in: c:/temp/cdr_output/" << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator("c:/temp/cdr_output")) {
            if (entry.is_regular_file()) {
                std::cout << "  📄 " << entry.path().filename().string() 
                         << " (" << std::filesystem::file_size(entry.path()) << " bytes)" << std::endl;
            }
        }
        
        std::cout << "\n🎉 CDR Comprehensive Test Completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
