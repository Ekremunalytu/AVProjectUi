#include "cdr/CdrSanitizer.h"
#include "cdr/CdrTypes.h"
#include "cdr/FileSanitizer.h"
#include "cdr/FilesystemCompat.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstdio>
#include <ctime>

// Use CDR filesystem compatibility layer
namespace fs = CDR::FileSystem;

namespace CDR {

// === Helper: File Type Detection (Internal to this CPP) ===
FileType detectFileTypeInternal(const std::string& filePath) {
    try {
        fs::Path pathObj(filePath); // Renamed to avoid conflict with function parameter
        if (!fs::exists(pathObj)) {
            // Optional: Log or handle non-existent file before trying to get extension
            // std::cerr << "detectFileTypeInternal: File does not exist - " << filePath << std::endl;
            return FileType::UNKNOWN_FILE; // Or some other appropriate default/error type
        }
        std::string extension = pathObj.extension();
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }

        if (extension == "docx" || extension == "xlsx" || extension == "pptx" ||
            extension == "docm" || extension == "xlsm" || extension == "pptm" ||
            extension == "doc" || extension == "xls" || extension == "ppt") {
            return FileType::OFFICE_DOCUMENT;
        }
        if (extension == "html" || extension == "htm") {
            return FileType::HTML_DOCUMENT;
        }
        if (extension == "pdf") {
            return FileType::PDF_DOCUMENT;
        }
        if (extension == "exe" || extension == "dll" || extension == "com" ||
            extension == "bat" || extension == "cmd" || extension == "msi" || extension == "app" || extension == "dmg") {
            return FileType::EXECUTABLE_FILE;
        }
        if (extension == "js" || extension == "vbs" || extension == "ps1" ||
            extension == "sh" || extension == "py" || extension == "rb" || extension == "pl") {
            return FileType::SCRIPT_FILE;
        }
        if (extension == "zip" || extension == "rar" || extension == "7z" ||
            extension == "tar" || extension == "gz" || extension == "bz2") {
            return FileType::ARCHIVE_FILE;
        }
        if (extension == "jpg" || extension == "jpeg" || extension == "png" ||
            extension == "gif" || extension == "bmp" || extension == "tiff" || extension == "svg" || extension == "webp") {
            return FileType::IMAGE_FILE;
        }
        if (extension == "xml") {
            return FileType::XML_DOCUMENT;
        }
        if (extension == "rtf") {
            return FileType::RTF_DOCUMENT;
        }
        if (extension == "txt" || extension == "csv" || extension == "log") {
            return FileType::TEXT_DOCUMENT;
        }
         if (extension == "eml" || extension == "msg") {
            return FileType::EMAIL_FILE;
        }

        return FileType::UNKNOWN_FILE;
    } catch (const fs::FilesystemError& fs_err) {
        std::cerr << "Filesystem error in detectFileTypeInternal for " << filePath << ": " << fs_err.what() << std::endl;
        return FileType::UNKNOWN_FILE;
    }
     catch (const std::exception& e) {
        std::cerr << "Generic error in detectFileTypeInternal for " << filePath << ": " << e.what() << std::endl;
        return FileType::UNKNOWN_FILE;
    }
}

// === Helper: Read PDF header safely ===
std::string readPdfHeader(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        char header[8]; // Read first 8 bytes to be safe
        if (file.read(header, 8)) {
            return std::string(header, 8);
        }
        return "";
    } catch (const std::exception& e) {
        return "";
    }
}

