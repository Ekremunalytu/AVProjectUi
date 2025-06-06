#include <iostream>
#include <fstream>
#include <string>

// Copy of detectPdfJavaScript function from CdrSanitizer.cpp
bool detectPdfJavaScript(const std::string& filePath) {
    std::cout << "[detectPdfJavaScript] Analyzing file: " << filePath << std::endl;
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[detectPdfJavaScript] Error: Could not open file: " << filePath << std::endl;
            return false;
        }
        
        // Read file in chunks to avoid memory issues
        const size_t CHUNK_SIZE = 4096;
        char buffer[CHUNK_SIZE];
        std::string content;
        
        while (file.read(buffer, CHUNK_SIZE) || file.gcount() > 0) {
            content.append(buffer, static_cast<size_t>(file.gcount()));
            
            // Check for JavaScript and other active content keywords in PDF
            // Existing keywords:
            if (content.find("/JavaScript") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /JavaScript" << std::endl; return true; }
            if (content.find("/JS") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /JS" << std::endl; return true; }
            if (content.find("/OpenAction") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /OpenAction" << std::endl; return true; }
            if (content.find("/AA") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /AA" << std::endl; return true; }
            if (content.find("/Launch") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Launch" << std::endl; return true; }
            if (content.find("this.print") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.print" << std::endl; return true; }
            if (content.find("app.alert") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: app.alert" << std::endl; return true; }
            if (content.find("/Action") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Action" << std::endl; return true; }
            if (content.find("/Names") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Names" << std::endl; return true; }
            if (content.find("/AcroForm") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /AcroForm" << std::endl; return true; }
            if (content.find("/XFA") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /XFA" << std::endl; return true; }
            if (content.find("eval(") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: eval(" << std::endl; return true; }
            if (content.find("unescape(") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: unescape(" << std::endl; return true; }
            if (content.find("String.fromCharCode") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: String.fromCharCode" << std::endl; return true; }
            if (content.find("app.launchURL") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: app.launchURL" << std::endl; return true; }
            if (content.find("this.getURL") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.getURL" << std::endl; return true; }
            if (content.find("this.submitForm") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.submitForm" << std::endl; return true; }
            if (content.find("this.mailDoc") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.mailDoc" << std::endl; return true; }
            if (content.find("/EmbeddedFile") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /EmbeddedFile" << std::endl; return true; }
            if (content.find("/EF") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /EF" << std::endl; return true; }
            if (content.find("/F") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /F" << std::endl; return true; }
            if (content.find("/URI") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /URI" << std::endl; return true; }
            
            // Keep only last part to check across chunk boundaries
            if (content.length() > 1000) {
                content = content.substr(content.length() - 500);
            }
        }
        std::cout << "[detectPdfJavaScript] No keywords found in: " << filePath << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[detectPdfJavaScript] Exception: " << e.what() << " while processing file: " << filePath << std::endl;
        return false; // Assume safe if we can't read
    }
}

int main() {
    std::string testFile = "/Volumes/Crucial/AVProjectUi/malicious_test.pdf";
    
    std::cout << "Testing PDF JavaScript detection on: " << testFile << std::endl;
    
    bool hasJS = detectPdfJavaScript(testFile);
    
    std::cout << "Result: " << (hasJS ? "JavaScript DETECTED" : "No JavaScript found") << std::endl;
    
    return 0;
}
