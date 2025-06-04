/**
 * @file test_compile.cpp
 * @brief Simple compilation test for CDR scanner functionality
 * @author AVProjectUi Team
 * @version 1.0
 * @date 2024
 */

// Simple test file to check CDR compilation
#include "Scanner/CDRScanner.h"
#include "Docker/include/cdr/CdrTypes.h"

/**
 * @brief Main function for testing CDR compilation
 * 
 * This function performs basic instantiation tests to verify that
 * the CDR scanner and related components compile correctly.
 * 
 * @return 0 on successful compilation test
 */
int main() {
    // Test basic CDR functionality
    CDR::CdrConfiguration config;
    config.inputDirectory = "test";
    config.outputDirectory = "output";
    config.quarantineDirectory = "quarantine";
    
    // Test CDRScanner instantiation
    CDRScanner scanner;
    
    return 0;
}
