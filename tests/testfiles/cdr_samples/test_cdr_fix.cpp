#include <iostream>
#include <memory>
#include "src/security/cdr/CdrManager.h"
#include "src/security/cdr/CdrTypes.h"

int main() {
    std::cout << "Testing CDR Manager with malicious PDF..." << std::endl;
    
    try {
        CDR::CdrManager manager;
        
        // Set up configuration
        CDR::CdrConfiguration config;
        config.analysisType = CDR::AnalysisType::COMPREHENSIVE_SCAN;
        config.autoSanitize = true;
        config.preserveOriginal = false;
        config.inputDirectory = "/Volumes/Crucial/AVProjectUi/test_cdr_scan";
        config.outputDirectory = "/tmp/cdr_test_output";
        config.quarantineDirectory = "/tmp/cdr_test_quarantine";
        config.timeoutSeconds = 60;
        
        std::cout << "Starting CDR analysis on directory: " << config.inputDirectory << std::endl;
        
        // Start analysis
        std::string analysisId = manager.startAnalysis(config.inputDirectory, config);
        std::cout << "Analysis started with ID: " << analysisId << std::endl;
        
        // Wait for analysis to complete
        std::cout << "Waiting for analysis to complete..." << std::endl;
        int maxAttempts = 30;
        int attempts = 0;
        
        while (attempts < maxAttempts) {
            CDR::CdrAnalysisResult result = manager.getAnalysisStatus(analysisId);
            
            std::cout << "Status: " << result.status << ", Progress: " << result.progressPercentage << "%" << std::endl;
            
            if (result.status == "completed" || result.status == "failed") {
                std::cout << "\nFinal Results:" << std::endl;
                std::cout << "- Status: " << result.status << std::endl;
                std::cout << "- Safe: " << (result.isSafe ? "YES" : "NO") << std::endl;
                std::cout << "- Files Scanned: " << result.totalFilesScanned << std::endl;
                std::cout << "- Threats Detected: " << result.threatsDetected << std::endl;
                std::cout << "- Files Quarantined: " << result.filesQuarantined << std::endl;
                std::cout << "- Clean Files: " << result.cleanFiles.size() << std::endl;
                
                if (!result.quarantinedFiles.empty()) {
                    std::cout << "- Quarantined Files:" << std::endl;
                    for (const auto& file : result.quarantinedFiles) {
                        std::cout << "  * " << file << std::endl;
                    }
                }
                
                if (!result.errorMessage.empty()) {
                    std::cout << "- Error: " << result.errorMessage << std::endl;
                }
                
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            attempts++;
        }
        
        if (attempts >= maxAttempts) {
            std::cout << "Analysis timed out after " << maxAttempts * 2 << " seconds" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