// === Helper: Simple PDF JavaScript detection ===
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
            // Skip sanitized patterns (those ending with _REMOVED, _DISABLED, or commented out)
            
            // Check for active JavaScript patterns (but not sanitized ones)
            if (content.find("/JavaScript") != std::string::npos && 
                content.find("/JavaScript_REMOVED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /JavaScript" << std::endl; return true; 
            }
            if (content.find("/JS") != std::string::npos && 
                content.find("/JS_REMOVED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /JS" << std::endl; return true; 
            }
            if (content.find("/OpenAction") != std::string::npos && 
                content.find("/OpenAction_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /OpenAction" << std::endl; return true; 
            }
            if (content.find("/AA") != std::string::npos && 
                content.find("/AA_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /AA" << std::endl; return true; 
            }
            if (content.find("/Launch") != std::string::npos && 
                content.find("/Launch_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /Launch" << std::endl; return true; 
            }
            if (content.find("/Action") != std::string::npos && 
                content.find("/Action_DISABLED") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: /Action" << std::endl; return true; 
            }
            
            // Check for JavaScript function calls (but not commented out ones)
            if (content.find("this.print") != std::string::npos && 
                content.find("//this.print") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: this.print" << std::endl; return true; 
            }
            if (content.find("app.alert") != std::string::npos && 
                content.find("//app.alert") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: app.alert" << std::endl; return true; 
            }
            if (content.find("app.launchURL") != std::string::npos && 
                content.find("//app.launchURL") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: app.launchURL" << std::endl; return true; 
            }
            if (content.find("eval(") != std::string::npos && 
                content.find("//eval(") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: eval(" << std::endl; return true; 
            }
            if (content.find("String.fromCharCode") != std::string::npos && 
                content.find("//String.fromCharCode") == std::string::npos) { 
                std::cout << "[detectPdfJavaScript] Found: String.fromCharCode" << std::endl; return true; 
            }
            
            // Other patterns that don't get sanitized but should be detected
            if (content.find("/Names") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Names" << std::endl; return true; }
            if (content.find("/AcroForm") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /AcroForm" << std::endl; return true; }
            if (content.find("/XFA") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /XFA" << std::endl; return true; }
            if (content.find("unescape(") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: unescape(" << std::endl; return true; }
            if (content.find("String.fromCharCode") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: String.fromCharCode" << std::endl; return true; } // JS obfuscation technique
            if (content.find("app.launchURL") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: app.launchURL" << std::endl; return true; }    // Launching URLs
            if (content.find("this.getURL") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.getURL" << std::endl; return true; }      // Getting URLs
            if (content.find("this.submitForm") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.submitForm" << std::endl; return true; }  // Submitting forms
            if (content.find("this.mailDoc") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: this.mailDoc" << std::endl; return true; }     // Mailing documents
            if (content.find("/EmbeddedFile") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /EmbeddedFile" << std::endl; return true; }    // Embedded files
            if (content.find("/EF") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /EF" << std::endl; return true; }              // Embedded File stream dictionary
            if (content.find("/F") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /F" << std::endl; return true; }               // File Specification dictionary (often with /EF)
            if (content.find("/URI") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /URI" << std::endl; return true; }             // URI actions
            if (content.find("/RichMedia") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /RichMedia" << std::endl; return true; }       // Rich media annotations (can embed Flash, etc.)
            if (content.find("/FlashVars") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /FlashVars" << std::endl; return true; }       // Variables for Flash content
            if (content.find("getAnnots") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: getAnnots" << std::endl; return true; }        // Accessing annotations (can be part of exploits)
            if (content.find("Collab.getIcon") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: Collab.getIcon" << std::endl; return true; }   // Known past vulnerability pattern
            if (content.find("util.printf") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: util.printf" << std::endl; return true; }      // Can be part of format string vulnerabilities
            if (content.find("/GoToR") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /GoToR" << std::endl; return true; }           // Remote Go-To action (to external PDF)
            if (content.find("/GoToE") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /GoToE" << std::endl; return true; }           // Embedded Go-To action (to embedded files)
            if (content.find("/ObjStm") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /ObjStm" << std::endl; return true; }          // Object Stream (can hide objects)
            if (content.find("/Annot") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Annot" << std::endl; return true; }           // Annotations can have actions
            if (content.find("/Widget") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Widget" << std::endl; return true; }          // Form field widgets can have actions
            if (content.find("/Filter") != std::string::npos &&          // Check for suspicious filter combinations
                 (content.find("/ASCIIHexDecode") != std::string::npos ||
                  content.find("/LZWDecode") != std::string::npos ||
                  content.find("/JBIG2Decode") != std::string::npos) ) { std::cout << "[detectPdfJavaScript] Found: /Filter with suspicious decode" << std::endl; return true; } // JBIG2Decode has known exploits
            if (content.find("/Encrypt") != std::string::npos) { std::cout << "[detectPdfJavaScript] Found: /Encrypt" << std::endl; return true; }           // Encrypted objects might hide malicious content
            
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

// === Helper: Simple script analysis ===
bool analyzeScriptContent(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Convert to lowercase for analysis
            std::string lowerLine = line;
            std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
            
            // Check for suspicious patterns
            if (lowerLine.find("eval(") != std::string::npos ||
                lowerLine.find("exec(") != std::string::npos ||
                lowerLine.find("system(") != std::string::npos ||
                lowerLine.find("shell_exec") != std::string::npos ||
                lowerLine.find("powershell") != std::string::npos ||
                lowerLine.find("cmd.exe") != std::string::npos ||
                lowerLine.find("download") != std::string::npos ||
                lowerLine.find("invoke-expression") != std::string::npos) {
                return true; // Suspicious content found
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        return true; // If we can't analyze, assume suspicious
    }
}

// === Helper: Image metadata detection ===
bool detectImageMetadata(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read first few KB to check for EXIF data
        const size_t CHECK_SIZE = 8192;
        std::vector<char> buffer(CHECK_SIZE);
        file.read(buffer.data(), CHECK_SIZE);
        
        std::string content(buffer.begin(), buffer.end());
        
        // Check for EXIF markers
        if (content.find("Exif") != std::string::npos ||
            content.find("EXIF") != std::string::npos ||
            content.find("GPS") != std::string::npos ||
            content.find("Camera") != std::string::npos ||
            content.find("Adobe") != std::string::npos) {
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}
bool detectOfficeMacros(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read file in chunks
        const size_t CHUNK_SIZE = 4096;
        char buffer[CHUNK_SIZE];
        std::string content;
        
        while (file.read(buffer, CHUNK_SIZE) || file.gcount() > 0) {
            content.append(buffer, static_cast<size_t>(file.gcount()));
            
            // Check for macro indicators
            if (content.find("vbaProject") != std::string::npos ||
                content.find("macros/") != std::string::npos ||
                content.find("Microsoft VBA") != std::string::npos ||
                content.find("VBA") != std::string::npos) {
                return true;
            }
            
            // Keep only last part for boundary checks
            if (content.length() > 1000) {
                content = content.substr(content.length() - 500);
            }
        }
        
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}
std::string getFileTypeName(FileType fileType) {
    switch (fileType) {
        case FileType::NOT_SET:
            return "Not Set";
        case FileType::OFFICE_DOCUMENT:
            return "Office Document";
        case FileType::PDF_DOCUMENT:
            return "PDF Document";
        case FileType::HTML_DOCUMENT:
            return "HTML Document";
        case FileType::XML_DOCUMENT:
            return "XML Document";
        case FileType::RTF_DOCUMENT:
            return "RTF Document";
        case FileType::TEXT_DOCUMENT:
            return "Text Document";
        case FileType::ARCHIVE_FILE:
            return "Archive File";
        case FileType::IMAGE_FILE:
            return "Image File";
        case FileType::EXECUTABLE_FILE:
            return "Executable File";
        case FileType::SCRIPT_FILE:
            return "Script File";
        case FileType::EMAIL_FILE:
            return "Email File";
        case FileType::UNKNOWN_FILE:
            return "Unknown File";
        default:
            return "Unknown Type";
    }
}

// === PDF Sanitization Helper Method ===
bool sanitizePdfJavaScript(const std::string& inputPath, const std::string& outputPath) {
    try {
        std::cout << "[sanitizePdfJavaScript] Attempting to sanitize PDF: " << inputPath << " -> " << outputPath << std::endl;
        
        // Read the input PDF file
        std::ifstream inputFile(inputPath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "[sanitizePdfJavaScript] Error: Could not open input file: " << inputPath << std::endl;
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(inputFile)),
                          std::istreambuf_iterator<char>());
        inputFile.close();
        
        if (content.empty()) {
            std::cerr << "[sanitizePdfJavaScript] Error: Input file is empty or could not be read" << std::endl;
            return false;
        }
        
        std::cout << "[sanitizePdfJavaScript] Original file size: " << content.length() << " bytes" << std::endl;
        
        // Keep track of changes made
        bool contentModified = false;
        size_t removedPatterns = 0;
        
        // Define JavaScript and active content patterns to remove/neutralize
        std::vector<std::pair<std::string, std::string>> sanitizationPatterns = {
            // JavaScript keywords and objects
            {"/JavaScript", "/JavaScript_REMOVED"},
            {"/JS", "/JS_REMOVED"},
            
            // JavaScript function calls - neutralize by commenting out
            {"app.alert", "//app.alert"},
            {"app.launchURL", "//app.launchURL"},
            {"this.print", "//this.print"},
            {"this.getURL", "//this.getURL"},
            {"this.submitForm", "//this.submitForm"},
            {"this.mailDoc", "//this.mailDoc"},
            
            // Dangerous JavaScript functions
            {"eval(", "//eval("},
            {"unescape(", "//unescape("},
            {"String.fromCharCode", "//String.fromCharCode"},
            {"document.write", "//document.write"},
            
            // PDF action types
            {"/OpenAction", "/OpenAction_DISABLED"},
            {"/AA", "/AA_DISABLED"},
            {"/Launch", "/Launch_DISABLED"},
            {"/Action", "/Action_DISABLED"},
            
            // Form and XFA related (can contain JavaScript)
            {"/AcroForm", "/AcroForm_DISABLED"},
            {"/XFA", "/XFA_DISABLED"}
        };
        
        // Apply sanitization patterns
        for (const auto& pattern : sanitizationPatterns) {
            size_t pos = 0;
            size_t patternCount = 0;
            
            while ((pos = content.find(pattern.first, pos)) != std::string::npos) {
                content.replace(pos, pattern.first.length(), pattern.second);
                pos += pattern.second.length();
                patternCount++;
                contentModified = true;
            }
            
            if (patternCount > 0) {
                std::cout << "[sanitizePdfJavaScript] Replaced " << patternCount 
                         << " occurrences of '" << pattern.first << "' with '" << pattern.second << "'" << std::endl;
                removedPatterns += patternCount;
            }
        }
        
        // Additional aggressive sanitization: Remove JavaScript streams entirely
        std::regex jsStreamRegex(R"(/JavaScript\s*<<[^>]*>>)", std::regex::icase);
        std::string jsReplacement = "/JavaScript_REMOVED << /Length 0 >>";
        if (std::regex_search(content, jsStreamRegex)) {
            content = std::regex_replace(content, jsStreamRegex, jsReplacement);
            contentModified = true;
            removedPatterns++;
            std::cout << "[sanitizePdfJavaScript] Removed JavaScript stream objects" << std::endl;
        }
        
        if (!contentModified) {
            std::cout << "[sanitizePdfJavaScript] No threatening patterns found to sanitize" << std::endl;
            // Still copy the file to output location
            std::ofstream outputFile(outputPath, std::ios::binary);
            if (outputFile.is_open()) {
                outputFile.write(content.c_str(), content.length());
                outputFile.close();
                return true;
            }
            return false;
        }
        
        std::cout << "[sanitizePdfJavaScript] Successfully sanitized " << removedPatterns 
                 << " threatening patterns. New file size: " << content.length() << " bytes" << std::endl;
        
        // Write the sanitized content to output file
        std::ofstream outputFile(outputPath, std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "[sanitizePdfJavaScript] Error: Could not create output file: " << outputPath << std::endl;
            return false;
        }
        
        outputFile.write(content.c_str(), content.length());
        outputFile.close();
        
        std::cout << "[sanitizePdfJavaScript] Sanitized PDF written successfully to: " << outputPath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[sanitizePdfJavaScript] Exception during sanitization: " << e.what() << std::endl;
        return false;
    }
}

// === FileSanitizer Base Class Protected Method Implementations ===

std::string FileSanitizer::calculateMD5(const std::string& filePath) {
    // Simple offline MD5 implementation without external libraries
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return "error-calculating-md5";
        }
        
        // Read file in chunks and calculate a simple hash
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Simple hash algorithm (not real MD5 but good enough for file tracking)
        std::hash<std::string> hasher;
        size_t hash = hasher(content);
        
        // Convert to hex string
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    } catch (const std::exception& e) {
        return "error-calculating-hash";
    }
}

bool FileSanitizer::copyFile(const std::string& src, const std::string& dst) {
    try {
        if (!fs::exists(src)) {
            std::cerr << "copyFile: Source file " << src << " does not exist." << std::endl;
            return false;
        }
        fs::Path dstPathObj(dst);
        if (!dstPathObj.parent_path().empty() && !fs::exists(dstPathObj.parent_path())) {
            fs::create_directories(dstPathObj.parent_path());
        }
        fs::copy_file(src, dstPathObj.string());
        return true;
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Failed to copy file from " << src << " to " << dst << ": " << e.what() << std::endl;
        return false;
    }
}

std::string FileSanitizer::readFileContent(const std::string& filePath) {
    try {
        if (!fs::exists(filePath)) {
             std::cerr << "readFileContent: File " << filePath << " does not exist." << std::endl;
            return "";
        }
        auto fileSize = fs::file_size(filePath);
        constexpr size_t MAX_FILE_SIZE_READ = 50 * 1024 * 1024;
        if (fileSize > MAX_FILE_SIZE_READ) {
            std::cerr << "File too large for content reading: " << filePath << ", size: " << fileSize << " bytes" << std::endl;
            return "";
        }

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return "";
        }
        
        // Read file content properly without corrupting binary data
        std::string content;
        content.resize(static_cast<size_t>(fileSize));
        if (file.read(&content[0], static_cast<std::streamsize>(fileSize))) {
            return content;
        }
        return "";
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Filesystem error reading file content from " << filePath << ": " << e.what() << std::endl;
        return "";
    }
     catch (const std::exception& e) {
        std::cerr << "Error reading file content from " << filePath << ": " << e.what() << std::endl;
        return "";
    }
}

bool FileSanitizer::writeFileContent(const std::string& filePath, const std::string& content) {
    try {
        fs::Path filePathObj(filePath);
        if (!filePathObj.parent_path().empty() && !fs::exists(filePathObj.parent_path())) {
            fs::create_directories(filePathObj.parent_path());
        }
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }
        file << content;
        return file.good();
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Filesystem exception while writing file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while writing file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

long long FileSanitizer::getFileSize(const std::string& filePath) {
    try {
        if (!fs::exists(filePath)) return 0;
        return static_cast<long long>(fs::file_size(filePath));
    } catch (const fs::FilesystemError& e) {
        std::cerr << "Error getting file size for " << filePath << ": " << e.what() << std::endl;
        return 0;
    }
}

// === OfficeSanitizer Implementation ===
SanitizationResult OfficeSanitizer::sanitize(const std::string& inputPath,
                                           const std::string& outputPath,
                                           const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalPath = inputPath;
    result.sanitizedPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::OFFICE_DOCUMENT;

    if (config.blockOfficeMacros && containsMacros(inputPath)) {
        result.success = false;
        result.errorMessage = "Office file blocked due to macro policy.";
        result.threatsDetected.push_back("MACROS_PRESENT");
        result.requiresQuarantine = true;
        result.quarantineReason = "Contains macros, blocked by policy.";
        return result;
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_FULL_SANITIZATION");
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy Office document during sanitization.";
    }
    return result;
}

bool OfficeSanitizer::canHandle(FileType type) const {
    return type == FileType::OFFICE_DOCUMENT;
}

std::vector<std::string> OfficeSanitizer::getDetectableThreats() const {
    return {"MACROS", "EMBEDDED_OBJECTS", "EXTERNAL_LINKS", "MALFORMED_XML"};
}

bool OfficeSanitizer::containsMacros(const std::string& filePath) {
    return detectOfficeMacros(filePath);
}
// OfficeSanitizer private method implementations
bool OfficeSanitizer::removeMacros(const std::string& zipPath) {
    try {
        // For real implementation, would need to:
        // 1. Extract ZIP to temp directory
        // 2. Look for vbaProject.bin files
        // 3. Remove them or clear their content
        // 4. Recreate the ZIP
        
        // Simple check for now - if macros detected, we already block in main sanitize()
        return !detectOfficeMacros(zipPath);
    } catch (const std::exception& e) {
        return false;
    }
}

bool OfficeSanitizer::removeExternalLinks(const std::string& zipPath) {
    try {
        // For real implementation, would need to:
        // 1. Extract and parse XML files
        // 2. Remove external references, hyperlinks
        // 3. Recreate clean ZIP
        
        // Placeholder: assume success if file exists
        return fs::exists(zipPath);
    } catch (const std::exception& e) {
        return false;
    }
}

bool OfficeSanitizer::removeEmbeddedObjects(const std::string& zipPath) {
    try {
        // Would remove OLE objects, embedded files from Office documents
        return fs::exists(zipPath);
    } catch (const std::exception& e) {
        return false;
    }
}

bool OfficeSanitizer::sanitizeXMLContent(const std::string& xmlContent, std::string& sanitized) {
    try {
        sanitized = xmlContent;
        
        // Remove common dangerous XML content
        sanitized = removeHyperlinksFromXml(sanitized);
        sanitized = removeOleObjectsFromXml(sanitized);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> OfficeSanitizer::extractZipFiles(const std::string& zipPath, const std::string& tempDir) {
    std::vector<std::string> extractedFiles;
    try {
        // Real implementation would use ZIP library to extract files
        // For now, return empty list indicating no extraction
        if (fs::exists(zipPath) && fs::exists(tempDir)) {
            // Placeholder: would extract ZIP contents here
        }
        return extractedFiles;
    } catch (const std::exception& e) {
        return extractedFiles;
    }
}

bool OfficeSanitizer::recreateZipFile(const std::vector<std::string>& files, const std::string& outputPath) {
    try {
        // Real implementation would recreate ZIP from file list
        // For now, just check if we have files and output path
        return !files.empty() && !outputPath.empty();
    } catch (const std::exception& e) {
        return false;
    }
}

std::string OfficeSanitizer::removeHyperlinksFromXml(const std::string& xmlContent) {
    std::string result = xmlContent;
    try {
        // Remove hyperlink references
        std::regex hyperlinkRegex(R"(<w:hyperlink[^>]*>.*?</w:hyperlink>)", std::regex_constants::icase);
        result = std::regex_replace(result, hyperlinkRegex, "");
        
        // Remove external relationships
        std::regex relationshipRegex(R"(r:id="[^"]*")", std::regex_constants::icase);
        result = std::regex_replace(result, relationshipRegex, "");
        
    } catch (const std::exception& e) {
        // If regex fails, return original content
    }
    return result;
}

std::string OfficeSanitizer::removeOleObjectsFromXml(const std::string& xmlContent) {
    std::string result = xmlContent;
    try {
        // Remove OLE object references
        std::regex oleRegex(R"(<w:object[^>]*>.*?</w:object>)", std::regex_constants::icase);
        result = std::regex_replace(result, oleRegex, "");
        
        // Remove embedded object references
        std::regex embedRegex(R"(<o:OLEObject[^>]*/>)", std::regex_constants::icase);
        result = std::regex_replace(result, embedRegex, "");
        
    } catch (const std::exception& e) {
        // If regex fails, return original content
    }
    return result;
}


// === PdfSanitizer Implementation ===

// For actual PDF manipulation, you would include a library like Poppler or PDFium.
// Example (conceptual - ensure you have the library and link against it):
// #include <poppler-cpp.h>
// #include <poppler-global.h>
// #include <poppler-document.h>
// #include <poppler-page.h>
// #include <poppler-action.h>
// #include <poppler-form.h>

SanitizationResult PdfSanitizer::sanitize(const std::string& inputPath,
                                        const std::string& outputPath,
                                        const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.fileType = FileType::PDF_DOCUMENT;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.success = false; // Default to failure

    std::cout << "[PdfSanitizer::sanitize] Starting PDF sanitization for: " << inputPath << " -> " << outputPath << std::endl;

    if (inputPath.empty() || outputPath.empty()) {
        result.errorMessage = "Input or output path is empty.";
        std::cerr << "[PdfSanitizer::sanitize] Error: " << result.errorMessage << std::endl;
        return result;
    }

    std::cout << "[PdfSanitizer::sanitize] Checking if input file exists: " << inputPath << std::endl;
    if (!fs::exists(inputPath)) {
        result.errorMessage = "Input file does not exist: " + inputPath;
        std::cerr << "[PdfSanitizer::sanitize] Error: " << result.errorMessage << std::endl;
        return result;
    }
    std::cout << "[PdfSanitizer::sanitize] Input file exists. Size: " << result.originalSize << " bytes." << std::endl;

    // --- Placeholder for actual PDF sanitization logic using a library ---
    // In a real implementation, you would use a PDF library to:
    // 1. Parse the PDF.
    // 2. Identify and remove/neutralize malicious content (JavaScript, actions, embedded files).
    // 3. Reconstruct a clean PDF.

    bool actualSanitizationPerformed = false;
    bool threatsFoundInPdf = false;

    std::cout << "[PdfSanitizer::sanitize] Calling detectPdfJavaScript for: " << inputPath << std::endl;
    bool hasActiveContent = detectPdfJavaScript(inputPath);
    std::cout << "[PdfSanitizer::sanitize] detectPdfJavaScript returned: " << (hasActiveContent ? "true" : "false") << std::endl;

    if (hasActiveContent) { 
        threatsFoundInPdf = true; 
        result.threatsDetected.push_back("JavaScript");
        result.threatsDetected.push_back("PDF_JavaScript_Detected");
        
        // Attempt to sanitize the PDF by removing JavaScript content
        std::cout << "[PdfSanitizer::sanitize] Attempting to sanitize PDF with JavaScript: " << inputPath << " -> " << outputPath << std::endl;
        
        if (sanitizePdfJavaScript(inputPath, outputPath)) {
            actualSanitizationPerformed = true;
            result.success = true;
            result.outputPath = outputPath;
            result.sanitizedPath = outputPath;
            result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
            result.md5Hash = FileSanitizer::calculateMD5(outputPath);
            result.actionsPerformed.push_back("JAVASCRIPT_REMOVED");
            result.actionsPerformed.push_back("PDF_SANITIZED");
            result.sanitizationDetails = "JavaScript and active content successfully removed from PDF";
            
            // Verify the sanitization was successful
            if (!detectPdfJavaScript(outputPath)) {
                std::cout << "[PdfSanitizer::sanitize] Sanitization successful - no JavaScript detected in output file." << std::endl;
            } else {
                std::cout << "[PdfSanitizer::sanitize] Warning: JavaScript still detected after sanitization." << std::endl;
                result.sanitizationDetails += " (Warning: Some JavaScript may remain)";
            }
        } else {
            // If sanitization fails, quarantine the original file
            std::cout << "[PdfSanitizer::sanitize] Sanitization failed, copying for quarantine: " << inputPath << " -> " << outputPath << std::endl;
            if (copyFile(inputPath, outputPath)) {
                result.success = true;
                result.outputPath = outputPath;
                result.sanitizedPath = outputPath;
                result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
                result.md5Hash = FileSanitizer::calculateMD5(outputPath);
                result.requiresQuarantine = true; 
                result.quarantineReason = "PDF contains JavaScript that could not be safely removed";
                result.actionsPerformed.push_back("COPIED_FOR_QUARANTINE");
                result.sanitizationDetails = "JavaScript detected but sanitization failed - file quarantined";
            } else {
                result.success = false;
                result.errorMessage = "Failed to sanitize PDF and failed to copy for quarantine";
            }
        }
        
        return result; 
    }

    // Further placeholder checks for embedded files, actions, etc., would go here.
    // For brevity, these are omitted but would follow a similar pattern:
    // - Call a (hypothetical) library function or internal helper to detect the threat.
    // - If a threat is found, update `threatsFoundInPdf`, `result.threatsDetected`.
    // - If policy dictates blocking, set error message and quarantine, then return.
    // - Otherwise, update `result.actionsPerformed`, `actualSanitizationPerformed`.

    std::cout << "[PdfSanitizer::sanitize] Attempting to copy file from " << inputPath << " to " << outputPath << std::endl;
    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedPath = outputPath;
        result.outputPath = outputPath;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        if (actualSanitizationPerformed) {
            result.sanitizationDetails = "PDF sanitized (simulated: JS and/or other threats handled).";
        } else if (threatsFoundInPdf) { // This branch should ideally not be hit if we return early
             result.sanitizationDetails = "PDF threats detected but sanitization actions are placeholders or were not fully applied.";
        } else {
            result.sanitizationDetails = "PDF copied. No specific threats detected by placeholder checks or no sanitization applied.";
            result.actionsPerformed.push_back("COPIED_AS_IS_NO_SPECIFIC_PDF_THREATS_HANDLED_BY_PLACEHOLDER");
        }
        std::cout << "[PdfSanitizer::sanitize] Placeholder: Copied file to " << outputPath << ". Size: " << result.sanitizedSize << ". " << result.sanitizationDetails << std::endl;
    } else {
        result.errorMessage = "Failed to copy PDF file during placeholder sanitization.";
        std::cerr << "[PdfSanitizer::sanitize] Error: " << result.errorMessage << std::endl;
    }
    // --- End of placeholder logic ---
    if(result.success){
        std::cout << "[PdfSanitizer::sanitize] PDF sanitization (placeholder) completed successfully for: " << inputPath << std::endl;
    } else {
        std::cout << "[PdfSanitizer::sanitize] PDF sanitization (placeholder) failed or file marked for quarantine for: " << inputPath << ". Reason: " << result.errorMessage << std::endl;
    }

    return result;
}

bool PdfSanitizer::canHandle(FileType type) const {
    return type == FileType::PDF_DOCUMENT;
}

std::vector<std::string> PdfSanitizer::getDetectableThreats() const {
    // These are example threats. Adjust them based on actual capabilities.
    return {"JAVASCRIPT_CONTENT", "EMBEDDED_FILES", "MALICIOUS_ACTIONS", "ENCRYPTED_CONTENT", "SUSPICIOUS_STRUCTURE"};
}


// === HtmlSanitizer Implementation ===
SanitizationResult HtmlSanitizer::sanitize(const std::string& inputPath,
                                         const std::string& outputPath,
                                         const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::HTML_DOCUMENT;

    std::string content = readFileContent(inputPath);
    if (content.empty() && result.originalSize > 0) {
        result.success = false;
        result.errorMessage = "Failed to read HTML file content.";
        return result;
    }

    std::string sanitizedContent = content;
    bool modified = false;

    sanitizedContent = removeScripts(sanitizedContent);
    if (sanitizedContent != content) { // A bit simplistic, better to check specific regex matches
        result.actionsPerformed.push_back("REMOVED_SCRIPT_TAGS");
        result.threatsDetected.push_back("JAVASCRIPT_CODE_IN_SCRIPT_TAGS");
        modified = true;
    }
    std::string prevContent = sanitizedContent;

    sanitizedContent = removeEventHandlers(sanitizedContent);
    if (sanitizedContent != prevContent) {
        result.actionsPerformed.push_back("REMOVED_EVENT_HANDLERS");
        result.threatsDetected.push_back("JAVASCRIPT_IN_EVENT_HANDLERS");
        modified = true;
        prevContent = sanitizedContent;
    }
    
    std::regex jsUrlRegex(R"((?:href|src|data|action)\s*=\s*["']?\s*javascript:[^"'>\s]+["']?)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    std::string tempContent = std::regex_replace(sanitizedContent, jsUrlRegex, "href=\"#blocked\"");
    if (tempContent != sanitizedContent) {
        sanitizedContent = tempContent;
        result.actionsPerformed.push_back("NEUTRALIZED_JAVASCRIPT_URLS");
        result.threatsDetected.push_back("JAVASCRIPT_URLS");
        modified = true;
        prevContent = sanitizedContent;
    }

    if (config.securityLevel >= CdrConfiguration::SecurityLevel::MEDIUM) {
        sanitizedContent = removeIframes(sanitizedContent);
         if (sanitizedContent != prevContent) {
            result.actionsPerformed.push_back("REMOVED_IFRAMES");
            result.threatsDetected.push_back("IFRAME_CONTENT");
            modified = true;
            prevContent = sanitizedContent;
        }
    }
    
    if (config.securityLevel >= CdrConfiguration::SecurityLevel::HIGH) {
        std::regex objectEmbedRegex(R"(<(?:object|embed)\b[^<]*(?:(?!<\/(?:object|embed)>)<[^<]*)*<\/(?:object|embed)>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
        tempContent = std::regex_replace(sanitizedContent, objectEmbedRegex, "<!-- object/embed removed -->");
        if (tempContent != sanitizedContent) {
            sanitizedContent = tempContent;
            result.actionsPerformed.push_back("REMOVED_OBJECT_EMBED_TAGS");
            result.threatsDetected.push_back("EMBEDDED_OBJECTS");
            modified = true;
            // prevContent = sanitizedContent; // Not strictly needed for last operation
        }
    }

    if (writeFileContent(outputPath, sanitizedContent)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        if (!modified && result.originalSize > 0) {
             result.actionsPerformed.push_back("PASSED_AS_IS_NO_THREATS_FOUND");
        } else if (!modified && result.originalSize == 0) {
            result.actionsPerformed.push_back("EMPTY_HTML_PROCESSED");
        }
    } else {
        result.success = false;
        result.errorMessage = "Failed to write sanitized HTML content.";
    }
    return result;
}

bool HtmlSanitizer::canHandle(FileType type) const {
    return type == FileType::HTML_DOCUMENT;
}

std::vector<std::string> HtmlSanitizer::getDetectableThreats() const {
    return {"JAVASCRIPT_CODE", "IFRAMES", "MALICIOUS_CSS", "EVENT_HANDLERS", "DANGEROUS_TAGS"};
}
// Implementations for HtmlSanitizer private methods
std::string HtmlSanitizer::removeScripts(const std::string& htmlContent) {
    std::regex scriptRegex(R"(<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, scriptRegex, "<!-- script removed -->");
}
std::string HtmlSanitizer::removeIframes(const std::string& htmlContent) {
    std::regex iframeRegex(R"(<iframe\b[^<]*(?:(?!<\/iframe>)<[^<]*)*<\/iframe>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, iframeRegex, "<!-- iframe removed -->");
}
std::string HtmlSanitizer::sanitizeCSS(const std::string& htmlContent) { 
    // Basic CSS sanitization - this is complex, placeholder for now
    // Could remove known dangerous properties or use a CSS parser/sanitizer library
    std::regex styleRegex(R"(<style\b[^<]*(?:(?!<\/style>)<[^<]*)*<\/style>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    // Example: remove all <style> tags for simplicity, or implement more granular logic
    // return std::regex_replace(htmlContent, styleRegex, "<!-- style block removed for safety -->");
    return htmlContent; // Placeholder: no change
}
std::string HtmlSanitizer::removeEventHandlers(const std::string& htmlContent) {
    std::regex eventHandlerRegex(R"(\s+on\w+\s*=\s*(?:"[^"]*"|'[^']*'|[^\s>]+))", std::regex_constants::icase | std::regex_constants::ECMAScript);
    return std::regex_replace(htmlContent, eventHandlerRegex, "");
}
std::string HtmlSanitizer::removeDangerousTags(const std::string& htmlContent) {
    // Example: remove <applet>, <embed>, <object> if not handled by higher security levels
    // std::regex dangerousTagRegex(R"(<(?:applet|object|embed)\b[^>]*>.*?<\/(?:applet|object|embed)>)", std::regex_constants::icase | std::regex_constants::ECMAScript);
    // return std::regex_replace(htmlContent, dangerousTagRegex, "<!-- dangerous tag removed -->");
    return htmlContent; // Placeholder
}
bool HtmlSanitizer::containsSuspiciousContent(const std::string& /*htmlContent*/) {
    // Placeholder for more advanced heuristic checks
    return false;
}


// === ScriptAnalyzer Implementation ===
SanitizationResult ScriptAnalyzer::sanitize(const std::string& inputPath,
                                          const std::string& outputPath,
                                          const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath; 
    result.originalSize = FileSanitizer::getFileSize(inputPath);    result.fileType = FileType::SCRIPT_FILE;    if (config.securityLevel >= CdrConfiguration::SecurityLevel::VERY_HIGH) { 
        result.success = false;
        result.errorMessage = "Script file quarantined due to strict security policy.";
        result.threatsDetected.push_back("SCRIPT_FILE_TYPE");
        result.actionsPerformed.push_back("QUARANTINED");
        result.requiresQuarantine = true;
        result.quarantineReason = "Script file type blocked by strict policy.";
        return result;
    }
    
    if (config.blockAllScripts) {
        result.success = false;
        result.errorMessage = "Script file blocked by configuration.";
        result.threatsDetected.push_back("SCRIPT_FILE_TYPE");
        result.actionsPerformed.push_back("BLOCKED_BY_POLICY");
        result.requiresQuarantine = true;
        result.quarantineReason = "All scripts are blocked by current policy.";
        return result;
    }

    // Analyze script content for threats
    std::string scriptContent = readFileContent(inputPath);
    if (!scriptContent.empty()) {
        bool hasSuspiciousContent = analyzeScriptContent(inputPath);
        bool hasObfuscation = containsObfuscation(scriptContent);
        
        if (hasSuspiciousContent) {
            result.threatsDetected.push_back("SUSPICIOUS_API_CALLS");
        }
        
        if (hasObfuscation) {
            result.threatsDetected.push_back("OBFUSCATION_DETECTED");
        }
        
        // If high security and threats detected, quarantine
        if ((hasSuspiciousContent || hasObfuscation) && 
            config.securityLevel >= CdrConfiguration::SecurityLevel::HIGH) {
            result.success = false;
            result.errorMessage = "Script file quarantined due to suspicious content.";
            result.requiresQuarantine = true;
            result.quarantineReason = "Script contains suspicious patterns or obfuscation.";
            return result;
        }
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_SCRIPT_ALLOWED");
        result.sanitizationDetails = "Script file allowed and copied. No content modification performed by default.";
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy script file.";
    }
    return result;
}

bool ScriptAnalyzer::canHandle(FileType type) const {
    return type == FileType::SCRIPT_FILE;
}

std::vector<std::string> ScriptAnalyzer::getDetectableThreats() const {
    return {"OBFUSCATION", "SUSPICIOUS_API_CALLS", "KNOWN_MALWARE_SIGNATURES"};
}
// ScriptAnalyzer private method implementations
bool ScriptAnalyzer::analyzeJavaScript(const std::string& content, std::vector<std::string>& threats) {
    bool hasThreats = false;
    
    try {
        // Check for dangerous JavaScript patterns
        if (content.find("eval") != std::string::npos) { threats.push_back("DANGEROUS_JS_FUNCTIONS"); hasThreats = true; std::cout << "Detected eval() in JavaScript." << std::endl; }
        if (content.find("Function(") != std::string::npos) { threats.push_back("DANGEROUS_JS_FUNCTIONS"); hasThreats = true; std::cout << "Detected Function() in JavaScript." << std::endl; }
        if (content.find("setTimeout(") != std::string::npos) { threats.push_back("DANGEROUS_JS_FUNCTIONS"); hasThreats = true; std::cout << "Detected setTimeout() in JavaScript." << std::endl; }
        if (content.find("setInterval(") != std::string::npos) { threats.push_back("DANGEROUS_JS_FUNCTIONS"); hasThreats = true; std::cout << "Detected setInterval() in JavaScript." << std::endl; }

        // Check for DOM manipulation
        if (content.find("document.write") != std::string::npos) { threats.push_back("DOM_MANIPULATION"); hasThreats = true; std::cout << "Detected document.write in JavaScript." << std::endl; }
        if (content.find("innerHTML") != std::string::npos) { threats.push_back("DOM_MANIPULATION"); hasThreats = true; std::cout << "Detected innerHTML in JavaScript." << std::endl; }
        if (content.find("outerHTML") != std::string::npos) { threats.push_back("DOM_MANIPULATION"); hasThreats = true; std::cout << "Detected outerHTML in JavaScript." << std::endl; }

        // Check for network operations
        if (content.find("XMLHttpRequest") != std::string::npos) { threats.push_back("NETWORK_OPERATIONS"); hasThreats = true; std::cout << "Detected XMLHttpRequest in JavaScript." << std::endl; }
        if (content.find("fetch(") != std::string::npos) { threats.push_back("NETWORK_OPERATIONS"); hasThreats = true; std::cout << "Detected fetch() in JavaScript." << std::endl; }
        if (content.find("websocket") != std::string::npos) { threats.push_back("NETWORK_OPERATIONS"); hasThreats = true; std::cout << "Detected websocket in JavaScript." << std::endl; }

        // Check for obfuscation
        if (containsObfuscation(content)) { threats.push_back("OBFUSCATED_JS"); hasThreats = true; std::cout << "Detected obfuscated JavaScript." << std::endl; }
        
    } catch (const std::exception& e) {
        // If analysis fails, assume suspicious
        threats.push_back("ANALYSIS_FAILED");
        hasThreats = true;
    }
    
    return hasThreats;
}

bool ScriptAnalyzer::analyzePowerShell(const std::string& content, std::vector<std::string>& threats) {
    bool hasThreats = false;
    
    try {
        // Check for dangerous PowerShell cmdlets
        if (content.find("Invoke-Expression") != std::string::npos ||
            content.find("IEX") != std::string::npos ||
            content.find("Invoke-Command") != std::string::npos ||
            content.find("Start-Process") != std::string::npos) {
            threats.push_back("DANGEROUS_PS_CMDLETS");
            hasThreats = true;
        }
        
        // Check for download operations
        if (content.find("Invoke-WebRequest") != std::string::npos ||
            content.find("wget") != std::string::npos ||
            content.find("curl") != std::string::npos ||
            content.find("DownloadString") != std::string::npos) {
            threats.push_back("DOWNLOAD_OPERATIONS");
            hasThreats = true;
        }
        
        // Check for base64 encoding (common in malware)
        if (content.find("FromBase64String") != std::string::npos ||
            content.find("[Convert]::") != std::string::npos) {
            threats.push_back("BASE64_ENCODING");
            hasThreats = true;
        }
        
        // Check for bypass techniques
        if (content.find("ExecutionPolicy") != std::string::npos ||
            content.find("Bypass") != std::string::npos ||
            content.find("Unrestricted") != std::string::npos) {
            threats.push_back("EXECUTION_POLICY_BYPASS");
            hasThreats = true;
        }
        
    } catch (const std::exception& e) {
        threats.push_back("ANALYSIS_FAILED");
        hasThreats = true;
    }
    
    return hasThreats;
}

bool ScriptAnalyzer::analyzeVBScript(const std::string& content, std::vector<std::string>& threats) {
    bool hasThreats = false;
    
    try {
        // Check for dangerous VBScript functions
        if (content.find("CreateObject") != std::string::npos ||
            content.find("GetObject") != std::string::npos ||
            content.find("Execute") != std::string::npos ||
            content.find("ExecuteGlobal") != std::string::npos) {
            threats.push_back("DANGEROUS_VBS_FUNCTIONS");
            hasThreats = true;
        }
        
        // Check for shell operations
        if (content.find("WScript.Shell") != std::string::npos ||
            content.find("Shell.Application") != std::string::npos ||
            content.find("Cmd.exe") != std::string::npos) {
            threats.push_back("SHELL_OPERATIONS");
            hasThreats = true;
        }
        
        // Check for file system operations
        if (content.find("FileSystemObject") != std::string::npos ||
            content.find("Scripting.FileSystemObject") != std::string::npos) {
            threats.push_back("FILE_SYSTEM_ACCESS");
            hasThreats = true;
        }
        
        // Check for network operations
        if (content.find("XMLHTTP") != std::string::npos ||
            content.find("WinHttp") != std::string::npos ||
            content.find("InternetExplorer") != std::string::npos) {
            threats.push_back("NETWORK_OPERATIONS");
            hasThreats = true;
        }
        
    } catch (const std::exception& e) {
        threats.push_back("ANALYSIS_FAILED");
        hasThreats = true;
    }
    
    return hasThreats;
}

bool ScriptAnalyzer::analyzeBatchScript(const std::string& content, std::vector<std::string>& threats) {
    bool hasThreats = false;
    
    try {
        // Check for dangerous batch commands
        if (content.find("format") != std::string::npos ||
            content.find("del ") != std::string::npos ||
            content.find("rmdir") != std::string::npos ||
            content.find("rd ") != std::string::npos) {
            threats.push_back("DESTRUCTIVE_COMMANDS");
            hasThreats = true;
        }
        
        // Check for registry operations
        if (content.find("reg add") != std::string::npos ||
            content.find("reg delete") != std::string::npos ||
            content.find("regedit") != std::string::npos) {
            threats.push_back("REGISTRY_MODIFICATION");
            hasThreats = true;
        }
        
        // Check for network operations
        if (content.find("ping") != std::string::npos ||
            content.find("telnet") != std::string::npos ||
            content.find("ftp") != std::string::npos ||
            content.find("curl") != std::string::npos) {
            threats.push_back("NETWORK_OPERATIONS");
            hasThreats = true;
        }
        
        // Check for system manipulation
        if (content.find("shutdown") != std::string::npos ||
            content.find("taskkill") != std::string::npos ||
            content.find("sc ") != std::string::npos ||
            content.find("net ") != std::string::npos) {
            threats.push_back("SYSTEM_MANIPULATION");
            hasThreats = true;
        }
        
    } catch (const std::exception& e) {
        threats.push_back("ANALYSIS_FAILED");
        hasThreats = true;
    }
    
    return hasThreats;
}
bool ScriptAnalyzer::containsObfuscation(const std::string& content) {
    // Simple obfuscation detection
    if (content.empty()) return false;
    
    // Count suspicious patterns
    size_t suspiciousCount = 0;
    
    // High ratio of special characters
    size_t specialChars = 0;
    for (char c : content) {
        if (!std::isalnum(c) && !std::isspace(c)) {
            specialChars++;
        }
    }
    
    if (specialChars > content.length() / 3) {
        suspiciousCount++;
    }
    
    // Check for common obfuscation patterns
    if (content.find("eval") != std::string::npos) suspiciousCount++;
    if (content.find("unescape") != std::string::npos) suspiciousCount++;
    if (content.find("fromCharCode") != std::string::npos) suspiciousCount++;
    if (content.find("\\x") != std::string::npos) suspiciousCount++;
    if (content.find("\\u") != std::string::npos) suspiciousCount++;
    
    return suspiciousCount >= 2;
}

bool ScriptAnalyzer::containsSuspiciousAPIs(const std::string& content) {
    return analyzeScriptContent(""); // We'll analyze content directly
}


// === ArchiveSanitizer Implementation ===
SanitizationResult ArchiveSanitizer::sanitize(const std::string& inputPath,
                                            const std::string& outputPath,
                                            const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::ARCHIVE_FILE;

    if (config.blockArchives) {
        result.success = false;
        result.errorMessage = "Archive file blocked by policy.";
        result.threatsDetected.push_back("ARCHIVE_FILE_TYPE");
        result.actionsPerformed.push_back("BLOCKED_BY_POLICY");
        result.requiresQuarantine = true;
        result.quarantineReason = "Archives are blocked by current policy.";
        return result;
    }
    
    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_FULL_ARCHIVE_SANITIZATION");
        result.sanitizationDetails = "Archive copied. Deep content sanitization not yet implemented.";
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy archive file.";
    }
    return result;
}

bool ArchiveSanitizer::canHandle(FileType type) const {
    return type == FileType::ARCHIVE_FILE;
}

std::vector<std::string> ArchiveSanitizer::getDetectableThreats() const {
    return {"NESTED_ARCHIVES", "EXECUTABLES_IN_ARCHIVE", "PASSWORD_PROTECTED_ARCHIVE", "MALICIOUS_FILES_IN_ARCHIVE"};
}
// ArchiveSanitizer private method implementations
bool ArchiveSanitizer::extractAndScanArchive(const std::string& archivePath, const std::string& tempDir) {
    try {
        // Real implementation would extract archive contents
        // For now, basic file existence check
        if (!fs::exists(archivePath) || !fs::exists(tempDir)) {
            return false;
        }
        
        // Would use libzip, 7zip, or similar to extract
        // Check for zip bombs, excessive compression ratios
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ArchiveSanitizer::sanitizeExtractedFiles(const std::string& tempDir, const CdrConfiguration& config) {
    try {
        // Would recursively sanitize each extracted file
        // Using appropriate sanitizer for each file type
        if (!fs::exists(tempDir)) {
            return false;
        }
        
        // Placeholder: assume sanitization successful
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ArchiveSanitizer::recreateCleanArchive(const std::string& tempDir, const std::string& outputPath) {
    try {
        // Would recreate archive from sanitized files
        if (!fs::exists(tempDir) || outputPath.empty()) {
            return false;
        }
        
        // Real implementation would use archive library
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ArchiveSanitizer::hasNestedArchives(const std::string& archivePath) {
    try {
        std::ifstream file(archivePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read archive header and check for nested archive signatures
        const size_t CHECK_SIZE = 2048;
        std::vector<char> buffer(CHECK_SIZE);
        file.read(buffer.data(), CHECK_SIZE);
        
        std::string content(buffer.begin(), buffer.end());
        
        // Check for nested archive headers within the content
        // This is a simplified check - real implementation would parse archive structure
        size_t zipCount = 0;
        size_t pos = 0;
        while ((pos = content.find("PK", pos)) != std::string::npos) {
            zipCount++;
            pos += 2;
            if (zipCount > 1) return true; // Multiple ZIP signatures suggest nesting
        }
        
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ArchiveSanitizer::containsExecutables(const std::string& archivePath) {
    try {
        // Real implementation would extract and check file extensions/headers
        // For now, check if file seems like it might contain executables
        std::ifstream file(archivePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read some content to look for executable signatures
        const size_t CHECK_SIZE = 4096;
        std::vector<char> buffer(CHECK_SIZE);
        file.read(buffer.data(), CHECK_SIZE);
        
        std::string content(buffer.begin(), buffer.end());
          // Look for executable file signatures within archive
        if (content.find("MZ") != std::string::npos ||       // PE executable
            content.find("\x7f""ELF") != std::string::npos ||   // ELF executable
            content.find("\xCA\xFE\xBA\xBE") != std::string::npos || // Mach-O
            content.find(".exe") != std::string::npos ||
            content.find(".dll") != std::string::npos ||
            content.find(".scr") != std::string::npos ||
            content.find(".com") != std::string::npos) {
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}


// === ImageSanitizer Implementation ===
SanitizationResult ImageSanitizer::sanitize(const std::string& inputPath,
                                          const std::string& outputPath,
                                          const CdrConfiguration& config) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalSize = FileSanitizer::getFileSize(inputPath);
    result.fileType = FileType::IMAGE_FILE;

    if (config.blockImagesWithMetadata && containsExcessiveMetadata(inputPath)) {
        result.success = false;
        result.errorMessage = "Image blocked due to metadata policy.";
        result.threatsDetected.push_back("EXCESSIVE_METADATA");
        result.requiresQuarantine = true;
        result.quarantineReason = "Image contains metadata blocked by policy.";
        return result;
    }

    if (copyFile(inputPath, outputPath)) {
        result.success = true;
        result.sanitizedSize = FileSanitizer::getFileSize(outputPath);
        result.md5Hash = FileSanitizer::calculateMD5(outputPath);
        result.actionsPerformed.push_back("COPIED_AS_IS_PENDING_METADATA_STRIPPING");
    } else {
        result.success = false;
        result.errorMessage = "Failed to copy image file.";
    }
    return result;
}

bool ImageSanitizer::canHandle(FileType type) const {
    return type == FileType::IMAGE_FILE;
}

std::vector<std::string> ImageSanitizer::getDetectableThreats() const {
    return {"EXIF_METADATA", "EMBEDDED_SCRIPTS_IN_SVG", "STEGANOGRAPHY_PATTERNS", "MALFORMED_IMAGE_DATA"};
}

bool ImageSanitizer::containsExcessiveMetadata(const std::string& filePath) {
    return detectImageMetadata(filePath);
}

// ImageSanitizer private method implementations
bool ImageSanitizer::stripMetadata(const std::string& inputPath, const std::string& outputPath) {
    try {
        // For real implementation, would use image processing library like ExifTool, ImageMagick, etc.
        // For now, basic copy without metadata stripping
        if (!fs::exists(inputPath)) {
            return false;
        }
        
        // Real implementation would:
        // 1. Read image data without metadata sections
        // 2. Create new image file with just pixel data
        // 3. Preserve quality while removing EXIF/XMP/IPTC data
        
        return copyFile(inputPath, outputPath);
    } catch (const std::exception& e) {
        return false;
    }
}

bool ImageSanitizer::sanitizeSVG(const std::string& svgContent, std::string& sanitized) {
    try {
        sanitized = svgContent;
        
        // Remove script tags from SVG
        std::regex scriptRegex(R"(<script[^>]*>.*?</script>)", std::regex_constants::icase);
        sanitized = std::regex_replace(sanitized, scriptRegex, "<!-- script removed -->");
        
        // Remove event handlers
        std::regex eventRegex(R"(\s+on\w+\s*=\s*(?:"[^"]*"|'[^']*'|[^\s>]+))", std::regex_constants::icase);
        sanitized = std::regex_replace(sanitized, eventRegex, "");
        
        // Remove javascript: URLs
        std::regex jsUrlRegex(R"((?:href|xlink:href)\s*=\s*["']?\s*javascript:[^"'>\s]+["']?)", std::regex_constants::icase);
        sanitized = std::regex_replace(sanitized, jsUrlRegex, "");
        
        // Remove foreign object elements that could contain HTML/scripts
        std::regex foreignObjectRegex(R"(<foreignObject[^>]*>.*?</foreignObject>)", std::regex_constants::icase);
        sanitized = std::regex_replace(sanitized, foreignObjectRegex, "<!-- foreign object removed -->");
        
        return sanitized != svgContent; // Return true if changes were made
    } catch (const std::exception& e) {
        return false;
    }
}

bool ImageSanitizer::checkSteganography(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Basic steganography detection - look for suspicious patterns
        // Real implementation would use more sophisticated algorithms
        
        // Check file size vs expected size for image dimensions
        auto fileSize = fs::file_size(filePath);
        
        // Read image header to get basic info
        std::vector<char> header(512);
        file.read(header.data(), 512);
        
        std::string headerStr(header.begin(), header.end());
        
        // Look for suspicious trailing data after image end markers
        if (headerStr.find("JPEG") != std::string::npos || headerStr.find("\xFF\xD8") != std::string::npos) {
            // For JPEG, check for data after FFD9 end marker
            file.seekg(-100, std::ios::end);
            std::vector<char> tail(100);
            file.read(tail.data(), 100);
            
            std::string tailStr(tail.begin(), tail.end());
            // Look for executable signatures or suspicious strings in tail
            if (tailStr.find("MZ") != std::string::npos || 
                tailStr.find("PK") != std::string::npos ||
                tailStr.find("#!/") != std::string::npos) {
                return true; // Suspicious trailing data
            }
        }
        
        // Check for unusual file size ratios that might indicate hidden data
        if (fileSize > 10 * 1024 * 1024) { // Files over 10MB are suspicious for basic images
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        return false; // Assume safe if can't analyze
    }
}

bool ImageSanitizer::validateImageStructure(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Read file header to validate basic image structure
        std::vector<char> header(32);
        file.read(header.data(), 32);
        
        std::string headerStr(header.begin(), header.end());
        
        // Check for valid image signatures
        // JPEG
        if (headerStr.substr(0, 2) == "\xFF\xD8") {
            return true;
        }
        
        // PNG
        if (headerStr.substr(0, 8) == "\x89PNG\x0D\x0A\x1A\x0A") {
            return true;
        }
        
        // GIF
        if (headerStr.substr(0, 6) == "GIF87a" || headerStr.substr(0, 6) == "GIF89a") {
            return true;
        }
        
        // BMP
        if (headerStr.substr(0, 2) == "BM") {
            return true;
        }
        
        // TIFF
        if (headerStr.substr(0, 4) == "II*\x00" || headerStr.substr(0, 4) == "MM\x00*") {
            return true;
        }
        
        // WebP
        if (headerStr.substr(8, 4) == "WEBP") {
            return true;
        }
        
        // SVG (XML-based)
        if (headerStr.find("<?xml") != std::string::npos || headerStr.find("<svg") != std::string::npos) {
            return true;
        }
        
        return false; // Unknown or invalid format
    } catch (const std::exception& e) {
        return false;
    }
}


// === CdrSanitizer (Manager Class) Implementation ===
CdrSanitizer::CdrSanitizer() {
    registerSanitizer(std::make_unique<OfficeSanitizer>());
    registerSanitizer(std::make_unique<PdfSanitizer>());
    registerSanitizer(std::make_unique<HtmlSanitizer>());
    registerSanitizer(std::make_unique<ScriptAnalyzer>());
    registerSanitizer(std::make_unique<ArchiveSanitizer>());
    registerSanitizer(std::make_unique<ImageSanitizer>()); 
}

CdrSanitizer::~CdrSanitizer() = default; 

void CdrSanitizer::registerSanitizer(std::unique_ptr<FileSanitizer> sanitizer) {
    if (sanitizer) {
        sanitizers_.push_back(std::move(sanitizer));
    }
}

SanitizationResult CdrSanitizer::sanitizeFile(const std::string& inputPath, 
                                            const std::string& outputPath, 
                                            const CdrConfiguration& config,
                                            FileType fileType) {
    SanitizationResult result;
    result.inputPath = inputPath;
    result.outputPath = outputPath;
    result.originalPath = inputPath;
    result.sanitizedPath = outputPath;
    result.fileType = fileType;

    // Validate input file exists
    if (!fs::exists(inputPath)) {
        result.success = false;
        result.errorMessage = "Input file does not exist: " + inputPath;
        return result;
    }

    // Validate file type before processing
    if (fileType == FileType::NOT_SET || fileType == FileType::UNKNOWN_FILE) {
        // Try to detect file type if not provided or unknown
        FileType detectedType = detectFileTypeInternal(inputPath);
        if (detectedType == FileType::UNKNOWN_FILE && !config.allowUnknownTypes) {
            result.success = false;
            result.errorMessage = "Unknown file type not allowed by configuration";
            result.requiresQuarantine = true;
            result.quarantineReason = "Unknown file type";
            return result;
        }
        fileType = detectedType;
        result.fileType = fileType;
    }

    // Validate output directory exists or can be created
    fs::Path outputPathObj(outputPath);
    if (!outputPathObj.parent_path().empty() && !fs::exists(outputPathObj.parent_path())) {
        try {
            fs::create_directories(outputPathObj.parent_path());
        } catch (const fs::FilesystemError& e) {
            result.success = false;
            result.errorMessage = "Cannot create output directory: " + std::string(e.what());
            return result;
        }
    }

    result.originalSize = FileSanitizer::getFileSize(inputPath);

    // Check file size limits
    if (result.originalSize > config.maxFileSizeMB * 1024 * 1024) {
        result.success = false;
        result.errorMessage = "File size exceeds maximum allowed size";
        result.requiresQuarantine = true;
        result.quarantineReason = "File too large";
        return result;
    }

    // Find appropriate sanitizer and validate it can handle the file type
    bool handled = false;
    for (const auto& sanitizer : sanitizers_) {
        if (sanitizer && sanitizer->canHandle(fileType)) {
            try {
                result = sanitizer->sanitize(inputPath, outputPath, config);
                handled = true;
                break;
            } catch (const std::exception& e) {
                result.success = false;
                result.errorMessage = "Sanitization failed: " + std::string(e.what());
                result.requiresQuarantine = true;
                result.quarantineReason = "Sanitization error";
                return result;
            }
        }
    }

    if (!handled) {
        result.success = false;
        result.errorMessage = "No sanitizer available for file type: " + getFileTypeName(fileType);
        result.requiresQuarantine = true;
        result.quarantineReason = "Unsupported file type";
    }

    // Additional security checks based on configuration
    if (result.success && config.securityLevel >= CdrConfiguration::SecurityLevel::HIGH) {
        if (fileType == FileType::SCRIPT_FILE && config.blockAllScripts) {
            result.success = false;
            result.errorMessage = "Script files blocked by high security policy";
            result.requiresQuarantine = true;
            result.quarantineReason = "Script file blocked by policy";
        }
        
        if (fileType == FileType::EXECUTABLE_FILE && config.blockExecutables) {
            result.success = false;
            result.errorMessage = "Executable files blocked by security policy";
            result.requiresQuarantine = true;
            result.quarantineReason = "Executable file blocked";
        }
    }

    return result;
}

// === CdrSanitizer File-Specific Public Methods ===

SanitizationResult CdrSanitizer::sanitizeOfficeFile(const std::string& inputPath, 
                                                   const std::string& outputPath, 
                                                   const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::OFFICE_DOCUMENT);
}

SanitizationResult CdrSanitizer::sanitizePdfFile(const std::string& inputPath, 
                                                 const std::string& outputPath, 
                                                 const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::PDF_DOCUMENT);
}

SanitizationResult CdrSanitizer::sanitizeHtmlFile(const std::string& inputPath, 
                                                  const std::string& outputPath, 
                                                  const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::HTML_DOCUMENT);
}

SanitizationResult CdrSanitizer::sanitizeArchiveFile(const std::string& inputPath, 
                                                     const std::string& outputPath, 
                                                     const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::ARCHIVE_FILE);
}

SanitizationResult CdrSanitizer::sanitizeScriptFile(const std::string& inputPath, 
                                                    const std::string& outputPath, 
                                                    const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::SCRIPT_FILE);
}

SanitizationResult CdrSanitizer::sanitizeImageFile(const std::string& inputPath, 
                                                   const std::string& outputPath, 
                                                   const CdrConfiguration& config) {
    return sanitizeFile(inputPath, outputPath, config, FileType::IMAGE_FILE);
}

std::vector<std::string> CdrSanitizer::getAvailableSanitizers() const {
    std::vector<std::string> sanitizerNames;
    
    // Return names of all registered sanitizers
    for (const auto& sanitizer : sanitizers_) {
        // Since we don't have RTTI or type names, we'll check what file types they can handle
        if (sanitizer->canHandle(FileType::OFFICE_DOCUMENT)) {
            sanitizerNames.push_back("OfficeSanitizer");
        }
        if (sanitizer->canHandle(FileType::PDF_DOCUMENT)) {
            sanitizerNames.push_back("PdfSanitizer");
        }
        if (sanitizer->canHandle(FileType::HTML_DOCUMENT)) {
            sanitizerNames.push_back("HtmlSanitizer");
        }
        if (sanitizer->canHandle(FileType::SCRIPT_FILE)) {
            sanitizerNames.push_back("ScriptAnalyzer");
        }
        if (sanitizer->canHandle(FileType::ARCHIVE_FILE)) {
            sanitizerNames.push_back("ArchiveSanitizer");
        }
        if (sanitizer->canHandle(FileType::IMAGE_FILE)) {
            sanitizerNames.push_back("ImageSanitizer");
        }
    }
    
    // If no sanitizers are registered, return the built-in ones
    if (sanitizerNames.empty()) {
        sanitizerNames = {
            "OfficeSanitizer",
            "PdfSanitizer", 
            "HtmlSanitizer",
            "ScriptAnalyzer",
            "ArchiveSanitizer",
            "ImageSanitizer"
        };
    }
    
    return sanitizerNames;
}

std::vector<std::string> CdrSanitizer::getSupportedFileTypes() const {
    return {
        "Microsoft Office Documents (.docx, .xlsx, .pptx)",
        "PDF Documents (.pdf)",
        "HTML Documents (.html, .htm)",
        "Script Files (.js, .ps1, .vbs, .bat, .sh, .py)",
        "Archive Files (.zip, .rar, .7z, .tar)",
        "Image Files (.jpg, .jpeg, .png, .gif, .svg, .bmp)",
        "Text Files (.txt, .xml, .json)",
        "Email Files (.eml, .msg)",
        "Rich Text Format (.rtf)",
        "OpenDocument Format (.odt, .ods, .odp)"
    };
}

// Static method implementation
FileType CdrSanitizer::detectAndValidateFileType(const std::string& filePath) {
    // Validate that file exists
    if (!fs::exists(filePath)) {
        return FileType::UNKNOWN_FILE;
    }
    
    // Validate that it's a regular file (not a directory or symlink)
    if (!fs::is_regular_file(filePath)) {
        return FileType::UNKNOWN_FILE;
    }
    
    // Use the internal detection method which already handles file content analysis
    FileType detectedType = detectFileTypeInternal(filePath);
    
    // Additional validation can be added here if needed
    // For now, just return the detected type
    return detectedType;
}
} // namespace CDR
