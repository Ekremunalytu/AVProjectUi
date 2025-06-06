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
            
            // Skip patterns that have been sanitized (contain _REMOVED, _DISABLED suffixes)
            if (content.find("/JavaScript_REMOVED") != std::string::npos ||
                content.find("/JS_REMOVED") != std::string::npos ||
                content.find("/Action_DISABLED") != std::string::npos ||
                content.find("/OpenAction_DISABLED") != std::string::npos ||
                content.find("/AA_DISABLED") != std::string::npos ||
                content.find("/Launch_DISABLED") != std::string::npos ||
                content.find("//app.alert") != std::string::npos ||
                content.find("//eval(") != std::string::npos) {
                std::cout << "[detectPdfJavaScript] Found sanitized patterns - file appears to have been cleaned" << std::endl;
            }
            
            // Check for active JavaScript patterns (but ignore sanitized ones)
            if (content.find("/JavaScript") != std::string::npos && content.find("/JavaScript_REMOVED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /JavaScript" << std::endl; return true; 
            }
            if (content.find("/JS") != std::string::npos && content.find("/JS_REMOVED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /JS" << std::endl; return true; 
            }
            if (content.find("app.alert") != std::string::npos && content.find("//app.alert") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: app.alert" << std::endl; return true; 
            }
            if (content.find("eval(") != std::string::npos && content.find("//eval(") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: eval(" << std::endl; return true; 
            }
            if (content.find("/OpenAction") != std::string::npos && content.find("/OpenAction_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /OpenAction" << std::endl; return true; 
            }
            if (content.find("/AA") != std::string::npos && content.find("/AA_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /AA" << std::endl; return true; 
            }
            if (content.find("/Launch") != std::string::npos && content.find("/Launch_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /Launch" << std::endl; return true; 
            }
            if (content.find("/Names") != std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /Names (note: this might be benign PDF structure)" << std::endl; 
            }
            
            // Keep only last part to check across chunk boundaries
            if (content.length() > 1000) {
                content = content.substr(content.length() - 500);
            }
        }
        std::cout << "[detectPdfJavaScript] No active JavaScript patterns found in: " << filePath << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[detectPdfJavaScript] Exception: " << e.what() << " while processing file: " << filePath << std::endl;
        return false; // Assume safe if we can't read
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <pdf_file_path>" << std::endl;
        return 1;
    }
    
    std::string testFile = argv[1];
    
    std::cout << "Testing PDF JavaScript detection on: " << testFile << std::endl;
    
    bool hasJS = detectPdfJavaScript(testFile);
    
    std::cout << "Result: " << (hasJS ? "JavaScript DETECTED" : "No active JavaScript found") << std::endl;
    
    return 0;
}
