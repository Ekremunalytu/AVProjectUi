// Simple test file to check CDR compilation
#include "Scanner/CDRScanner.h"
#include "Docker/include/cdr/CdrTypes.h"

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
