#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "cdr/CdrManager.h"

int main() {
    std::cout << "=== CDR Multi-File Type Test ===" << std::endl;
    
    try {
        CdrManager cdr;
        
        // Test file types
        std::vector<std::pair<std::string, std::string>> testFiles = {
            {"test.html", "<html><script>alert('xss')</script><body>content</body></html>"},
            {"test.js", "var x = eval('malicious code'); document.write('<script>hack</script>');"},
            {"test.pdf", "%PDF-1.4\n<</Type/Catalog/Pages<</Type/Pages/Kids[<</Type/Page>>]>>>>"},
            {"test.docx", "PK\x03\x04test doc with macro content"}
        };
        
        // Create temp directory
        std::filesystem::create_directories("c:/temp/multi_test");
        
        for (auto& [filename, content] : testFiles) {
            std::string inputPath = "c:/temp/multi_test/" + filename;
            std::string outputPath = "c:/temp/multi_test/clean_" + filename;
            
            // Write test file
            std::ofstream file(inputPath);
            file << content;
            file.close();
            
            std::cout << "\nTesting: " << filename << std::endl;
            
            // Test different security levels
            for (auto level : {SecurityLevel::LOW, SecurityLevel::MEDIUM, SecurityLevel::HIGH, SecurityLevel::STRICT}) {
                std::string levelName = (level == SecurityLevel::LOW) ? "LOW" :
                                       (level == SecurityLevel::MEDIUM) ? "MEDIUM" :
                                       (level == SecurityLevel::HIGH) ? "HIGH" : "STRICT";
                
                std::cout << "  Security Level: " << levelName << " - ";
                
                auto result = cdr.sanitizeFile(inputPath, outputPath, level);
                
                if (result.success) {
                    std::cout << "SUCCESS (Threats: " << result.threatsDetected.size() 
                             << ", Actions: " << result.actionsPerformed.size() << ")" << std::endl;
                    
                    // Show detected threats
                    for (auto threat : result.threatsDetected) {
                        std::cout << "    Threat: " << static_cast<int>(threat) << std::endl;
                    }
                } else {
                    std::cout << "FAILED: " << result.errorMessage << std::endl;
                }
            }
        }
        
        std::cout << "\n=== Test Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
